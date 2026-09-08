/*
   Hauptprogramm und Dateieinlese-Schnittstelle des Plotterprogrammes

   (c) P. Vranken, Juli 1992
*/

# include <stdlib.h>
# include <stdio.h>
# include <tos.h>

# include "plotter.h"

/* Definitionen und Konstanten */
# define VERSIONSSTRING "0.9 (Juli 1992)"
enum { linear, logarithmisch, x1_verzerrung };

/* lokale Typdefinitionen */
typedef struct
   {
      int anzahl1; /* wieviele Punkte? */
      int anzahl2;
      int art1;    /* welche Achsverzerrungen? */
      int art2;
      int spalten; /* wieviele Spalten von Werten gibt es? */
      int x2pos;   /* welche Position haben x und y Wert innerhalb der */
      int y2pos;   /* Zeilen der Eingabedatei? */
   }
   fileheader;

/* lokale Prototypen */
static grafik *readfile(FILE *input);
static int comp_modus(int x_verz, int y_verz);
static void readheader(FILE *input, fileheader *header);
static boolean instr(char **string, char *muster);
static boolean readzeile(FILE *input, char **string);

/**************************************************************************/

int main(int argc, char *argv[])
{
FILE *input;
grafik *funktion;

   printf("\x1bpPLOTTER - " VERSIONSSTRING " - (c) P.Vranken\x1bq\n");

   if(argc != 2)
      exit_error("Anwendung: plotter ascii_daten_file");

   if((input=fopen(argv[1], "r")) == NULL)
      exit_error("angegebene Datei existiert nicht!");

   funktion = readfile(input);
   init_graph();
   plotte(funktion);

   Cnecin();
   exit_graph();
   return(0);
}

/***************************************************************************
* Fehlerbehandlung: Meldung ausgeben, auf Taste warten und Programm abbre-
* chen. Evtl. Dateien schlieûen.
*/

void exit_error(char *message)
{
   printf("%s\nBitte eine Taste...\n", message);
   Cnecin();
   exit(-1);
}

/***************************************************************************
* Hilfsroutinen fÅr die Headerauswertung: Zeile lesen, Stringvergleich zur
* "SchlÅsselworterkennung", Leerzeichen Åberlesen.
*/

# define nospace(string) {while(*string==' ' || *string=='\t')++string;}

static boolean readzeile(FILE *input, char **string)
{
static char zeile[256];
int i=0, ch;

   while((ch=getc(input))!='\n' && ch!=EOF)
   {
      zeile[i] = ch;
      if(i<255)
         ++ i;
   }
   zeile[i] = '\0';
   *string = zeile;
   return(i!=0 || ch!=EOF); /* es konnte eine Zeile mehr gelesen werden */
}
      
static boolean instr(char **string, char *muster)
{
int i=0;

   while(muster[i])
      if(muster[i]!=(*string)[i++])
         return(false);
   *string += i;
   return(true);
}

/***************************************************************************
* Modus der Darstellung aus den beiden Achsverzerrungen bestimmen
*/
static int comp_modus(int x_verz, int y_verz)
{
   if(x_verz == x1_verzerrung)
      return(y_verz==linear? onlyy: onlyylog);
   else if(x_verz == linear)
      return(y_verz==linear? xy: xylog);

   return(y_verz==linear? xlogy: xlogylog);
}   
   
/***************************************************************************
* Unterfunktion des Dateilesens: Header auswerten, um festzustellen, was
* alsdann an Zahlen erwartet wird.
*/
# define error(str) (fclose(input),exit_error(str))
# define getstring(str) {if(fscanf(input, "%s", (str))!=1)\
                            error("Fehler beim Lesen des Headers!");}
# undef getstring
# define getint(ip) {if(fscanf(input, "%d", (ip))!=1)\
                            error("Fehler beim Lesen des Headers!");}

