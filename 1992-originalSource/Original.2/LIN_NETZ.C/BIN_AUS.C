/***************************************************************************
*                                                                          *
*  Teil VII des Programms zur Erstellung eines Gleichungssystems aus einer *
*  Knotenliste.                                                            *
*                                                                          *
*  Hier stehen die Ausgaberoutinen, die die Aufbereitung der Ergebnisse in *
*  maschinenlesbare Form zur Aufgabe haben. Es handelt sich um das Anlegen *
*  einer BinÑrdatei, die alle erforderlichen Informationen Åber die berech-*
*  neten Funktionen (4 StÅck an der Zahl) enthalten. Ein numerisches Hilfs-*
*  programm kann diese dann auswerten und Daten fÅr die Plotterroutine er- *
*  zeugen.                                                                 *
*                                                                          *
***************************************************************************/

/*
   (c) Peter Vranken 11/91
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lin_netz.h"

/***************************************************************************
* Modulinterne globale Daten.
*/
byte *Wp;        /* Schreibposition ins Ausgabefeld */

/***************************************************************************
* Ausgabe einer Liste (=Funktion) des gewandelten Formates
*/
static void liste_baus(liste)
ergsummand *liste;
{
int p, anzp[MAX_VAR+1], i, sum;
ergsummand *l;
byte max_s_pot;

   if(liste == E_null) /* leere Liste ist Nullfunktion */
   {
      *Wp++ = 0; /* erstes Byte: Anzahl der Terme, hier also Null */
      return;
   }

   /* ab hier wird ermittelt, wieviele Summanden jede Potenz von s hat,
      sowie die hîchste vorkommende Potenz in s */

   for(i=0; i<=Anz_var; ++i) /* ZÑhlfeld initialisieren */
      anzp[i] = 0;

   max_s_pot = 0; /* weniger ist unmîglich, da Liste nicht leer */
   l = liste;

   do /* ganze Liste durchsehen */
   {
      max_s_pot = MAX(l->s_potenz, max_s_pot),
      ++anzp[l->s_potenz],
      l = l->next;
   }
   while(l != E_null);

   *Wp++ = max_s_pot+1; /* Anzahl der Terme unterschiedlicher s-Potenz */

   for(p=max_s_pot; p>=0; --p) /* nach fallenden Potenzen von s ausgeben */
   {
      *Wp++ = anzp[p];/* wieviele Summanden hat der nÑchste Koeffizient? */

      l = liste;
      for(sum=0; sum<anzp[p]; ++sum) /* Schleife Åber diese Summanden */
      {
         while(l->s_potenz != p) /* nÑchsten passenden Term suchen */
            l = l->next;

         /* Ausgabe der Variablenkombination des l-Termes */
         {
         unsigned long k;
         int i, anzv;
         byte *potenzen;

            potenzen = l->potenz;
            /* maschinenunabhÑngige Zerlegung des long Koeffizienten in 4 Bytes */
            k = l->koef & 0xff000000l; k>>=24;
            *Wp++ = (byte)k;
            k = l->koef & 0x00ff0000l; k>>=16;
            *Wp++ = (byte)k;
            k = l->koef & 0x0000ff00l; k>>=8;
            *Wp++ = (byte)k;
            k = l->koef & 0x000000ffl;
            *Wp++ = (byte)k;

            /* ermitteln, wieviele Variablen der Term enthÑlt */
            for(i=anzv=0; i<Anz_var; ++i)
               anzv += potenzen[i]>0;

            *Wp++ = anzv; /* Ergebnis ist nÑchste Information */

            /* nun die Variablen, die vorkommen, angeben ... */
            for(i=0; i<Anz_var; ++i)
               if(potenzen[i]>0)
                  *Wp++ = (byte)i,      /* ... mit Nummer */
                  *Wp++ = potenzen[i];  /* ... und ihrer Potenz */

            /* dieser Summand des Koeffizienten wÑre beschrieben */
         }
         l = l->next;
      }/* Ende der for-Schleife Åber die Summanden derselben s-Potenz */
   }/* Ende der for-Schleife Åber alle mîglichen s-Potenzen */
}/* fertig mit dieser Funktion */

/***************************************************************************
* Ausgabe der Variablennamen, damit das Numerik-Programm den Benutzer im
* Klartext nach den Eingabewerten fragen kann.
*/
static void namen_baus()
{
int i;
byte *anz_vname;
char *vname;

   anz_vname = Wp++;
   for(i=*anz_vname=0; i<Anz_var; ++i)
      if(Zweigptr[i]->refp == NULL) /* sonst wurde Variable durch andere ersetzt */
      {
         ++ *anz_vname;
         vname = Zweigptr[i]->name;
         *Wp++ = (byte)i; /* welche Nummer hat sie? */
         do
            *Wp++ = *vname;
         while(*vname++); /* Stringende-0 ist schon mitgeschrieben */
      }
}

/***************************************************************************
* Hauptprogramm: Ausgabe der vier Funktionen in einen Speicherblock, dann
* anschlieûendes Schreiben in Datei
*/
boolean erg_baus(zg, ng, zri, nri)
ergsummand *zg, *ng, *zri, *nri;
{
byte *feld, *ende;
int ch;
boolean esc;

#if ATARI
#define FELDSIZE 0x10000l
#else
#define FELDSIZE 0x1000l
#endif

   if((feld=(byte*)malloc(FELDSIZE)) == NULL)
   {
      printf("nicht genÅgend Speicher fÅr BinÑrausgabe vorhanden!\n");
      return(TRUE);
   }
   Wp = feld+1;  /* in das erste Byte kommt spÑter die PrÅfsumme */
   ende = feld+FELDSIZE;
   namen_baus(); /* Variablennamen mit Nummern bekannt machen */
   if(Wp >= ende)
      abbruch(5);

   liste_baus(zg);
   if(Wp >= ende)
      abbruch(5);
   liste_baus(ng);
   if(Wp >= ende)
      abbruch(5);
   liste_baus(zri);
   if(Wp >= ende)
      abbruch(5);
   liste_baus(nri);
   if(Wp >= ende)
      abbruch(5);

   ende = Wp;
   for(Wp=feld, *Wp++=0; Wp<ende; ++Wp) /* PrÅfsumme errechnen */
      *feld += *Wp;
   for(Wp=feld; Wp<ende; ++Wp)          /* Feld rausschreiben */
   {
      switch(ch=(int)*Wp) /* Zeilenendezeichen werden von fputc verÑndert, */
      {                   /* daher durch Fluchtsymbolmechanismus sichern */
         case 27: /* auch das Fluchtsymbol selbst behandeln */
         case 10:
         case 13:
         case  0: esc = TRUE; /* Zeichen, die nicht gefahrlos geschrieben */
                  ch  += 128; /* werden kînnen, werden um 128 vergrîûert */
                  break;
         default: esc = FALSE;
      }
      if(esc)
         fputc(27, Binout);
   
      fputc(ch, Binout);
   }
   free(feld);
   return(FALSE);
}
