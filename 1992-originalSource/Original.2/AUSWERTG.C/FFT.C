/*
   FFT-Algorithmus fÅr Zweierpotenzen nach Cooley und Tukey

   Implementiert von Peter Vranken am 28/3/90
*/

/* einfache DFT wird mit Åbersetzt, wenn "DFT_wanted" definiert wird. */

/* Die Daten werden in zwei Arrays double X_re[_N], X_im[_N] erwartet.*/
/* i: enthÑlt stets die Potenz der Einheitswurzel, die in der Butter- */
/*    flyoperation mit den Daten multipliziert werden muss.           */
/* mask: das Bit, welches hierin als einziges gesetzt ist, gibt in je-*/
/*    der Stufe das Datenadressbit an, das sich bei den Daten unter-  */
/*    scheidet, die durch die Butterflyoperation zusammengefÅhrt wer- */
/*    den mÅssen.                                                     */
/* Bis auf die Normierung der Ergebnisse ist die Routine allgemein    */
/* verwendbar.                                                        */

/* énderung fÅr das Auswertungsprogramm des Netzwerk-Projektes:       */
/* -) die Normierung wird nicht mehr in diesem Modul vorgenommen      */
/* -) Erstellung eines Hauptprogrammes zur Erzeugung konstanter Daten */


#include <math.h>

#define max_n        10  /* Anzahl der Bits der StÅtzstellenadressen */
#define max_N      1024  /* Anzahl der StÅtzstellen */
#define max_N_halbe 512

#define PI 3.14159265358979323846    /* Pi aus c't 9/88, Seite 72 */

typedef enum {FALSE=(1==0), TRUE=(1==1) } boolean;

#define schreibe_felder FALSE

/* die vorher erzeugte Datei mit initialisierten Feldern einfÅgen */
#if !schreibe_felder
#include "fft.dat"
#else
static int _N, _n, bit_revers[max_N], res_permutation[max_N+1];
static double sinus[max_N_halbe], cosinus[max_N_halbe];
#endif

static int powr_of_two[]={1,2,4,8,16,32,64,128,256,512,1024};

/***************************************************************************
* Prototypen der Funktionen.
*/
void FFT(boolean invers, double X_re[], double X_im[]);
void DFT(void);
int set_size(int n);
int Bit_reverse(unsigned n);
void Ampl_spektr(int flag, double ampl[]);
void Powr_spektr(int flag, double powr[]);

/**************************************************************************/

void FFT(invers, X_re, X_im)/* invers=TRUE: es wird die inverse Transformation berechnet */
boolean invers;
double X_re[], X_im[];
{
int s, i, j, mask, h;                      /* s,j: SchleifenzÑhler */
double k_re, k_im, x_mal_w_re, x_mal_w_im; /* Zwischenspeicher */

   for(s=1; s<=_n; ++s)/* Schleife Åber alle zusammenzufÅhrenden Bitstellen */
   {                  /* der Datenadressen (=Arrayindex) */
      mask = powr_of_two[_n-s];

   /* die folgende Schleife lÑuft Åber alle Zahlen 0 bis _N-1, bei denen     */
   /* das durch mask bezeichnete Bit nicht gesetzt ist, dies sind genau _N/2 */
      for(j=0; j<_N; ++j & mask? j+=mask: j)
      {
         h = j^mask;         /* j,h enthalten nun zu verkoppelnde Indices */
         k_re = X_re[j], k_im = X_im[j]; /* merken (werden Åberschrieben) */

         /* Berechnung der zu verwendenden Potenz i der Einheitswurzel */
         i = (bit_revers[j]<<(_n-s))&(_N-1);

         /* hier folgt relle Realisation von X[h]*exp((+/-)j*2*pi*i/_N) */
         if(invers) /* IFFT: positiver Exponent */
            x_mal_w_re = X_re[h]*cosinus[i]-X_im[h]*  sinus[i],
            x_mal_w_im = X_re[h]*  sinus[i]+X_im[h]*cosinus[i];
         else       /*  FFT: negativer Exponent */
            x_mal_w_re = X_re[h]*cosinus[i]+X_im[h]*  sinus[i],
            x_mal_w_im =-X_re[h]*  sinus[i]+X_im[h]*cosinus[i];

         /* hier folgt nun die komplexe Butterflyoperation */
         X_re[j] += x_mal_w_re,        X_im[j] += x_mal_w_im;
         X_re[h]  = k_re - x_mal_w_re, X_im[h]  = k_im - x_mal_w_im;
      }
   }  /* fertig, aber die Daten sind noch permutiert */

   /* hier wird die Permutierung der Ergebnisdaten in die richtige Reihen- */
   /* folge vorgenommen. */
   for(j=res_permutation[0], h=res_permutation[i=1]; j!=-1;
       j=res_permutation[++i], h=res_permutation[++i])
      k_re=X_re[h], X_re[h]=X_re[j], X_re[j]=k_re,
      k_im=X_im[h], X_im[h]=X_im[j], X_im[j]=k_im;

#if 0
   /* nun muss noch normiert werden */
   /* hier weicht die Routine vom Åblichen ab, um in der vorliegende Demo- */
   /* anwendung die Zahlenwerte der analytischen FT zu ergeben */
   if(invers)
      k_re = 1/dt/(double)_N; /* == df */
   else
      k_re = dt;
   for(j=0; j<_N; ++j)
      X_re[j]*=k_re, X_im[j]*=k_re;
#endif
}  /* Ende der FFT */

