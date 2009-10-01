/*
 * Plus Purple plugin - Color tags by the Plus! Messenger for Windows
 * Copyright (C) 2008-2009 Daniele Ricci <daniele.athome@gmail.com>
 * Copyright (C) 2009 Andrea Piccinelli <frasten@gmail.com>
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
#include <string.h>
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

/* Finds the color associated to a color code, and also reads colors in
 * #RRGGBB format. */
static char *findColor(char *str) {
	int index;
	char *p = str;
	char *color = NULL;
	if (*p == '#') {
		color = (p+1);
	}
	else {
		index = atoi(p);
		if (index >= sizeof(colorCodes) / sizeof(colorCodes[0]))
			return NULL;
		color = (char *)colorCodes[index];
	}

	return g_strdup(color);
}

/* converts a Plus! tag to a Pango markup tag
 * tag must not contains trailing [ and ] */
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
		color = findColor(p);
		if(*ptag == 'c' || *ptag == 'C') pretag = "foreground";
		if(*ptag == 'a' || *ptag == 'A') pretag = "background";
		if(pretag) {
			char *tmp = g_strdup_printf("<span %s=\"#%s\">", pretag, color);
			g_free(color);
			return tmp;
		}
	}
	return NULL;
}

/* Converts an hexadecimal string into a decimal integer.
 * @param str: the hex string
 * @param size: only parse the first n chars.
 * */
static int hexDec(char *str, char size) {
	int i, j, tot = 0;
	for (i = size - 1; i>=0; i--) {
		int digit = 0, uppercase = 0;
		/* uppercase ? */
		if (str[i] >= 'A' && str[i] <= 'F') {
			uppercase = 32;
		}

		if (str[i] >= '0' && str[i] <= '9')
			digit = str[i] - '0';
		else if ((str[i] >= 'a' && str[i] <= 'f') ||
			(str[i] >= 'A' && str[i] <= 'F'))
			digit = str[i] - 'a' + 10 + uppercase;

		/* Esponente */
		for (j = 0;j < size - (i + 1);j++) {
			digit *= 16;
		}

		tot += digit;
	}
	return tot;
}

