/***************************************************************************
*                                                                          *
*  Teil III des Programms zur Berechnung der šbertragungsfunktion eines    *
*  Netzwerkes aus linearen, idealen, Bauelementen, das durch eine Knoten-  *
*  liste beschrieben ist.                                                  *
*                                                                          *
*  Hier stehen die Vereinfachung, šberprfung und Auswertung der von       *
*  der Einleseroutine aufgebauten Datenstruktur, die das Netz beschreibt.  *
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
int Alt[MAX_KNOTEN], Neu[MAX_KNOTEN], Anz_upot;

/***************************************************************************
* Hier wird die šbrtragungsfunktion, sowie die Eingangsimpedanz ermittelt.
* Die eigentlichen Berechnungen erfolgen durch unten stehende Routinen, hier
* wird nur ihre Aufrufabfolge gesteuert.
*/
void uebertragungsfkt()
{
summand *d, *din, *doutp, *doutm, *dout;

   neue_pot_nr();/* zweckm„ige Umbenennung der Knotenpotentiale vornehmen */
   bilde_det(-1);/* Hauptdeterminante bilden */
   if(Verbose >= 1) /* Ausgabe des LGSs gewnscht */
      zeige_lgs();
   /* Determinante berechnen, A wird gel”scht */
   d = determinante(Anz_upot, "Hauptdeterminante");
   /* Z„hlerdeterminante der Eingangsspannung bilden und berechnen */
   bilde_det(Kliste[U1p].pot_nr);
   din = determinante(Anz_upot, "Eingangspotential");

   /* dito die beiden Z„hlerdeterminanten der Ausgangsspannung */
   if(U2p != Masse)
      bilde_det(Kliste[U2p].pot_nr),
      doutp = determinante(Anz_upot, "Ausgangspluspotential");
   else
      doutp = S_null;

   if(U2m != Masse)
      bilde_det(Kliste[U2m].pot_nr),
      doutm = determinante(Anz_upot, "Ausgangsminuspotential");
   else
      doutm = S_null;

   /* nun die Z„hlerdeterminanten der Ausgangspotentiale subtrahieren, */
   /* ergibt die Ausgangsspannung */
   dout = differenz(doutp, doutm);
   lfree(doutm, SUMMAND); /* doutp wird von differenz freigegeben */

   ergebnis(d, din, dout);

   /* Speicher wieder freigeben */
   lfree(d, SUMMAND), lfree(din, SUMMAND), lfree(dout, SUMMAND);
}

