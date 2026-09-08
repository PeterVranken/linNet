/***************************************************************************
*                                                                          *
*  Teil IV des Programmprojekts analytische Berechnung linearer Netzwerke  *
*                                                                          *
*  Der Lîsungsalgorithmus fÅr Determinanten, deren Elemente Variablen ent- *
*  halten dÅrfen. Es muû dabei die Bedingung erfÅllt sein, daû keine Un-   *
*  terdeterminante existiert, die eine der Variablen mit einer Potenz >1   *
*  oder <0 enthÑlt.                                                        *
*                                                                          *
***************************************************************************/

/*
   (c) Peter Vranken 11/91
*/

#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>
#include "lin_netz.h"

/***************************************************************************
* Hier folgt das modifizierte Gaussverfahren zur Determinantenberechnung.
* Es erfordert keine Division mehr und kann daher z.b. auch mit natÅrlichen 
* Zahlen durchgefÅhrt werden. Die Arithmetik der verwendeten algebraischen
* Struktur muû eine Addition sowie eine Multiplikation umfassen. Sie wird 
* aber ausschlieûlich in der Routine Elementaroperation verwendet, hier sind
* nur die Steuerstrukturen realisiert. Ist bei der Multiplikation jedoch
* mit numerischen Fehlern zu rechnen, dann kann eine Pivotisierung erforder-
* lich werden, die bereits in dieser Routine BerÅcksichtigung der Arithmetik
* erforderlich machen kann.
*/
summand *determinante(n, name)
int n;      /* n ist Grîûe der Matrix, Indexbereich 0...n-1 */
char *name; /* so kann angegeben werden, welche Determinante berechnet wird */
{
summand *t, *h;
int el_s, piv, z, s;
boolean vz;

   if(Verbose >= 2)
      fprintf(Fpout, "\n****************************************************\n"
                       " Berechnung von:\n %s\n"
                       " (vollstÑndiges Protokoll der Eliminationsschritte)\n"
                       "****************************************************\n\n",
                     name);

   t = S_eins; /* Teiler mit Einselement initialisieren */

   for(el_s=0, vz=FALSE; el_s<n-1; ++el_s)/* Schleife Åber die n-1 Eliminationsschritte */
   {
      /* Pivotisierung: nur Spalte != Null nach links (wenn nîtig) */
      if(!A[piv=el_s][el_s]->koef)
      {
         do
            if(++piv>=n)   /* dann ist Determinante Null */
            {
               if(Verbose >= 2)
                  fprintf(Fpout, "vorzeitige Beendigung der Elimination: det(A)=0\n");
               /* die restlichen Elemente freigeben */
               for(z=el_s; z<n; ++z)
                  for(s=0; s<n; ++s)
                     lfree(A[z][s], SUMMAND);
               if(el_s > 0) /* dann ist t ein noch belegtes Elemenet von A */
                  lfree(t, SUMMAND);
                  
               return(S_null); /* und fertig */
            }
         while(A[el_s][piv]->koef == 0);
         h=A[el_s][el_s], A[el_s][el_s]=A[el_s][piv], A[el_s][piv]=h;
         vz = !vz; /* Spaltenvertauschung wegen Vorzeichen merken */
      }
      for(z=el_s+1; z<n; ++z)/* Schleife Åber alle unten liegenden Zeilen */         
      {
         h=A[z][el_s], A[z][el_s]=A[z][piv], A[z][piv]=h;/* Pivotspalte nach links */
         for(s=el_s+1; s<n; ++s) /* Schleife Åber alle rechts liegenden Spalten */
            elementaroperation(el_s, z, s, t);/* Eliminierung bei (z,s) */
      }
      t = A[el_s][el_s]; /* neuen Teiler zuweisen */

      /* nunmehr ÅberflÅssige Elemente freigeben, i.e. Zeile el_s */
      for(s=0; s<n; s++)
         if(s!=el_s)
            lfree(A[el_s][s], SUMMAND);
         else if(s) /* Diagonalelement wird noch als nÑchster Teiler gebraucht */
            lfree(A[s-1][s-1], SUMMAND);/* vorheriges kann aber verschwinden */

      if(Verbose >= 2)  /* dann eine Ausgabe nach jedem Schritt */
         el_schritt_aus(n, el_s, piv);
      
   }  /* Ende der Schleife Åber die Eliminationsschritte */
   if(vz) /* also eine ungerade Anzahl Zeilenvertauschungen */
   {
      h = A[n-1][n-1]; /* Liste durchlaufen und VZ Ñndern */
      while(h != S_null)
         h->koef = - h->koef,
         h = h->next;
   }
   if(n>1)
      lfree(t, SUMMAND);/* Teiler in Spalte/Zeile n-2 freigeben */
   for(s=0; s<n-1; ++s) /* nun noch die letzte Zeile freigeben */
      lfree(A[n-1][s], SUMMAND);
   return(A[n-1][n-1]); /* wobei das letzte Element als Ergebnis stehenbleibt */
}