#ifdef DFT_wanted
/**************************************************************************/

/* Zum Vergleich der Ergebnisse hier die konventionelle DFT */
/* Eingabedaten werden aus X_re[] gelesen (reell) und Aus-  */
/* gabedaten nach Dx_re[], Dx_im[] geschrieben.             */

extern double Dx_re[], Dx_im[];

void DFT()
{
int i,t;

   for(i=0; i<_N; ++i) /* Schleife Åber die Koeffizienten */
   {
      Dx_im[i]=Dx_re[i]=0;
      for(t=0; t<_N; ++t) /* Schleife Åber die Skalarproduktsummanden */
      {
         /* komplex: Dx[i] += X[t]*exp(j*2*pi*t/_N*i), wobei Im{X[t]}==0 */
         Dx_re[i] += X_re[t]*cos(2*PI*t*i/_N);
         Dx_im[i] += X_re[t]*sin(2*PI*t*i/_N);
      }
   }
}  /* Ende DFT */
#endif

/**************************************************************************/

int set_size(n) /* Bereitstellung der benîtigten konstanten Daten. Es wird */
int n;          /* immer Speicherplatz fÅr den Maximalfall reserviert.     */
{
int i,j;

   if(n == _n) /* gewÅnschte Grîsse ist bereits eingestellt */
      return(0);
   if(n<2 || n>max_n)
      return(-1);
   _n = n, _N = powr_of_two[n];
   for(i=0; i<(_N>>1); ++i)
      cosinus[i] = cos(2.0*PI*(double)i/(double)_N);

   for(i=0; i<(_N>>1); ++i)
      sinus[i] = sin(2.0*PI*(double)i/(double)_N);

   /* Generierung der Permutationsfunktion, die zur ZurechtrÅckung der */
   /* Ergebnisdaten nîtig ist */
   for(i=1, j=0; i<_N-1; ++i)
   {
      if(Bit_reverse(i)<=i)
         continue;
      res_permutation[j++] = i;
      res_permutation[j++] = Bit_reverse(i);
   }
   res_permutation[j] = -1; /* Als Abschlusskennzeichen */

   /* Generierung der Funktion g(i,j): g liefert die Potenz der Einheits- */
   /* wurzel im j-ten Schritt des Algogrithmus, bei Zugriff auf Array-    */
   /* element i. Hier wird aber nur eine Hilfsfunktion h(i) erzeugt, die  */
   /* die vollstÑndige Berechnung von g(i,j) zur Laufzeit ermîglicht. Es  */
   /* ist h die Bit-Revers-Permutation, bei der die BinÑrdarstellung des  */
   /* Argumentes um die Mitte gespiegelt den Wert ergibt. g(i,j) erhÑlt   */
   /* man durch einfaches Shiften wie folgt daraus:g(i,j)=h(i)<<(n-j).An- */
   /* schliessend muss das Ergebnis noch auf die korrekte LÑnge (n Bit) ge- */
   /* kÅrzt werden. (TatsÑchlich heisst h jedoch bit_revers.)              */
   for(i=0; i<_N; ++i)
      bit_revers[i] = Bit_reverse(i);

   return(0);
}

