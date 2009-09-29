/*
 * Plus Purple plugin - Color tags by the Plus! Messenger for Windows
 * Copyright (C) 2004 Stu Tomlinson <stu@nosnilmot.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02111-1301, USA.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <glib.h>

#define PURPLE_PLUGINS
#include <pidgin.h>
#include <version.h>
#include <gtkblist.h>

#include <notify.h>
#include <request.h>
#include <signals.h>
#include <util.h>

/*
esempio drastico:

[c=8][a=3]ciao bello![/a][/c][c=1]mondi [a=0]paralleli[/a=1][/c]

risultato con i tag:
<span foreground="8"><span background=

*/

const char * colorCodes[] = {
"ffffff","000000","00007D","009100","FF0000","7D0000","9A009A","FC7D00",
"FFFF00","00FC00","009191","00FFFF","1E1EFC","FF00FF","7D7D7D","D2D2D2",
"E7E6E4","cfcdd0","ffdea4","ffaeb9","ffa8ff","c1b4fc","bafbe5","ccffa3",
"fafda2","b6b4b7","a2a0a1","f9c152","ff6d66","ff62ff","6c6cff","68ffc3",
"000000","f9ff57","858482","6e6d7b","ffa01e","F92411","FF1EFF","1E29FF",
"7dffa5","60f913","fff813","5e6464","4b494c","d98812","eb0505","de00de",
"0000d3","03cc88","59d80d","d4c804","333335","18171c","944e00","9b0008",
"980299","01038c","01885f","389600","9a9e15","473400","4d0000","5f0162",
"000047","06502f","1c5300","544d05"};

// converts a Plus! tag to a Pango markup tag
// tag must not contains trailing [ and ]
static char *convert_tag(const char *ptag)
{
	char *buf = g_malloc0(100);
	char *p = (char *)ptag;
	char *color = NULL,*pretag = NULL;
	if(*p == '/') p++;
	if(*p == 'b' || *p == 'B' || *p == 'i' || *p == 'I' || *p == 'u' || *p == 'U' || *p == 's' || *p == 'S')
	{
		return g_strdup_printf("<%s>",ptag);
	}
	else if(*ptag == '/' && (*p == 'c' || *p == 'C' || *p == 'a' || *p == 'A'))
	{
		return g_strdup("</span>");
	}
	if(*++p == '=')
	{
		p++;
		if(*p == '#') color = (p+1);
		else color = (char *)colorCodes[atoi(p)];
		if(*ptag == 'c' || *ptag == 'C') pretag = "foreground";
		if(*ptag == 'a' || *ptag == 'A') pretag = "background";
		if(pretag) return g_strdup_printf("<span %s=\"#%s\">",pretag,color);
	}
	return NULL;
}

static char *plus_nick_changed_cb(PurpleBuddy *buddy)
{
	char * ret = NULL;
	purple_debug_misc("plusblist","Screename is \"%s\", server alias is \"%s\"\n",buddy->name,buddy->server_alias);

	// do not apply colorization on user-aliased names
	if (buddy->alias != NULL) return NULL;

	gboolean setting = purple_blist_node_get_bool(&buddy->node, "disable_plus");
	if(!setting)
	{
		GString *buf = g_string_new("");
		// get an escaped version of the alias
		char *esc = g_markup_escape_text(buddy->server_alias,-1);
		purple_debug_misc("plusblist","Parsing tags to \"%s\"\n",esc);
		if(!esc) return NULL;	// oops...
		char *p = esc,*conv = NULL,*tmp = NULL;
		int i;
		for(;*p;p++)
		{
			purple_debug_misc("plusblist","Pointer now points to \"%s\"\n",p);
			if(*p == '[')
			{
				tmp = g_strdup(p);
				for(i=0;i<12;i++)
				{
					if(tmp[i] == ']')
					{
						tmp[i] = '\0';
						purple_debug_misc("plusblist","Converting tag \"%s\"\n",tmp+1);
						conv = convert_tag(tmp+1);
						if(conv)
						{
							// tmp = [c=38 len = 5
							// tmp+1 = c=38 len = 4
							// [c=38]Daniele
							purple_debug_misc("plusblist","Conversion done! \"%s\"\n",conv);
							g_string_append(buf,conv);
							p += strlen(tmp+1)+1;	// avanza della lunghezza del tag +2 ([,])
						}
						break;
					}
				}
				g_free(tmp);
				// add normally...
				if(!conv) g_string_append_c(buf,*p);
				conv = NULL;
			}
			else g_string_append_c(buf,*p);
			purple_debug_misc("plusblist","String value is \"%s\"\n",buf->str);
		}
		g_free(esc);
		ret = g_string_free(buf,FALSE);
		purple_debug_misc("plusblist","Return value will be \"%s\"\n",ret);
		if(!pango_parse_markup(ret,-1,0,NULL,NULL,NULL,NULL))
		{
			// parse failed!
			g_free(ret);
			ret = NULL;
		}
	}
	//g_debug("PLUS! -----------------------");
	return ret;
}