static void readheader(FILE *input, fileheader *header)
{
char *zeile, fkt_nr;
int anz_fkten, modus, anzahl, x_verz, y_verz;
boolean f1_komplett, f2_komplett;

   readzeile(input, &zeile);

   /* neues Headerformat ist an einem fÅhrenden '#' erkennbar */
   if(zeile[0] == '#')
   {
      f2_komplett = f1_komplett = false;
      do
      {
         if(!readzeile(input, &zeile)) /* Dateiende erreicht */
            goto err;
         nospace(zeile);
         if(zeile[0]=='#' || zeile[0]=='\0') /* Kommentar- und Leerzeilen */
            continue;                        /* Åberlesen */

         if(!instr(&zeile, "f"))
            goto err;

         if((fkt_nr=*zeile)=='1' || fkt_nr=='2')
            ++ zeile;
         nospace(zeile);
         if(* zeile++  != ':')
            goto err;
         nospace(zeile);

         if(instr(&zeile, "lin"))
            x_verz = linear;
         else if(instr(&zeile, "log"))
            x_verz = logarithmisch;
         else if(instr(&zeile, "x1") && fkt_nr=='2')
            x_verz = x1_verzerrung;
         else
            goto err;

         nospace(zeile);
         if(* zeile++  != ',')
            goto err;
         nospace(zeile);

         if(instr(&zeile, "lin"))
            y_verz = linear;
         else if(instr(&zeile, "log"))
            y_verz = logarithmisch;
         else
            goto err;

         nospace(zeile);
         if(*zeile == ',') /* Angabe einer Anzahl Punkte */
         {
            ++ zeile;
            nospace(zeile);
            anzahl = atoi(zeile);
            if((anzahl=MIN(anzahl, 500)) < 0)
err:
printf("fkt_nr: %c, anzahl: %d, xverz: %d, y_verz: %d\n", fkt_nr, anzahl, x_verz, y_verz),
               error("Fehler im Header, erwarte korrekte Funktionsbeschreibung!");
         }
         else /* von maximaler Punktezahl ausgehen */
            anzahl = 500;

         /* nun sind alle Informationen Åber eine Funktion ermittelt */
         switch(fkt_nr)
         {
            case '1':
               f1_komplett = true;
               header->art1 = comp_modus(x_verz, y_verz);
               header->anzahl1 = anzahl;
               break;
            case '2':
               f2_komplett = true;
               if(x_verz==x1_verzerrung)
                  header->spalten = 3,
                  header->x2pos = 0,
                  header->y2pos = 2;
               else
                  header->spalten = 4,
                  header->x2pos = 2,
                  header->y2pos = 3;
               header->art2 = comp_modus(x_verz, y_verz);
               header->anzahl2 = anzahl;
               break;
            default:
               f1_komplett = f2_komplett = true;
               header->spalten = 2;
               header->x2pos = 0;
               header->y2pos = 0;
               header->art1 = comp_modus(x_verz, y_verz);
               header->art2 = not;
               header->anzahl1 = anzahl;
               header->anzahl2 = 0;
         } /* Ende switch(1, 2, oder Doppel-Funktion) */
      }
      while(!f1_komplett || !f2_komplett);
   }
   else /* Format von PLOT_X00 einlesen */
   {
      anz_fkten = atoi(zeile); /* 1 oder 2 Funktionen */
      if(anz_fkten!=1 && anz_fkten!=2)
         error("Nur ein oder zwei Funktionen angeben!");
      getint(&anzahl); /* nÑchste Angabe: Anzahl der Stellen */
      header->anzahl1 = MIN(500, abs(anzahl));
      getint(&modus);  /* gemeinsamer Darstellungsmodus */
      modus = abs(modus)%5; /* gleiche Fehlerbehandlung wie PLOT_X00 */
      switch(modus)
      {
         case 0: /* lineare Darstellung */
            header->art1 = xy; break;
         case 1: /* y-logarithmisch */
            header->art1 = xylog; break;
         case 2: /* x-logarithmisch */
            header->art1 = xlogy; break;
         case 3: /* xy-logarithmisch */
            header->art1 = xlogylog; break;
         case 4:
            error("Polar-Darstellung existiert leider nicht!");
      }

      /* nun ist alle erforderliche Info eingelesen und der Header kann fer-
         tig aufgefÅllt werden */
      if(anz_fkten == 2) /* dann die meisten Werte kopieren */
      {
         header->anzahl2 = header->anzahl1,
         header->x2pos   = 0,
         header->y2pos   = 2,
         header->spalten = 3;
         switch(header->art1)
         {
            case xlogy:
            case xy:
               header->art2 = onlyy; break;
            case xlogylog:
            case xylog:
               header->art2 = onlyylog; break;
         }
      }
      else
         header->art2    = not,
         header->anzahl2 = 0,
         header->x2pos   = 0,
         header->y2pos   = 0,
         header->spalten = 2;
   }
}
# undef getstring
# undef getint

