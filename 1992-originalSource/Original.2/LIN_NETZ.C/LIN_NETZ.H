/***************************************************************************
*                                                                          *
*  Headerdatei des Programmes zur Erzeugung eines linearen Gleichungssys-  *
*  tems aus einer Knotenliste                                              *
*                                                                          *
*  Sie muû von allen Moduln eingelesen werden. Hier werden allgemeine Typ- *
*  definitionen vorgenommen, Prototypen angegeben und globale Daten be-    *
*  kannt gemacht.                                                          *
*                                                                          *
***************************************************************************/

/*
   (c) Peter Vranken 11/91
*/

/***************************************************************************
* es kann zwischen der öbersetzung fÅr ATARI ST oder Msdos-Rechner gewÑhlt
* werden.
*/
#define ATARI 2  /* Versionen: 2: Felder >32K fÅr TC 2.0
                               1: Felder <32K fÅr TC 1.0 und 1.1
                               0: Ms-Dos */

#undef TEST     /* Abfrage auf interne Fehler aktivieren/deaktivieren */

#if ATARI /* Zeichenfunktion fÅr ein Zeichen ohne Echo heiût unterschiedlich */

#define Conin  (char)Cnecin

long Crawio(int w); /* Prototypen nur fÅr nÑchstes Makro */
long Cnecin(void);
#define TASTE {while(Crawio(0xff));if('q'==(char)Cnecin())exit(-1);} /* auf Taste warten */

#else /* Msdos-Version */

#define Conin  (char)getch

int getch(void); /* Prototyp */
#define TASTE {if('q'==(char)getch())exit(-1);} /* auf Taste warten */

#endif /* Atari oder Msdos */

/***************************************************************************
* Makros
*/
#define MAX(a,b) ((a)>(b)? (a):  (b))
#define MIN(a,b) ((a)<(b)? (a):  (b))
#define ABS(a)   ((a)> 0 ? (a): (-a))

/***************************************************************************
* allgemeine Konstanten
*/
#define MAX_KNOTEN 50    /* max. Anzahl Knoten, viel mehr als Variablen ist unsinnig */
#define MAX_ZWEIGE 50    /* max. Anzahl Zweige */
#define MAX_VAR 32       /* maximale Anzahl der Unbekannten VORSICHT BEI */
                         /* éNDERUNG: TYP VVEK MUû SICH DANN AUCH éNDERN */

/* Definitionen von Tokens fÅr den Parser */
          /* -1 ist EOF */
#define IMP  -4 /* 'L' */
#define CAP  -5 /* 'C' */
#define RES  -6 /* 'R' */
#define LW   -7 /* 'Y' */
#define QI   -11 /* Stromquelle */
#define OP   -12 /* OperationsverstÑrker */
#define OPQU -13 /* Pseudoelement: Spannungsquelle im OP-Ausgang gegen Masse */
#define OPZI -14 /* Pseudoelement: Innenwiderstand des OP zw. den EingÑngen */
#define UIN  -15 /* Eingangsspannungsangabe */
#define UOUT -16 /* Ausgangsspannungsangabe */
#define US   -17 /* Pseudoelement: Steuerspannungseinfluû auf eine Quelle */

/***************************************************************************
* Typdefinitionen
*/
typedef enum {FALSE = (1==0), TRUE = (1==1)} boolean;
typedef unsigned char byte;
#define MAXBYTE 255        /* Darstellungsbereiche, werden fÅr öberlaufer- */
#define MAXLONG 2147483648l /* kennung gebraucht */
typedef unsigned long vvek;/* darin wird Info Åber im Term enthaltene Varia- */
typedef long koeffizient;  /* blen und der zugehîrige Koeffizient gespeichert */

typedef struct _zweig_info  /* Beschreibung einer Verbindung zweier Knoten */
{
   int art;               /* was fÅr ein Bauelement ? */
   int btnr;              /* Zuordnung einer Variablen fÅr interne Rechnung */
   vvek vv;               /* und ihre interne Darstellung */
   int usp, usm;          /* Steuerspannungspole falls Quelle */
   int ak;                /* Nummer des Anfangsknotens des Zweiges */
   int ek;                /* Nummer des Endknotens des Zweiges */
   struct _zweig_info *refp; /* Zeiger, auf Bezugsbauteil bzgl. arithmeti- */
   int  reffak;           /* scher Relation, hier Faktor der Relation */
   char name[9];          /* symbolischer Name des Bauteils */
} zweig_info;

/***************************************************************************
* die beiden folgenden Blîcke sind solche, die von der Speicherverwaltung
* vergeben und zurÅckgenommen werden kînnen. Ihr Name, unter dem sie alloziert
* werden kînnen, ist durch die Typdefinition mem_typen bestimmt.
* SEHR WICHTIG:
* Alle diese Blîcke mÅssen einen Nachfolgerzeiger als ERSTES Element haben.
* Sollte aus irgendwelchen GrÅnden der next-Zeiger nicht ohne Offset vorne
* in dem Verbund liegen, dann funktioniert die Speicherverwaltung nicht!
*/
typedef enum {SUMMAND, ZWEIG, ERGSUMMAND, ANZ_TYPEN} mem_typ;

