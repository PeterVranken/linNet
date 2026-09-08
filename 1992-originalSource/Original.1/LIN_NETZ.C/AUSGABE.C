/***************************************************************************
*                                                                          *
*  Teil VI des Programmes zur Erstellung eines Gleichungssystems aus einer *
*  Knotenliste.                                                            *
*                                                                          *
*  Hier stehen die Ausgaberoutinen, die die Aufbereitung der Ergebnisse in *
*  leserliche Form zur Aufgabe haben. Dazu gehîrt insbesondere das Ein-    *
*  setzen der gegenseitigen Bauteilrelationen und die Zusammenfassung der  *
*  Terme. Dazu muû auch noch eine Transformation der Widerstandsbauteile   *
*  von der internen Leitwertdarstellung in die gewÅnschte vorgenommen wer- *
*  den und die komplexe Variable s eingefÅhrt werden.                      *
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
* Ausgabe einer Liste aus Summanden in leserlicher Form. Sie muû mit dem
* auf sich selbst zeigenden WÑchter enden. Die Ausgabe berÅcksichtigt alle
* oben genannten Aspekt nicht und wird daher auch nur fÅr die Protokollaus-
* gaben wÑhrend des Programmlaufs benutzt. Hier ist die NÑhe der Darstellung
* zu den programminternen AblÑufen wichtiger, als optimale Lesbarkeit, wie
* sie im Endergebnis gewÅnscht ist.
*/
void liste_aus(sum, tab)
summand *sum;
int tab;
{
vvek vv;
long i, maxpz;
int t=0, btl_nr;
boolean erster;

   /* Anzahl der darstellbaren Terme je Zeile abschÑtzen */
   maxpz = Anz_var>6? Anz_var>10? 3: 5: 6;
   do
   {
      i = (long)sum->koef;
      /* damits fÅr verschiedene Typen von 'koeffizient' klappt */
      fprintf(Fpout, "%c", i<0? '-': '+');
      erster = TRUE;
      if((i!=1 && i!=-1) || !sum->var_vek)
         fprintf(Fpout, "%ld", i<0? -i: i),
         erster = FALSE;
      for(vv=sum->var_vek, btl_nr=0;  vv;  vv>>=1, ++btl_nr)
         if(vv & 1)
         {
            if(!erster)
               fprintf(Fpout, "*");
            erster = FALSE;
            fprintf(Fpout, "%s", Zweigptr[btl_nr]->name);
         }
            
      sum = sum->next;
      if(sum!=S_null && ++t==maxpz) /* Zeile voll */
      {
         fprintf(Fpout, "\n"); /* WagenrÅcklauf */
         for(t=tab; t; --t) /* und einrÅcken */
            fprintf(Fpout, " ");
      }
   }
   while(sum!=S_null);
   fprintf(Fpout, "\n");
}

/***************************************************************************
* Hier wird ein Bruch so erweitert, daû die Variablen hinterher als Impedan-
* zen zu interpretieren sind. WÑhrend der gesamten Rechnung sind es ja Leit-
* werte. Diese Transformation erfolgt bei den Elementen R, L, Z.
*/
static void l_nach_w(z, n)
summand *z, *n;
{
vvek erw, maske;
summand *h;
int typ, i;

   /* welche Elemente sollen transformiert werden? */
   for(maske=i=0; i<Anz_var; ++i)
   {
      typ = Zweigptr[i]->art;  /* was fÅr ein Bauelement? */
      if(typ==RES || typ==IMP) /* ein zu transformierendes */
         maske |= Zweigptr[i]->vv;
   }
   /* ZunÑchst den Erweiterungsterm bestimmen */
   erw = 0;
   h = z;
   while(h != S_null)    /* dazu Produkt aller in ZÑhler und Nenner */
      erw |= h->var_vek, /* vorkommender Terme bilden */
      h = h->next;
   h = n;
   while(h != S_null)
      erw |= h->var_vek,
      h = h->next;

   erw &= maske;         /* nun die gewÅnschten ausfiltern */

   /* nun damit erweitern */
   h = z;
   while(h != S_null)
      h->var_vek ^= erw,
      h = h->next;
   h = n;
   while(h != S_null)
      h->var_vek ^= erw,
      h = h->next;
}

