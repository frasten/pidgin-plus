/* Non vanno correttamente:
 * ./test [c=1][a=2]a[/a=0][/c=0]bcde i finali, dopo la fine del gradiente,
 *                                    ancora con gradiente, non dovrebbero.
 * 
 * Stringhe malformate:
 * ./test [c=1]a[ss[u]dasd[/c]
 * ./test [c=1]a[bc[/c] (si incasina.. beh, e' malformata forte...)
 * */

#include <glib.h>
#include <string.h>
#include <stdio.h>
#include <util.h>

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

static char *findColor(const char *str) {
	unsigned int index;
	const char *color = NULL;
	if (*str == '#') {
		color = (str + 1);
	}
	else {
		index = atoi(str);
		if (index >= sizeof(colorCodes) / sizeof(colorCodes[0]))
			return NULL;
		color = colorCodes[index];
	}

	return g_strdup(color);
}

static char *convert_tag(const char *ptag)
{
	const char *p = ptag;
	char *color = NULL;
	const char *pretag = NULL;
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

static int hexDec(char *str, char size) {
	int i, tot = 0;
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

		// Power: digit *= pow(16, size - i - 1);
		digit *= (1<<((size - i -  1) << 2));
		tot += digit;
	}
#ifdef PLUS_DEBUG
	printf("Totale: %s => %i\n", str, tot);
#endif
	return tot;
}


