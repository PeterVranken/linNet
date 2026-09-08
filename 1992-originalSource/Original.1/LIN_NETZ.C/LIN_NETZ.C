/***************************************************************************
*                                                                          *
*  Teil II des Programmes zur Erstellung eines Gleichungssystems aus einer *
*  Knotenliste.                                                            *
*                                                                          *
*  Hier stehen das Hauptprogramm, die Ein/Ausgabefunktionen und die Spei-  *
*  cherverwaltung.                                                         *
*                                                                          *
***************************************************************************/

/*
   (c) Peter Vranken 11/91
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <math.h>
#include "lin_netz.h"

#if ATARI
#include <filename.h>
#else
#include "filename.h"
#endif

/***************************************************************************
* Modulinterne globale Daten
*/
static FILE *Fpin;
static int Zl_nr;
static boolean Fehler;
static struct
{
   int modus;               /* lineare oder logarithmische Darstellung */
   int anzahl;              /* Anzahl der berechneten Punkte */
   double fu, fo;           /* Frequenzgrenzen der Darstellung */
   double varvalue[MAX_VAR];/* Variablenwerte, sofern angegeben */
} Plotinfo = { 3, 128, 10.0, 25E3 };

/**************************************************************************/

int main(anz, koms) /* Parameter: Name der Datei mit Knotenliste */
int anz;
char *koms[];
{
int i;
char input[69]="", output[69]="", binoutput[69]="";
boolean binfile=FALSE;
d_name fnamei, fnameo, fnameb;

   printf("--------------------------------------------------------------------------\n"
          " LIN_NETZ - Das Programm zur formalen Analyse linearer elektrischer Netze\n"
          " Copyright (c) 1991 Peter Vranken\n"
          "--------------------------------------------------------------------------\n");

   for(i=1; i<anz; ++i) /* Kommandos untersuchen */
   {
      if(koms[i][0]=='-')
         switch(toupper(koms[i][1])) /* 1. Zeichen spezifiziert wie Åblich Kommando */
         {
            case 'A': strncpy(output, koms[i]+2, 64); break;
            case 'V': Verbose = atoi(koms[i]+2); break;
            case 'B': binfile = TRUE;
                      if(koms[i][2] !=  '\0')
                        strncpy(binoutput, koms[i]+2, 64);
                      else
                        binoutput[0] = '\0';
                      break;
         }
      else
         strncpy(input, koms[i], 64);
   }
   /* input, output, binoutput beinhalten nun die Namen der zugehîrigen Da-
      teien. Die beiden letzteren dÅrfen auch Leerstrings enthalten, dann
      wird fÅr sie der Name der Inputdatei Åbernommen. Das Flag binfile gibt
      an, ob Åberhaupt eine BinÑrdatei gewÅnscht ist. */

   if(!input[0]) /* es wurde keine Eingabedatei spezifiziert */
   {

      printf("Anwendung: LIN_NETZ.ttp [-Aausgabe]  (Name der Ausgabedatei)\n"
             "                        [-Vn]        (Umfang des Berechnungsprotokolls)\n"
             "                        [-B[binaer]] (Erstellung einer Funktionsdatei)\n"
             "                        eingabe      (Name der Eingabedatei)\n");
      return(0);
   }

   /* nun den Namen der Eingabedatei syntaktisch zergliedern */
   dateiname(&fnamei, input);
   if(FN_err) /* ist syntaktisch unkorrekt */
   {
error1:
      printf("Fehler im Namen der spezifizierten Datei(en)!\n");
      return(-6);
   }

   /* Erweiterung .ckt anhÑngen */
   strcpy(fnamei.komplett, fnamei.pfad);   /* Pfad und Dateinamen Åber- */
   strcat(fnamei.komplett, fnamei.n_o_ext);/* nehmen, */
   strcat(fnamei.komplett, "ckt");         /* Erweiterung hinzufÅgen */

   /* Nun den Namen der Listingsdatei korrekt zusammenstellen */

   if(output[0] == '\0') /* also den Namen der Eingabe Åbernehmen */
   {
      strcpy(fnameo.komplett, fnamei.pfad);   /* Pfad und Dateinamen von  */
      strcat(fnameo.komplett, fnamei.n_o_ext);/* input Åbernehmen */
   }
   else
   {
      dateiname(&fnameo, output);
      if(FN_err)
         goto error1;
      strcpy(fnameo.komplett, fnameo.pfad); /* Pfad und Dateinamen wie ange- */
      strcat(fnameo.komplett, fnameo.n_o_ext);/* geben Åbernehmen */
   }
   strcat(fnameo.komplett, "lis"); /* Erweiterung ist aber fest vorgegeben */

   /* Dasselbe nochmal fÅr die BinÑrdatei, wenn nîtig */

   if(binfile)
   {
      if(binoutput[0] == '\0') /* also den Namen der Eingabe Åbernehmen */
      {
         strcpy(fnameb.komplett, fnamei.pfad);   /* Pfad und Dateinamen von  */
         strcat(fnameb.komplett, fnamei.n_o_ext);/* input Åbernehmen */
      }
      else
      {
         dateiname(&fnameb, binoutput);
         if(FN_err)
            goto error1;
         strcpy(fnameb.komplett, fnameb.pfad); /* Pfad und Dateinamen wie ange- */
         strcat(fnameb.komplett, fnameb.n_o_ext);/* geben Åbernehmen */
      }
      strcat(fnameb.komplett, "fkt"); /* Erweiterung ist aber fest vorgegeben */
   }

   init_balloc();
#ifdef TEST
   cnt_mem(TRUE, ZWEIG);
   cnt_mem(TRUE, SUMMAND);
   cnt_mem(TRUE, ERGSUMMAND);
#endif
   if((Fpin=fopen(fnamei.komplett,"r")) == NULL)
   {
      printf("Fehler beim ôffnen der Eingabedatei %s!\n", fnamei.komplett);
      return(-2);
   }

   if((Fpout=fopen(fnameo.komplett,"w")) == NULL)
   {
      printf("Fehler beim ôffnen der Ausgabedatei %s!\n", fnameo.komplett);
      fclose(Fpin);
      return(-3);
   }
   if(einlesen())
   {
      printf("Fehler in der Eingabedatei!\n");
      fclose(Fpin);
      return(-4);
   }
   printf("fehlerfrei eingelesen! %d Knoten\n", Anz_knoten);
   fclose(Fpin);
   titel(fnamei.komplett);
   if(Verbose >= 2)
      anzeige_kliste();
   printf("Topologietest ");
   if(!topologietest())
   {
      printf("verlief erfolgreich!\n");
      if(binfile)
         if((Binout=fopen(fnameb.komplett,"w")) == NULL)
            printf("Fehler beim ôffnen der BinÑrdatei %s, BinÑrdatei wird nicht angelegt!\n",
                    fnameb.komplett);

      uebertragungsfkt();
      printf("Analyse erfolgreich beendet!\n");
      fclose(Fpout);
      if(Binout != NULL)
         fclose(Binout);

      /* nun noch eine Datei mit numerischen Defaultwerten fÅr die Bauteile anlegen */
      if(binfile && ((Fpout=fopen("auswertg.inf", "w")) != NULL))
      {
         fprintf(Fpout, "%e\n%e\n%d\n", Plotinfo.fu, Plotinfo.fo, Plotinfo.anzahl);
         for(i=0; i<Anz_var; ++i)
            if(Plotinfo.varvalue[i] < HUGE_VAL)
               fprintf(Fpout, "%e\n", Plotinfo.varvalue[i]);
         fprintf(Fpout, "%d\n", Plotinfo.modus);
         fclose(Fpout);
      }
   }
   else
   {
      fclose(Fpout);
      return(-5);
   }

#ifdef TEST
{
int i;
   for(i=0; i<Anz_knoten; i++)
      lfree(Kliste[i].zw1, ZWEIG);
   cnt_mem(FALSE, ZWEIG);
   cnt_mem(FALSE, SUMMAND);
   cnt_mem(FALSE, ERGSUMMAND);
}
#endif
   return(0);
}
/***************************************************************************
* In erster Linie fÅr Testzwecke: eine Anzeige der Datenstruktur.
*/
static void anzeige_kliste()
{
int i;
zweig *p;
zweig_info *p2;
char *s;

   fprintf(Fpout, "\n**************************************\n"
                    " Kontrollausgabe der Netzverbindungen\n"
                    "**************************************\n\n"
                    "Masseknoten: %s\nEingangsspannung: zwischen %s und Masse\n"
                    "Ausgangsspannung: zwischen %s und %s\n", Knotname[Masse],
                  Knotname[U1p], Knotname[U2p], Knotname[U2m]);

   for(i=0; i<Anz_knoten; ++i)
   {
      p = Kliste[i].zw1;
      fprintf(Fpout, "von Knoten %s %smit dem Potential des Knotens %s nach:\n",
                      Knotname[i], Kliste[i].ist_qop? "(ist OP-Ausgang) ": "",
                      Knotname[Kliste[i].pot_nr]);
      while(p!=Z_null)
      {
         p2 = p->was;
         fprintf(Fpout, "   %s Åber ", Knotname[p2->ak==i? p2->ek: p2->ak]);
         switch(p2->art)
         {
            case IMP: s = "die Spule"; break;
            case CAP: s = "den Kondensator"; break;
            case RES: s = "den Widerstand"; break;
            case LW : s = "den Leitwert"; break;
            case QI : s = "die Stromquelle"; break;
            case OPZI:s = "die OperationsverstÑrkereingÑnge"; break;
            case OPQU:s = "den OperationsverstÑrkerausgang"; break;
            case UIN: s = "die Eingangsspannung"; break;
            case UOUT:s = "die Ausgangsspannung"; break;
            case US:  s = "den Steuerspannungseinfluû"; break;

            default : s=  "interner Fehler: unbekannter Zweigtypus!\n";
         }
         fprintf(Fpout, "%s %s", s, p2->name);
         if(p2->art==QI)
            fprintf(Fpout, ", Steuerspannung: + %s und - %s",
                           Knotname[p2->usp], Knotname[p2->usm]);
         if(p2->btnr >= 0)
            fprintf(Fpout, ", Var.-Nr: %d\n", p2->btnr);
         else
            fprintf(Fpout, "\n");
         p=p->next;
      }
   }
}