/***************************************************************************
* Eine Liste kopieren. Wird nur fÅr die Ergebnisausgabe benîtigt: die ZÑh-
* lerderterminante des Eingangsknotens wird hier zweimal gebraucht, muû aber
* in beiden FÑllen verÑndert werden kînnen.
*/
static summand *kopiere(liste)
summand *liste;
{
summand *erg, *h;

   h = erg = differenz(S_null, liste);/* Null-liste ist negiertes GewÅnschtes */
   while(h != S_null)
      h->koef = - h->koef,    /* daher VZ wieder umdrehen */
      h = h->next;

   return(erg);
}

/***************************************************************************
* Anzeige des Ergebnisses. Da viel Wert auf saubere Darstellung gelegt wer-
* den muû, sind hier auch aufwendige Operationen wie Variablensubstitution
* und Erkennung, Einsetzung, Zusammenfassung gleicher Variablen erforderlich.
*/
void ergebnis(d, din, dout)
summand *d, *din, *dout;
{
summand *din_copy;
ergsummand *zg, *ng, *zri, *nri;
int i;

   if(din==S_null)
      abbruch(13);

   din_copy = kopiere(din);

   if(Verbose >= 1) /* dann auch die noch nicht manipulierte Rohform des */   
   {                /* Ergebnisses ausgeben */
      fprintf(Fpout, "\n******************************************************************\n"
                       " Lîsung des LGSs ohne RÅcksubstitution der Variablen\n"
                       " Bauteilangaben sind nach wie vor als Leitwerte zu interpretieren\n"
                       "******************************************************************\n\n");
      fprintf(Fpout, " i) öbertragungsfunktion: G = Z/N, mit\n"
                     " Z=");
      liste_aus(dout, 3);
      fprintf(Fpout, " N=");
      liste_aus(din_copy, 3);
      fprintf(Fpout, "\nii) Eingangsimpedanz: ");
      if(d != S_null)
         fprintf(Fpout, "Zi = Z/N, mit\n Z="),
         liste_aus(din, 3),
         fprintf(Fpout, " N="),
         liste_aus(d, 3);
      else
         fprintf(Fpout, "ﬂ\n");

      fprintf(Fpout, "\neingesetzte Grîûenrelationen der Bauelemente:\n");
      for(i=1; i<Anz_var; ++i)
         if(Zweigptr[i]->refp != NULL)
            fprintf(Fpout, "%s = %d * %s\n",    Zweigptr[i]->name,
                           Zweigptr[i]->reffak, Zweigptr[i]->refp->name);
   
   }
   l_nach_w(din, d);        /* Bruch in Darstellung mit Impedanzen wandeln */
   l_nach_w(dout, din_copy);/* Bruch in Darstellung mit Impedanzen wandeln */

   fprintf(Fpout, "\n****************\n"
                    "* Das Ergebnis *\n"
                    "****************\n\n");

   /* aus der internen Darstellung die ausgabeorientierte bilden, gleich- */
   /* zeitig Variablenrelationen einsetzen */
   zg  = erg_format(dout), ng  = erg_format(din_copy),
   zri = erg_format(din),  nri = erg_format(d);

   /* durch die Variablensubstitution kînnen die BrÅche nun wieder kÅrzbar sein */
   kuerze_erg(zg,  ng),
   kuerze_erg(zri, nri);

   /* ausgeben */
   fprintf(Fpout, " i) öbertragungsfunktion: G(s) = Z(s)/N(s), mit\n"
                  " Z(s)=");
   ergliste_aus(zg, 6);
   fprintf(Fpout, " N(s)=");
   ergliste_aus(ng, 6);

   fprintf(Fpout, "\nii) Eingangsimpedanz: ");
   if(nri != E_null)
      fprintf(Fpout, "Zi(s) = Z(s)/N(s), mit\n Z(s)="),
      ergliste_aus(zri, 6),
      fprintf(Fpout, " N(s)="),
      ergliste_aus(nri, 6);
   else
      fprintf(Fpout, "ﬂ\n");

   /* falls eine BinÑrausgabe gewÅnscht ist: */
   if(Binout != NULL)
      erg_baus(zg, ng, zri, nri);

   /* nun noch die neuangelegten Listen an die Speicherverwaltung zurÅck */

   lfree(zg,  ERGSUMMAND),    lfree(ng,  ERGSUMMAND),
   lfree(zri, ERGSUMMAND),    lfree(nri, ERGSUMMAND);
   
   lfree(din_copy, SUMMAND);
}