/***************************************************************************
* in dieser Routine wird ein Element der Matrix neu berechnet. Dies nach
* der Formel: A(z,s)=( A(z,s)*A(el_s,el_s)-A(el_s,s)*A(z,el_s) )/t. Der Clou
* besteht darin, daû die Division stets aufgeht. Man kann dies bei der Re-
* alisierung berÅcksichtigen. Hier wird die Operation fÅr Terme der auf-
* tretenden speziellen Klasse durchgefÅhrt.
*/
static void elementaroperation(el_s, z, s, t)
int el_s, z, s;
summand *t;
{
summand *op1, *op2, *op2anf, *rest, *erg, *h, **hv;
int vz;
koeffizient koef12;
vvek vv12, op1vv, op2vv, tvv;

   rest = S_null; /* Initialisierung der Resttermliste mit Nullelement */

   tvv = t->var_vek;
   /* Schleife fÅr 2 Produkte mit unterschiedlichem VZ */
   for(vz= 1, op1=A[z][s],    op2anf=A[el_s][el_s];
       vz>=-1;
       vz-=2, op1=A[el_s][s], op2anf=A[z][el_s])
   {
      while(op1->koef) /* Schleife bis zum letzten Glied des 1. Operanden */
      {
         op2 = op2anf; /* mit jedem Glied von op1 wird op2 ganz durchlaufen */
         op1vv = op1->var_vek;
         while(op2->koef)
         {
            op2vv = op2->var_vek;
            /* Gibt op1*op2/t einen (brauchbaren) Beitrag zum Ergebnis? */
            if(!((~op1vv & ~op2vv &  tvv)
                |( op1vv &  op2vv & ~tvv)))
            /* 1. Konjunktion true => Teiler ist gar nicht enthalten */
            /* 2. Konjunktion true => Teiler hat zu hohe Potenzen */
            {  /* ist also ein brauchbarer Term */
               vv12 = op1vv ^ op2vv  ^ tvv;/* ist unter dieser Bed. korrekt */
               koef12 = op1->koef*(vz>0? op2->koef: -op2->koef);
                                    /* Division erfolgt spÑter */

               /* nun in die Resttermliste eintragen */
               h  =  rest, hv = &rest;

               while(h->var_vek > vv12)
                  hv = &h->next, h = h->next;

               if(h==S_null || h->var_vek<vv12)/* neuen Summand erzeugen */
                  *hv = balloc(SUMMAND),
                  (*hv)->next    = h,
                  (*hv)->koef    = koef12,
                  (*hv)->var_vek = vv12;
               else if((h->koef+=koef12) == 0)
                  (*hv) = h->next,
                  -- h->koef,
                  bfree(h, SUMMAND);
               /* fertig mit Eintragung in Resttermliste */
            }/* Ende if brauchbarer Term */
            op2 = op2->next;
         }/* Ende der Schleife Åber die Liste des 2. Operanden */
         op1 = op1->next;
      }/* Ende der Schleife Åber die Liste des 1. Operanden */
   }/* Ende for-Schleife Åber beide Produkte des ZÑhlers */

   /* Divisionsteil */
   {
   summand *rl, *h1, **h1v, *n;
   koeffizient r, q, rmkoef, tkoef;
   vvek rmvv, nvv, ergvv;

   rl = rest, erg = S_null, tkoef = t->koef;

   /* Liste mit allen verbleibenden, aufzuteilenden Termen abarbeiten */
   while(rl != S_null)
   {
      q = rl->koef / tkoef,  r = rl->koef % tkoef;
      if(q) /* Teilung mîglich */
      {
         /* Einsortierung in Ergebnisliste */
         h = erg, hv = &erg, ergvv = rl->var_vek; /* ergkoef ist q */

         while(h->var_vek > ergvv)
            hv = &h->next, h = h->next;

         if(h==S_null || h->var_vek<ergvv)/* neuen Summand erzeugen */
            *hv = balloc(SUMMAND),
            (*hv)->next    = h,
            (*hv)->koef    = q,
            (*hv)->var_vek = ergvv;
         else if((h->koef+=q) == 0)
            (*hv) = h->next,
            -- h->koef,
            bfree(h, SUMMAND);
         /* Ende der Einsortierung in Ergebnisliste */

         /* nun die RÅckmultiplikation */
         n = t;
         do
         {
            nvv = n->var_vek;
#ifdef TEST
   if(n!=t && (((ergvv & nvv) | ~tvv)==0xffff) && tvv)
   {printf("Fehler: tvv != 0, aber nach RÅckmultiplikation noch quadratisch enthalten!\n");TASTE}
#endif
            if(!((nvv & ergvv & ~tvv)|(~nvv & ~ergvv & tvv)))
            {
               /* Einsortierung in die Restliste */
               h1 = rest, h1v = &rest,
               rmkoef = - n->koef*q, rmvv = nvv ^ ergvv ^ tvv;

               while(h1->var_vek > rmvv)
                  h1v = &h1->next, h1 = h1->next;
   
               if(h1==S_null || h1->var_vek<rmvv)/* neuen Summand erzeugen */
                  *h1v = balloc(SUMMAND),
                  (*h1v)->next    = h1,
                  (*h1v)->koef    = rmkoef,
                  (*h1v)->var_vek = rmvv;
               else if((h1->koef+=rmkoef) == 0)
                  (*h1v) = h1->next,
                  -- h1->koef,
                  bfree(h1, SUMMAND);
               /* Ende der Einsortierung in die Restliste */
            }
         }
         while((n=n->next) != S_null); /* Ende RM-Schleife Åber Nennerterme */
      }/* Ende Abfrage, ob Division des Terms Ergebnisbeitrag liefert */
      if(r) /* Teilung konnte nicht vollstÑndig erfolgen */
      {
         /* rl auf Nachfolger bzw. Anfang der Liste, wenn Ende erreicht ist */
         rl = (rl=rl->next)==S_null? rest: rl;
      }
      else /* kein Rest, also existiert Element h nicht mehr */
         rl = rest; /* Liste wieder von vorne untersuchen */
      }
   }/* Ende von while Restliste noch Terme endhÑlt */

#ifdef TEST
   if(rest!=S_null){printf("Fehler: Restliste ist nicht vollstÑndig gelîscht!\n");TASTE}
#endif

   /* Ende des Blockes Division */

   /* Ergebnis in der Matrix eintragen */

   lfree(A[z][s], SUMMAND); /* Platz des bisherigen Elementes freigeben */
   A[z][s] = erg; /* berechnetes Ergebnis eintragen */
}

