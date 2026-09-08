/*
   Eine einfache Diagrammroutine zur skalierten Darstellung von zwei unab-
   hÑngigen Funktionen.
   Sie geht von festen Bildschirmmaûen und starren Achseinteilungen aus,
   ist daher recht simpel und nicht allgemein zu verwenden, hat aber ander-
   erseits keine Probleme mit speziellen Situationen, wie die komfortablere
   Routine von Georg Dittie.
   Da sie auf dem Borland-Graphics-Interface aufsetzt, ist sie nur mit
   Turbo-C 2.0 zu Åbersetzen.

   (c) P.Vranken, Juli 1992
*/

# include <stdio.h>
# include <math.h>
# include <graphics.h>

# include "plotter.h"

/* Definitionen und Konstanten */
# define USERBITS 0xFFF0
enum { AUFRUNDEN, ABRUNDEN };
enum { oben=0, unten, links, rechts }; /* Angabe, welche Skala zu zeichnen */

/* globale Variablen */

/* lokale Prototypen */
static double rundeauf125(double f);
static double rundeab125(double f);
static double runde125(double f, int wie);
static double next(double zahl, double schritt);
static void f_out_at(double f, int x, int y);
static void achse(double min, double max, double (*scale)(double), int wo);
static void graph(double *x, double xmin, double xmax,
                  double *y, double ymin, double ymax, int anzahl);

/***************************************************************************
* Initialisierung und Terminierung der Routinen des Graphikpaketes.
*/

void init_graph()
{
   int driver=DETECT, mode;

   initgraph(&driver, &mode, "\TC"); /* Routinen einschalten */
   cleardevice();                    /* Bildschirm lîschen */
   setviewport(0, 0, 639, 399, true); /* Clipping einschalten */
   settextjustify(CENTER_TEXT, CENTER_TEXT);
   setwritemode(COPY_PUT);
}

void exit_graph()
{
   cleardevice();
   closegraph();
}

/***************************************************************************
* Ausgabe einer Flieûkommazahl in einer Form, daû die LÑnge des Strings
* stets identisch ist (7 Zeichen). Dazu Verwendung symbolischer Exponenten.
*/
static void f_out_at(double f, int x, int y)
{
char vz, string[10];

   if(f>MAXF || x<28 || x>611 || y<6 || y>391)
      exit_error("BereichsÅberschreitung in f_out_at()");

   vz = f>=0.0? ' ': (f=-f, '-');

   if(f>MINF) /* dann die Zahl 4stellig mit symbolischem Exponenten angeben */
   {
   int symexp=7;
   static char symbols[]={'<', 'a', 'f', 'p', 'n', 'Ê', 'm', ' ',
                          'K', 'M', 'G', 'T', 'E', 'P', '>'};

      while(f >= 1000.0)
         f /= 1000.0,
         ++symexp;
      while(f < 1.0)
         f *= 1000.0,
         --symexp;
      /* f liegt nun in [1.0; 1000.0) */

      /* nun in jedem Fall die Mantisse mit 5 Zeichen angeben */
      if(f >= 99.95)
          sprintf(string, "%c%.1f%c", vz, f, symbols[symexp]);
      else if(f >= 9.995)
          sprintf(string, "%c%.2f%c", vz, f, symbols[symexp]);
      else
          sprintf(string, "%c%.3f%c", vz, f, symbols[symexp]);
   }
   else /* auf Null abrunden */
      sprintf(string, "0.0");
   moveto(x,y);
   outtext(string);
}

/***************************************************************************
* Eine Zahl auf die nÑchste "glatte" Zahl runden. Das sind Zahlen, deren
* Mantissen nur eine 1, 2 oder 5 ist.
*/
static double rundeauf125(double f)
{
   if(f < 0.0)
      return(-runde125(-f, ABRUNDEN));
   else if(f > 0.0)
      return(runde125(f, AUFRUNDEN));
   else
      return(0.0);
}

static double rundeab125(double f)
{
   if(f < 0.0)
      return(-runde125(-f, AUFRUNDEN));
   else if(f > 0.0)
      return(runde125(f, ABRUNDEN));
   else
      return(0.0);
}

