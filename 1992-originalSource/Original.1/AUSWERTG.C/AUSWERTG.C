/***************************************************************************
*                                                                          *
*  Ein Hilfsprogramm zum Netzwerkprogramm:                                 *
*  Es wertet die BinÑrdatei aus, fragt Variablenwerte ab und legt mit die- *
*  sen Tabellen mit den FrequenzgÑngen an, die dann wiederum von PLOT_x00  *
*  gezeichnet werden kînnen. Durch diese dreifache Aufteilung kann einer-  *
*  seits erreicht werden, daû der Åberwiegende Teil des Gesamtprojektes    *
*  rechnerunabhÑngig bleibt, andererseits aber auch das Kernprogramm       *
*  LIN_NETZ numerikfrei bleibt.                                            *
*                                                                          *
***************************************************************************/

/*
   (c) Peter Vranken 12/1991
*/

/***************************************************************************
* es kann zwischen der öbersetzung fÅr ATARI ST oder Msdos-Rechner gewÑhlt
* werden.
*/
#define ATARI 1       /* FÅr Msdos-Rechner auf Null setzen */
#define TEST          /* Abfrage auf interne Fehler aktivieren */
#define FKTANZEIGEN 0 /* Kontrollausgabe der Koeffizienten der eingelesenen Funktion */

/**************************************************************************/
#if ATARI /* Zeichenfunktion fÅr ein Zeichen ohne Echo heiût unterschiedlich */

long Cnecin(void);
#define Conin  (char)Cnecin

#ifdef TEST
long Crawio(int w); /* Prototyp nur fÅr nÑchstes Makro */
#define TASTE {while(Crawio(0xff));if('q'==(char)Cnecin())exit(-1);} /* auf Taste warten */
#endif /* TEST */

/* Bildschirmsteuerung bei Atari ST */
#define CLS         {printf("\33E");}
#define HOME        {printf("\33H");}
#define CURPOS(z,s) {printf("\33Y%c%c", z+32, s+32);}
#define MARKAN(c)   {printf("\33p ");}/* bei Revers on/off muû ein Freiraum */
#define MARKAUS(c)  {printf(" \33q");}/* erzeugt werden, weil Msdos-Makro dies */
                                      /* auch unvermeidlich tut */
#define CURON       {printf("\33e");}
#define CUROFF      {printf("\33f");}

#else /* Msdos-Version */

int getch(void);
#define Conin  (char)getch

#ifdef TEST
int getch(void); /* Prototyp */
#define TASTE {if('q'==(char)getch())exit(-1);} /* auf Taste warten */
#endif /* TEST */

/* Msdos-Variante der Bildschirmsteuerung */
void clear(void); /* zunÑchst die Prototypen der dafÅr notwendigen Funktionen */
void setcur(int zl, int sp);
#define CLS         {clear();}
#define HOME        {setcur(0,0);}
#define CURPOS(z,s) {setcur(z,s);}
#define MARKAN(c)   {printf("%c", c);}
#define MARKAUS(c)  {printf("%c", c);}
#define CURON       { /* ? */; }
#define CUROFF      { /* ? */; }

#endif /* Atari oder Msdos */
/**************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <math.h>
#include <string.h>

#if ATARI
#include <filename.h>
#else
#include "filename.h"
#include <dos.h>
#endif


/***************************************************************************
* Konstanten, weitere Makros, Typdefinitionen
*/
#define MAX_VAR 32 /* muû mindestens so groû wie MAX_VAR in lin_netz sein */
#if ATARI
#define FELDSIZE 0x10000l
#else
#define FELDSIZE 0x4000l
#endif
#define MAX(a,b) ((a)>(b)? (a):  (b))
#define MIN(a,b) ((a)<(b)? (a):  (b))

typedef enum {FALSE=(1==0), TRUE=(1==1)} boolean;
typedef enum {linear, ylog, xlog, xylog} plotmode;
typedef enum {KARTESISCH, POLAR} komplexart;
typedef unsigned char byte;

/* Beschreibung der mîglichen Funktionen */

/* zunÑchst die Listenelemente, die die Summanden der Koeffizienten des Poly-
   nomes in s aufnehmen, zu jeder Potenz von s gehîren ja Åblicherweise
   mehrere Terme. */

typedef struct _summand
{
   struct _summand *next;
   long koef;
   byte potenz[MAX_VAR]; /* hier stehen die Potenzen der maximal 32 verknÅpften Variablen */
} summand;

typedef struct
{
   int shigh;               /* hîchste Potenz von s */
   summand *skoef[MAX_VAR+1];/* Koeffizienten des Polynomes in s, max. Anzahl 32 */
                            /* der Variablen ist auch max. Potenz von s */
   double  snkoef[MAX_VAR+1];/* numerischer Wert des Koeffizienten nach Variablenvorgabe */
} funktion;

typedef struct
{
   char name[9];
   double value;
} variable;