/***************************************************************************
* Hier folgt ein Parser fÅr das Einlesen der Eingabedatei. ZunÑchst eine 
* Routine, die Zeichen liest und dabei einige Umwandlungen vornimmt, die
* die Interpretation vereinfachen. So werden z.B. die mîglichen Zeilenende-
* darstellungen vereinheitlicht. Zudem wird der globale ZeilenzÑhler mitge-
* fÅhrt, und die Mîglichkeit des ZeichenzurÅckgebens realisiert, das benî-
* tigt wird, wenn eine Abfrage auf ein SchlÅsselwort mit mehreren Zeichen
* negativ verlÑuft. 
*/
static int scanner(lesen, tok)
boolean lesen;
int tok;
{
static alt[100], sp=0;
int ch;

   if(lesen == FALSE) /* es soll also ein Zeichen zurÅckgeschrieben werden */
      if(sp == 100)   /* Stapel voll, Einlesen versagt */
         abbruch(11); /* kann normalerweise kaum auftreten */
      else
      {
         alt[sp++] = tok;
         return(0);
      }

   /* also ein Zeichen holen */
   if(sp > 0) /* zunÑchst den Stapel abarbeiten */
      return(alt[--sp]);

   ch = getc(Fpin); /* sonst ein neues Zeichen einlesen */
   if(ch == '\n')    /* eine neue Zeile */
      ++Zl_nr;
   if(ch=='\n' || ch=='\r' || ch==';')/* Vereinheitlichung der Trennzeichen */
      return(';');
   if(ch=='\t')
      return(' ');

   /* sonst keine weitere Manipulation */
   return(ch);
}

