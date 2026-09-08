/*
   Hier werden die eingelesenen Werte einer Aufbereitung unterzogen. Sie
   werden auf öberlÑufe geprÅft, entsprechend der gewÅnschten Skala verzerrt,
   und ihre Extrema bestimmt. Sodann werden sie in eine *grafik-Struktur ge-
   packt.

   (c) P. Vranken, Juli 1992
*/

# include <stdlib.h>
# include <stdio.h>
# include <math.h>
# include <float.h>
# include <tos.h>

# include "plotter.h"

/* Definitionen und Konstanten */

/* lokale Prototypen */
static double linear_skala(double x);
static double log_skala(double x);
static void transform(double *x, int anzahl, double (*verzerrung)(double),
                      double *max, double *min);

/***************************************************************************
* Die zwei verwendeten Verzerrungsfunktionen der Werte, auch invers, da
* sie in dieser Form zur Beschriftung der Achsskalierung herangezogen werden.
*/
static double linear_verzerrung(double x)
{
   return(x);
}
# define linear_skala linear_verzerrung

# define log_verzerrung log10
static double log_skala(double x)
{
   return(pow(10.0, x));
}

/***************************************************************************
* Einen Vektor transformieren. D.h. Verzerrung anwenden, Extremwerte be-
* stimmen, öberlÑufe ermitteln, markieren und bewerten. Bei einem Anteil von
* mehr als 10% öberlÑufe wird der Programmlauf abgebrochen. Undefinierte Wer-
* te der Verzerrungsfunktion mÅssen durch Åbergroûe Werte dargestellt werden,
* so daû sie auf diesem Wege erkannt werden.
*/
static void transform(double *x, int anzahl, double (*verzerrung)(double),
                      double *max, double *min)
{
int i, ueber;
double a;

   *max = -DBL_MAX, *min = DBL_MAX;

   for(ueber=i=0; i<anzahl; ++i)
   {
      if((a=verzerrung(*x))>MAXF  || a<-MAXF)
         ++ ueber,
         *x = DBL_MAX;
      else
      {
         if(a> *max)
            *max = a;
         if(a< *min)
            *min = a;
         *x = a;
      }
      ++ x;
   }
   if(ueber && anzahl/ueber<10) /* zu viele Fehler */
      exit_error("Es liegen zuviele Werte auûerhalb des darstellbaren Bereichs!");
   /* konstante Funktion erfordert Sonderbehandlung */
   if(*max - *min < 1000.0*DBL_EPSILON)
      if(fabs(*max) < 1000.0*DBL_EPSILON)
         *min = -(*max = 0.1);
      else
         *max += fabs(0.1* *max),
         *min -= *max - *min;
}

/***************************************************************************
* Hier wird die Struktur *grafik gebildet.
*/
# define INVERS(f) ((f)==log_verzerrung? log_skala: linear_skala)

grafik *bilde_grafik(double *x1, double *y1, int modus1, int anzahl1,
                     double *x2, double *y2, int modus2, int anzahl2)
{
grafik *funktion;

double maxx1, minx1, maxy1, miny1,
       (*verz_x1)(double), (*verz_y1)(double),
       (*verz_x2)(double), (*verz_y2)(double);

   MALLOC(funktion, 1);
   /* die erste Funktion muû immer vorhanden sein! (x1, y1) */

   /* x1-Achse logarithmisch? */
   verz_x1 = (modus1==xlogy || modus1==xlogylog)? log_verzerrung:
                                                  linear_verzerrung;
   transform(x1, anzahl1, verz_x1, &maxx1, &minx1);

   /* y1-Achse logarithmisch? */
   verz_y1 = (modus1==xylog || modus1==xlogylog)? log_verzerrung:
                                                  linear_verzerrung;
   transform(y1, anzahl1, verz_y1, &maxy1, &miny1);

   /* eine zweite (abhÑngige) Funktion? */
   if(modus2==not) /* nein, es bleibt bei der ersten */
   {
      funktion->kombination = eine_Funktion;
      funktion->typ.f1.x = x1;
      funktion->typ.f1.y = y1;
      funktion->typ.f1.anz = anzahl1;
      funktion->typ.f1.x_achs.min = minx1;
      funktion->typ.f1.x_achs.max = maxx1;
      funktion->typ.f1.x_achs.scale = INVERS(verz_x1);
      funktion->typ.f1.y_achs.min = miny1;
      funktion->typ.f1.y_achs.max = maxy1;
      funktion->typ.f1.y_achs.scale = INVERS(verz_y1);
   }
   else if(modus2==onlyy || modus2==onlyylog) /* zweite, abhÑngige Funktion */
   {
      if(anzahl1 != anzahl2)
         exit_error("Doppelfunktion mit unterschiedlich vielen Y-Werten!");
      funktion->kombination = doppel_Funktion;
      funktion->typ.f12.x  = x1;
      funktion->typ.f12.y1 = y1;
      funktion->typ.f12.y2 = y2;
      funktion->typ.f12.anz = anzahl1;
      funktion->typ.f12.x_achs.min = minx1;
      funktion->typ.f12.x_achs.max = maxx1;
      funktion->typ.f12.x_achs.scale = INVERS(verz_x1);
      funktion->typ.f12.y1_achs.min = miny1;
      funktion->typ.f12.y1_achs.max = maxy1;
      funktion->typ.f12.y1_achs.scale = INVERS(verz_y1);

      verz_y2 = (modus2==onlyylog)? log_verzerrung: linear_verzerrung;
      transform(y2, anzahl2, verz_y2, &funktion->typ.f12.y2_achs.max,
                                      &funktion->typ.f12.y2_achs.min);
      funktion->typ.f12.y2_achs.scale = INVERS(verz_y2);
   }
   else /* zweite, unabhÑngige Funktion */
   {
      funktion->kombination = zwei_Funktionen;
      funktion->typ.f1f2.x1 = x1;
      funktion->typ.f1f2.y1 = y1;
      funktion->typ.f1f2.x2 = x2;
      funktion->typ.f1f2.y2 = y2;
      funktion->typ.f1f2.anz1 = anzahl1;
      funktion->typ.f1f2.anz2 = anzahl2;
      funktion->typ.f1f2.x1_achs.min = minx1;
      funktion->typ.f1f2.x1_achs.max = maxx1;
      funktion->typ.f1f2.x1_achs.scale = INVERS(verz_x1);
      funktion->typ.f1f2.y1_achs.min = miny1;
      funktion->typ.f1f2.y1_achs.max = maxy1;
      funktion->typ.f1f2.y1_achs.scale = INVERS(verz_y1);

      verz_x2 = (modus2==xlogy || modus2==xlogylog)? log_verzerrung:
                                                     linear_verzerrung;
      transform(x2, anzahl2, verz_x2, &funktion->typ.f1f2.x2_achs.max,
                                      &funktion->typ.f1f2.x2_achs.min);
      funktion->typ.f1f2.x2_achs.scale = INVERS(verz_x2);

      verz_y2 = (modus2==xylog || modus2==xlogylog)? log_verzerrung:
                                                     linear_verzerrung;
      transform(y2, anzahl2, verz_y2, &funktion->typ.f1f2.y2_achs.max,
                                      &funktion->typ.f1f2.y2_achs.min);
      funktion->typ.f1f2.y2_achs.scale = INVERS(verz_y2);
   }
   return(funktion);
}
# undef INVERS