/***************************************************************************
* Diese Subtraktion zweier Listen wird bei der Bildung des Endergebnisses
* benîtigt. Es wird a = a-b berechnet, b bleibt unverÑndert erhalten. Der
* neue Anfang von a wird zurÅckgegeben.
*/
summand *differenz(a, b)
summand *a, *b;
{
summand *h, **hv;
vvek bvv;

   while(b != S_null) /* alle Elemente von b in a einsortieren, wenn mîglich */
   {                  /* sonst neues Element anhÑngen */
      /* Einsortierung in die Liste a */
      h   = a, hv = &a,
      bvv = b->var_vek;
      
      while(h->var_vek > bvv)
         hv = &h->next, h = h->next;
      
      if(h==S_null || h->var_vek<bvv)/* neuen Summand erzeugen */
         *hv = balloc(SUMMAND),
         (*hv)->next    = h,
         (*hv)->koef    = - b->koef,
         (*hv)->var_vek = bvv;
      else if((h->koef-=b->koef) == 0)
         *hv = h->next,
         -- h->koef,
         bfree(h, SUMMAND);
      /* Ende der Einsortierung in die Liste a */

      b = b->next;
   }
   return(a);
}
           
/***************************************************************************
* Ausgabe der Matrix im Klartext, sie wird in eine Datei ausgegeben, da die
* Datenmengen recht groû sind. Aufbau: jedes Element wird mit einer Angabe
* seiner Position in eine eigene Zeile geschrieben.
*/
static void el_schritt_aus(n, el_s, piv)
int n, el_s, piv;
{
static int vz;/* Vorzeichenvertauschungen durch Pivotisierung mitfÅhren */
int z, s;

   if(el_s==0) /* Initialisierung Åber static int vz=0; geht hier nicht, da */
      vz  = piv!=el_s;/* die Determinantenelimination mehrfach aufgerufen wird */
   else
      vz += piv!=el_s;/* also ein Inkrement falls Pivotisierung (dann ist piv!=el_s) */
   
   fprintf(Fpout, "%d. Eliminationsschritt ist beendet:\n", el_s+1);
   if(el_s != piv)
      fprintf(Fpout, "Pivotisierung: es wurde Spalte %d mit Spalte %d vertauscht!\n",
                     el_s, piv);
   
   for(z=el_s+1; z<n; ++z)
   {
      
      if(el_s != n-2) /* also nicht der letzte Schritt */
         fprintf(Fpout, "Zeile %d:\n", z);
      for(s=el_s+1; s<n; ++s)
      {
         if(el_s == n-2) /* also der letzte Schritt */
            fprintf(Fpout, "%cdet(A)= ", vz&1? '-': '+');
         else
            fprintf(Fpout, "(%2d,%2d): ", z, s);
         liste_aus(A[z][s], 9);
      }
   }
   fprintf(Fpout, "Speicherbedarf betrug bisher maximal %ld Blîcke\n\n", mem_bedarf(SUMMAND));
}