/***************************************************************************
* Prototypen
*/
int main(int anz, char *koms[]);
boolean einlesen(void);
boolean lies_fkt(funktion *fkt);
void show_fkt(funktion *fkt);
int dialog(void);
void next(int *i);
void prev(int *i);
boolean schreibdaten(void);
boolean liesdaten(void);
float mantisse(double a);
char expo(double a);
double getfloat(char c);
int calc(char was);
int antworten(void);
void calc_skoef(funktion *fkt);
void calc_fkt(funktion *f, double x, double *absy, double *argy, boolean wie);

/* aus dem FFT-Modul */
void FFT(boolean invers, double Re[], double Im[]);
int set_size(int n);

/***************************************************************************
* Modulinterne globale Daten
*/
static FILE *Input, *Output;
static byte *Feld, *Ende, *Rp;
static funktion Zg, Ng, Zri, Nri; /* fÅr jede der Funktionen eine Datenstruktur */
static int Anz_var;              /* Anzahl der einzugebenden Variablen */
static variable Var[MAX_VAR+3],
               *Bauteil=&Var[3];/* ihre Namen und Werte*/
static int Modus=2;
static double PIh;
static double PI;

/**************************************************************************/

int main(anz, koms)
int anz;
char *koms[];
{
char inpname[257], outpname[257], x;
d_name inp, outp;
int i, rc, c;
boolean o_def, i_def, direkt;
byte pr_sum;

   PI = 4*atan(1.0), PIh = 2*atan(1.0);
   MARKAN('\r')
   printf("Auswertung der von LIN_NETZ erstellten BinÑrdatei");
   MARKAUS(' ')
   printf("\nein Programm von Peter Vranken (November 1991)\n");

   if((Rp=Feld=(byte*)malloc(FELDSIZE)) == NULL)
   {
      printf("zu wenig Speicher!\n");
      return(-1);
   }
   if(anz < 2)
   {
      printf("Anwendung: [-Aausgabe]  (Ausgabedatei)\n"
             "           [-Dx]        (Direktaufruf von MenÅpunkt x mit Startwerten)\n"
             "           eingabe      (Eingabedatei)\n");
      return(-1);
   }
   else
      for(i=1, direkt=i_def=o_def=FALSE; i<anz; ++i)
         if(koms[i][0] == '-' && toupper(koms[i][1])=='A')
            strncpy(outpname, koms[i]+2, 256),
            o_def = TRUE;
         else if(koms[i][0] == '-' && toupper(koms[i][1])=='D')
            direkt = TRUE,
            x = toupper(koms[i][2]);
         else
            strncpy( inpname, koms[i], 256),
            i_def = TRUE;

   if(!i_def)
      goto error1;
   dateiname(&inp, inpname);
   if(inp.ext[0]=='\0')
      strcat(inp.komplett, ".fkt");
   if(FN_err)
   {
error1:
      printf("Fehler in Dateiname\n");
      return(-1);
   }
   if(o_def)
   {
      dateiname(&outp, outpname);
      if(FN_err)
         goto error1;
      strcpy(outp.komplett, outp.pfad);
      strcat(outp.komplett, outp.n_o_ext);
   }
   else
   {
      strcpy(outp.komplett, inp.pfad);
      strcat(outp.komplett, inp.n_o_ext);
   }
   strcat(outp.komplett, "dat");
   printf("Eingabe: %s, Ausgabe: %s\n", inp.komplett, outp.komplett);

   if((Input=fopen(inp.komplett, "r")) == NULL)
      goto error2;
   if((Output=fopen(outp.komplett, "w")) == NULL)
   {
      fclose(Input);
error2:
      printf("Fehler beim ôffnen der Dateien\n");
      return(-1);
   }
   pr_sum = (byte)fgetc(Input);
   for(;;)
   {
      if((c=fgetc(Input)) == 27) /* Fluchtsymbol? */
         c = fgetc(Input)-128;   /* dann ist dies das richtige Zeichen */
      if(c == EOF)
         break;
      pr_sum -= (byte)c; /* PrÅfsumme mitfÅhren */
      *Rp++   = (byte)c; /* und Byte ablegen */
   }
   if(pr_sum != 0)
   {
error3:
      printf("falsches Eingabedateiformat!\n");
      fclose(Input);
      fclose(Output);
      return(-1);
   }
   Ende = Rp;
   Rp   = Feld;
   if(einlesen())
      goto error3;
   fclose(Input);
   liesdaten(); /* Variablenwerte vom letzten Lauf holen */
   if(direkt) /* öbertragungsfunktion mit Startwerten berechnen */
      switch(x)
      {
         case 'A': printf("Berechnung des Betrages von öbertragungsfunktion "
                          "und Eingangsimpedanz...\n");
                   rc = calc('A'); /* MenÅauswahl 'A' anspringen */
                   break;
         case 'B': printf("Berechnung der öbertragungsfunktion in Betrag und "
                          "Phase...\n");
                   rc = calc('B'); /* MenÅauswahl 'B' */
                   break;
         case 'C': printf("Berechnung der Eingangsimpedanz in Betrag und Phase...\n");
                   rc = calc('C'); /* MenÅauswahl 'C' */
                   break;
         case 'D': printf("Berechnung von Sprung und Stoûantwort...\n");
                   rc = calc('D'); /* MenÅauswahl 'D' */
                   break;
          default: printf("gewÑhlter MenÅpunkt existiert nicht!\n");
      }/* Ende switch welcher MenÅpkt soll direkt ausgefÅhrt werden? */
   else /* MenÅ aufrufen, in Benutzerdialog treten */
      rc = dialog(); /* rc: Meldung an die Shell, was als nÑchstes geschehen soll */
   fclose(Output);
   free(Feld);
   return(rc);
}