static double runde125(double f, int wie)/* Hilfsfunktion zu den zwei vorigen */
{
double diskr, integ;

   /* f ist sicher >= MINF */

   diskr = log10(f), diskr -= (integ=floor(diskr));/* Nachkommateil des log */
   if(wie == AUFRUNDEN)
      if(diskr > 0.6989)     /* Mantisse ist grîûer als 4.999 */
         return(pow10(integ+1));
      else if(diskr > 0.301) /* Mantisse ist grîûer als 1.9999 */
         return(5.0*pow10(integ));
      else
         return(2.0*pow10(integ));
   else /* abrunden */
      if(diskr > 0.699)       /* Mantisse ist grîûer als 5.0003 */
         return(5.0*pow10(integ));
      else if(diskr > 0.3011) /* Mantisse ist grîûer als 2.0003 */
         return(2.0*pow10(integ));
      else
         return(pow10(integ));
}

/***************************************************************************
* In einem angegebenen Raster aufrunden. Dies vorzeichenrichtig.
*/
static double next(double zahl, double schritt)
{
   return(ceil(zahl/schritt)*schritt); /* es ist schritt > 0 */
}

/***************************************************************************
* Hier wird eine der vier mîglichen Achsen gezeichnet. Um fÅr alle FÑlle
* gerÅstet zu sein, ohne eine FÅlle von Fallunterscheidungen vornehmen zu
* mÅssen, werden die Achsen grundsÑtzlich am Bildrand dargestellt. max und
* min mÅssen schon Mantissen von 1, 2 oder 5 haben.
*/

static void achse(double min, double max, double (*scale)(double), int wo)
{
double spanne,
       nextline, dnextline,
       nextmark, dnextmark,
       x, dx,
       y, dy;

int pix_pos;

	spanne = max-min;
   dnextmark = rundeauf125(spanne/8.0);  /* max. 8 Markierungen */
   dnextline = 0.2*dnextmark;            /* jede Markierung 5* unterteilen */
   nextmark  = next(min, dnextmark);     /* erste Markierung im Bereich */
   nextline  = next(min, dnextline);
   
   switch(wo)
   {
      case oben:
         /* obere Achse (X-Achse 2) zeichnen */
         dx = spanne/(604-28);                 /* x-Schritt pro Pixel */

         /* eigentliche Achse ziehen */
         setlinestyle(USERBIT_LINE, USERBITS, NORM_WIDTH);
         line(14, 21, 625, 21);
         setlinestyle(SOLID_LINE, 0, NORM_WIDTH);
         
         /* und die Querstriche mit Markierung */
         for(pix_pos=28, x=min; pix_pos<=604; ++pix_pos, x+=dx)
         {
            if(x >= nextmark) /* langer Querstrich mit Markierung fÑllig */
               line(pix_pos, 21+4, pix_pos, 21-4),
               f_out_at(scale(nextmark), pix_pos, 6),
               nextline += dnextline,
               nextmark += dnextmark;
            else if(x >= nextline) /* ein kurzer Querstrich ist fÑllig */
               line(pix_pos, 21+2, pix_pos, 21-2),
               nextline += dnextline;
         }
         break;
      case links:
         /* linke Achse (Y-Achse 1) zeichnen */
         dy = spanne/(375-23);                 /* y-Schritt pro Pixel */

         /* eigentliche Achse ziehen */
         line(6, 16, 6, 383);
         
         /* und die Querstriche mit Markierung */
         for(pix_pos=375, y=min; pix_pos>=23; --pix_pos, y+=dy)
         {
            if(y >= nextmark) /* langer Querstrich mit Markierung fÑllig */
            {
               line(6+4, pix_pos, 6-4, pix_pos);
               if(pix_pos>=41 && pix_pos<369)
                  f_out_at(scale(nextmark), 39, pix_pos-2);
               nextline += dnextline;
               nextmark += dnextmark;
            }
            else if(y >= nextline) /* ein kurzer Querstrich ist fÑllig */
               line(6+2, pix_pos, 6-2, pix_pos),
               nextline += dnextline;
         }
         break;
      case rechts:
         /* rechte Achse (Y-Achse 2) zeichnen */
         dy = spanne/(375-23);                 /* y-Schritt pro Pixel */

         /* eigentliche Achse ziehen */
         setlinestyle(USERBIT_LINE, USERBITS, NORM_WIDTH);
         line(633, 16, 633, 383);
         setlinestyle(SOLID_LINE, 0, NORM_WIDTH);
         
         /* und die Querstriche mit Markierung */
         for(pix_pos=375, y=min; pix_pos>=23; --pix_pos, y+=dy)
         {
            if(y >= nextmark) /* langer Querstrich mit Markierung fÑllig */
            {
               line(633+4, pix_pos, 633-4, pix_pos);
               if(pix_pos>=41 && pix_pos<369)
                  f_out_at(scale(nextmark), 599, pix_pos-2);
               nextline += dnextline;
               nextmark += dnextmark;
            }
            else if(y >= nextline) /* ein kurzer Querstrich ist fÑllig */
               line(633+2, pix_pos, 633-2, pix_pos),
               nextline += dnextline;
         }
         break;
       default:
         /* untere Achse (X-Achse 1) zeichnen */
         dx = spanne/(604-28);                 /* x-Schritt pro Pixel */

         /* eigentliche Achse ziehen */
         line(14, 379, 625, 379);
         /* und die Querstriche mit Markierung */
         for(pix_pos=28, x=min; pix_pos<=604; ++pix_pos, x+=dx)
         {
            if(x >= nextmark) /* langer Querstrich mit Markierung fÑllig */
               line(pix_pos, 379+4, pix_pos, 379-4),
               f_out_at(scale(nextmark), pix_pos, 391),
               nextline += dnextline,
               nextmark += dnextmark;
            else if(x >= nextline) /* ein kurzer Querstrich ist fÑllig */
               line(pix_pos, 379+2, pix_pos, 379-2),
               nextline += dnextline;
         }
   }
}