/***************************************************************************
* da Kommentare wirklich Åberall stehen kînnen, es aber sehr lÑstig wÑre,
* auch jedesmal darauf abzufragen, wird der Benutzung der Routine scanner()
* noch ein Filter fÅr Kommentare vorgeschaltet.
*/
static int next_tok()
{
int i, tok1, tok2;
boolean kanf, kend, eof;

   for(;;) /* Schleife Åber alle hintereinanderliegenden Kommentare */
   {
      tok1 = scanner(TRUE, 0);
      tok2 = scanner(TRUE, 0);
      kanf = tok1=='/' && tok2=='*';
      
      if(!kanf) /* keinen Kommentaranfang gefunden */
      {
         scanner(FALSE, tok2); /* dann war 2. Zeichen zuviel */
         return(tok1);
      }

      /* sonst befindet man sich nun im Kommentar */   
      i = 1;
      do
      {
         tok1 = scanner(TRUE, 0);
         tok2 = scanner(TRUE, 0);
         kanf = tok1=='/' && tok2=='*';
         kend = tok1=='*' && tok2=='/';
         eof  = tok1==EOF || tok2==EOF;

         if(kend)
            --i;
         else if(kanf)
            ++i;
         else if(eof)
            return(EOF);
         else
            scanner(FALSE, tok2);
      }
      while(i);
   }
}

/***************************************************************************
* hier werden Leerzeichen Åberlesen und ihre Anzahl festgestellt. Dadurch
* werden sie entfernt, und es kann festgestellt werden, ob mindestens eines
* vorhanden war.
*/
static boolean space()
{
int tok;
boolean ok;

   while((tok=next_tok()) == ' ')
      ok = TRUE;
   scanner(FALSE, tok); /* 1. Zeichen nach Freiraum zurÅck */
   return(ok);
}
   
/***************************************************************************
* Datei einlesen und interpretieren, unter Verwendung des Scanners. Da
* Kommentare ausgefiltert sind, besteht die Datei nur aus einer Folge von
* Zweigbeschreibungen.
*/
static boolean einlesen()
{
int i, tok;

   Zl_nr = 1, Fehler = FALSE;
   Anz_knoten = Anz_zweige = Anz_var = 0;
   Masse = U1p = U2p = U2m = -1;

   for(i=0; i<MAX_KNOTEN; ++i)
      Kliste[i].pot_nr = i,
      Kliste[i].ist_qop = FALSE,
      Kliste[i].zw1 = Z_null;

   /* Schleife Åber alle Befehlszeilen */
   for(;;)
   {
      space(); /* fÅhrenden Leerraum Åberlesen */
      tok = next_tok();
      if(tok == EOF)      /* es gibt keine weitere Zeile */
         break;
      else if(tok == ';') /* Leerzeile, Åberlesen */
         continue;
      else if(tok == '*') /* Kommentarzeile */
         kommentar();     /* zugehîrige Routine Åberliest ihn */
      else if(tok == '.') /* Frequenzvorgabe, mit .ac eingeleitet */
         plot_info();     /* zugehîrige Routine wertet sie aus */
      else                /* sonst eine Normalzeile, Bauteildefinition */
         scanner(FALSE, tok), /* Zeichen wird dort noch benîtigt */
         bauteil_def();   /* Auswertung aufrufen */

      /* nun noch ÅberprÅfen, ob das Zeilenende wirklich erreicht ist, denn
         nach einer an sich korrekten Zeile darf nichts mehr stehen. */
      space();
      tok = next_tok();
      if(tok!=';' && tok!=EOF)
         printf("es steht öberflÅssiges am Ende der Zeile %d!\n", Zl_nr),
         sync();
   }

   if(U1p<0 || U2p<0 || Masse<0 || U2m<0)
      printf("Ein/AusgÑnge nicht richtig definiert!\n"),
      Fehler = TRUE;
   return(Fehler);
}      

/***************************************************************************
* hier folgen einige Hilfsroutinen fÅr den Parser
*/
static boolean lies_tok(tok)/* öberprÅfen, ob ein bestimmtes Zeichen folgt */
int tok;
{
   if(next_tok() != tok) /* Untersuchung auf Syntaxfehler */
   {
      printf("erwarte '%c' in Zeile %d!\n", (char)tok, Zl_nr);
      sync(); /* bei nÑchstem Zeilenanfang weitermachen */
      return(TRUE);
   }
   return(FALSE);
}

/***************************************************************************
* im Fehlerfall durch Abwarten des nÑchsten Zweigbeschreibungsausdrucks den
* Parser wieder mit der Inputdatei synchronisieren, um mehrere Fehler in
* einer Datei finden zu kînnen, ohne allzuviele Folgefehler zu erzeugen.
*/
static void sync()
{
int tok;

   Fehler = TRUE;/* natÅrlich ist die erstellte Knotenliste falsch und darf */
                 /* nicht verwendet werden. */
   do
      tok = next_tok();
   while(tok!=';' && tok!=EOF);/* alle Trenner werden in ; verwandelt */
   scanner(FALSE, tok);
}