static char *plus_nick_changed_cb(PurpleBuddy *buddy)
{
	char * ret = NULL;
	purple_debug_misc("plusblist","Screename is \"%s\", server alias is \"%s\"\n",buddy->name,buddy->server_alias);

	gboolean setting = purple_blist_node_get_bool(&buddy->node, "disable_plus");
	if(!setting)
	{
		/* get an escaped version of the alias */
		char *esc;

		/* Colorization on alias, if set. */
		if (buddy->alias != NULL)
			esc = g_markup_escape_text(buddy->alias, -1);
		else
			esc = g_markup_escape_text(buddy->server_alias,-1);

		purple_debug_misc("plusblist","Parsing tags to \"%s\"\n",esc);
		if(!esc) return NULL;	/* oops... */

		int gradientIndexFG, gradientIndexBG, ncharsFG, ncharsBG;
		int begColorFG[3], endColorFG[3], deltaColorFG[3];
		int begColorBG[3], endColorBG[3], deltaColorBG[3];
		unsigned char gradientBG = FALSE, gradientFG = FALSE, insideTag = FALSE;
		char *p = esc;

		/* Ciclo di lettura caratteri */
		GString *buf = g_string_new("");
		for (;*p;p++) {
			if (*p == '[') {
				/* Controllo tag */

				/* Faccio un fast forward per cercare il corrispondente ],
				 * determinando quindi se si tratta di un tag oppure no. */
				int i;
				for(i = 1; i < 12; i++) {
					if (p[i] == ']') {
						char *replace;
						char gradientTag = FALSE;

						/* Ho trovato la fine del tag, sono dentro! */
						insideTag = TRUE;

						/* Controllo gradiente */

						/* Try to unificate c/a*/
						char tagCharLowerCase, tagCharUpperCase;
						if (p[1] == 'c' || p[1] == 'C') {
							tagCharLowerCase = 'c';
							tagCharUpperCase = 'C';
						}
						else if (p[1] == 'a' || p[1] == 'A') {
							tagCharLowerCase = 'a';
							tagCharUpperCase = 'A';
						}
						else {
							/* sarebbe carino fargli skippare la parte di controllo gradiente */
						}

						if ((p[1] == tagCharLowerCase || p[1] == tagCharUpperCase) && p[2] == '=') {
							if (tagCharLowerCase == 'c') {
								gradientFG = FALSE; /* TODO: necessario? */
								ncharsFG = 0;
							}
							else {
								gradientBG = FALSE; /* TODO: necessario? */
								ncharsBG = 0;
							}

							gchar *iter = p + i + 1;

							char insideTagFastForward = FALSE;
							int fastForwardCharCounter = 0;
							/* Vado avanti e cerco il finale corrispondente */
							for (;*iter;*iter++) {

								if (iter[0] == '[' && iter[1] == '/' &&
									(iter[2] == tagCharLowerCase || iter[2] == tagCharUpperCase)
								) {
									purple_debug_misc("plusblist", "Gradient end found.\n");
									if (iter[3] == '=') {
										gradientTag = TRUE;
										/*  */
										char *initialColor = findColor(p + 3);
										char *finalColor = findColor(iter + 4);
										if (!initialColor || !finalColor) break;

										purple_debug_misc("plusblist", "Beginning color: %s\n", initialColor);

										int j;
										for (j = 0;j <= 2;j++) {
											if (tagCharLowerCase == 'c') {
												begColorFG[j] = hexDec(initialColor + 2 * j, 2);
												endColorFG[j] = hexDec(finalColor + 2 * j, 2);
												deltaColorFG[j] = endColorFG[j] - begColorFG[j];
											}
											else {
												begColorBG[j] = hexDec(initialColor + 2 * j, 2);
												endColorBG[j] = hexDec(finalColor + 2 * j, 2);
												deltaColorBG[j] = endColorBG[j] - begColorBG[j];
											}
										}

										purple_debug_misc("plusblist", "Ending color: %s\n", finalColor);
										g_free(initialColor);
										g_free(finalColor);

										if (tagCharLowerCase == 'c') {
											gradientFG = TRUE;
											gradientIndexFG = 0;
											purple_debug_misc("plusblist", "Number of chars inside the gradient: %i\n", ncharsFG);
										}
										else {
											gradientBG = TRUE;
											gradientIndexBG = 0;
											purple_debug_misc("plusblist", "Number of chars inside the gradient: %i\n", ncharsBG);
										}
										/* Calcolare il numero di caratteri effettivi (escludendo i tag),
										 * e suddividere il Delta R, G, B diviso il numero di caratteri,
										 * ottenendo l'incremento da aggiungere (o sottrarre)
										 * ad ogni carattere.
										 * Subito PRIMA dell'ultimo carattere, mettere il colore finale.
										 */
									}
									break;
								}
								else {
									if (tagCharLowerCase == 'c') ncharsFG++;
									else ncharsBG++;

								}

								if (iter[0] == '[') {
									/* sono FORSE all'interno di un tag*/
									insideTagFastForward = TRUE; /* TODO: non e' vero, limite massimo caratteri */
									fastForwardCharCounter = 0;
								}
								else if (iter[0] == ']' && insideTagFastForward) {
									/* ero all'interno di un tag ed ora l'ho chiuso */
									insideTagFastForward = FALSE;
									if (tagCharLowerCase == 'c')
										ncharsFG -= (fastForwardCharCounter + 2); /* 2 = squares []*/
									else
										ncharsBG -=  (fastForwardCharCounter + 2); /* 2 = squares []*/
								}
								else if (insideTagFastForward) {
									fastForwardCharCounter++;
								}

							}
						} /* fine controllo gradiente */

						/* Non devo tradurre il tag di fine gradiente: */
						if (p[1] == '/' && p[3] == '=') {
							gradientTag = TRUE;
							if (tagCharLowerCase == 'c')
								gradientFG = FALSE;
							else
								gradientBG = FALSE;
						}

						/* Tag convertito ed aggiunto solo se non sono in un gradiente.
						 * Infatti in questo caso viene gestito dopo. */
						if (!gradientTag) {
							purple_debug_misc("plusblist", "Translating tag %s\n", g_strndup(p + 1, i - 1));
							replace = convert_tag(g_strndup(p + 1, i - 1));
							if (replace) {
								g_string_append(buf, replace);
							}
							g_free(replace);
						}
						break; /* Ne ho trovata una, non cerco le seguenti. */
					} /* Fine if p = ] */
				} /* Fine ciclo for per cercare la fine del tag ] */





			}
			else if (*p == ']' && insideTag) {
				insideTag = FALSE;
				continue; /* TODO: e' ok? */
			}

			if (!insideTag) {
				if (gradientFG || gradientBG) {
					/* Aggiungo i caratteri colorati del gradiente */

					int j;
					int color[3];
					char *fgAttribute = NULL, *bgAttribute = NULL;
					if (gradientFG) {
						for (j = 0; j <= 2; j++) {
							int delta = 0;
							if (ncharsFG > 1)
								delta = deltaColorFG[j] * gradientIndexFG / (ncharsFG - 1);
							color[j] = begColorFG[j] + delta;
						}
						fgAttribute = g_strdup_printf(" foreground=\"#%02x%02x%02x\"", color[0], color[1], color[2]);
					}
					else fgAttribute = g_strdup("");

					if (gradientBG) {
						for (j = 0; j <= 2; j++) {
							int delta = 0;
							if (ncharsBG > 1)
								delta = deltaColorBG[j] * gradientIndexBG / (ncharsBG - 1);
							color[j] = begColorBG[j] + delta;
						}
						bgAttribute = g_strdup_printf(" background=\"#%02x%02x%02x\"", color[0], color[1], color[2]);
					}
					else bgAttribute = g_strdup("");

					char *tag = g_strdup_printf("<span%s%s>%c</span>", fgAttribute, bgAttribute, p[0]);
					g_free(fgAttribute);
					g_free(bgAttribute);

					g_string_append(buf, tag);
					g_free(tag);
					if (gradientFG) gradientIndexFG++;
					if (gradientBG) gradientIndexBG++;
				}
				else {
					/* Carattere normale, senza essere in un gradiente */
					g_string_append_c(buf, p[0]);
				}
			}
		}
		g_free(esc);
		ret = g_string_free(buf,FALSE);
		purple_debug_misc("plusblist","Return value will be \"%s\"\n", ret);
		if(!pango_parse_markup(ret,-1,0,NULL,NULL,NULL,NULL))
		{
			/* parse failed! */
			g_free(ret);
			ret = NULL;
		}
	}
	/*g_debug("PLUS! -----------------------"); */
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
	/* a quanto pare pidgin disconnette da soli i segnali... boh! :S
	 * purple_signal_disconnect(pidgin_blist_get_handle(),"drawing-buddy",plugin,PURPLE_CALLBACK(plus_nick_changed_cb));
	 */
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
	/*info.name = "Plus! color tags";
		info.summary = "Support for Plus! color tags.";
		info.description = "Parses server aliases for Plus! Live Messenger color tags.\nParsing can be deactivated on a per-nick basis.";
	*/
}

PURPLE_INIT_PLUGIN(plus, init_plugin, info)