/***************************************************************************
* Einlesen der Datei, Header auswerten, also den Modus bestimmen. Dann die
* Zahlenkolonnen einlesen.
*/

static grafik *readfile(FILE *input)
{
fileheader header;
double *x1, *y1, *x2, *y2;
int i1, i2;
char *zeile;

   readheader(input, &header); /* Info fÅr weiteres Vorgehen holen */
   MALLOC(x1, 1+header.anzahl1);/* eine Funktion gibts immer */
   MALLOC(y1, 1+header.anzahl1);
   MALLOC(x2, 1+header.anzahl2);/* die zweite erfordert mindestens Platz */
   MALLOC(y2, 1+header.anzahl2);/* fÅr einen Wert, wegen folgender Schleife */

   i1 = i2 = 0;
   while(i1<header.anzahl1 || i2<header.anzahl2)
   {
   double a[4];
   char *formatstring;
   
      /* aus einer Zeile die gewÅnschte Anzahl Zahlen lesen. Es dÅrfen zu
         viele, jedoch nicht zu wenige Zahlen in der Zeile stehen. In letz-
         terem Fall werden keine weiteren Werte mehr angenommen */
      do
      {
         if(!readzeile(input, &zeile)) /* Dateiende, abbrechen */
            break;
         nospace(zeile);
      }
      while(*zeile=='\0' || *zeile=='#'); /* Leer- und Kommentarzeile */
                                          /* sind erlaubt */

      /* eine Gruppe Zahlen lesen */
      switch(header.spalten)
      {
         case 4: formatstring = "%lf %lf %lf %lf"; break;
         case 3: formatstring = "%lf %lf %lf";     break;
        default: formatstring = "%lf %lf";
      }
      if(sscanf(zeile, formatstring, a, a+1, a+2, a+3)
         < header.spalten)  /* es kînnen nicht genÅgend Werte gelesen werden */
         break;             /* i1 und i2 geben die gelesenen Anzahlen an */

      x1[i1] = a[0], /* die erste Funktion steht immer in den vorderen */
      y1[i1] = a[1]; /* Spalten */

      x2[i2] = a[header.x2pos], /* wo sich die andere Funktion befindet ist */
      y2[i2] = a[header.y2pos]; /* variabel */

      if(i1<header.anzahl1)
         ++ i1;

      if(i2<header.anzahl2)
         ++ i2;

      if(i2==header.anzahl1 || i2==header.anzahl2)
         /* weniger Spalten, kein x2, y2 mehr */
         header.spalten = 2,
         header.x2pos = 0,
         header.y2pos = 1;

   }/* Ende des while noch Werte zu lesen */
   fclose(input);
   if(i1<2 || (i2<2 && header.anzahl2))
      exit_error("es wurden zu wenig Werte gelesen!");

   if(i1!=header.anzahl1 || i2!=header.anzahl2)
      printf("Es wurden %d (f1) bzw. %d (f2) Wertepaare gelesen\n", i1, i2);

   /* nun kann die gelesene Information in eine entsprechende Struktur ge-
      fÅgt werden */
   return(bilde_grafik(x1, y1, header.art1, i1,
                       x2, y2, header.art2, i2));
}
# undef error
   