/***************************************************************************
* Eine Kommentarzeile, die durch einen Stern am Anfang kenntlich ist, Åber-
* lesen.
*/
static void kommentar()
{
int tok;

   do
      tok = next_tok();
   while(tok!=';' && tok!=EOF);/* alle Trenner werden in ; verwandelt */
   scanner(FALSE, tok);
}

/***************************************************************************
* Eine besondere Zeile: es werden Angaben Åber die spÑtere Erstellung nume-
* rischer Daten gemacht.
*/
static void plot_info()
{
int dec, i;
char sw[4];
double a;

   if(toupper(next_tok())!='A' || toupper(next_tok())!='C')
   {
error1:
      printf("Syntaxfehler bei Frequenzangabe in Zeile %d!\n", Zl_nr);
error2:
      sync();
      return;
   }
   if(!space()) /* kein Leerraum gelassen */
      goto error1;

   for(i=0; i<3; ++i)
      sw[i] = toupper(next_tok());
   sw[3] = '\0';
   if((dec=strcmp(sw, "LIN")) && strcmp(sw, "DEC"))
   {
      printf("erwarte Angabe Åber Darstellungsart in Zeile %d!\n", Zl_nr);
      goto error2;
   }
   if(dec)
      Plotinfo.modus = 3;
   else
      Plotinfo.modus = 0;

   if(!space()) /* kein Leerraum gelassen */
      goto error1;

   if(integer(&Plotinfo.anzahl)) /* Anzahl StÅtzstellen bzw. Punkte je Dekade einlesen */
      return;

   if(!space()) /* kein Leerraum gelassen */
      goto error1;

   if(zahl(&Plotinfo.fu)) /* untere Grenzfrequenz einlesen */
      return;

   if(!space()) /* kein Leerraum gelassen */
      goto error1;

   zahl(&Plotinfo.fo); /* obere Grenzfrequenz einlesen */

   /* nun noch bei logarithmischer Skalierung die Angabe der Anzahl Punkte
      je Dekade in die erforderliche Gesamtanzahl umrechnen. Dazu zunÑchst
      eine PlausibilitÑtsprÅfung der gelesenen Werte. */
   Plotinfo.fo = MIN(Plotinfo.fo, 1E12), Plotinfo.fo = MAX(Plotinfo.fo, 1E-12);
   Plotinfo.fu = MIN(Plotinfo.fu, 1E12), Plotinfo.fu = MAX(Plotinfo.fu, 1E-12);
   if(Plotinfo.fo == Plotinfo.fu)
      Plotinfo.fo *=1.1;
   else if(Plotinfo.fo < Plotinfo.fu)
      a = Plotinfo.fo, Plotinfo.fo = Plotinfo.fu, Plotinfo.fu = a;
   if(Plotinfo.modus == 3) /* also logarithmische Darstellung */
      a = (double)Plotinfo.anzahl * log10(Plotinfo.fo/Plotinfo.fu),
      Plotinfo.anzahl = MAX((int)a, 25),
      Plotinfo.anzahl = MIN(Plotinfo.anzahl, 500);
}

