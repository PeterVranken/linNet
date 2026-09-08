/***************************************************************************
*                                                                          *
* Dieses Programm wertet einen String als komplette Pfadangabe einer Datei *
* aus. Die Åbergebene Struktur enthÑlt nach Aufruf 5 Strings die mit       *
* -) kompletter Eingabe, -) komplettem Pfad, -) Dateiname mit Erweiterung  *
* -) dito ohne, jedoch noch mit '.' und -) Erweiterung gefÅllt sind        *
*                                                                          *
***************************************************************************/

/*
   (c) Peter Vranken 6/89
*/

#define fuerImport 1  /* d.h, dass main() nicht Åbersetzt wird */

#define erlaube_Leername 0
         /* wenn auch reine Pfadangabe mîglich sein soll, kann */
         /* hiermit der Leerstring als Dateiname zugelassen werden */

#include <stdio.h>
#include <string.h>

#define  fuer_filename.c
#include "filename.h"

/* Prototypes der nicht exportierten Funktionen */

int  isbuch(char);
int  Name(char*, char*, char*);
#ifndef fuerImport
  int  main(int, char**);
#endif

/***************************************************************************/

#define islaufwk(c) ((c)>='a' && (c)<='p' || (c)>='A' && (c)<='P')

static int isbuch(c)
char c;
{
   return(c>='a' && c<='z' || c>='A' && c<='Z' || c>='0' && c<='9' || c=='_' || c=='?' || c=='*');
}

void dateiname(nptr, filename)
d_name *nptr;
char *filename;
{
char c1, c2, *namanf;
int i;

   FN_ptr = filename;
   FN_err = 0;
   c1 = *FN_ptr;
   if(islaufwk(c1) && (c2= *(FN_ptr+1))==':')
   {
      nptr->komplett[0] = c1;
      nptr->komplett[1] = c2;
      FN_ptr += 2;
      namanf = nptr->komplett+2;
      i = 2;
   }
   else
   {
      namanf = nptr->komplett;
      i = 0;
   }
   if( (c1=*FN_ptr) ==' ' || !c1)
   {
#if !erlaube_Leername
      FN_err = 1;
      return;
#else
      nptr->n_o_ext[0] = nptr->ext[0] = '\0';/* n_m_ext wird unten gesetzt */
#endif
   }
   else if(c1!='\\')
   {
      i += Name(namanf=nptr->komplett+i, nptr->ext, nptr->n_o_ext);
      if( *FN_ptr != ' '  &&  *FN_ptr != '\0' && *FN_ptr !='\\')
         FN_err=1;
      if(FN_err)
         return;
   }
   while(*FN_ptr=='\\')
   {
      FN_ptr++;
      nptr->komplett[i++] = '\\';
      i += Name(namanf=nptr->komplett+i, nptr->ext, nptr->n_o_ext);
      if( !((c1= *FN_ptr)=='\\' || c1==' ' || c1=='\0') )
         FN_err=1;
      if(FN_err)
         return;
   }

   if(!i)
   {
      FN_err = 1;
      return;
   }
   nptr->komplett[i] = '\0';
   strcpy(nptr->n_m_ext, namanf);
   c1 = *namanf;
   *namanf = '\0';
   strcpy(nptr->pfad, nptr->komplett);
   *namanf = c1;
}

/***************************************************************************/

static int Name(ziel, ext, name_o_ext)
char *ziel, *ext, *name_o_ext;
{
int i=0, t=0;

   if(!isbuch(*FN_ptr))
   {
#if !erlaube_Leername
      FN_err = 1;
#endif
      *ext = *name_o_ext = '\0';
      return(0);
   }
   name_o_ext[i] = *FN_ptr;
   ziel[i++]     = *FN_ptr++;
   while(i<8)
      if(isbuch(*FN_ptr))
      {
         name_o_ext[i] = *FN_ptr;
         ziel[i++]     = *FN_ptr++;
      }
      else
         break;

   name_o_ext[i]   = '.';
   name_o_ext[i+1] = '\0';
   if(*FN_ptr!='.')
   {
      *ext = '\0';
      return(i);
   }
   FN_ptr++; ziel[i++] = '.';

   while(t<3)
      if(isbuch(*FN_ptr))
      {
         ext[t++]  = *FN_ptr;
         ziel[i++] = *FN_ptr++;
      }
      else
         break;

   ext[t] = '\0';
   return(i);
}

/***************************************************************************/

#if !fuerImport

              /* HAUPTPROGRAMM (nur als Anwendungsbeispiel) */

int main()
{
char buf[129];
d_name filename;

   do
   {
      puts("Bitte einen Dateinamen mit Pfad angeben ('q' fÅr Ende):");
      gets(buf);
      if(buf[0]=='q')
         break;
      FN_err = 0;
      dateiname(&filename, buf);
      if(FN_err)
      {
         puts("Syntaxfehler!");
         continue;
      }
      printf("komplett: >%s<\n", filename.komplett);
      printf("Pfad:     >%s<\n", filename.pfad);
      printf("Programm: >%s<\n", filename.n_m_ext);
      printf("ohne Ext.:>%s<\n", filename.n_o_ext);
      printf("Extension:>%s<\n", filename.ext);
   }
   while(1);
   return(0);
}
#endif