/***************************************************************************
* Hier wird die Datei, die ja genauestens strukturiert ist, eingelesen, dann
* werden die vier Funktionen geeignet abgepeichert, um spÑter im Dialog mit
* dem Benutzer sinnvoll ausgewertet zu werden.
*/
boolean einlesen(void)
{
int var_nr, i, j;
char c1;

   if((Anz_var=*Rp++) > MAX_VAR)
   {
error1:
      printf("zu viele Variablen!\n");
      return(TRUE);
   }
   strcpy(Var[0].name, "fu");
   strcpy(Var[1].name, "fo");
   strcpy(Var[2].name, "Punkte");
   for(i=0; i<Anz_var; ++i)
      Bauteil[i].name[0] = '\0';

   for(i=0; i<Anz_var; ++i)
   {
      if((var_nr=*Rp++) >= MAX_VAR)
         goto error1;

      c1 = (char)toupper((int)*Rp); /* ersten Buchstaben merken */
      j = 0;
      do
      {
         Bauteil[var_nr].name[j++] = *Rp;
         if(j > 8)
         {
            printf("Variablenname zu lang!\n");
            return(TRUE);
         }
      }
      while(*Rp++); /* Null des Stringendes wird mitgeschrieben */

      /* Defaultbelegung fÅr die Variablen wÑhlen */
      Bauteil[var_nr].value = c1=='R'? 2.0E4: c1=='C'? 1.0E-7: c1=='L'? 1.0: 1.0E-4;
   }
   Var[0].value = 10.0;
   Var[1].value = 25.0E3;
   Var[2].value = 128.0;

   /* das war der Vorspann, nun folgen die vier Funktionen */

   if(lies_fkt(&Zg))
      return(TRUE);
   if(lies_fkt(&Ng))
      return(TRUE);
   if(lies_fkt(&Zri))
      return(TRUE);
   if(lies_fkt(&Nri))
      return(TRUE);

   return(Rp!=Ende);
}

/***************************************************************************
* Eine der Funktionen einlesen.
*/
boolean lies_fkt(fkt)
funktion *fkt;
{
int i, spot, ko, var;
summand *h;
unsigned long k;

   for(i=0; i<=MAX_VAR; ++i)
      fkt->skoef[i] = NULL;

   fkt->shigh = *Rp++ -1; /* hîchste Potenz von s, die vorkommt */

   for(spot=fkt->shigh; spot>=0; --spot) /* alle Koeffizienten einlesen */
   {
      for(ko=*Rp++; ko; --ko) /* alle Summanden des Koeffizienten einlesen */
      {
         /* neuen Summanden anlegen */
         if((h=(summand*)malloc(sizeof(summand))) == NULL)
         {
            printf("zu wenig Speicher!\n");
            return(TRUE);
         }
         /* ihn fÅllen */
         h->next = fkt->skoef[spot]; /* alte Liste hintenan */

         k  = (unsigned long)*Rp++; k <<= 8; /* Koeffizient zusammensetzen */
         k |= (unsigned long)*Rp++; k <<= 8;
         k |= (unsigned long)*Rp++; k <<= 8;
         k |= (unsigned long)*Rp++;
         h->koef = (long)k;

         for(i=0; i<MAX_VAR; ++i) /* Potenzen zunÑchst Null */
            h->potenz[i] = 0;
         for(i=*Rp++; i; --i) /* nun die von Null verschiedenen eintragen */
            if(*Rp > MAX_VAR)
            {
               printf("Fehler in Dateistruktur!\n");
               return(TRUE);
            }
            else
               var = *Rp++,
               h->potenz[var] = *Rp++;
         /* Summand ist fertig gefÅllt */

         fkt->skoef[spot] = h;       /* ihn als neue Liste einhÑngen */
      }/* Ende der Schleife Åber die Summanden eines Koeffizienten */
      fkt->snkoef[spot] = 0.0; /* da der echte Wert ja noch nicht berechnet ist */
   }
   /* Ende der Schleife Åber die Koeffizienten */
   return(FALSE);
}

