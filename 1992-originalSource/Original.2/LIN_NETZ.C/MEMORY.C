/***************************************************************************
*                                                                          *
*  Hier stehen die Routinen der Speicherverwaltung.                        *
*                                                                          *
***************************************************************************/

/*
   (c) Peter Vranken 11/91
*/

#include <stdio.h>
#include <stdlib.h>
#include "lin_netz.h"

/***************************************************************************
* Speicherverwaltung: Frei- und RÅckgabe von Blîcken fester LÑnge, daher
* sehr einfache, schnelle und zuverlÑssige Methode durch Freelistverfahren.
* Falls kein Speicher mehr zur VerfÅgung steht, wird ein Abort durchgefÅhrt.
* Im Falle TC 2.0 wird die Feldgrîûe so bemessen, daû eine Arbeit auf einem 
* 1MB-Rechner noch gut mîglich ist.
*/
#if ATARI==2              /* Turbo-C-Version 2.0 mit Feldern beliebiger Grîûe */
#define SMEM_SIZE 25000l  /* Anzahl Blîcke der Speicherverwaltung */
#define EMEM_SIZE 2500l   /* Blîcke fÅr die Ergebnissummanden */
#define ZMEM_SIZE 1000l   /* Blîcke fÅr Verweise auf Zweigbeschreibungen */
#elif ATARI==1            /* Turbo-C-Version 1.0, 1.1 mit Feldern <32K */
#define SMEM_SIZE 2700l
#define EMEM_SIZE 500l
#define ZMEM_SIZE 500l
#else                     /* MS-DOS */
#define SMEM_SIZE 2000l
#define EMEM_SIZE 200l
#define ZMEM_SIZE 200l
#endif

/***************************************************************************
* interne Information, die fÅr alle Routinen der Speicherverwaltung relevant
* ist und auf diese Weise verborgen werden soll.
*/
typedef struct
{
   summand sum_block[SMEM_SIZE];/* Speicherblock fÅr Typ SUMMAND */
   zweig   zwg_block[ZMEM_SIZE];/* dito Zweigstrukturen, weniger genÅgt hier */
   ergsummand erg_block[EMEM_SIZE]; /* dito Ergebnissummanden */
   void *freelist[ANZ_TYPEN];   /* Freilistenzeiger fÅr alle Typen */
   void *null[ANZ_TYPEN];       /* Zeiger auf Listenende */
   unsigned long mem_cnt[ANZ_TYPEN],/* Registriert den aktuellen Speicherverbrauch */
                 max_mem[ANZ_TYPEN],/* Registriert den maximalen Speicherverbrauch */
                 mem_size[ANZ_TYPEN];/* Anzahl verfÅgbarer Blîcke je Typ */
} mem_info;

static mem_info Mem;

