/* Non vanno correttamente:
 * ./test [a=1]abcdsadsa[/a] (non gestisco il gradiente su BG)
 * ./test [a=1][c=0]abcdsadsa[/c=1][/a] (non gestisco il gradiente su BG)
 * ./test [c=1]abc[u]ds[/u]adsa[/c=0] (nchar sbagliato)
 * ./test [c=0]abcdsadsa[/c=1] (nota DeltaColor: lo sbaglia, forse no, è negativo.)
 *                             MA: non finisce a 000000, ma a ff1d1d -_-
 * ./test [c=#000000]abcdsadsa[/c=111111] (forse è sbagliato, ma non dovrebbe segfaultare)
 * ./test [c=#000000]abcdsadsa[/c=#111111] (mette numeri a caso...)
 * controllare la funzione della proporzione che per me è sbagliata
 * */

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
		color = (char *)colorCodes[index]; /* e se e' piu' lunga? [/c=1] */
	}

	return color;
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

static int hexDec(char *str, char size) {
	// printf("Sono in hedDex(%c%c).\n",str[0],str[1]);
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
	printf("Totale: %s => %i\n", str, tot);
	return tot;
}


int main(int argc, char *argv[]) {
	char * p = strdup(argv[1]);
	int gradientIndex, nchars;
	int begColor[3], endColor[3], deltaColor[3];
	unsigned char gradient = FALSE, insideTag = FALSE;

	/* Ciclo di lettura caratteri */
	GString *buf = g_string_new("");
	for (;*p;p++) {
		// printf("Leggo il carattere %c\n", *p);
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
					printf("Primo carattere del tag: %c\n", p[1]);

					/* TODO: controllo gradiente */
					if ((p[1] == 'c' || p[1] == 'C' || p[1] == 'a' || p[1] == 'A') && p[2] == '=') {
						printf("Controllo gradienti.\n");
						gradient = FALSE; /* TODO: necessario? */
						gchar *iter = p + i + 1;
						nchars = 0;

						/* Vado avanti e cerco il finale corrispondente */
						/* TODO: anche il caso con [a] */
						for (;*iter;*iter++) {
							if ((p[1] == 'c' || p[1] == 'C') &&
								iter[0] == '[' && iter[1] == '/' &&
								(iter[2] == 'c' || iter[2] == 'C')
							) {
								printf("ho trovato un finale\n");
								if (iter[3] == '=') {
									gradientTag = TRUE;
									/*  */
									char *initialColor = findColor(p + 3);
									char *finalColor = findColor(iter + 4);

									printf("Colore iniziale: %s\n", initialColor);

									int j;
									for (j = 0;j <= 2;j++) {
										begColor[j] = hexDec(initialColor + 2*j, 2);
										endColor[j] = hexDec(finalColor + 2*j, 2);
										deltaColor[j] = endColor[j] - begColor[j];
									}

									printf("Colore finale: %s\n", finalColor);

									// Calcolare il numero di caratteri effettivi (escludendo i tag),
									// e suddividere il Delta R, G, B diviso il numero di caratteri,
									// ottenendo l'incremento da aggiungere (o sottrarre)
									// ad ogni carattere.
									// Subito PRIMA dell'ultimo carattere, mettere il colore finale.

									gradient = TRUE;
									gradientIndex = 0;
									printf("gradiente\n");
									printf("numero caratteri: %i\n", nchars);
								}
								else {
									printf("non gradiente\n");
								}
								break;
							}
							else {
								nchars++; // TODO: devono essere effettivi, non cosi'.
							}
						}
					} /* fine controllo gradiente */
					
					/* Non devo tradurre il tag di fine gradiente: */
					if (p[1] == '/' && p[3] == '=') {
						gradientTag = TRUE;
						gradient = FALSE;
					}

					/* Tag convertito ed aggiunto solo se non sono in un gradiente.
					 * Infatti in questo caso viene gestito dopo. */
					if (!gradientTag) {
						printf("Provo il tag %s\n", g_strndup(p + 1, i - 1));
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
			if (gradient) {
				/* TODO: aggiungo i caratteri colorati del gradiente */

				int j;
				int color[3];
				for (j = 0; j <= 2; j++) {
					int delta = 0;
					if (nchars > 1)
						delta = deltaColor[j] * gradientIndex / (nchars - 1);
					color[j] = begColor[j] + delta;
//					printf("Deltacolor: %i; delta=%i; color[%i]=%i\n", deltaColor[j], delta, j, color[j]);
//					printf("delta[%i] = %i\n", j, delta);
				}
				
				char *tag = g_strdup_printf("<span foreground=\"#%02x%02x%02x\">%c</span>", color[0], color[1], color[2], p[0]);
				g_string_append(buf, tag);
				gradientIndex++;
				g_free(tag);
			}
			else {
				/* Carattere normale, senza essere in un gradiente */
				g_string_append_c(buf, p[0]);
			}
		}
	}
	/* Finito, restituisco buf */
	/* TODO: return */
	printf("Risultato finale: %s\n", buf->str);
}