#if FKTANZEIGEN
/***************************************************************************
* Kontollausgabe der eingelesenen Funktion.
*/
void show_fkt(fkt)
funktion *fkt;
{
int i,j;
summand *l;

   for(i=fkt->shigh; i>=0; --i)
   {
      l = fkt->skoef[i];
      if(l != NULL)
      {
         printf("Koef.liste zu s^%d: ", i);
         do
         {
            /* Summanden des Koeffizienten anzeigen */
            printf("%c%ld", l->koef>0? '+': '\0', l->koef);
            for(j=0; j<MAX_VAR; ++j)
               if(l->potenz[j])
                  printf("*'%s'^%d", Bauteil[j].name, l->potenz[j]);
            l = l->next;
         }
         while(l != NULL);
         printf(", num.: %.4e\n", fkt->snkoef[i]);
      }

   }
}
#endif

/***************************************************************************
* Nun der Benutzerdialog, der einerseits bequem sein soll, aber andererseits
* rechnerunabhÑngig sein muû, also nur auf Standardsteuerzeichen zurÅckgrei-
* fen darf.
*/
static int dialog()
{
#define UP '/' /* Festlegung der vier Tasten zur Bewegung der Variablenmar- */
#define DN '*' /* kierung. Wegen der rechnerunabhÑngigen Gestaltung des Pro */
#define RT '+' /* gramms kînnen die Cursortasten leider nicht abgefragt werden, */
#define LT '-' /* beim Atari liefern sie gar keine char-Kodes */

int i, j, akt;
boolean first;
static char c, *modusstr[]={"linear)               ",
                            "y-logarithmisch)      ",
                            "x-logarithmisch)      ",
                            "doppelt logarithmisch)"};
double a;

   akt = 0;
   CLS
   MARKAN(' ')
   printf("           ************** Eingabe der Variablenwerte **************           ");
   MARKAUS(' ')
   printf("\n");
   for(first=TRUE;;first=FALSE)
   {
      CUROFF
      CURPOS(2,0)
      for(i=j=0; j<Anz_var+3; ++j, next(&i))
         if(i!=akt)
            printf(" %9s: %7.2f%c  %s", Var[i].name, mantisse(Var[i].value),
                   expo(Var[i].value), j%3==2? "\n": " ");
         else
         {
            MARKAN('>')
            printf("%9s: %7.2f%c ", Var[i].name, mantisse(Var[i].value), expo(Var[i].value));
            MARKAUS('<')
            printf("%s", j%3==2? "\n": " ");
         }
      if(j%3!=0)
         printf("\n");
      if(first)
         printf("\n(a): Berechnung der BetrÑge von öbertragungsfunktion und Eingangsimpedanz\n"
                "(b): Berechnung von Betrag und Phase der öbertragungsfunktion\n"
                "(c): Berechnung von Betrag und Phase der Eingangsimpedanz\n"
                "(d): Berechnung von Sprung- und Stoûantwort\n");
      else
         printf("\n\n\n\n\n");

      printf("(q): Ende ohne Berechnung      (e): Modus Ñndern (%s\n"
             "                              \r",
              modusstr[Modus]);

      c = Conin();
      switch(toupper(c))
      {
         case  RT: next(&akt);
                   continue;
         case  LT: prev(&akt);
                   continue;
         case  UP: prev(&akt), prev(&akt), prev(&akt);
                   continue;
         case  DN: next(&akt), next(&akt), next(&akt);
                   continue;
         case 'A':
         case 'B':
         case 'C':
         case 'D': schreibdaten();
                   return(calc(toupper(c)));
         case 'E': Modus = (Modus+1)%4; break;
         case 'Q': return(0);/* bei dieser Art zu beenden werden die Daten */
                             /* nicht gemerkt, es ist ja eine Art Abbruch */

          default: /* Eingabe eines neuen Zahlenwertes */
                   if(!isdigit(c) && c!='.') /* also kein Zahlanfang */
                     continue;
                   CURON /* Cursor anschalten */
                   a = getfloat(c);
                   if(a<=999.9E12 && a>=0.1E-12) /* Grenzen der Darstellung */
                   {
                      /* fehlerhafte Frequenzeingabe? */
                      if(akt==0 && a>=Var[1].value)
                         continue;
                      else if(akt==1 && a<=Var[0].value)
                         continue;

                      /* unlautere Anforderung an Punkt-Auflîsung? */
                      else if(akt==2 && (a>500.0 || a<25.0))
                        continue;
                      else if(akt==2) /* Rundung vornehmen */
                        a = floor(a);

                      /* ansonsten kann die Zuweisung vorgenommen werden */
                      Var[akt].value = a;
                      next(&akt);
                   }
      } /* Ende switch(welche Taste) */
   }
}

/***************************************************************************
* zwei Hilfsroutinen von dialog(): nÑchsten oder vorigen Eintrag in der Var-
* iablenliste suchen.
*/
void next(i)
int *i;
{
   do
      *i = (*i+1)%(MAX_VAR+3);
   while(Var[*i].name[0] == '\0');
}

void prev(i)
int *i;
{
   do
      *i = (*i+MAX_VAR+2)%(MAX_VAR+3);
   while(Var[*i].name[0] == '\0');
}
         
