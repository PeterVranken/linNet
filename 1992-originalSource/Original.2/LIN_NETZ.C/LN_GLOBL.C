/***************************************************************************
*                                                                          *
*  Teil I des Programmes zur Erstellung eines linearen Gleichungssystems   *
*  aus einer Knotenliste                                                   *
*                                                                          *
*  Hier stehen alle globalen Daten, die aus mehreren Moduln sichtbar sein  *
*  mÅssen.                                                                 *
*                                                                          *
***************************************************************************/

/*
   (c) Peter Vranken 11/91
*/

#include <stdio.h>
#include "lin_netz.h"

/***************************************************************************
* In diesen Feldern werden die dynamisch gespeicherte Netzstrukturdaten
* abgelegt.
*/
zweig_info Zwliste[MAX_ZWEIGE];/* Liste der Zweigbeschreibungen */
zweig_info *Zweigptr[MAX_VAR];
      /* Verweise Åber Bauteilnummer auf zugehîrige Bauteilbeschreibungen */

char Knotname [MAX_KNOTEN][9]; /* symbolische Namen der Knoten */
knoten Kliste[MAX_KNOTEN];     /* Liste der Knotenbeschreibungen */

int Anz_knoten;            /* Anzahl der EintrÑge in Kliste */
int Anz_zweige;            /* Anzahl der EintrÑge in Zwliste */
int Anz_var;               /* Anzahl der Unbekannten */

int Verbose=0;             /* Detaillierte Ausgabe 0-2 */
int Masse;                 /* Nummer des Masseknotens */
int U1p, U2p, U2m;         /* Potentialknoten von Ein-/Ausgangsspannung */

/* Null-Zeiger der Speicherverwaltung (hier als WÑchter am Listenende)
   werden nicht nur in die mem_info-Struktur gelegt, weil sie zu oft ge-
   braucht werden. */
summand *S_null; /* Zeiger auf Listenende (=Nullelement) */
summand *S_eins; /* Zeiger auf Einselement */
ergsummand *E_null; /* Zeiger auf Listenende Ergebnissummanden */
zweig   *Z_null; /* Zeiger auf Listenende Zweige */

summand *A[MAX_VAR][MAX_VAR];/* Berechnungsfeld der Determinantenroutinen */

FILE *Fpout, *Binout;          /* Zeiger auf Ausgabedateien */

/***************************************************************************
* Fehlermeldungen
*/
char *Errors[]=
{
    "Kein freier SUMMAND-Block bei der Speichervergabe mehr"   /* 0 */
   ,"Kein freier ZWEIG-Block bei der Speichervergabe mehr"     /* 1 */
   ,"Kein freier ERGSUMMAND-Block bei der Speichervergabe mehr"/* 1 */
   ,"",""                /* 3, 4 fÅr weitere Blocktypen reservieren */
   ,"es wurde Åber eine Feldgrenze hinausgeschrieben! Absturzgefahr"
#ifdef TEST
   ,"next-Zeiger steht in SUMMAND nicht ganz vorne"            /* 6 */
   ,"next-Zeiger steht in ZWEIG nicht ganz vorne"
   ,"next-Zeiger steht in ERGSUMMAND nicht ganz vorne"
#else
   , "", "", ""
#endif
   ,"",""               /* 9, 10 fÅr weitere Blocktypen reservieren */
   ,"Eingabedatei ist zu komplex zum Einlesen"                /* 11 */
   ,"auftretender Koeffizient ist nicht darstellbar"          /* 12 */
   ,"Berechnungsfehler (verschwindende Determinanten), evtl. ein Fehler im Netzwerk"

   /* 14: */
};