/***************************************************************************
* Hier erfolgt die Umformatierung einer Liste in das neue Format. Dieses ist
* in der Lage, auch hîhere Potenzen einer Variablen darzustellen, wie sie bei
* gegenseitiger Bezugnahme von Bauelementen auftreten kînnen. Bei der Um-
* formung werden die BezÅge benutzt und eingesetzt.
*/
static ergsummand *erg_format(term)
summand *term;
{
ergsummand *erg, *h, **hv, zwierg;
vvek altvv;
boolean gef;
zweig_info *pzi;
int i;

   erg = E_null;            /* nimmt neue, zusammengefaûte Liste auf */
   
   while(term != S_null) /* ganze Liste des alten Formates abarbeiten */
   {
      for(i=0; i<MAX_VAR; ++i) /* Zwischenergebnis (gewandelter Teilsummand) */
         zwierg.potenz[i] = 0; /* lîschen */
      zwierg.s_potenz = 0;
      zwierg.koef = term->koef;

      altvv = term->var_vek;
      for(i=0; altvv; ++i, altvv>>=1) /* i ist untersuchte Bauteilnummer */
         if(altvv & 1) /* Variable i ist enthalten */
         {
            pzi = Zweigptr[i];   /* diese nun auf BezÅge untersuchen */
            if(pzi->refp != NULL)/* es gibt einen */
            {
               zwierg.potenz[pzi->refp->btnr]++;
               if(ABS(zwierg.koef) >= MAXLONG/ABS(pzi->reffak))
                  abbruch(12);
               zwierg.koef *= pzi->reffak;
            }
            else/* kein Bezug auf anderes Element, bei sich selbst eintragen */
               zwierg.potenz[i] = 1;/* Bezugsbasis hat sicher kleinere Nr. */
                                    /* als die Bezugnehmenden */

            /* nun noch die Potenz von s mitfÅhren */
            if(pzi->art==IMP || pzi->art==CAP)
               ++zwierg.s_potenz;
         }  
      /* Ende der Untersuchung, welche Variablen im Teilsummand stehen */

      /* Zwi(schen)erg(ebnis) enthÑlt nun den ins neue Format gebrachten Teil-
         summaneden. Er wird nun wie gehabt in die Ergebnisliste einsortiert.
         Es ist allerdings aufwendiger, weil der Vergleich auf identische
         Variablenkombinationen nicht mehr durch einen Bitbefehl zu reali-
         sieren ist. */
      if(zwierg.koef == 0)
      {
         term = term->next;
         continue;
      }

      /* sonst Einsortierung in die Liste erg */
      h = erg, hv = &erg, gef = FALSE;
      
		while(h!=E_null && !(gef=ist_gleichartig(&zwierg, h)))
         hv = &h->next, h = h->next;
      
      if(gef == FALSE)            /* neuen Summand erzeugen */
         *hv = balloc(ERGSUMMAND),
         (*hv)->next    = h,
         eintragen(*hv, &zwierg);
      else /* gleiche Summanden zusammenfassen */
         if(ABS(h->koef) < MAXLONG-ABS(zwierg.koef))
         {
            if((h->koef+=zwierg.koef) == 0) /* zusammengefaûte Summanden */
               *hv = h->next,               /* heben sich auf */
               bfree(h, ERGSUMMAND);
         }
         else
            abbruch(12);
         
      /* Ende der Einsortierung in die Liste erg */

      term = term->next;
   }/* Ende der Schleife Åber Terme der zu wandelnden Liste */      
   return(erg);
}