/***************************************************************************
* Der öbersichtlichkeit halber sollen Bauteilwerte in wissenschaftlicher
* Flieûkommanotation ausgegeben werden, bei der die Exponenten in 3er-Schritten
* gewÑhlt sind. Die nÑchsten beiden Routinen ermitteln Mantisse und symboli-
* schen Exponenten dieser Darstellung
*/
static float mantisse(a) /* float genÅgt, da nur fÅr 4stellige Ausgabe */
double a;
{
int expo, vz;

   vz = a>=0.0? 1: -1;
   a = fabs(a);

   if(a == 0.0)
      return(0.0);
      
   expo = (int)floor(log10(a));
   a /= pow10(expo);
   if(expo%3)
      a*=10,
      expo--;
   if(expo%3)
      a*=10;

   return((float)(vz*a));
}

static char expo(a)
double a;
{
int expo;
static char _symb[]={'p', 'n', 'Ê', 'm', ' ', 'K', 'M', 'G', 'T'},
            *symb = &_symb[4];

   a = fabs(a);
   if(a == 0.0)
      return(' ');

   expo = (int)floor(log10(a));
   if(expo%3)
      a*=10,
      expo--;
   if(expo%3)
      expo--;


   expo = expo>12 || expo<-12? 5: expo/3;
   return(symb[expo]);
}

/***************************************************************************
* Einlesen einer Flieûkommazahl. Zwei GrÅnde sprechen fÅr eine solche Funk-
* tion: Wenn die Warteschleife die MenÅauswahl untersucht hat, sollte das
* Zeichen schon das erste der Zahl sein, um mîglichst flÅssige Bedienung zu
* ermîglichen. Liest man den Rest mit gets() ein, dann lÑût sich diese Zei-
* chen aber nicht mehr korrigieren. Auûerdem ist es sehr angenehm, wenn man
* den Exponenten symbolisch eingeben kann.
*/
#define BS '\10'  /* ASCII-Kode der Lîschtaste bei Atari- und Msdos-Tastatur */

static double getfloat(c)
char c;
{
#define LEN 20
char buf[LEN+1], ch;
int i, expo;
double a;

   buf[0] = c, i = 1, putchar(c);/* erstes Zeichen war schon im MenÅ eingegeben */
   for(;;)
   {
      ch = Conin();/* Zeichen ohne Echo auf dem Bildschirm einlesen */
      if(ch=='\15' || ch=='\12')
         break;
      if(toupper(ch) != 'M')
         ch = toupper(ch);
      if(ch == BS)
      {
         if(i > 0)
            printf("\b \b"),
            --i;
         continue;
      }
      if(i<LEN && (isalnum(ch) || ch=='.'))
         buf[i++] = ch,
         putchar(ch);
   }
   buf[i] = '\0';
   i = 0;
   while(isdigit(buf[i]))
      ++i;
   
   if(buf[i] == '.') /* Nachkommastellen gibts auch */
   {
      ++i;
      while(isdigit(buf[i]))
         ++i;
   }
   switch(buf[i]) /* evtl. ein symbolischer Exponent */
   {
      case 'P': expo = -12; break;
      case 'N': expo =  -9; break;
      case 'U': expo =  -6; break;
      case 'm': expo =  -3; break;
      case 'K': expo =   3; break;
      case 'M': expo =   6; break;
      case 'G': expo =   9; break;
      case 'T': expo =  12; break;
       default: expo =   0;
   }
   a = atof(buf);
   a *= pow10(expo);
         
   return(a);
}

/***************************************************************************
* Die nÑchsten beiden Routinen sichern bzw. lesen die aktuellen Bauteilwer-
* te, damit diese beim nÑchsten Programmstart wieder zur VerfÅgung stehen. 
* Das Konzept der Trennung von Auswertung und Plotten der Funktionen bedingt
* ja schlieûlich einen hÑufigen Neustart diese Programmes.
*/
static boolean schreibdaten()
{
FILE *out;
int i, j;

   if((out=fopen("auswertg.inf", "w")) == NULL)
      return(TRUE);
   for(i=j=0; i<Anz_var+3; ++i) /* Anzahl der zu sichernden Variablen */
   {
      /* es kann ruhig mit j=0 angefangen werden, weil Variable 0 und 1 die
         stets vorhandenen Frequenzen sind */
      fprintf(out, "%e\n", Var[j].value);
      next(&j); /* j auf nÑchste verwendete Variable stellen */
   }
   fprintf(out, "%d\n", Modus);/* Achseinteilungsmodus der Ausgabe */
   fclose(out);
   return(FALSE);
}