int main(int argc, char *argv[]) {
	char *p = argv[1];

	int gradientIndexFG, gradientIndexBG, ncharsFG, ncharsBG;
	int begColorFG[3], endColorFG[3], deltaColorFG[3];
	int begColorBG[3], endColorBG[3], deltaColorBG[3];
	unsigned char gradientBG = FALSE, gradientFG = FALSE, insideTag = FALSE;

	/* Ciclo di lettura caratteri */
	GString *buf = g_string_new("");
	for (;*p;p = g_utf8_next_char(p)) {
#ifdef PLUS_DEBUG
		// printf("Leggo il carattere %c\n", *p);
#endif
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
#ifdef PLUS_DEBUG
					printf("Primo carattere del tag: %c\n", p[1]);
#endif
					/* Controllo gradiente */

					/* Try to unificate c/a*/
					char tagCharLowerCase = 0, tagCharUpperCase = 0;
					if (p[1] == 'c' || p[1] == 'C') {
						tagCharLowerCase = 'c';
						tagCharUpperCase = 'C';
					}
					else if (p[1] == 'a' || p[1] == 'A') {
						tagCharLowerCase = 'a';
						tagCharUpperCase = 'A';
					}
					else if (p[1] == 'b' || p[1] == 'B' || p[1] == 'i' || p[1] == 'I' ||
						p[1] == 'u' || p[1] == 'U' || p[1] == 's' || p[1] == 'S' || p[1] == '/') {
						/* sarebbe carino fargli skippare la parte di controllo gradiente */
					}
					else {
						break;
					}
					insideTag = TRUE;


					if ((p[1] == tagCharLowerCase || p[1] == tagCharUpperCase) && p[2] == '=') {
						gchar *iter = p + i + 1;
						char insideTagFastForward = FALSE;
						int fastForwardCharCounter = 0;
#ifdef PLUS_DEBUG
						printf("Controllo gradienti.\n");
#endif
						if (tagCharLowerCase == 'c') {
							gradientFG = FALSE; /* TODO: necessario? */
							ncharsFG = 0;
						}
						else {
							gradientBG = FALSE; /* TODO: necessario? */
							ncharsBG = 0;
						}

						/* Vado avanti e cerco il finale corrispondente */
						for (;*iter;iter = g_utf8_next_char(iter)) {

							if (iter[0] == '[' && iter[1] == '/' &&
								(iter[2] == tagCharLowerCase || iter[2] == tagCharUpperCase)
							) {
#ifdef PLUS_DEBUG
								printf("ho trovato un finale\n");
#endif
								if (iter[3] == '=') {
									gradientTag = TRUE;
									/*  */
									char *initialColor = findColor(p + 3);
									char *finalColor = findColor(iter + 4);
									if (!initialColor || !finalColor) break;

#ifdef PLUS_DEBUG
									printf("Colore iniziale: %s\n", initialColor);
#endif
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

#ifdef PLUS_DEBUG
									printf("Colore finale: %s\n", finalColor);
#endif
									g_free(initialColor);
									g_free(finalColor);

									if (tagCharLowerCase == 'c') {
										gradientFG = TRUE;
										gradientIndexFG = 0;
#ifdef PLUS_DEBUG
										printf("numero caratteri: %i\n", ncharsFG);
#endif
									}
									else {
										gradientBG = TRUE;
										gradientIndexBG = 0;
#ifdef PLUS_DEBUG
										printf("numero caratteri: %i\n", ncharsBG);
#endif
									}
									// Calcolare il numero di caratteri effettivi (escludendo i tag),
									// e suddividere il Delta R, G, B diviso il numero di caratteri,
									// ottenendo l'incremento da aggiungere (o sottrarre)
									// ad ogni carattere.
									// Subito PRIMA dell'ultimo carattere, mettere il colore finale.


#ifdef PLUS_DEBUG
									printf("gradiente\n");
#endif
								}
								else {
#ifdef PLUS_DEBUG
									printf("non gradiente\n");
#endif
								}
								break;
							}
							else {
								if (tagCharLowerCase == 'c') ncharsFG++; // TODO: devono essere effettivi, non cosi'.
								else ncharsBG++;
							}

							if (iter[0] == '[') {
								/* sono FORSE all'interno di un tag*/
								if (iter[1] == 'b' || iter[1] == 'B' ||
									iter[1] == 'i' || iter[1] == 'I' ||
									iter[1] == 'u' || iter[1] == 'U' ||
									iter[1] == 's' || iter[1] == 'S' ||
									iter[1] == 'a' || iter[1] == 'A' ||
									iter[1] == 'c' || iter[1] == 'C' ||
									iter[1] == '/') {
									insideTagFastForward = TRUE; /* TODO: non e' vero, limite massimo caratteri */
									fastForwardCharCounter = 0;
								}
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
						else if (tagCharLowerCase == 'a')
							gradientBG = FALSE;
					}

					/* Tag convertito ed aggiunto solo se non sono in un gradiente.
					 * Infatti in questo caso viene gestito dopo. */
					if (!gradientTag) {
#ifdef PLUS_DEBUG
						printf("Provo il tag %s\n", g_strndup(p + 1, i - 1));
#endif
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
			continue;
		}

		if (!insideTag) {
			gchar *thischar_unescaped, *thischar;
			thischar_unescaped = g_new0(char, 10);
			g_utf8_strncpy(thischar_unescaped, p, 1);
			thischar = g_markup_escape_text(thischar_unescaped, -1);
			g_free(thischar_unescaped);

			if (gradientFG || gradientBG) {
				/* Aggiungo i caratteri colorati del gradiente */

				int j;
				int color[3];
				char *tag, *fgAttribute = NULL, *bgAttribute = NULL;
				if (gradientFG) {
					for (j = 0; j <= 2; j++) {
						int delta = 0;
						if (ncharsFG > 1)
							delta = deltaColorFG[j] * gradientIndexFG / (ncharsFG - 1);
						color[j] = begColorFG[j] + delta;
#ifdef PLUS_DEBUG
//					printf("Deltacolor: %i; delta=%i; color[%i]=%i\n", deltaColor[j], delta, j, color[j]);
//					printf("delta[%i] = %i\n", j, delta);
#endif
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
#ifdef PLUS_DEBUG
//					printf("Deltacolor: %i; delta=%i; color[%i]=%i\n", deltaColor[j], delta, j, color[j]);
//					printf("delta[%i] = %i\n", j, delta);
#endif
					}
					bgAttribute = g_strdup_printf(" background=\"#%02x%02x%02x\"", color[0], color[1], color[2]);
				}
				else bgAttribute = g_strdup("");
#ifdef PLUS_DEBUG
				// printf("%s\n", g_utf8_offset_to_pointer(p, 2));
#endif

				tag = g_strdup_printf("<span%s%s>%s</span>", fgAttribute, bgAttribute, thischar);

				g_free(fgAttribute);
				g_free(bgAttribute);

				g_string_append(buf, tag);
				g_free(tag);
				if (gradientFG) gradientIndexFG++;
				if (gradientBG) gradientIndexBG++;
				if (gradientIndexFG >= ncharsFG) gradientFG = FALSE;
				if (gradientIndexBG >= ncharsBG) gradientBG = FALSE;
			}
			else {
				/* Carattere normale, senza essere in un gradiente */
				g_string_append(buf, thischar);
			}
			g_free(thischar);
		}
	}
	/* Finito, restituisco buf */
	g_assert(g_utf8_validate(buf->str, -1, NULL));

#ifdef PLUS_DEBUG
	printf("Risultato finale: ");
#endif
	printf("%s\n", buf->str);
}