/***************************************************************************
* Hier werden die Funktionen dargestellt, die bereits vorverarbeitet sind,
* d.h. öberlÑufe sind erkannt und ausgemerzt, die absoluten Maxima ermittelt,
*/
void plotte(grafik *funktion)
{
   switch(funktion->kombination) /* 1, 2 oder kombinierte Funktionen? */
   {
   double xmin, xmax, ymin, ymax, x2min, x2max, y2min, y2max;
   
      case eine_Funktion:
         achse(xmin=funktion->typ.f1.x_achs.min,  /* X-Achse darstellen */
               xmax=funktion->typ.f1.x_achs.max,
                    funktion->typ.f1.x_achs.scale, unten);
         achse(ymin=funktion->typ.f1.y_achs.min,  /* Y-Achse darstellen */
               ymax=funktion->typ.f1.y_achs.max,
                    funktion->typ.f1.y_achs.scale, links);
         graph(funktion->typ.f1.x, xmin, xmax,    /* eigentliche Funktion zeichnen */
               funktion->typ.f1.y, ymin, ymax,
               funktion->typ.f1.anz);
               break;
      case zwei_Funktionen:
         achse(xmin=funktion->typ.f1f2.x1_achs.min,  /* X1-Achse darstellen */
               xmax=funktion->typ.f1f2.x1_achs.max,
                    funktion->typ.f1f2.x1_achs.scale, unten);
         achse(ymin=funktion->typ.f1f2.y1_achs.min, /* Y1-Achse darstellen */
               ymax=funktion->typ.f1f2.y1_achs.max,
                    funktion->typ.f1f2.y1_achs.scale, links);
         achse(x2min=funktion->typ.f1f2.x2_achs.min,  /* X2-Achse darstellen */
               x2max=funktion->typ.f1f2.x2_achs.max,
                     funktion->typ.f1f2.x2_achs.scale, oben);
         achse(y2min=funktion->typ.f1f2.y2_achs.min,/* Y2-Achse darstellen */
               y2max=funktion->typ.f1f2.y2_achs.max,
                     funktion->typ.f1f2.y2_achs.scale, rechts);
         graph(funktion->typ.f1f2.x1, xmin, xmax,   /* Funktion 1 zeichnen */
               funktion->typ.f1f2.y1, ymin, ymax,
               funktion->typ.f1f2.anz1);
         /* andere Linienart zur Unterscheidung einstellen */
         setlinestyle(USERBIT_LINE, USERBITS, NORM_WIDTH);
         graph(funktion->typ.f1f2.x2, x2min, x2max, /* Funktion 2 zeichnen */
               funktion->typ.f1f2.y2, y2min, y2max,
               funktion->typ.f1f2.anz2);
         setlinestyle(SOLID_LINE, 0, NORM_WIDTH);
         break;      
      case doppel_Funktion:
         achse(xmin=funktion->typ.f12.x_achs.min,  /* X-Achse darstellen */
               xmax=funktion->typ.f12.x_achs.max,
                    funktion->typ.f12.x_achs.scale, unten);
         achse(ymin=funktion->typ.f12.y1_achs.min, /* Y1-Achse darstellen */
               ymax=funktion->typ.f12.y1_achs.max,
                    funktion->typ.f12.y1_achs.scale, links);
         achse(y2min=funktion->typ.f12.y2_achs.min,/* Y2-Achse darstellen */
               y2max=funktion->typ.f12.y2_achs.max,
                     funktion->typ.f12.y2_achs.scale, rechts);
         graph(funktion->typ.f12.x,  xmin, xmax,   /* Funktion 1 zeichnen */
               funktion->typ.f12.y1, ymin, ymax,
               funktion->typ.f12.anz);
         /* andere Linienart zur Unterscheidung einstellen */
         setlinestyle(USERBIT_LINE, USERBITS, NORM_WIDTH);
         graph(funktion->typ.f12.x,   xmin,  xmax, /* Funktion 2 zeichnen */
               funktion->typ.f12.y2, y2min, y2max,
               funktion->typ.f12.anz);
         setlinestyle(SOLID_LINE, 0, NORM_WIDTH);
   }
}

