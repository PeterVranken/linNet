/***************************************************************************
*                                                                          *
* Implementierung eines Runge-Kutta-Integrationsalgorithmusses vierter     *
* Ordnung fÅr lineare Differentialgleichungen beliebiger Ordnung.          *
* Als Anregung darf aber nur die Funktion x ohne ihre Ableitungen vorkom-  *
* men. Es kînnen damit DGLen der Form                                      * 
*                                                                          *
* Y(n) + a(n-1)*Y(n-1) + ... + a(1)*Y' + a(0)*Y = X(t)                     *
* ( Y(i) := (d/dx)^i Y; a(n) ist konstanter Koeffizient der DGL)           *
*                                                                          *
* integriert werden.                                                       *
* Zudem muû ein Anfangswertevektor mit den Startwerten fÅr alle Y(i) vor-  *
* handen sein, auûer i=n.                                                  *
* Die Implementierung erfolgt gemÑû E.SchrÅfer, "Signalverarbeitung", Han- *
* ser 1990, S.132ff.                                                       *
*                                                                          *
* DarÅber hinaus enthÑlt das Modul eine Routine, um die Anfangswerte zu    *
* berechnen, die sich ergeben, wenn man die Berechnung von Stoû- und       *
* Sprungantwort beliebiger gebrochen rationaler öbertragungsfunktionen auf *
* eine DGL der obigen Form zurÅckfÅhrt.                                    *
*                                                                          *
***************************************************************************/

/*
   (c) Peter Vranken 27/3/1992
*/

#include <stdio.h>
#include <stdlib.h>
#include <float.h>
#include <math.h>

#include <RK4.h>

/***************************************************************************
* Prototypen der internen Funktionen
*/
int _anfangswerte(double *Z, int gz, double *N, int gn, double *Y0);

/***************************************************************************
* Makro fÅr die Implementierung der erforderlichen Vektoroperation F. Diese
* soll aus ZeitgrÅnden nicht als Unterprogramm realisiert werden.
*
* folgende Variablen mÅssen im Anwendungsbereich sichtbar sein:
* a[n]: Feld mit den double-Koeffizienten der DGL
* ae: Zeiger auf den letzten Koeffizienten, also ae = &a[n-1]
*
* Berechnung des Ableitungsvektors gemÑû der DGL aus dem derzeitigen
* Ableitungsvektor y und dem skalaren aktuellen Eingangswert Xn:
*/
#define F(e, Xn, y) {double *ep, *yp, *ap, ynm;                   \
                     for(ep=e, yp=y, ap=a, ynm=Xn; ap<ae; )       \
                        ynm  -= (*ap++) * (*yp++),                \
                        *ep++ = *yp;                              \
                     *ep = ynm - (*ap) * (*yp);}

/* F kann am Platz arbeiten (d.h. 1.Operand darf identisch mit 3. sein),
   muû es aber nicht */

/***************************************************************************
* Jeder Aufruf der Routine RK liefert einen extrapolierten Wert. Daher mÅs-
* sen vor dem ersten Aufruf alle relevanten Parameter bereits eingestellt
* sein. Das erledigt diese Routine, indem sie entsprechende modulglobale
* Variablen vorbesetzt.
* Bedeutung der Parameter:
* t0: Startzeit der Simulation
* Y0: Ableitungsvektor mit den Startwerten, die zum Startzeitpunkt t0 vor-
*     liegen sollen
* n: Ordnung der DGL, LÑnge aller auftretenden Vektoren
* X: Zeiger auf double-Fkt, die den Wert der anregenden Fkt zur Zeit t>t0
*    zurÅckgibt.
* stepsize: Schrittweite, d.h. der n-te Aufruf von RK gibt den Wert
*           Y(t0+n*stepsize)
* xp, yp, tp: Hier werden der aufrufenden Funktion Zeiger auf die Werte der
*             aktuellen Simulationszeit, der anregenden Funktion X(t) und
*             der Ausgangsfkt Y=Y{X}(t) Åbergeben, vermittels derer diese
*             die spÑteren Ergebnisse von RK() auslesen kann.
*
* die besetzten globalen Daten:
*/
static int n, nm;
static double h, h0_5, h0_25, h0_167, h2, t, Y[NMAX], (*X)(double), a[NMAX],
              *k1=NULL, *k2, *k3, *k4, *ae,
              xn, xn_05, xn_1;

int init_RK(t0, Y0, koef_vek, _n, Xt, stepsize, xp, yp, tp)
double t0, *Y0, *koef_vek, (*Xt)(double), stepsize, **xp, **yp, **tp;
int _n;
{
int i;

   t = t0;
   *xp = &xn;
   *yp = Y; /* denn Y[0] enthÑlt den aktuellen Wert von Y(t) */
   *tp = &t;

   if(_n<1 || _n>NMAX)
      return(1);

   n  = _n--;
   nm = _n;
   ae = a + nm;

   h      = stepsize;
   h0_5   = stepsize/2.0;
   h0_25  = stepsize/4.0;
   h0_167 = stepsize/6.0;
   h2     = stepsize*2.0;

   for(i=0; i<n; ++i)
      Y[i] = Y0[i],
      a[i] = koef_vek[i];

   X = Xt;
   
   if(k1 != NULL)
      free(k1);
   k1 = (double*)malloc(4*n*sizeof(double));
   if(k1 == NULL)
      return(1);
   k2 = k1+n;
   k3 = k2+n;
   k4 = k3+n;

   xn    = (*X)(t0); /* den ersten Wert der Anregungsfkt ausrechnen */

   return(0);
}