/***************************************************************************
* Hauptroutine: Erstellen einer Determinante des Gleichungssystemes aus der
* Datenstruktur. Es drfen sich nur noch Operationsverst„rker und Stromquel-
* len im Netz befinden.
*/
void bilde_det(det_nr)
int det_nr;
{
zweig *pz;
summand *h;
zweig_info *pzi;
int z, s, eigen, ende, i, pp, mp, be, massepot, vz, endknt;

   massepot = Anz_upot;/* Masse hat immer die h”chste Potentialnummer, diese */
                       /* ist gleich der Zahl unabh„ngiger Potentiale */

   /* Sollten die Elemente von A noch Listen enthalten, dann gehen sie der
      Speicherverwaltung hier verloren! Weiter entsteht jedoch kein Schaden. */
   for(s=0; s<Anz_upot; ++s)
      for(z=0; z<Anz_upot; ++z)/* A als Null-Matrix initialisieren */
         A[z][s] = S_null;

   for(i=z=0; i<Anz_knoten; ++i)/* Schleife ber alle Knoten, ber die eine Aus- */
   {                            /* sage 'Summe aller I gleich Null' m”glich ist */
      if(Kliste[i].ist_qop /* z.B. nicht m”glich bei OP-Ausg„ngen */
       || i==Masse)        /* Masseknoten w„re linear abh„nig von den brigen */
         continue;

      /* z z„hlt lediglich die Anzahl der erstellten Gleichungen, ist damit
         die Zeile in der Matrix, in die in diesem Durchgang eingetragen wird */

#ifdef TEST
      if(z>=Anz_upot || z>=MAX_VAR)
      {
         printf("interner Fehler: mehr Gleichungen (%d) als Unbekannte oder zul„ssig!\n", z);
         TASTE
         break;
      }
#endif

      /* ber den Knoten i eine Stromsummenaussage machen */

      eigen = Kliste[i].pot_nr;/* Potential-Unbekannte des aktuellen Knotens */
      pz = Kliste[i].zw1;/* Liste mit Zweigen, die von i ausgehen, abarbeiten */
      while(pz!=Z_null)
      {
         pzi = pz->was;  /* pzi zeigt auf Zweiginformation */
         be  = pzi->art; /* welchen Tpys ist das Bauelement des Zweigs? */
         if(be == QI)    /* Stromquelle */
         {
            pp = Kliste[pzi->usp].pot_nr, mp = Kliste[pzi->usm].pot_nr;
            vz = pzi->ak==i? -1  /* Quelle zuflieend in Knoten i */
                           : +1; /* Quelle abflieend aus Knoten i */

            /* Hinweis auf diesen Zweig bei den Steuerspannungspolen einh„ngen */
            if(pp != massepot)
               h = balloc(SUMMAND),
               h->next = A[z][pp], h->var_vek = pzi->vv, h->koef = vz,
               A[z][pp] = h;
            if(mp != massepot)
               h = balloc(SUMMAND),
               h->next = A[z][mp], h->var_vek = pzi->vv, h->koef = -vz,
               A[z][mp] = h;

         }/* Ende der Stromquellenbearbeitung */
         else if(be==RES || be==CAP || be==LW || be==IMP)/* pasives Element */
         {
            /* Knotennummer und Potential-Unbekannte des anderen Endes: */
            endknt = pzi->ak==i? pzi->ek: pzi->ak;
            ende   = Kliste[endknt].pot_nr;
            if(eigen != ende) /* Strom fliet nur bei Potentialdifferenz */
            {
               /* Verweis auf diesen Zweig bei den beteiligten Spalten einh„ngen.
                  Eintrag entf„llt, wenn der betreffende Knoten Massepotential
                  hat, denn dann kann sein Potential (es ist ja Null) keinen
                  Strom erzeugen und geht somit in die Gleichung nicht ein.
                  Wegen virtueller Masseknoten in OP-Schaltungen (die im Ge-
                  gensatz zum linear abh„ngigen Masseknoten trotzdem sinnvolle
                  Stromsummenaussagen erlauben), kann dies auch fr den gera-
                  de betrachteten Knoten i selbst gelten. */

               if(eigen != massepot)
                  h = balloc(SUMMAND),
                  h->next = A[z][eigen], h->var_vek = pzi->vv, h->koef = +1,
                  A[z][eigen] = h;

               if(ende != massepot)
                  h = balloc(SUMMAND),
                  h->next = A[z][ende],  h->var_vek = pzi->vv, h->koef = -1,
                  A[z][ende] = h;
            }
         }/* Ende Behandlung passiver Zweig */
#ifdef TEST
         else if(be!=OPQU && be!=OPZI && be!=UIN && be!=UOUT && be!=US)
         /* fataler interner Programmfehler */
         {
            printf("interner Fehler: unerlaubtes Zweigelement (%d) bei LGS-Erstellung gefunden!\n", be);
            TASTE
            exit(-1);
         }
#endif
         pz = pz->next;
      }/* Ende der Schleife ber alle Zweige vom Knoten i */

      /* rechte Seite, schon in die richtige Spalte eingetragen: */
      if(det_nr >= 0)/* sonst ist Haupt-, bzw. Nennerdeterminante gewnscht */
      {
         lfree(A[z][det_nr], SUMMAND);
         if(i == U1p) /* nur in U1p wird ein externer Strom eingespeist */
            h = balloc(SUMMAND),
            h->koef    = 1,
            h->var_vek = 0,
            h->next    = S_null,
            A[z][det_nr] = h;
         else
            A[z][det_nr] = S_null;
      }
      ++z; /* mit n„chster Zeile anfangen */
   }/* Ende der Schleife i ber die Gleichungen */
}