static boolean liesdaten()
{
double a;
FILE *in;
int i, j;

   if((in=fopen("auswertg.inf", "r")) == NULL)
      return(TRUE);
   for(i=j=0; i<Anz_var+3; ++i) /* Anzahl der zu sichernden Variablen */
   {
      /* es kann ruhig mit j=0 angefangen werden, weil Variable 0 und 1 die
         stets vorhandenen Frequenzen sind */
      fscanf(in, "%le", &Var[j].value);
      Var[j].value = MIN(Var[j].value, 1E12);
      Var[j].value = MAX(Var[j].value, 1E-12);
      next(&j); /* j auf nÑchste verwendete Variable stellen */
   }
   fscanf(in, "%d", &Modus); /* Achseinteilungsmodus */
   fclose(in);
   Modus  = abs(Modus)%4;         /* das ist normalerweise wirkungslos */
   Var[2].value = MIN(fabs(Var[2].value), 500.0);/* mehr zu berechnen wÑre sinnlos */
   Var[2].value = floor(MAX(Var[2].value, 25.0));/* und zu wenige Punkte auch */
   if(Var[0].value == Var[1].value) /* Grenzfrequenzen sind gleich */
      Var[1].value *= 1.1;
   if(Var[0].value > Var[1].value)  /* Grenzfrequenzen sind vertauscht */
                 a = Var[1].value,
      Var[1].value = Var[0].value,
      Var[0].value = a;

   return(TRUE);
}

/***************************************************************************
* die eigentliche Auswertung der Funktion, Erstellung der Tabellen.
*/
static int calc(was)
char was;
{
int stp, pkte;
double v, w, dv, dw, fakvw, absG,  argG,  n, re, reZ, imZ,
                            absRi, argRi,    im, reN, imN;
#if ATARI
boolean y_ist_log;
#endif

   calc_skoef(&Zg);/* die Koeffizienten sind nach der Variablenbelegung konstant */
   calc_skoef(&Ng);
   calc_skoef(&Zri);
   calc_skoef(&Nri);

   if(was == 'D') /* Sprung- und Stoûantwort berechnen */
      return(antworten());

   pkte = (int)Var[2].value;

/* Msdos-Version braucht keinen Header fÅr das dort verwendete Plotprogramm */

#if ATARI
   fprintf(Output, "%d\n%d\n%d\n", 2, -pkte, Modus==xlog || Modus==xylog? xlog: linear);
#endif
   v     = Var[0].value;
   dv    = (Var[1].value-v)/(pkte-1.0); /* Schrittweite bei linearer Skala */
   fakvw = pow(Var[1].value/v, 1/(pkte-1.0));/* Faktor bei logarithmischer Skala */
   w  = 2*PI*v;   
   dw = 2*PI*dv;

   for(stp=0; stp<pkte; ++stp)
   {
      if(was != 'C') /* öbertragungsfkt berechnen */
      {
         calc_fkt(&Zg, w, &reZ, &imZ, KARTESISCH),
         calc_fkt(&Ng, w, &reN, &imN, KARTESISCH),
         n  = reN*reN+imN*imN, /* Nenner des komplexen Quotienten */
         re = (reZ*reN+imZ*imN)/n,
         im = (imZ*reN-reZ*imN)/n;
         
         if(im == 0.0) /* dann ist atn(re/|im|) nicht definiert */
            argG = re>0.0? 0.0: PI,
            absG = fabs(re);
         else
            absG = pow(im*im+re*re, 0.5),
            argG = -atan(re/im)+(im>0.0? PIh: -PIh);
      }

      if(was != 'B') /* Eingangsimpedanz berechnen */
      {
         calc_fkt(&Zri, w, &reZ, &imZ, KARTESISCH),
         calc_fkt(&Nri, w, &reN, &imN, KARTESISCH),
         n  = reN*reN+imN*imN, /* Nenner des komplexen Quotienten */
         re = (reZ*reN+imZ*imN)/n,
         im = (imZ*reN-reZ*imN)/n;
         
         if(im == 0.0) /* dann ist atn(re/|im|) nicht definiert */
            argRi = re>0.0? 0.0: PI,
            absRi = fabs(re);
         else
            absRi = pow(im*im+re*re, 0.5),
            argRi = -atan(re/im)+(im>0.0? PIh: -PIh);
      }
#if ATARI /* in diesem Fall Logarithmierung und Umrechnung in dB hier vor-  */
          /* nehmen, bei Ms-dos muû eine entsprechende Einstellung im Plot- */
          /* Programm vorgenommen werden */

      y_ist_log = Modus==ylog || Modus==xylog;
      switch(was) /* gewÅnschte Werte ausgeben */
      {

         
         case 'A': fprintf(Output, "%.5e %.5e %.5e\n", v,
                                   y_ist_log?  20*log10(absG): absG,
                                   y_ist_log?  20*log10(absRi): absRi);
                                   break;
         case 'B': fprintf(Output, "%.5e %.5e %.5e\n", v,
                                   y_ist_log?  20*log10(absG): absG,
                                   argG);
                                   break;
         case 'C': fprintf(Output, "%.5e %.5e %.5e\n", v,
                                   y_ist_log?  20*log10(absRi): absRi,
                                   argRi);
      }

#else
      switch(was) /* gewÅnschte Werte ausgeben */
      {
         case 'A': fprintf(Output, "%.5e %.5e %.5e\n", v, absG,  absRi); break;
         case 'B': fprintf(Output, "%.5e %.5e %.5e\n", v, absG,  argG);  break;
         case 'C': fprintf(Output, "%.5e %.5e %.5e\n", v, absRi, argRi);
      }
#endif

      switch(Modus) /* Modus des Diagrammes bestimmt nÑchsten Frequenzwert */
      {
         case linear:
         case   ylog: v += dv, w += dw; break;

         case   xlog:
         case  xylog: v *= fakvw, w *= fakvw;
      }
   }
   return(was-'A'+1); /* Meldung an die Shell, die ja noch das */
}                     /* Plotterprogramm aufrufen muû */