/***************************************************************************
* Hier wird eine normale Zeile, d.h. die Definition eines Bauelementes zer-
* gliedert. Im Fehlerfall wird das Zeilenende abgewartet.
*/
static void bauteil_def()
{
int tok1, tok2, bau_el, faktor, knt1, knt2, knt3, knt4;
double value;
zweig_info *refptr;
char name[9];

   tok1 = next_tok(),  tok2 = next_tok();

   if(toupper(tok1)=='O' && toupper(tok2)=='P')/* OperationsverstÑrker */
      bau_el = OP;
   else if(toupper(tok1)=='U'  &&  (toupper(tok2)=='1' || toupper(tok2)=='2'))
        /* Festlegung der Ein- oder Ausgangsspannung */
      bau_el = toupper(tok2)=='1'? UIN: UOUT;
   else if(toupper(tok1)=='R' || toupper(tok1)=='C' || toupper(tok1)=='L'
        || toupper(tok1)=='Y' || toupper(tok1)=='G' || toupper(tok1)=='S')
   {
      switch(toupper(tok1))
      {
         case 'R': bau_el = RES; break;
         case 'Y': bau_el = LW;  break;
         case 'C': bau_el = CAP; break;
         case 'L': bau_el = IMP; break;
         case 'S': /* damit Stromquellen auch mit S wie Steilheit bezeichnet werden kînnen */
         case 'G': bau_el = QI;
      }
      scanner(FALSE, tok2);
      scanner(FALSE, tok1); /* Zeichen zurÅck zum Lesen des vollen Namens */
      if(lies_name(name))
         return;
   }
   else
   {
      printf("erwarte Bauteilangabe in Zeile %d!\n", Zl_nr);
      sync();
      return;
   }

   /* nun folgen in jedem Fall zwei Knotenangaben */
   if(!space())
   {
error1:
      printf("Syntaxfehler bei Bauteildefinition in Zeile %d!\n", Zl_nr);
      sync();
      return;
   }
   if((knt1=knotennr()) < 0)
      return;

   if(!space())
      goto error1;
   
   if((knt2=knotennr()) < 0)
      return;

   if(knt1 == knt2)
   {
error2:
      printf("unsinnige Verschaltung in Zeile %d\n", Zl_nr);
      sync();
      return;
   }

   if(bau_el==UIN || bau_el==UOUT) /* Ein-/Ausgangsspannung */
   {
      einhaengen(bau_el, 0, 0, knt1, knt2, 0, 0, NULL, "");
      return;
   }
   else if(bau_el==RES || bau_el==CAP || bau_el==IMP || bau_el==LW)
   {
      space();
      if(zuweisung(bau_el, &value, &faktor, &refptr))
         return;
         
      /* nun ist die volle Information bestimmt, um das neue Element in
         die Datenstruktur aufzunehmen. */

      einhaengen(bau_el, 0, 0, knt1, knt2, 0, faktor, refptr, name);

      /* das Bauelement hat sicher die Variablennummer Anz_var-1 bekommen, da
         diese Variable nach jedem neuen Bauteil mit Namen inkrementiert wird.
         Somit kann jetzt der Defaultwert fÅr den numerischen Wert der Var-
         iablen eingetragen werden. */
      Plotinfo.varvalue[Anz_var-1] = value;

      return;
   }/* Ende if(passives Element) */

   /* Stromquelle und OperationsverstÑrker brauchen weitere Knoten */
   if(!space())
      goto error1;

   if((knt3=knotennr()) < 0)
      return;

   if(bau_el == QI) /* Stromquelle braucht noch einen 4. Knoten */
   {
      if(!space())
         goto error1;
      if((knt4=knotennr()) < 0)
         return;
      if(knt3 == knt4)
         goto error2;
      /* nun noch auf mîgliche Zuweisung untersuchen */
      space();
      tok1 = next_tok();
      scanner(FALSE, tok1);   /* Zeichen nur fÅr die Entscheidung lesen */
      if(tok1!=';' && tok1!=EOF) /* es erfolgt also eine Angabe */
      {
         if(wertangabe(QI, &value, &faktor, &refptr))
            return;
      }
      else /* keine Wertangabe, also Defaultwerte nehmen */
      {
         value = 0.05; /* also 50 mS fÅr Stromquellensteilheit */
         refptr = NULL;/* und keine Bezugnahme auf anderes Element */
         faktor = 0;
      }
      einhaengen(QI, knt3, knt4, knt2, knt1, 0, faktor, refptr, name);
      Plotinfo.varvalue[Anz_var-1] = value;

      return;
   }
   /* else muû es ein OperationverstÑrker sein */

   if(Masse < 0) /* sie muû fÅr die Verarbeitung bekannt sein */
   {
      printf("vor Verwendung von OperationsverstÑrkern bitte die Eingangsspannung definieren!\n");
      sync();
      return;
   }
   einhaengen(OP, 0, 0, knt1, knt2, knt3, 0, NULL, "");
}      
      
/***************************************************************************
* Zuweisung liest den numerischen Defaultwert eines Bauteiles ein, oder er-
* mittelt die symbolische Bezugnahme
* auf ein anderes bereits definiertes Bauelement. Sollte beides nicht ge-
* lingen, dann werden Standartwerte vergeben. Zu deren Wahl: alle Zeitkon-
* stanten, die sich aus R, L und C ergeben, sollen zu Frequenzen im NF-Be-
* reich fÅhren, der ja auch default ist. (R*C, L/R, ˚L*C)
*/
static boolean zuweisung(bau_el, value, faktor, refptr)
int bau_el;
double *value;
int *faktor;
zweig_info **refptr;
{
int tok;
boolean ok;

   space();
   tok = next_tok();
   if(tok==';' || tok==EOF)/* optionale Zuweisung wurde nicht vorgenommen */
   {
      scanner(FALSE, tok);
      switch(bau_el) /* Standartwerte fÅr Bauelemente vergeben */
      {
         case RES: *value = 20000.0; break;
         case CAP: *value = 100E-9;  break;
         case IMP: *value = 1.0;     break;
         case LW:  *value = 0.0001;
      }
      *faktor = 0;
      *refptr = NULL;
      return(FALSE);
   }
   else if(bau_el != QI) /* dann muû Zuweisung mit z.B. R= ... anfangen */
   {
      switch(bau_el)
      {
         case RES: ok = toupper(tok)=='R'; break;
         case CAP: ok = toupper(tok)=='C'; break;
         case IMP: ok = toupper(tok)=='L'; break;
         case LW : ok = toupper(tok)=='Y';
      }
      if(!ok)
      {
         printf("widersprÅchliche Bauteilangabe in Zuweisung in Zeile %d!\n", Zl_nr);
error1:
         sync();
         return(TRUE);
      }
      space();
      if(next_tok() != '=')
      {
         printf("erwarte '=' in Zeile %d!\n", Zl_nr);
         goto error1;
      }
   }
   return(wertangabe(bau_el, value, faktor, refptr));
}