/***************************************************************************
* Hier stehen die erforderlichen Initialisierungen, die unbedingt vor der
* Benutzung der anderen Routinen aufgerufen werden muû.
* SEHR WICHTIG:
* Diese Routine beinhaltet Anweisungen, die fÅr jeden Typ, der verwaltet wer-
* den soll, spezifisch sind. Daher ist sie bei énderung der Typvielfalt je-
* weils anzupassen.
*/
void init_balloc() /* Initialisierung der Speicherverwaltung */
{
unsigned long i;
summand *ps;
ergsummand *pe;
zweig   *pz;
int j;

#ifdef TEST
   /* ZunÑchst ÅberprÅfen, ob wirklich der .next_Zeiger ohne Offset im zu
      verwaltenden Verbund steht: */
   if((void*)(&Mem.sum_block[0]) != (void*)(&Mem.sum_block[0].next))
      abbruch(6);
   if((void*)(&Mem.zwg_block[0]) != (void*)(&Mem.zwg_block[0].next))
      abbruch(7);
   if((void*)(&Mem.erg_block[0]) != (void*)(&Mem.erg_block[0].next))
      abbruch(8);

#if ATARI<2
   /* wegen der Probleme von TC mit groûen Feldern lieber eine Warnung aus-
      geben, wenn dieser Fall eintritt: */
#pragma warn -rch
   if(sizeof(summand)   *SMEM_SIZE > 32767l
   || sizeof(zweig)     *ZMEM_SIZE > 32767l
   || sizeof(ergsummand)*EMEM_SIZE > 32767l)
   {
      printf("Vorsicht: es werden Felder mit mehr als 32K verwendet!\n"
             "suumand: %ld, zweig: %ld, ergsummand: %ld\n",
              sizeof(summand)   *SMEM_SIZE,
              sizeof(zweig)     *ZMEM_SIZE,
              sizeof(ergsummand)*EMEM_SIZE);
      TASTE
   }
#pragma warn .rch
#endif
#endif

   /* Freilistenverkettung von sum_block vornehmen, gleichzeitig erfolgt */
   /* hier eine Vorbelegung der Blockstrukturvariable .koef */
   ps = Mem.sum_block;
   for(i=0; i<SMEM_SIZE-2; ++i) /* freie Blîcke (bis auf einen) verketten */
      ps[i].next = &Mem.sum_block[i+1],
      ps[i].koef = -1; /* 0 ist unsinnig und wird daher als
                          Kennzeichen fÅr den WÑchter reserviert */
   /* WÑchter installieren, zeigt auf sich selbst */
   ps = &Mem.sum_block[SMEM_SIZE-2];
   ps->next = ps;
   ps->var_vek = 0, ps->koef = 0;

   /* Einselement einrichten */
   S_eins = ps+1;
   S_eins->koef = 1, S_eins->var_vek = 0, S_eins->next = ps;

   /* Freilistenverkettung des Zweig-Blockes vornehmen */
   pz = Mem.zwg_block;
   for(i=0; i<ZMEM_SIZE-1; ++i) /* alle freien Blîcke verketten */
      pz[i].next = &Mem.zwg_block[i+1];

   /* WÑchter installieren, zeigt auf sich selbst */
   pz = &Mem.zwg_block[ZMEM_SIZE-1];
   pz->next = pz;

   /* Freilistenverkettung des Erg_summand-Blockes vornehmen */
   pe = Mem.erg_block;
   for(i=0; i<EMEM_SIZE-1; ++i) /* alle freien Blîcke verketten */
      pe[i].next = &Mem.erg_block[i+1];

   /* WÑchter installieren, zeigt auf sich selbst */
   pe = &Mem.erg_block[EMEM_SIZE-1];
   pe->next = pe;

   /* WÑchter global bekanntgeben, aber auch intern merken */
   Mem.null[SUMMAND]    = S_null = ps;
   Mem.null[ERGSUMMAND] = E_null = pe;
   Mem.null[ZWEIG]      = Z_null = pz;

   /* Zeiger auf ListenanfÑnge initialisieren */
   Mem.freelist[SUMMAND]    = Mem.sum_block,
   Mem.freelist[ERGSUMMAND] = Mem.erg_block,
   Mem.freelist[ZWEIG]      = Mem.zwg_block;

   /* ZÑhler zur Speicherplatzgrîûenbestimmung einrichten */
   for(j=0; j<ANZ_TYPEN; ++j)
      Mem.mem_cnt[j] = Mem.max_mem[j] = 0;

   /* bekannt geben, wie groû die einzelnen Bereiche sind */
   Mem.mem_size[SUMMAND]    = SMEM_SIZE,
   Mem.mem_size[ERGSUMMAND] = EMEM_SIZE,
   Mem.mem_size[ZWEIG]      = ZMEM_SIZE;
}   

/***************************************************************************
* Vergabe eines Blockes des verlangten Typs.
*/
void *balloc(typ)
mem_typ typ;
{
void *fp, *neu; /* Freelistzeiger */
unsigned long l;

   /* WÑchter noch nicht erreicht? */
   if((fp=Mem.freelist[typ]) != Mem.null[typ])
   {
      neu = fp;   /* Block am Listenanfang abgeben */

      /* und Freelist auf Nachfolger, daher MUû .NEXT VORNE STEHEN !! */
      Mem.freelist[typ] = *(void**)fp; 

      l = ++Mem.mem_cnt[typ];
      Mem.max_mem[typ] = MAX(l, Mem.max_mem[typ]);
   }
   else
      abbruch(typ); /* Abbruch des Programmes, kein Speicher mehr */

   return(neu);
}