/***************************************************************************
* Hier die eigentliche Integration, die nun nicht mehr sehr aufwendig ist.
*/
void RK()
{
double *ep, *up, *vp, *wp, *ee;

   /* zunÑchst neue Werte der Anregung berechnen */
   xn_05 = (*X)(t+h0_5);
   xn_1  = (*X)(t+=h); /* hier zugleich Zeitpunkt des neuen Abtastwerts er- */
                       /* mitteln */

   /* Schritt 1, (4.88) im Buch */
   F(k1, xn, Y);

   /* Berechnung des Argumentes von Schritt 2 */
   for(ep=k2, ee=k2+n, up=Y, vp=k1; ep<ee; )
      *ep++ = (*up++) + h0_5*(*vp++);

   /* Schritt 2, (4.89) im Buch */
   F(k2, xn_05, k2);

   /* Berechnung des Argumentes von Schritt 3 */
   for(ep=k3, ee=k3+n, up=Y, vp=k1, wp=k2; ep<ee; ++ep)
      *ep = (*up++) + (h0_25*((*vp++) + (*wp++)));

   /* Schritt 3, (4.90) im Buch */
   F(k3, xn_05, k3);

   /* Berechnung des Argumentes von Schritt 4 */
   for(ep=k4, ee=k4+n, up=Y, vp=k2, wp=k3; ep<ee; )
      *ep++ = (*up++) - h*(*vp++) + h2*(*wp++);
   
   /* Schritt 4, (4.91) im Buch */
   F(k4, xn_1, k4);

   /* alten Ableitungsvektor durch neuberechneten ersetzen */
   for(ep=Y, ee=Y+n, up=k1, vp=k3, wp=k4; ep<ee; ++ep)
      *ep = *ep + h0_167*((*up++) + 4.0*(*vp++) + (*wp++));

   /* Jetziger Wert der Anregung ist zugleich der erste des nÑchsten Zyklusses */
   xn = xn_1;
}

/***************************************************************************
* Die Berechnung der Anfangswerte einer öbertragungsfunktion, die durch
* ZÑhler und Nennerpolynom (gegeben durch reellen Koeffizientenvektor) be-
* schrieben ist. ZÑhlergrad (m) muû kleiner als Nennergrad (n)sein, da die
* zu berechnenden Werte sonst nicht existieren. Sonst eine Fehlermeldung. Die
* berechneten n Anfangswerte werden in Y0[] geschrieben. Wenn man sie als An-
* fangswerte der homogenen DGL, die aus den Koeffizienten des Nenners gebil-
* det wird, nimmt, dann erhÑlt man die Stoûantwort der öbertragungsfunktion.
* O.B.d.A. wird angenommen, daû der Koeffizient zur hîchsten Potenz des Nen-
* ners Eins ist, N[n] wird also nicht verwendet, hingegen kann Z[m] beliebig
* sein.
*/
static int _anfangswerte(Z, m, N, n, Y0)
double *Z, *N, *Y0;
int m, n;
{
int i, k, l;
double h0[NMAX];

   for(i=0; i<n; ++i)
      Y0[i] = 0.0;

   if(m >= n)
      return(1);

   for(l=0; l<=m; ++l)/* alle Summanden des ZÑhlers sind gesondert zu behandeln */
   {
      h0[n-l-1] = 1.0; /* n-l-1 ist stets grîûer gleich Null */

      /* i ist Ordnung der Ableitung , zu der gerade der Wert berechnet wird */
      for(i=n-l; i<n; ++i) /* das sind die Restlichen, keine bei l=0 */
         for(h0[i]=0.0, k=1; k<=i+1+l-n; ++k) /* >= 1 Durchlauf */
            h0[i] -= h0[i-k]*N[n-k];

      for(i=n-l-1; i<n; ++i) /* Akkumulierend Åbertragen */
         Y0[i] += Z[l]*h0[i];
   }
   return(0);
}

/***************************************************************************
* Falls die Bedingung m<n nicht erfÅllt ist, dann ist die Stoûantwort keine
* gewîhnliche Funktion mehr, kann aber als Summe aus Distributionen bis zur
* Ordnung m-n und einer gewîhnlichen Funktion dargestellt werden. Hier
* wird dies entsprechend aufgespalten, dann die eigentliche Anfangswerte-
* berechnung aufgerufen. Anschlieûend enthÑlt Y0[0...n-1] die Anfangswerte
* des gewîhnlichen Funktionsanteiles, Y[n] das Gewicht des Stoûes in der
* Impulsantwort, Y[n+1 ... m] die entsprechenden Gewichte der Distributionen
* hîherer Ordnung. Z[] und N[] werden nicht verÑndert.
*/
void anfangswerte(Z, m, N, n, Y0)
double *Z, *N, *Y0;
int m, n;
{
double Zk[NMAX], Nk[NMAX];
int i, j;

   for(i=0; i<=m; ++i) /* ZÑhler wird verÑndert, daher kopieren */
      Zk[i] = Z[i]/N[n];

   for(i=0; i<n; ++i) /* Nenner so normieren, daû 1.Koeff. = 1 */
      Nk[i] = N[i]/N[n]; /* N[n] wird nicht mehr gebraucht */

   for(i=m-n; i>=0; --i, --m) /* abweisend, wenn Aufspaltung unnîtig ist */
   {
      Y0[i+n] = Zk[i+n];/* Divisionsergebnis, Nennerkoeffizient s^n ist stets 1 */
      for(j=0; j<n; ++j) /* subtrahierendes RÅckmultiplizieren */
         Zk[j+i] -= Y0[i+n]*Nk[j];
      /* auf explizites Nullsetzen des aufgeteilten ZÑhlerkoeffizienten kann
         verzichtet werden, weil er ohnehin nicht mehr benîtigt wird. */
   }

   /* nun kann ohne weiteres _anfangswerte aufgerufen werden */
   _anfangswerte(Zk, m, Nk, n, Y0);
}