/***************************************************************************
* Eine sehr Ñhnliche Funktion, die jedoch die Abtastwerte von Sprung- und
* Stoûantwort berechnet. Als Sprungantwort wird die erste HÑlfte einer Peri-
* ode einer Rechteckkurve dargestellt, die ein Puls-Pausen-VerhÑltnis von
* 3:1 hat. Dadurch liegt die abfallende Flanke mitten im nicht dargestellten
* Bereich, durch sie entstehende öberschwinger sollten verborgen bleiben.
*/
static int antworten()
{
#define N       512l /* l nur wegen fehlerhafter Feldbehandlung von TC 1.0 */
#define N_halbe 256l
#define ld_N    9

static double xe[N], ye[N], xd[N], yd[N];
double re, im, n, reZ, imZ, reN, imN, w, dw, t, dt, nke, nkd, a, b;
long stp, i, punkte;

   if(set_size(ld_N))
      return(-1);

   /* es wird zwar immer die gleiche Anzahl Punkte berechnet, aber die dar-
      gestellte Anzahl darf kleiner gewÑhlt werden, um eine Ausschnittsver-
      grîûerung zu ermîglichen. */
   punkte = MIN((long)Var[2].value, N_halbe);

/* Msdos-Version braucht keinen Header fÅr das dort verwendete Plotprogramm */

#if ATARI
   fprintf(Output, "2\n%ld\n0\n", -punkte);/* immer nur lineare Darstellung */
#endif

   w = dw = 2*PI*Var[1].value/(double)N_halbe;/* Ñquidistante Frequenzschritte bis */
                                              /* zur Frequenzobergrenze */

   calc_fkt(&Zg, 0.0, &reZ, &imZ, KARTESISCH);
   calc_fkt(&Ng, 0.0, &reN, &imN, KARTESISCH);
   xd[0] = reZ/reN,     yd[0] = 0.0;
   xe[0] = 0.75*PI*reZ/reN, ye[0] = 0.0;

   for(stp=1; stp<N_halbe; ++stp, w+=dw)
   {
      /* Wert der öbertragungsfunktion berechnen (im (re,im)-Darstellung) */
      calc_fkt(&Zg, w, &reZ, &imZ, KARTESISCH),
      calc_fkt(&Ng, w, &reN, &imN, KARTESISCH),
      n = reN*reN+imN*imN,
      re = (reZ*reN+imZ*imN)/n, im = (reN*imZ-reZ*imN)/n;
      
      /* daraus die Frequenzkoeffizienten der Sprungantwort */
      if((stp&3) == 1)      /* also stp = 1, 5, 9, ... */
         a = -0.5, b = -0.5;
      else if((stp&3) == 2) /* also stp = 2, 6, 10, ... */
         a =  0.0, b = -1.0;
      else if((stp&3) == 3) /* also stp = 3, 7, 11, ... */
         a =  0.5, b = -0.5;
      else                  /* fÅr andere stp sind Koeffizienten Null */
         goto stossant;
   
      xe[N-stp] =    xe[stp] = (a*re-b*im)/(double)stp,
      ye[N-stp] = - (ye[stp] = (a*im+b*re)/(double)stp);

stossant:
      /* Frequenzkoeffizienten der Stoûantwort */
      xd[N-stp] =    xd[stp] = re;
      yd[N-stp] = - (yd[stp] = im);
   }

   /* Symmetrie- und PeriodizitÑtsbedingung am letzten Koeffizienten erfÅllen */
   xd[N_halbe] = yd[N_halbe] = xe[N_halbe] = ye[N_halbe] = 0.0;

   FFT(TRUE, xd, yd); /* inverse FT ergibt die Stoûantwort */
   FFT(TRUE, xe, ye); /* inverse FT ergibt die Sprungantwort */

   /* Normierungskonstante aus den absoluten Frequenzwerten */
   nke = 1.0/PI;
   nkd = 2*Var[1].value/(double)N;

   /* die sich ergebenden Zeitschritte */
   dt = 0.5/Var[1].value;

   for(i=0, t=0.0; i<punkte; ++i, t+=dt)
      fprintf(Output, "%.5e %.5e %.5e\n", t, nke*xe[i], nkd*xd[i]);

   return('D'-'A'+1); /* Meldung an die Shell, die ja noch das */
}                     /* Plotterprogramm aufrufen muû */