/***************************************************************************
* Hiermit kann eine ganze Liste, die Åber die .next-Zeiger verknÅpft ist, zu-
* rÅckgegeben werden. Sie darf aber auch leer sein, d.h. b_nr darf bei Aufruf
* auf den WÑchter zeigen.
*/
void lfree(b_nr, typ)
void *b_nr;
mem_typ typ;
{
unsigned long len=1;
void *h, *null;

   /* leere Liste gesondert abfragen */
   if(b_nr == (null=Mem.null[typ]))
      return;

   /* das StÅck vorne einhÑngen */
   h = Mem.freelist[typ], Mem.freelist[typ] = b_nr;

   while(*(void**)b_nr != null) /* solange Nachfolger nicht WÑchter ist */
      b_nr = *(void**)b_nr,     /* Liste durchlaufen und */
      ++len;                    /* ihre Elemente zÑhlen  */


   *(void**)b_nr = h; /* bisherige Freiliste statt des WÑchters einhÑngen */
   Mem.mem_cnt[typ] -= len; /* Speicherfreigabe registrieren */
}

/***************************************************************************
* Hier wird nur die Mîglichkeit geschaffen, einen Block einer Liste zurÅck-
* zugeben. Da nicht auf Null-Element abgefragt wird, sollte man sicher sein,
* daû b_nr bei Aufruf nicht auf den WÑchter zeigt! Dies ist in der zeitauf-
* wendigen Routine elementaroperation(), fÅr die die Routine in erster Linie
* konzipiert ist, gewÑhrleistet, daher wird hier keine Abfrage eingebaut.
*/
void bfree(b_nr, typ)
void *b_nr;
mem_typ typ;
{
   *(void**)b_nr = Mem.freelist[typ];/* bisherige Freiliste statt des Listennachfolgers einhÑngen */
   Mem.freelist[typ] = b_nr; /* das StÅck vorne einhÑngen */
   --Mem.mem_cnt[typ];       /* Speicherfreigabe registrieren */
}

#ifdef TEST
/***************************************************************************
* Hier folgt eine Routine, die die freien Speicherblîcke zÑhlt. Je nach Auf-
* ruf merkt sie sich das Ergebnis als Referenzwert, oder sie vergleicht es
* mit dem alten Referenzwert. Dadurch lÑût sich ÅberprÅfen, ob am Ende des
* Programmlaufes wieder alle Blîcke freigegeben worden sind. Fehler im Um-
* gang mit den Zeigern kînnen so festgestellt werden.
*/
long cnt_mem(cal, typ)
boolean cal;
mem_typ typ;
{
static long size[ANZ_TYPEN]={0};
long k;
void *h, *null;
   
   h    = Mem.freelist[typ];
   null = Mem.null[typ];
   k = 0;
   while(h != null)
      ++k, h = *(void**)h;

   if(!cal && size[typ]-k != mem_belegt(typ))
      printf("Vorsicht: Speichergrîûenwert des Typs %d stimmt nicht mit ZÑhlung Åberein! (%d)\n",
              typ, size[typ]-k-mem_belegt(typ));

   if(cal) /* Kalibrierung vornehmen: Initialwert abspeichern */
      size[typ] = k;
   else if(k!=size[typ])
   {
      printf("Vorsicht: Speichergrîûe des Typs %d weicht vom Referenzwert ab! (%ld)\n",
             typ, k-size[typ]);
      TASTE
   }
   return(k);
}
#endif

/***************************************************************************
* Bisheriger Maximalverbrauch an Speicher.
*/
long mem_bedarf(typ)
mem_typ typ;
{
   return(Mem.max_mem[typ]);
}

/***************************************************************************
* Derzeitiger Speicherbedarf.
*/
long mem_belegt(typ)
mem_typ typ;
{
   return(Mem.mem_cnt[typ]);
}