typedef struct _zweig /* Element zur Abbildung des Netzes auf eine dynamische */
{                     /* Datenstruktur der entsprechneden 'Topologie' */
   struct _zweig *next;/* Zeiger auf Listennachfolger */
   zweig_info *was;    /* Zeiger auf detaillierte Zweigbeschreibung */
} zweig;

typedef struct _summand
{
   struct _summand *next;
   koeffizient koef;
   vvek var_vek;
} summand;

typedef struct _ergsummand
{
   struct _ergsummand *next;
   long koef;
   byte potenz[MAX_VAR];
   byte s_potenz;
} ergsummand;

/**************************************************************************/

typedef struct    /* Aufbau des Feldelements des Knotenarrays */
{
   int pot_nr;    /* Nummer des unbekannten Potentiales, das am Knoten an- */
                  /* liegt. Wegen OPs ist dies nicht immer der Knoten selbst */
   boolean ist_qop; /* Ausgangsknoten von OPs werden anders behandelt */
   zweig *zw1;    /* Zeiger auf ersten Zweig, der vom Knoten ausgeht */
} knoten;

/***************************************************************************
* Bekanntmachung globaler Daten
*/
extern summand *A[MAX_VAR][MAX_VAR];

/* Null-Zeiger (hier als WÑchter am Listenende) werden nicht nur in die
   mem_info-Struktur gelegt, weil sie zu oft gebraucht werden. */
extern summand *S_null; /* Zeiger auf Listenende (=Nullelement) */
extern ergsummand *E_null; /* Zeiger auf Listenende Ergebnissummanden */
extern zweig   *Z_null; /* Zeiger auf Listenende Zweige */
extern summand *S_eins; /* Zeiger auf Einselement */

extern zweig_info Zwliste[];  /* Feld mit Zweigbeschreibungen */
extern zweig_info *Zweigptr[MAX_VAR];
extern char Knotname [MAX_KNOTEN][9]; /* symbolische Namen der Knoten */
extern knoten Kliste[];       /* Feld mit KnoteneintrÑgen */
extern int Anz_knoten;        /* Anzahl der EintrÑge in Kliste */
extern int Anz_zweige;        /* Anzahl der EintrÑge in Zwliste */
extern int Anz_var;           /* Anzahl der Unbekannten */

extern int Verbose;           /* Detaillierte Ausgabe 0-2 */
extern int Masse;             /* Nummer des Masseknotens */
extern int U1p, U2p, U2m;     /* Potentialknoten von Ein-/Ausgangsspannung */

extern char *Errors[]; /* Fehlermeldungsstrings */
extern FILE *Fpout, *Binout;  /* Ausgabedateien */

/***************************************************************************
* Prototypen...
*/

/* ...des Moduls LIN_NETZ.C */
int main(int anz, char *koms[]);
boolean einlesen(void);
boolean einhaengen(int bau_el, int usp, int usm, int von, int nach\
, int qop, int reffaktor, zweig_info *refbauteil, char *name);
int fuell_zweiginfo(int bau_el, int usp, int usm, int von, int nach\
, int reffaktor, zweig_info *refbauteil, char *name);
void anzeige_kliste(void);
int scanner(boolean lesen, int backtok);
int next_tok(void);
void kommentar(void);
void plot_info(void);
void bauteil_def(void);
boolean space(void);
boolean lies_name(char *name);
boolean integer(int *faktor);
boolean zahl(double *a);
boolean wertangabe(int bau_el, double *value, int *faktor, zweig_info **refptr);
boolean zuweisung(int bau_el, double *value, int *faktor, zweig_info **refptr);
boolean bezugsbasis(int *btnr, int art);
int knotennr(void);
void sync(void);
void titel(char *name);
void abbruch(int err_nr);

/* ... des Moduls MACH_LGS.C */
void uebertragungsfkt(void);
void bilde_det(int det_nr);
void neue_pot_nr(void);
void zeige_lgs(void);
boolean topologietest(void);
boolean voll_baum(int test_knoten, int testart);

/* ... des Moduls MEMORY.C */
void init_balloc(void);
void *balloc(mem_typ typ);
void bfree(void *b_nr, mem_typ typ);
void lfree(void *b_nr, mem_typ typ);
long mem_bedarf(mem_typ typ);
long mem_belegt(mem_typ typ);
long cnt_mem(boolean cal, mem_typ typ);

/* ... des Moduls SOLVER.C */
summand *determinante(int n, char *name);
void elementaroperation(int el_s, int z, int s, summand *t);
summand *differenz(summand *a, summand *b);
void el_schritt_aus(int n, int el_s, int piv);

/* ... des Moduls AUSGABE.C */
void liste_aus(summand *liste, int tab);
void ergebnis(summand *d, summand *din, summand *dout);
void l_nach_w(summand *z, summand *n);
summand *kopiere(summand *original);
ergsummand *erg_format(summand *term);
boolean ist_gleichartig(ergsummand *sum1, ergsummand *sum2);
void eintragen(ergsummand *ziel, ergsummand *quelle);
void kuerze_erg(ergsummand *zaehler, ergsummand *nenner);
void ergliste_aus(ergsummand *liste, int tab);

/* ... des Moduls BINAUS.C */
void liste_baus(ergsummand *liste);
void namen_baus(void);
boolean erg_baus(ergsummand *zg, ergsummand *ng, ergsummand *zri, ergsummand *nri);