/***************************************************************************
* Hier wird eine Angabe Åber eine Bauteilgrîûe gelesen. Dazu muû entweder
* eine Flieûkommazahl mit dem numerischen Wert, oder ein ganzzahliges Ver-
* hÑltnis zu einem anderen Bauelement derselben Art angegeben sein.
*/
static boolean wertangabe(bau_el, value, faktor, refptr)
int bau_el;
double *value;
int *faktor;
zweig_info **refptr;
{
double a;
int tok, btnr;

   space();
   tok = next_tok();
   scanner(FALSE, tok);
   if(isdigit(tok) || tok=='.') /* nur so kann eine Zahl anfangen */
   {
      if(zahl(&a)) /* diese einlesen */
      {
error1:
         *value = 0.0;
         *faktor = 0;
         *refptr = NULL;
         return(TRUE);
      }
      space(); /* Leerzeichen bis evtl. '*' Åberlesen */
      tok = next_tok();
      if(tok == '*') /* dann ists eine Bezugnahme, Zahl auf zulÑssigen Bereich prÅfen */
      {
         if(a!=floor(a) || a>9999.0 || a<1.0)
         {
            printf("nur ganze Zahlen <10000 als BauteilverhÑltnis zugelassen! (Zeile %d)\n",
                    Zl_nr);
            sync();
            goto error1;
         }
         /* damit spÑter erkannt werden kann, daû das in der aktuellen Zeile
            definierte Bauteil keinen eigenen Wert hat, da es als Vielfaches
            eines anderen definiert wird, wird seine Wertangabe auf einen re-
            servierten, sonst unmîglichen Wert gesetzt. */
         *faktor = (int)a; /* Umwandlung klappt ja fehlerfrei */
         *value = HUGE_VAL;

         /* nun noch die Bezugsbasisvariable ermitteln */
         space();
         if(bezugsbasis(&btnr, bau_el))
            goto error1;
         *refptr = Zweigptr[btnr];/* Zeiger auf Zweiginfo des Bezugsbasiselementes */
      }
      else /* kein '*' nach Zahl, also numerische Angabe */
      {
         scanner(FALSE, tok),
         *refptr = NULL,
         *faktor = 0,
         *value  = a;
      }
   }
   else /* keine Zahl, also muû nur eine Bezugsbasis angegeben sein (mit Faktor 1) */
   {
      *faktor = 1;
      *value  = HUGE_VAL;
      if(bezugsbasis(&btnr, bau_el))
         goto error1;
      *refptr = Zweigptr[btnr];/* Zeiger auf Zweiginfo des Bezugsbasiselementes */
   } /* Ende des 'if' mit oder ohne Vorfaktor */
   return(FALSE);
}
  
/***************************************************************************
* Eine Zeichenkette als Namen eines Bauteiles einlesen
*/
static boolean lies_name(name)
char *name;
{
int l, tok;

   if(!isalpha(tok=next_tok()))/* nur Buchstaben sind als 1.Zeichen erlaubt */
   {
      printf("Fehler im Bezeichner in Zeile %d!\n", Zl_nr);
      sync();
      return(TRUE);
   }
   l = 0;
   do
   {
      if(l<8)
         name[l++] = tok;
      tok = next_tok(); /* hier keine Leerzeichen Åberlesen */
   }
   while(isalnum(tok)); /* also Ziffer oder Buchstabe */
   name[l] = '\0';
   scanner(FALSE, tok);
   return(FALSE); /* alles OK */

error:
   printf("Fehler im Bezeichner in Zeile %d!\n", Zl_nr);
   sync();
   return(TRUE);
}

/***************************************************************************
* Folgende Routine liest eine symbolische Knotenbezeichnung ein, speichert
* sie und ordnet ihr eine laufende Nummer zu. Die neu vergebene oder durch
* Vergleich mit frÅher gespeicherten Namen ermittelte Nummer des Knotens wird
* zurÅckgegeben.
*/
static int knotennr()
{
char neuer[9];
int i, l, tok;

   if(!isalnum(tok=next_tok())) /* nur Ziffern und Buchstaben sind erlaubt */
   {
      printf("Fehler im Bezeichner in Zeile %d!\n", Zl_nr);
      sync();
      return(-1);
   }
   l = 0;
   do
   {
      if(l<8)
         neuer[l++] = tok;
      tok = next_tok();  /* hier keine Leerzeichen Åberlesen */
   }
   while(isalnum(tok));  /* also Ziffer oder Buchstabe */
   neuer[l] = '\0';
   scanner(FALSE, tok);  /* ÅberflÅssiges Zeichen zurÅckschreiben */

   for(i=0; i<Anz_knoten; ++i)  /* mit allen schon gespeicherten vergleichen */
      if(!strcmp(neuer, Knotname[i])) /* gefunden */
         return(i);

   if(i == MAX_KNOTEN) /* zuviele Namen */
   {
      printf("zu viele Knoten verwendet!\n");
      sync();
      return(-1); /* Fehleranzeige */
   }
   ++Anz_knoten;
   strcpy(Knotname[i], neuer);
   return(i);
}

/***************************************************************************
* Einlesen einer Long-Integer grîûer als Null. Nur diese sind als VerhÑltnis
* von Bauteilen zueinander erlaubt. Die Zahlen dÅrfen maximal 4stellig sein,
* mehr wÑre unsinnig und provozierte öberlauffehler.
*/
static boolean integer(faktor)
int *faktor;
{
int tok, i;
char asc[5];

   tok = next_tok();
   if(!isdigit(tok))
   {
error1:
      printf("erwarte eine positive Ganzzahlkonstante in Zeile %d!\n", Zl_nr);
      goto error;
   }
   i = 0;
   do
   {
      asc[i++] = (char)tok;
      tok = next_tok();
   }
   while(i<4 && isdigit(tok));
   asc[i] = '\0';

   if(i==4 && isdigit(tok))
   {
      printf("Konstante ist zu groû in Zeile %d!\n", Zl_nr);
error:
      *faktor = 0;
      sync();
      return(TRUE);
   }
   scanner(FALSE, tok);
   if((*faktor=atoi(asc)) <= 0)
      goto error1;
   return(FALSE);
}

