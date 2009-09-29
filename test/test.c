#include <glib.h>
#include <string.h>
#include <stdio.h>

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

/* [c=5]aha[/c=4] */
/* [c=2] mi compro 10 mc3000[/c=10] */
/* [a=15][c=39]A[a=16][c=4]le[/c][/a]X[/c=11][/a] */
/* [a=16][c=16]°º¤ø,¸¸,ø¤º°`°º¤ø,¸Å£€x°º¤ø,¸¸,ø¤º°`°º¤ø,¸[/c=15][/a=15] */
/* [b][u][c=9][a=1],..,XandeR ,.., [/a][/c][/u][/b] */
/* [c=37]Alessio[/c] */
/* [c=1]eyyyyyyyy[/c=3] */
/* [c=27][a=5]♥††♠Silvia♠††♥ Mi hai insegnato tanto!!!! peccato!!!!! [/a=1][/c=4] */

static char *findColor(char *str) {
	char *p = str;
	char *color = NULL;
	if(*p == '#') color = (p+1);
	else color = (char *)colorCodes[atoi(p)]; /* e se e' piu' lunga? [/c=1] */
	return color;
}

static int hexDec(char *str, char size) {
	printf("Sono in hedDex(%c%c).\n",str[0],str[1]);
	// todo: controlla maiuscole
	int i, j, tot = 0;
	for (i = size - 1; i>=0; i--) {
		int digit = 0, uppercase = 0;
		// uppercase ?
		if (str[i] >= 'A' && str[i] <= 'F') {
			uppercase = 32;
		}
		
		if (str[i] >= '0' && str[i] <= '9')
			digit = str[i] - '0';
		else if ((str[i] >= 'a' && str[i] <= 'f') ||
			(str[i] >= 'A' && str[i] <= 'F'))
			digit = str[i] - 'a' + 10 + uppercase;

		// Esponente
		for (j = 0;j < size - (i + 1);j++) {
			digit *= 16;
		}
//		printf("Da destra: %i\n",digit);
		tot += digit;
	}
	printf("Totale: %i\n", tot);
	return tot;
}


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
		if(pretag) return g_strdup_printf("<span %s=\"#%s\">",pretag,color);
	}
	return NULL;
}





int main(int argc, char *argv[]) {
	char * p = strdup(argv[1]);
	char *conv = NULL, * tmp = NULL;
	int i;
	GString *buf = g_string_new("");
			for(;*p;p++)
		{
			printf("Pointer now points to \"%s\"\n",p);
			if(*p == '[')
			{
				tmp = g_strdup(p);
				for(i=0;i<12;i++)
				{
					if(tmp[i] == ']')
					{
						/* Controllo gradienti */
						if (p[1] == 'c' || p[1] == 'C' || p[1] == 'a' || p[1] == 'A') {
							printf("Controllo gradienti.\n");
							char gradiente = 0;
							gchar *iter = p + i;
							for (;*iter;*iter++) {
								/* cerco il /c corrispondente */
								if ((p[1] == 'c' || p[1] == 'C') &&
									iter[0] == '[' && iter[1] == '/' &&
									(iter[2] == 'C' || iter[2] == 'c')
								) {
									printf("ho trovato un finale\n");
									if (iter[3] == '=') {
										/*  */
										char *finalColor = findColor(iter + 4);
										char r, g, b;
										r = hexDec(finalColor, 2);
										g = hexDec(finalColor + 2, 2);
										b = hexDec(finalColor + 4, 2);
										
										printf("Colore finale: %s\n", findColor(iter + 4));
										gradiente = 1;
										printf("gradiente\n");
									}
									else {
										printf("non gradiente\n");
										break;
									}
									printf("esco\n");
									return;
								}
							}
						}
						
						tmp[i] = '\0';
						printf("primo carattere del tag: %c\n", (char) p[1]);
						printf("Converting tag \"%s\"\n",tmp+1);
						conv = convert_tag(tmp+1);
						printf("conv %s\n", conv);
						if(conv)
						{
							/* tmp = [c=38 len = 5
							 * tmp+1 = c=38 len = 4
							 * [c=38]Daniele
							 */
							printf("Conversion done! \"%s\"\n",conv);
							g_string_append(buf,conv);
							p += strlen(tmp+1)+1;	/* avanza della lunghezza del tag +2 ([,]) */
						}
						break;
					}
				}
				g_free(tmp);
				/* add normally... */
				if(!conv) g_string_append_c(buf,*p);
				conv = NULL;
			}
			else g_string_append_c(buf,*p);
			printf("String value is \"%s\"\n",buf->str);
		}
}