/***************************************************************************
* Hier folgt eine Routine, die die Potentialnummern „ndert. Derzeit sind die-
* se nicht ohne Lcken vergeben worden, Ursache ist die šbertragung eines 
* Knotenpotentiales auf einen anderen Knoten bei der Behandlung der OP-Ein-
* g„nge. Hier wird so umnummeriert, da die Nummern hinterher bei Null anfan-
* gen und dicht an dicht liegen. Zugleich werden die Umrechnungsfunktionen
* alt->neu und neu->alt erstellt.
*/
static void neue_pot_nr()
{
int i, j, h, npn, ge, a, b, masspot;

   for(i=0; i<Anz_knoten; ++i) /* Initialisierung der Umrechnungsfunktionen */
      Alt[i] = i,     /* als Identit„ts-Funktion */
      Neu[i] = i;

   for(i=npn=0; i<Anz_knoten; ++i)
   {
      if((ge=Kliste[i].pot_nr)>npn)
      {
         a = Alt[npn], b = Alt[ge],
         h = Neu[a], Neu[a] = Neu[b], Neu[b] = h,/* Elemente von neu vertauschen */
         Alt[npn] = b, Alt[ge] = a;              /* und jene von alt */

         Kliste[i].pot_nr = npn;         /* nachdem die Umbenennung pro- */
         for(j=i+1; j<Anz_knoten; ++j)   /* tokolliert ist, wird sie ber- */
            if(Kliste[j].pot_nr == ge)   /* all durchgefhrt. */
               Kliste[j].pot_nr = npn;
            else if(Kliste[j].pot_nr == npn)
               Kliste[j].pot_nr = ge;
      }
      npn += ge>=npn;
   }

   /* nun noch dafr sorgen, da Massepotential das mit der h”chsten Nummer
      ist. Dann sind die unbekannten, unabh„ngigen Potentiale linear von
      Null an durchnummeriert, und ihre Nummer ist zugleich Position in der
      Matrix. Dies vereinfach das Aufstellen des LGSs. */

   --npn;
   masspot = Kliste[Masse].pot_nr;
   for(i=0; i<Anz_knoten; ++i)
      if(Kliste[i].pot_nr == masspot)
         Kliste[i].pot_nr = npn;
      else if(Kliste[i].pot_nr == npn)
         Kliste[i].pot_nr = masspot;

   /* Protokollierung der Vertauschung: */
   a = Alt[npn], b = Alt[masspot],
   h = Neu[a], Neu[a] = Neu[b], Neu[b] = h,
   Alt[npn] = b, Alt[masspot] = a;

   Anz_upot = npn;
}

/***************************************************************************
* Ausgabe des LGSs. A[][] mu bei Aufruf die Hauptdeterminante enthalten.
*/
static void zeige_lgs(void)
{
int i, z, s, u1ppot, u2ppot, u2mpot;

   fprintf(Fpout, "\n*************************************************************\n"
                    " das sich ergebende Gleichungssystem:\n"
                    " Bauteilangaben sind stets als deren Leitwerte zu verstehen!\n"
                    "*************************************************************\n\n");

   u1ppot   = Kliste[U1p].pot_nr,
   u2mpot   = Kliste[U2m].pot_nr,
   u2ppot   = Kliste[U2p].pot_nr;

   fprintf(Fpout, "G=(Phi(%s)-Phi(%s))/Phi(%s)\n\n",
                   Knotname[Alt[u2ppot]], Knotname[Alt[u2mpot]],
                   Knotname[Alt[u1ppot]]);

   for(i=z=0; i<Anz_knoten; ++i)/* Schleife ber alle Knoten, ber die eine Aus- */
   {                            /* sage 'Summe aller I gleich Null' m”glich ist */
      if(Kliste[i].ist_qop /* z.B. nicht m”glich bei OP-Ausg„ngen */
       || i==Masse)        /* Masseknoten w„re linear abh„nig von den brigen */
         continue;

      /* nun die Zeile im Klartext ausgeben */
      fprintf(Fpout, "Stromsummenaussage ber Knoten %s:\n", Knotname[i]);
      for(s=0; s<Anz_upot; ++s)
      {
         fprintf(Fpout, "Phi(%s): ", Knotname[Alt[s]]);
         liste_aus(A[z][s], (int)strlen(Knotname[Alt[s]])+7);
      }
      fprintf(Fpout, "rechte Seite: %s\n", i==U1p? "Iin": "0");
      ++z;
   }/* Ende der Schleife i ber die Gleichungen */
}

/***************************************************************************
* Topologietest: zun„chst wird untersucht, ob das Netz berhaupt zusammen-
* h„ngend ist. Dies mu nur wegen des Knotenpotentialverfahrens erfolgen,
* mit gesteuerten Quellen sind auch topologisch v”llig getrennte Netze 
* physikalisch gekoppelt, sinnvoll und berechenbar. Das KPV versagt hier
* aber, weil die Teilnetze keine definierte Potentialdifferenz haben. Der
* Haupttest auf nicht stromfhrende, tote Teilnetze liee ein solches Netz
* aber zu. Fehler: Returncode TRUE.
*/
#define EINHEIT  0
#define TOTEARME 1