/***************************************************************************
* Nachdem die Variablen einmal feste Werte angenommen haben, sind die Koef-
* fizienten des Polynomes in s, also die ganzen Listen jeweils Konstanten.
* Man kann also den Wert der Listen einfÅrallemal ausrechnen. AbhÑngig von
* der Potenz von (jw) kann auch gleich das j^n mit verarbeitet werden, wobei
* dann natÅrlich nicht vergessen werden darf, daû die geraden Potenzen zum
* Realteil, und die ungeraden zum ImaginÑrteil gehîren.
*/
void calc_skoef(fkt)
funktion *fkt;
{
int spot, var;
summand *l;
double ko, nxsum;

   for(spot=0; spot<=fkt->shigh; ++spot) /* Schleife Åber die Koeffizienten */
   {
      ko = 0.0;
      l = fkt->skoef[spot];/* Liste der Summanden des Koeffizienten abarbeiten */
      while(l != NULL)/* Schleife abweisend: Koeffizient=0 ist leere Liste */
      {
         /* nÑchsten Summanden (Variablenprodukt) berechnen */
         nxsum = (double)l->koef;
         for(var=0; var<MAX_VAR; ++var)
            switch(l->potenz[var])
            {
               case 0: continue;
               case 1: nxsum *= Bauteil[var].value; break;
               case 2: nxsum *= Bauteil[var].value
                               *Bauteil[var].value; break;
              default: nxsum *= pow(Bauteil[var].value, l->potenz[var]);
            }
         /* zum bisherigen Koeffizienten addieren */
         ko += nxsum;

         /* weiter mit nÑchstem Summanden */
         l = l->next;
      }/* vorlÑufiges Ende der Koeffizientenberechnung */

      /* nun noch den Einfluû von (jw)^spot berÅcksichtigen */
      ko = spot&2 ? -ko: ko;

      /* und abspeichern */
      fkt->snkoef[spot] = ko;
   }
}

/***************************************************************************
* Eine einzelne Funktionsauswertung an einer bestimmten Stelle vornehmen.
*/
static void calc_fkt(f, x, absy, argy, wie)
funktion *f;
double x, *absy, *argy;
komplexart wie;
{
double w, summanden[MAX_VAR+1], *snk, *sum, im, re;
int spot, spm;

   snk = f->snkoef;
   sum = summanden;
   spm = f->shigh;
   w = 1.0;

   for(spot=0; spot<=spm; ++spot, w*=x) /* Summanden des Polynomes ausrechnen */
      *sum++ = *snk++ * w;
   
   /* um nach Mîglichkeit bei der Addition mit den kleinsten Gliedern des
      Polynomes anzufangen, wird zwischen x>1, x<1 unterschieden */

   /* ImaginÑrteil: nur die Glieder mit ungerader Potenz */
   /* Realteil: nur die Glieder mit gerader Potenz */
   im = re = 0.0;

   if(x >= 1.0) /* s^0 ist mit der grîûten Wahrscheinlichkeit der Kleinste */
   {
      sum = summanden;
      for(spot=0; spot<=spm; ++spot)
         if(spot & 1) /* ungerade Potenz */
            im += *sum++;
         else
            re += *sum++;
   }
   else /* sonst mit der hîchsten Potenz in s anfangen */
   {
      sum = &summanden[spm];
      for(spot=spm; spot>=0; --spot)
         if(spot & 1) /* ungerade Potenz */
            im += *sum--;
         else
            re += *sum--;
   }
   if(wie == KARTESISCH) /* dann ist Ergebnis in dieser Form gewÅnscht */
   {
      *absy = re;
      *argy = im;
      return;
   }

   /* sonst in Polardarstellung umwandeln */
   if(im == 0.0) /* dann ist atn(re/|im|) nicht definiert */
      *argy = re>0.0? 0.0: PI,
      *absy = fabs(re);
   else
      *absy = pow(im*im+re*re, 0.5),
      *argy = -atan(re/im)+(im>0.0? PIh: -PIh);

   return;
}

#if ATARI==0
/***************************************************************************
* Hier folgen die Msdos-Unterprogramme, die fÅr die Bildschirmsteuermakros
* auf diesem Rechner erforderlich sind.
*/
union REGS inregs, outregs;

static void clear()
{
   /* erforderliche Registerbesetzung: AH=6, AL=0, CH=1, CL=1, DH=25, DL=80,
                                       BH=Farbe ? */
   inregs.h.ah = 6,
   inregs.h.al = 0,
   inregs.h.ch = 0,
   inregs.h.cl = 0,
   inregs.h.dh = 24,
   inregs.h.dl = 79,
   inregs.h.bh = 7, /* durch ausprobieren: 7 ist ok, Bit 3 ist Fettschrift,
                       eines der Bits 3 bis 8 ist Blinken, ein anderes Invers-
                       betrieb. */
   int86(0x10, &inregs, &outregs);
   setcur(0, 0);
}

static void setcur(zl, sp)
int zl, sp;
{
   /* erforderliche Registerbesetzung: AH=2, BH=?, DH=Zeile, DL=Spalte */
   inregs.h.ah = 2,
   inregs.h.bh = 0,
   inregs.h.dh = (unsigned char)zl,
   inregs.h.dl = (unsigned char)sp;
   int86(0x10, &inregs, &outregs);
}
#endif /* öbersetzung fÅr Msdos */