static void
plusmenu_cb(PurpleBlistNode *node, gpointer data)
{
	PurpleBuddyList *blist = purple_get_blist();
	PurpleBlistUiOps *ops = purple_blist_get_ui_ops();
	if (PURPLE_BLIST_NODE_IS_BUDDY(node))
	{
		gboolean setting = !purple_blist_node_get_bool(node, "disable_plus");
		purple_blist_node_set_bool(node, "disable_plus",setting);
		ops->update(blist,node);
	}
	else if (PURPLE_BLIST_NODE_IS_CONTACT(node))
	{
		PurpleBlistNode *bnode;
		gboolean setting = !purple_blist_node_get_bool(node, "disable_plus");

		purple_blist_node_set_bool(node, "disable_plus", setting);
		for (bnode = node->child; bnode != NULL; bnode = bnode->next) {
			purple_blist_node_set_bool(bnode, "disable_plus", setting);
			ops->update(blist,bnode);
		}
	}
	else
	{
		g_return_if_reached();
	}
}

static void
plus_extended_menu_cb(PurpleBlistNode *node, GList **m)
{
	PurpleMenuAction *bna = NULL;

	if (purple_blist_node_get_flags(node) & PURPLE_BLIST_NODE_FLAG_NO_SAVE)
		return;

	if (!PURPLE_BLIST_NODE_IS_CONTACT(node) && !PURPLE_BLIST_NODE_IS_BUDDY(node))
		return;

	*m = g_list_append(*m, bna);

	/*
	se disable_plus e' true, allora NON usare i tag, quindi chiedi se vogliamo usarli
	se e' false, allora USA i tag e chiedi se NON vogliamo usarlo
	*/

	if (purple_blist_node_get_bool(node, "disable_plus"))
		bna = purple_menu_action_new("Use Plus! tags", PURPLE_CALLBACK(plusmenu_cb),
										NULL, NULL);
	else
		bna = purple_menu_action_new("Use flat nick", PURPLE_CALLBACK(plusmenu_cb),
										NULL, NULL);

	*m = g_list_append(*m, bna);
}

static gboolean first_launch(void)
{
	PurpleBuddyList *blist = purple_get_blist();
	pidgin_blist_refresh(blist);
	return FALSE;
}


static gboolean
plugin_load(PurplePlugin *plugin)
{
	purple_signal_connect(purple_blist_get_handle(), "blist-node-extended-menu",
						plugin, PURPLE_CALLBACK(plus_extended_menu_cb), NULL);
/*
	purple_signal_connect(purple_blist_get_handle(), "buddy-signed-on",
						plugin, PURPLE_CALLBACK(plus_nick_changed_cb), NULL);
	purple_signal_connect(purple_blist_get_handle(), "buddy-status-changed",
						plugin, PURPLE_CALLBACK(plus_nick_changed_cb), NULL);
*/
	purple_signal_connect(pidgin_blist_get_handle(), "drawing-buddy",
						plugin, PURPLE_CALLBACK(plus_nick_changed_cb), NULL);
	purple_timeout_add(100,(GSourceFunc)first_launch,NULL);
	return TRUE;
}

static gboolean
plugin_unload(PurplePlugin *plugin)
{
	// a quanto pare pidgin disconnette da soli i segnali... boh! :S
	//purple_signal_disconnect(pidgin_blist_get_handle(),"drawing-buddy",plugin,PURPLE_CALLBACK(plus_nick_changed_cb));
	purple_timeout_add(100,(GSourceFunc)first_launch,NULL);
	return TRUE;
}

static PurplePluginInfo info =
{
	PURPLE_PLUGIN_MAGIC,
	PURPLE_MAJOR_VERSION,							/**< major version	*/
	PURPLE_MINOR_VERSION,
	PURPLE_PLUGIN_STANDARD,							/**< type			*/
	NULL,											/**< ui_requirement	*/
	0,												/**< flags			*/
	NULL,											/**< dependencies	*/
	PURPLE_PRIORITY_DEFAULT,						/**< priority		*/

	"purple-plugin-plus",					/**< id				*/
	"Plus! color tags",								/**< name			*/
	PP_VERSION,										/**< version		*/
	"Support for Plus! color tags.",				/**  summary		*/
	"Parses server aliases for Plus! Live Messenger color tags.\nParsing can be deactivated on a per-nick basis.",		/**  description	*/
	"Daniele Ricci <daniele.athome@gmail.com>",			/**< author			*/
	NULL,										/**< homepage		*/

	plugin_load,									/**< load			*/
	plugin_unload,										/**< unload			*/
	NULL,											/**< destroy		*/

	NULL,											/**< ui_info		*/
	NULL,											/**< extra_info		*/
	NULL,											/**< prefs_info		*/
	NULL,											/**< actions		*/
	NULL,											/**< reserved 1		*/
	NULL,											/**< reserved 2		*/
	NULL,											/**< reserved 3		*/
	NULL											/**< reserved 4		*/
};


static void
init_plugin(PurplePlugin *plugin) {
	//info.name = "Plus! color tags";
	//info.summary = "Support for Plus! color tags.";
	//info.description = "Parses server aliases for Plus! Live Messenger color tags.\nParsing can be deactivated on a per-nick basis.";
}

PURPLE_INIT_PLUGIN(plus, init_plugin, info)