/***************************************************************************
* Zwei Hilfsroutinen von erg_format(): zwei Summanden auf Åbereinstimmende
* Variablenkombinationen untersuchen, sowie öbertragung der Werte von einem
* Summanden auf einen anderen.
*/
static boolean ist_gleichartig(sum1, sum2)
ergsummand *sum1, *sum2;
{
int i;
byte *p1, *p2;

   p1 = sum1->potenz, p2 = sum2->potenz;
   
   for(i=0; i<Anz_var && p1[i]==p2[i]; ++i)
      ;
   return(i==Anz_var);
}

static void eintragen(ziel, quelle)
ergsummand *ziel, *quelle;
{
int i;
byte *p1, *p2;

   p1 = ziel->potenz, p2 = quelle->potenz;
   
   for(i=0; i<MAX_VAR; ++i)
      p1[i] = p2[i];

   ziel->koef     = quelle->koef;
   ziel->s_potenz = quelle->s_potenz;
}

/***************************************************************************
* Durch die Variablensubstitution werden verschiedene Terme zu gleichen. Da-
* her kann es sein, daû der vorher optimal gekÅrzte Bruch nun nicht mehr ge-
* kÅrzt ist. Das wird hier nachgebessert, auf die KÅrzung der Koeffizienten
* wird allerdings verzichtet.
*/
static void kuerze_erg(z, n)
ergsummand *z, *n;
{
ergsummand *l;
int i, max_s_pot;
byte dspot, gem_potenz[MAX_VAR];
boolean flag, n_ist_neg;

   if(z==E_null || n==E_null) /* Trivialfall, keine KÅrzung mîglich */
      return;

   for(i=0; i<Anz_var; ++i) /* zunÑchst Potenzen des ersten Elementes als */
      gem_potenz[i] = z->potenz[i];  /* gemeinsame Potenz ansetzen */

   l = z->next;
   while(l != E_null) /* jeweils die 'Schnittmenge' aus neuem Glied und bis- */
   {                  /* heriger bilden */
      for(i=0; i<Anz_var; ++i)
         gem_potenz[i] = MIN(gem_potenz[i], l->potenz[i]);
      l = l->next;
   }
   l = n; max_s_pot = -1;
   while(l != E_null)  /* nun das Gleiche mit dem Nenner */
   {
      for(i=0; i<Anz_var; ++i)
         gem_potenz[i] = MIN(gem_potenz[i], l->potenz[i]);

      /* Vorzeichenbestimmung des 1. Nennertermes, das soll positiv werden */
      if((int)l->s_potenz > max_s_pot)
         max_s_pot = (int)l->s_potenz,
         n_ist_neg = l->koef<0;

      l = l->next;
   }
   
   /* Falls gem_potenz nun noch etwas enthÑlt, dann kann gekÅrzt werden.
      Dies kann festgestellt werden, weil das Feld ohnehin durchgesehen
      muû, um die énderung von s_potenz zu ermitteln. */

   flag = FALSE, dspot = 0;
   for(i=0; i<Anz_var; ++i)
      if(gem_potenz[i] > 0)
      {
         flag = TRUE;
         if(Zweigptr[i]->art==IMP || Zweigptr[i]->art==CAP)
            dspot += gem_potenz[i];
      }

   /* nun die eigentliche KÅrzung vornehmen, falls mîglich */
   if(flag || n_ist_neg)
   {
      l = z;
      while(l != E_null) /* jeweils um die gemeinsamen Potenzen verringern */
      {
         for(i=0; flag && i<Anz_var; ++i)
            l->potenz[i] -= gem_potenz[i];
         l->s_potenz  -= dspot;
         l->koef = n_ist_neg? - l->koef: l->koef;
         l = l->next;
      }
      
      l = n;
      while(l != E_null)  /* nun das gleiche mit dem Nenner */
      {
         for(i=0; flag && i<Anz_var; ++i)
            l->potenz[i] -= gem_potenz[i];
         l->s_potenz  -= dspot;
         l->koef = n_ist_neg? - l->koef: l->koef;
         l = l->next;
      }
   }
   /* fertig */
}      
   
