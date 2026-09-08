/*
   Include-Datei zum Funktionen-Plotter

   (c) P. Vranken, Juli 1992
*/

/* Definitionen und Konstanten */
# define MIN(a,b) ((a)>(b)? (b):  (a))
# define MAX(a,b) ((a)>(b)? (a):  (b))
# define MALLOC(p, anz) {if((p=malloc((anz)*sizeof(*p)))==NULL)\
                            exit_error("zu wenig Speicher");}
# define FREE(p) free(p)
# define MAXF 1.0E18
# define MINF 1.0E-18
enum { eine_Funktion, zwei_Funktionen, doppel_Funktion };
enum { not, onlyy, onlyylog, xy, xlogy, xylog, xlogylog, xyy };

/* globale Variablen */

/* Typdefinitionen */
typedef enum {true=(1==1), false=(1==0) } boolean;

typedef struct
   {
      double min, max;
      double (*scale)(double x);
   }
   skala;

typedef struct
   {
      double *x, *y;
      int anz;
      skala  x_achs, y_achs;
   }
   uni_func;

typedef struct
   {
      double *x, *y1, *y2;
      int anz;
      skala x_achs, y1_achs, y2_achs;
   }
   combi_func;

typedef struct
   {
      double *x1, *y1, *x2, *y2;
      int anz1, anz2;
      skala x1_achs, x2_achs, y1_achs, y2_achs;
   }
   double_func;

typedef struct
   {
      int kombination; /* welche Art von Funktionsangaben? */
      union
      {
         uni_func    f1;
         combi_func  f12;
         double_func f1f2;
      } typ;
   }
   grafik;

/* Prototypen aus grafik.c */

void init_graph(void);
void exit_graph(void);
void plotte(grafik *funktion);

/* Prototypen aus datei.c */

int main(int argc, char *argv[]);
void exit_error(char *message);

/* Prototypen aus berechne.c */

grafik *bilde_grafik(double *x1, double *y1, int modus1, int anzahl1,
                     double *x2, double *y2, int modus2, int anzahl2);

