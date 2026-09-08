/*
   Includedatei zu filename.c. Hier werden alle Funktionen prototypisiert,
   die zur Benutzung des Modules importiert werden mÅssen. Ebenso werden die
   benîtigten globalen Variablen deklariert und Typdefinitionen vorgenommen.
   Bemerkung: Soll auch der Leerstring als Dateiname zugelassen werden, um so
   auch reine Pfadbezeichnungen eingeben zu kînnen, kann dies durch einen
   #define-Schalter in filename.c eingestellt werden.
*/

/* Typdeklarationen */

typedef struct
{
   char komplett[257];/* MaximallÑnge nach Bedarf */
   char pfad[257-12];
   char n_m_ext[13];
   char n_o_ext[10]; /* '.' steht noch mit drin! */
   char ext[4];
} d_name;

/* Prototypes der aus filename.c exportierten Funktionen */

void dateiname(d_name*, char* filename);

/* in filename.c definierte externe Variablen */

#ifndef fuer_filename.c
  extern int FN_err;/* !=0 nach Aufruf von Dateiname, wenn Fehler auftrat */
  extern char *FN_ptr;/* Zeiger auf Filename, wird verÑndert, zeigt hinterher */
                    /* auf das Zeichen hinter dem erkannten Namen (muû Space */
                    /* oder Stringende sein!) */
#else
  int  FN_err;
  char *FN_ptr;
#endif