/***************************************************************************
* Einlesen einer Flieûkommazahl. Darstellung mit symbolischem oder gewîhn-
* lichem Exponenten.
*/
static boolean zahl(a)
double *a;
{
#define LEN 40
char buf[LEN+1], exbuf[4];
int i, j, expo, tok;

   for(i=0;;)
   {
      if(i>=LEN) /* Zahl zu lang, kann nicht eingelesen werden */
      {
         printf("Zahl zu lang in Zeile %d!\n", Zl_nr);
         sync();
         *a = 0.0;
         return(TRUE);
      }
      tok = next_tok();
      if(isdigit(tok))
         buf[i++] = (char)tok;
      else
         break;
   }
   if(tok == '.') /* dann folgt ein Nachkommateil */
   {
      buf[i++] = '.';
      for(;;)
      {
         tok = next_tok();
         if(!isdigit(tok))
            break;
         if(i<LEN)
            buf[i++] = (char)tok;
      }
   }
   buf[i] = '\0';

   switch(tok) /* evtl. ein (symbolischer) Exponent? */
   {
      case 'p':
      case 'P': expo = -12; break;
      case 'n':
      case 'N': expo =  -9; break;
      case 'u':
      case 'U': expo =  -6; break;
      case 'm': expo =  -3; break;

      case 'k':
      case 'K': expo =   3; break;

      case 'M': expo =   6; break;
      case 'g':
      case 'G': expo =   9; break;
      case 't':
      case 'T': expo =  12; break;
      case 'e':
      case 'E': tok = next_tok();
                j = 0;
                if(tok=='-' || tok=='+')
                  exbuf[j++] = (char)tok,
                  tok = next_tok();
                if(!isdigit(tok)) /* mindestens 1 Ziffer nîtig fÅr Exponent */
                {
error1:
                   printf("Syntaxfehler in Flieûkommazahl in Zeile %d!\n", Zl_nr);
                   sync();
                   *a = 0.0;
                   return(TRUE);
                }
                for(;;)
                {
                   exbuf[j++] = (char)tok;
                   if(j >= 4)
                      goto error1;
                   tok = next_tok();
                   if(!isdigit(tok)) /* Zahl zu Ende */
                      break;
                }
                exbuf[j] = '\0';
                expo = atoi(exbuf);
                scanner(FALSE, tok); /* zuvielgelesenes Zeichen zurÅck */
                break;

       default: expo = 0;
                scanner(FALSE, tok); /* zuvielgelesenes Zeichen zurÅck */
   }/* Ende switch(welche Art von Exponent) */


   /* Ergebnis aus Mantisse und Exponent zusammensetzen */
   *a  = atof(buf);
   *a *= pow10(expo);
         
   return(FALSE);
}

/***************************************************************************
* Hier wird versucht, ein Bauteil zu finden, dessen Namen als nÑchstes aus
* der Inputdatei gelesen wird. Existiert es, dann wird seine Bauteilnummer
* in den Parameter geschrieben. Return-Kode liefert Aufschluû Åber Erfolg.
*/
static boolean bezugsbasis(btnr, typ)
int *btnr, typ;
{
char name[9];
int i;

   if(lies_name(name))  /* Referenzbauelementnamen einlesen */
   {
      *btnr = 0;
      return(TRUE);
   }

   for(i=0; i<Anz_var; ++i)/* alle bereits bestimmten Namen mit diesem vergleichen */
      if(!strcmp(name, Zweigptr[i]->name)) /* gefunden */
         if(Zweigptr[i]->refp != NULL)/* Bezugselement ist selbst relativ zu weiterem */
         {
            printf("verkettete Bauteilrelationen in Zeile %d!\n", Zl_nr);
            goto error;
         }
         else if(typ != Zweigptr[i]->art)/* Bezugname nur zwischen gleichen Bauteilen */
         {
            printf("Grîûenrelation zwischen verschiedenartigen Bauteilen in %d!\n", Zl_nr);
            goto error;
         }
         else
         {
            *btnr = i;
            return(FALSE);
         }

   /* war nicht dabei */
   printf("Zeile %d: Bezug auf noch nicht definiertes Bauteil %s!\n", Zl_nr, name);
error:
   *btnr = 0;
   sync();
   return(TRUE);
}

/***************************************************************************
* Nachdem eine Zweig-Information fehlerfrei gelesen werden konnte, muû sie
* nun in der Knotenliste abgespeichert werden. Dabei wird sie sowohl beim
* Anfangs-, wie auch beim Endknoten gespeichert.
*/
static boolean einhaengen(bau_el, usp, usm, von, nach, qop, faktor, ref_el, name)
int bau_el, usp, usm, von, nach, qop;
int faktor;
zweig_info *ref_el;
char *name;
{
int i, pot;

   if(bau_el != OP)
   {
      if(fuell_zweiginfo(bau_el, usp, usm, von, nach, faktor, ref_el, name) < 0)
         return(TRUE);
      if(bau_el == QI) /* dann noch zwei Pseudozweige zur BerÅcksichtigung */
      {                /* des Steuerspannungseinflusses einhÑngen */
         if(fuell_zweiginfo(US, 0, 0, usp, von,  faktor, ref_el, "") < 0
         || fuell_zweiginfo(US, 0, 0, usm, nach, faktor, ref_el, "") < 0
         || fuell_zweiginfo(US, 0, 0, usp, nach, faktor, ref_el, "") < 0
         || fuell_zweiginfo(US, 0, 0, usm, von,  faktor, ref_el, "") < 0)
            return(TRUE);
      }
   }
   else
   {
      if(Kliste[qop].ist_qop) /* 2 OP-AusgÑnge an einem Knoten */
      {
         printf("OperationsverstÑrkerausgÑnge nicht zusammenschalten!\n");
         sync();
         return(TRUE);
      }
      else if(qop == U1p)
      {
         printf("OperationsverstÑrkerausgang nicht an Eingang schalten!\n");
         sync();
         return(TRUE);
      }
      Kliste[qop].ist_qop = TRUE;

      /* Bestimmung des relevanten Potentialknotens, default ist Knoten selbst */

      /* Potential des ersten Einganges ist auch das des anderen */
      pot = Kliste[nach].pot_nr; /* bisheriges Potential des 2. Einganges */
      /* nun in allen Knoten die pot als relevantes Potential haben, dieses */
      /* durch das neue Åberschreiben. */
      for(i=0; i<Anz_knoten; ++i)
         if(Kliste[i].pot_nr == pot)
            Kliste[i].pot_nr = Kliste[von].pot_nr;
      
      if(fuell_zweiginfo(OPZI, 0, 0, von, nach,  0, NULL, name) < 0
      || fuell_zweiginfo(OPQU, 0, 0, qop, Masse, 0, NULL, name) < 0)
         return(TRUE);
   }/* Ende if OP */      
   return(FALSE);
}