boolean topologietest()
{
int i;

   if(Anz_knoten<2)
   {
      printf("Netz hat zu wenige Knoten!\n");
      return(TRUE);
   }
   if(!voll_baum(U1p, EINHEIT))
   {
      printf("ergab nichtzusammenh„ngendes Netz!\n");
      return(TRUE);
   }
   for(i=0; i<Anz_knoten && voll_baum(i, TOTEARME); ++i)
      ;
   if(i != Anz_knoten)
   {
      printf("fand totes Teilnetz!\n");
      return(TRUE);
   }
   return(FALSE);
}

/***************************************************************************
* Ermittlung eines vollst„ndigen Baumes. Ausgehend vom ersten Knoten wird
* nach dem Verfahren des Backtrackings ein Pfad gesucht. Das Feld livb[] 
* registriert, welche Knoten bereits im vollst„ndigen Baum liegen und somit
* nicht mehr berhrt werden drfen. Damit ist auch sichergestellt, da keine
* Zweige wiederholt verwendet werden, denn dann wrden Anfangs- und Endkno-
* ten doppelt benutzt. Der Pfad selbst wird nicht protokolliert, da es hier
* nur auf eine Existenzprfung ankommt. Das an sich rekursive Backtracking
* wird hier iterativ implementiert. Da stets versucht wird m”glichst frh
* zu verzweigen, erh„lt man einen linearen Pfad als bevorzugte Struktur des
* Baumes. 
*/
static boolean voll_baum(test_knot, flag)
int test_knot;
int flag;
{
int sp, i, ende, erster;
boolean livb[MAX_KNOTEN];
zweig_info *pzi;
zweig *pz, *s[MAX_KNOTEN];

   for(sp=0; sp<Anz_knoten; ++sp) /* alle Knoten sind noch unbenutzt */
      livb[sp] = FALSE;
                                    
   /* Anfang des voll. Baumes festlegen, darf nicht der Test-Knoten sein */
   /* der Baum mu stets von den Eingangsklemmen ausgehen */
   if(flag == EINHEIT)/* einfach den Baum bilden ohne US-Zweige zu verwenden */
      erster = test_knot,
      i = 1; /* Startknoten ist schon abgehakt */
   else /* tote Arme suchen */
      erster = test_knot==U1p? Masse: U1p,
      i = 2,/* hier sind schon zwei Knoten abgehakt, Testknoten ist gesperrt */
      livb[test_knot] = TRUE;

   livb[erster] = TRUE;

   /* Stapelinitialisierung */
   s[0] = Kliste[erster].zw1;
   sp = 1; 

   while(sp)
   {
      pz = s[sp-1];    /* obersten Zweig auf dem Stapel lesen */
      if(pz == Z_null) /* war schon das Ende der Liste mit Wegen vom unter- */
      {                /* suchten Knoten aus => Rckschritt */
         --sp;         /* dazu das Element l”schen */
         continue;
      }   

      pzi =  pz->was; /* nun die Zweiginformation auswerten */

      ende = livb[pzi->ak]==TRUE? pzi->ek: pzi->ak;
      /* anderes Ende des Zeiges wird so nicht wirklich bestimmt: wenn beide
         Knoten TRUE sind, dann ist unsicher, ob man Anfang oder Ende erwischt
         hat. In diesem Falle wird aber in jedem Fall die n„chste if-Abfrage
         richtig ausgefhrt, soda eine Vertauschung unwesentlich ist.
         (Dazu: Anfangsknoten ist in jedem Falle TRUE) */
         
      if(pzi->art   == UOUT              /* dieser Zweig ist geperrt */
      || (flag==EINHEIT && pzi->art==US) /* dieser nur im Falle EINHEIT */
      || livb[ende] == TRUE)/* dann war der Endpunkt dieses Zweiges */
      {                     /* schon besucht worden */
         /* jedenfalls ist hier entschieden, da es mit diesem Zweig nicht */
         /* weitergeht, Vorw„rtsschritt unm”glich. Es kann aber */
         /* der n„chste Zweig vom aktuellen Knoten aus untersucht werden */
         s[sp-1] = pz->next;
      }
      else /* mit diesem Zweig wird ein noch unberhrter Knoten erreicht, */
      {    /* also in den Pfad aufnehmen */
         livb[ende] = TRUE;
         if(++i == Anz_knoten) /* dann ist der v.B. gefunden, k Knoten erreicht */
            break;
         /* mit dem 1. Zweig des neuen Knotens weiterprobieren */
         s[sp++] = Kliste[ende].zw1;
      }
   }/* Ende der while-Schleife ber noch zu untersuchende Zweige */
   return(i==Anz_knoten);/* TRUE ist Rckgabewert, wenn ein v.B. ermittelt wurde */
}