/* Hier folgt ein optionales Hauptprogramm, das ein Fragment eines C-Quell-
   textes erzeugt, welches mit einem Editor in den richtigen Quelltexte ein-
   gefÅgt werden kann. Es enthÑlt die nîtigen Felddeklarartionen mit den
   Initialisierungen, die fÅr ein angebbares n berechnet werden. Die PrÑpro-
   zessorvariable schreibe_felder muss dazu von Null verschieden sein. */

#if schreibe_felder /* fÅr ein bevorzugtes n konstante Daten erzeugen */
#include <stdlib.h>
#include <stdio.h>

int main(int anz, char *koms[])
{
int n, i, npz;
FILE *out;

   if(anz != 2)
      return(-1);
   n = atoi(koms[1]);
   if(set_size(n))
      return(-1);

   /* sonst eine C-Syntax-Ñhnliche Datei mit konstanten Daten anlegen */
   if((out=fopen("fft.dat", "w")) == NULL)
      return(-1);
   fprintf(out, "/*\n   Konstante Daten fÅr die FFT-Routine\n   n = %d\n*/\n\n", n);

   fprintf(out, "static int _N=%d, _n=%d;\n", powr_of_two[n], n);
   fprintf(out, "static double cosinus[] = {\n  %.22e", cosinus[0]);
   for(npz=i=1; i<max_N_halbe; ++i, ++npz)
   {
      if(npz == 3)
         npz = 0,
         fprintf(out, "\n");
      fprintf(out, ", %.22e", i>=_N/2? 0.0: cosinus[i]);
   }
   fprintf(out, "\n};\n\n");

   fprintf(out, "static double sinus[] = {\n  %.22e", sinus[0]);
   for(npz=i=1; i<max_N_halbe; ++i, ++npz)
   {
      if(npz == 3)
         npz = 0,
         fprintf(out, "\n");
      fprintf(out, ", %.22e", i>=_N/2? 0.0: sinus[i]);
   }
   fprintf(out, "\n};\n\n");

   fprintf(out, "static int bit_revers[] = {\n  %d", bit_revers[0]);
   for(npz=i=1; i<max_N; ++i, ++npz)
   {
      if(npz == 16)
         npz = 0,
         fprintf(out, "\n");
      fprintf(out, ", %d", i>=_N? 0: bit_revers[i]);
   }
   fprintf(out, "\n};\n\n");
   
   fprintf(out, "static int res_permutation[] = {\n  %d", res_permutation[0]);
   for(npz=i=1; res_permutation[i]!=-1; ++i, ++npz)
   {
      if(npz == 16)
         npz = 0,
         fprintf(out, "\n");
      fprintf(out, ", %d", res_permutation[i]);
   }
   fprintf(out, ", -1");
   for(npz++, i++; i<=max_N; ++i, ++npz)
   {
      if(npz == 24)
         npz = 0,
         fprintf(out, "\n");
      fprintf(out, ", 0");
   }
   fprintf(out, "\n};");
   
   fclose(out);
   return(0);
}
#endif
/* Ende des optionalen Hauptprogrammes zur Erzeugung konstanter Datenfelder */

static int Bit_reverse(n)/* Berechnung der Integerzahl mit umgedrehten Bits */
unsigned n;
{
unsigned erg, lmask, hmask;

   for(erg=0, lmask=1, hmask=_N/2; hmask>=lmask; lmask<<=1, hmask>>=1)
      erg |= ((n&lmask)? hmask: 0)|((n&hmask)? lmask: 0);
   return(erg);
}

#if 0
/**************************************************************************/

/* die nÑchste Routine berechnet das Amplitudendichtebetragsspektrum */

void Ampl_spektr(flag, ampl) /* flag: 1, wenn reell, sonst 0 */
int flag;
double ampl[];
{
int t;

   for(t=_N/(flag? 2: 1)-1; t>=0; t--)
      ampl[t] = pow(spek_re[t]*spek_re[t]+spek_im[t]*spek_im[t], 0.5);
}

/* die nÑchste Routine berechnet das Leistungsdichtespektrum */

void Powr_spektr(flag, powr) /* flag: 1, wenn reell, sonst 0 */
int flag;
double powr[];
{
int t;

   for(t=_N/(flag? 2: 1)-1; t>=0; t--)
      powr[t] = spek_re[t]*spek_re[t]+spek_im[t]*spek_im[t];
}
#endif