/* Hilfsroutine von einhaengen(): anlegen und beschreiben einer Zweig-Information */
/* Return-Kode ist Index des angelegten Elementes, -1 bei Fehler */

static int fuell_zweiginfo(bau_el, usp, usm, von, nach, faktor, ref_el, name)
int bau_el, usp, usm, von, nach, faktor;
zweig_info *ref_el;
char *name;
{
static vvek vvk=1;  /* nÑchste zu vergebende Variablendarstellung */
zweig *p;

   /* zunÑchst eine detaillierte Beschreibung des Zweiges im Felde */
   /* Zwliste[] ablegen */
   if(Anz_zweige == MAX_ZWEIGE) /* es lÑuft Åber */
   {
      printf("zu viele Zweige im Netz!\n");
      sync();
      return(-1);
   }
   /* sonst Daten eintragen */
   Zwliste[Anz_zweige].art = bau_el; /* was fÅr ein Bauelement? */
   Zwliste[Anz_zweige].usp = usp;    /* Anfangs- (= +) und Endknoten (= -) */
   Zwliste[Anz_zweige].usm = usm;    /* der Steuerspannung falls Quelle */
   Zwliste[Anz_zweige].ak  = von;    /* Anfangs- und Endknoten des Zweiges */
   Zwliste[Anz_zweige].ek  = nach;
   Zwliste[Anz_zweige].reffak  = faktor; /* Grîûenbeziehung zu einem anderen */
   Zwliste[Anz_zweige].refp = ref_el;    /* Bauteil */

   /* und nun noch eine interne Variable vergeben, falls nîtig */
   if(bau_el!=OPQU && bau_el!=OPZI && bau_el!=UIN && bau_el!=UOUT && bau_el!=US)
   {
      if(Anz_var >= MAX_VAR)
      {
         printf("zu viele Variablen!\n");
         sync();
         return(-1);
      }
      Zwliste[Anz_zweige].vv   = vvk;
      Zwliste[Anz_zweige].btnr = Anz_var;
      Zweigptr[Anz_var] = &Zwliste[Anz_zweige];/* Verweise auf Bauteile mit Namen */
      strcpy(Zwliste[Anz_zweige].name, name);/* symbolischen Namen eintragen */
      ++Anz_var;
      vvk <<= 1;
   }
   else
      Zwliste[Anz_zweige].name[0] = '\0', /* keine Name, Nullstring */
      Zwliste[Anz_zweige].btnr = -1;/* namenlose Bauteile brauchen keine Nummer */

   /* nachdem die Zweiginformation erstellt ist, kann ein Verweis darauf bei */
   /* den beiden beteiligten Knoten erfolgen */

   /* i) Anfangsknoten */
   p = balloc(ZWEIG); /* neues Element besorgen */
   /* und vorne einhÑngen und fÅllen */
   p->next = Kliste[von].zw1, Kliste[von].zw1 = p;
   p->was  = &Zwliste[Anz_zweige];
   
   /* ii) Endknoten, Steuerspannungseinfluû ist gerichtet, wird daher hier */
   /*                nicht eingehÑngt. */
   if(bau_el != US)
   {
      p = balloc(ZWEIG); /* neues Element besorgen */
      /* und vorne einhÑngen und fÅllen */
      p->next = Kliste[nach].zw1, Kliste[nach].zw1 = p;
      p->was  = &Zwliste[Anz_zweige];
   }
   /* Bestimmung der besonderen Knoten */
   if(bau_el == UIN) /* Eingangsspannungsknoten */
      if(U1p != -1)
      {
         printf("Ein");
         goto error1;
      }
      else
         U1p = von, Masse = nach;
    else if(bau_el == UOUT) /* Ausgangsspannungsknoten */
      if(U2p != -1)
      {
         printf("Aus");
error1:
         printf("gangsspannung wiederholt definiert in Zeile %d!\n", Zl_nr);
         sync();
         return(-1);
      }
      else
         U2p = von, U2m = nach;

   return(Anz_zweige++);
}

/***************************************************************************
* Titel der Ausgabedatei anlegen
*/
static void titel(name)
char *name;
{
time_t time32;
char *timeasc;

   time32 = time(NULL), timeasc = asctime(localtime(&time32));
   fprintf(Fpout, "LIN_NETZ - Das Programm zur Analyse linearer Netzwerke %s"
                  "Copyright (c) 1991 Peter Vranken\n"
                  "Bearbeitung von %s\n" , timeasc, name);
}

/***************************************************************************
* Hier wird eine Mîglichkeit realisiert, das Programm im Fehlerfalle abzu-
* brechen, ohne aus allen geschachtelten Unterprogrammen zurÅckspringen zu
* mÅssen.
*/
void abbruch(err_nr)
int err_nr;
{
   printf("Es trat ein Fehler auf (%d), der zum Programmabbruch fÅhrt!\n%s!\n",
           err_nr, Errors[err_nr]);
   TASTE;
   exit(-1);
}
