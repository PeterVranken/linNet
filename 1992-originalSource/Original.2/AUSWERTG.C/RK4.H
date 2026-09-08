/*
   Definitionsdatei zum Runge-Kutta-Integrationsmodul

   Integrierbare DGLen: linear, beliebige Ordnung, rechte Seite darf eine be-
                        liebige Funktion der Zeit sein, aber nicht als Linear-
                        kombination aus Ableitungen einer Funktion X(t) vor-
                        liegen. Dieser Fall kann aber leicht auf den vorlie-
                        genden zurÅckgefÅhrt werden, wenn man die Ableitungen
                        von Y, die ja als Vektor alle vorliegen, zum Ergebnis
                        hinzunimmt. Mir sind die hieraus erwachsenden prak-
                        tischen, numerischen Konsequenzen nicht bekannt.
   Funktionen:
   init_RK: Einstellung der Parameter der anschlieûenden Integration
      t0: Startzeitpunkt der Simulationszeit
      Y0: Vektor der Anfangswerte, Y0[0] enthÑlt Y(0), Y0[1] enthÑlt Y'(0), etc.
      a: Vektor der konstanten Koeffizienten der linken Seite, a[0] gehîrt zu
         Y, a[1] zu Y', usw..Der Koeffizient der hîchsten Ableitung ist immer
         Eins.
      Y0[] und a[] werden nicht verÑndert.
      n: Ordnung der DGL, also gleich der hîchsten Ableitung, der Anzahl der
         Anfangswerte und der Anzahl der Koeffizienten (n>=1).
      Xt: die Zeitfunktion, die die Anregung, also die rechte Seite der DGL
          beschreibt, z.B. Xt = sin fÅr harmonische Anregung.
      stepsize: Schrittweite der Simulationszeit zwischen je zwei Aufrufen
                von RK(). Sollte stets so gewÑhlt werden, daû sich das er-
                wartete Y in diesem Zeitraum nur wenig Ñndert. Das kann im
                allgemeinen natÅrlich nur a posteriori entschieden werden.
      xp, yp, tp: Zeiger auf die Variablen der aufrufenden Funktion, die dann
                  nach init_RK auf die Ergebniswerte x(t), y(t), t deuten. Vor
                  dem ersten Aufruf liefert **... die Startwerte, danach die
                  aktuellen. öber yp erhÑlt man zudem durch Indizierung alle
                  Ableitungen von Y, da diese in einem Vektor liegen, und Y
                  das erste Element desselben ist.
      Falls die Initalisierung nicht mîglich sein sollte, dann wird FALSE
      zurÅckgegeben, sonst TRUE.
   RK: Ein Schritt der Integration. Anschlieûend ist die Simulationszeit um
       stepsize fortgeschritten, und die drei Zeiger, die in init_RK auf die
       Ergebnisse gerichtet wurden, zeigen auf den neuen aktuellen Wert.
   anfangswerte: Wenn sich die DGL aus der Berechnung der Impulsantwort ei-
                 ner beliebigen gebrochen rationalen öbertragungsfunktion er-
                 gibt, dann braucht man neben der DGL noch die zu diesem
                 Problem passenden Anfangswerte. Sie werden hier ermittelt.
      Z: Zeiger auf den Vektor mit gz+1 konstanten Koeffizienten des ZÑhler-
         polynoms der öbertragungsfunktion. Z[i] gehîrt zu s^i.
      N: entsprechender Vektor zur Nennerbeschreibung.
      gz, gn: Grad von ZÑhler und Nenner.
      Y0: in diesen Vektor werden die berechneten Werte geschrieben. Er muû
          die LÑnge gz haben. Wenn es sich um eine Tiefpaûfunktion handelt
          (gz<gn), dann enthalten Y0[0] ... Y0[n-1] die Anfangswerte der er-
          sten n Ableitungen von Y, sonst fÅgen sich in den weiteren Y0 die
          Gewichte der Distributionen in der Stoûantwort an.
*/

#define NMAX 100

/***************************************************************************
* Prototypen  der externen Funktionen des Modules RK4.c
*/
int init_RK(double t0, double *Y0, double *a, int n, double (*Xt)(double),\
            double stepsize, double **xp, double **yp, double **tp);
void RK(void);
void anfangswerte(double *Z, int gz, double *N, int gn, double *Y0);