/***************************************************************************
* Klartextausgabe der Liste des neuen Formates. Es wird dabei nach den Po-
* tenzen von s sortiert.
*/
static void ergliste_aus(liste, tab)
ergsummand *liste;
int tab;
{
static char endestr[]=" * s^";
int sp, p, anzp[MAX_VAR+1], i;
ergsummand *l;

   if(liste == E_null) /* leere Liste ist Nullfunktion */
   {
      fprintf(Fpout, " 0\n");
      return;
   }
   for(i=0; i<=Anz_var; ++i)/* wieviele Terme gibts zu jeder Potenz in s? */
      anzp[i] = 0;
   l = liste;
   do
      ++anzp[l->s_potenz],
      l = l->next;
   while(l != E_null);
   while(tab<4) /* minimale EinrÅckung bei dieser Darstellung sind 4 Spalten */
      fprintf(Fpout, " "),
      ++tab;
   sp = tab; /* in Spalte sp steht der Cursor nun */

   for(p=Anz_var; p>=0; --p) /* nach fallenden Potenzen von s ausgeben */
   {
      if(anzp[p] == 0) /* Potenz ist gar nicht vertreten */
         continue;

      while(sp<tab) /* einrÅcken, denn bei jeder neuen Potenz eine neue Zeile */
         fprintf(Fpout, " "),
         ++sp;

      if(anzp[p]==1 || p==0) /* dann keine Klammerung erforderlich */
         endestr[0] = ' ',
         fprintf(Fpout, "  ");
      else
         endestr[0] = ')',
         fprintf(Fpout, "+(");
      ++sp;

      l = liste;
      do
      {
         while(l->s_potenz != p) /* nÑchsten passenden Term suchen */
            l = l->next;

         /* Ausgabe der Variablenkombination des l-Termes */
         {
         int i;
         char mal;
         boolean flag; /* wurde Åberhaupt etwas ausgegeben? (wegen Eins-Term) */

            sp += fprintf(Fpout, "%c", l->koef>0? '+': '-');
            if(ABS(l->koef) != 1)
               sp += fprintf(Fpout, "%ld", ABS(l->koef)),
               flag = TRUE,
               mal = '*';
            else
               flag = FALSE,
               mal = '\0';
            for(i=0; i<Anz_var; ++i)
					if(l->potenz[i] == 1)
					{
						if(mal == '*')
							fprintf(Fpout, "*"),
							++sp;
						sp += fprintf(Fpout, "%s", Zweigptr[i]->name),
                  flag = TRUE, /* mindestens 1 Variable ausgegeben */
						mal = '*';
					}
					else if(l->potenz[i] > 1)
					{
						if(mal == '*')
							fprintf(Fpout, "*"),
							++sp;
						sp += fprintf(Fpout, "%s^%u", Zweigptr[i]->name,
																(unsigned)l->potenz[i]),
                  flag = TRUE,
						mal = '*';
					}
            if(!flag)     /* dann steht ein VZ verloren in der Gegend rum */
               fprintf(Fpout, "1");  /* die soll ihm Gesellschaft leisten */
         }
         /* Ende der Ausgabe der Variablenkombination */

         if(--anzp[p] && sp>=70) /* neue Zeile anfangen */
         {
            fprintf(Fpout, " ...\n");
            for(sp=0; sp<=tab-4; ++sp) /* einrÅcken */
               fprintf(Fpout, " ");
            fprintf(Fpout, "... ");
            sp = tab+1;
         }

         l = l->next;
      }
      while(anzp[p] > 0);
      /* Ende der Schleife Åber die Terme derselben s-Potenz */

      if(p>0) /* keine Angabe s^p falls p = 0 */
         fprintf(Fpout, "%s%d", endestr, p);
      fprintf(Fpout, "\n");
      sp = 0;
   }/* Ende der for-Schleife Åber alle mîglichen s-Potenzen */
}