/***************************************************************************
* Eigentliche Funktionszeichen-Routine. Alle relevanten Grîûen werden Åber-
* geben, keine zusÑtzlichen Operationen mehr nîtig. Die zunÑchst definierten
* Konstanten sind strengstens auf den verwendeten Bildschirmausschnitt abge-
* stimmt.
*/
enum { XMIN=28,           /* erste verwendbare Pixelspalte */
       XMAX=604,          /* letzte verwendbare Pixelspalte */
       YMIN=23,           /* erste verwendbare Pixelzeile */
       YMAX=375,          /* letzte verwendbare Pixelzeile */
       XINT=XMAX-XMIN,    /* Breite des horizontalen Intervalls */
       YINT=YMAX-YMIN};   /* Breite des vertikalen Intervalls */
       
# define iX(x) ((int)(XINT*(((x)-xmin)/xspanne)))
# define iY(y) (YINT-(int)(YINT*((y)-ymin)/yspanne))

static void graph(double *x, double xmin, double xmax,
                  double *y, double ymin, double ymax, int anzahl)
{
double xspanne, yspanne;
boolean vorgaenger;
int i;

   if((xspanne=xmax-xmin)<=MINF || (yspanne=ymax-ymin)<=MINF)
      exit_error("Wertebereich einer Funktion ist zu klein!");

   /* zur Sicherheit zunÑchst das Clipping-Fenster einstellen. Dabei wird
      auch gleich die Verschiebung vorgenommen, da die Koordinaten der
      lineto und moveto Funktionen relativ zum Fenster sind */
   setviewport(XMIN, YMIN, XMAX, YMAX, true);

   for(vorgaenger=false, i=0; i<anzahl; ++i) /* Schleife Åber alle Pixel */
   {
      if(*x>MAXF || *y>MAXF) /* Unterbrechung der durchgehenden Linie */
         vorgaenger = false;
      else
         if(vorgaenger)/* dann Verbindung zum gÅltigen Vorgaenger herstellen */
            lineto(iX(*x), iY(*y));
         else          /* neue Linie anfangen */
            moveto(iX(*x), iY(*y)),
            vorgaenger = true;
      ++ x, ++ y;      /* zum nÑchsten Punkt der Liste */
   }

   /* Fenster wieder auf volle Grîûe */
   setviewport(0, 0, 639, 399, true);
}
#undef iX
#undef iY
