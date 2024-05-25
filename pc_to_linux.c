/*************************************************************************
* Program pc_to_linux
* To convert IBM PC extended keyboard to ISO 8859-1 ASCII accents
*
* 1. Conversion PC -> TEX     (accents to TeX)
* 2. Conversion LINUX -> TEX  (accents to TeX)
* 3. Conversion PC -> LINUX   (keeping the accents)
* 4. Conversion LINUX -> PC   (keeping the accents)
* 5. Conversion TEX -> PC     (TeX to accents)
* 6. Conversion TEX -> LINUX  (TeX to accents)
* 7. Conversion DOS -> LINUX  (Removes ^M)
*
* JLP
* Version 02/04/2011
*************************************************************************/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#define NMAX 40

/*
#define DEBUG
*/

static int compute_conversion(unsigned int *pc_in, unsigned int *linux_out, 
                             char *tex_out, int *nn);
static int load_conversion(unsigned int *pc_in, unsigned int *linux_out, 
                           char *tex_out, int *nn);
static int accent_to_tex(unsigned int *pc_in, char *tex_out, FILE *fp_in,
                     FILE *fp_out, int nn);
static int tex_to_accent(unsigned int *pc_in, char *tex_out, FILE *fp_in,
                         FILE *fp_out, int nn);
static int pc_to_linux(unsigned int *pc_in, unsigned int *linux_out,
                       FILE *fp_in, FILE *fp_out, int nn);
static int remove_CR(unsigned int *pc_in, FILE *fp_in, FILE *fp_out);

int main(int argc, char *argv[])
{
int status, nn, iopt;
unsigned int pc_in[NMAX],linux_out[NMAX];
char tex_out[6*NMAX], filein[60], fileout[60];
FILE *fp_in, *fp_out;

if(argc == 7 && *argv[3]) argc = 4;
if(argc == 7 && *argv[2]) argc = 3;
if(argc == 7 && *argv[1]) argc = 2;
if(argc != 4)
  {
  printf(" Syntax: pc_to_linux option in_file out_file \n");
  printf(" Options available: (PC = IBM PC extended keyboard) (LINUX=ISO 8859-1 ASCII)\n");
  printf(" 1. Conversion PC -> TEX     (accents to TeX) \n");
  printf(" 2. Conversion LINUX -> TEX  (accents to TeX)\n");
  printf(" 3. Conversion PC -> LINUX   (keeping the accents)\n");
  printf(" 4. Conversion LINUX -> PC   (keeping the accents)\n");
  printf(" 5. Conversion TEX -> PC     (TeX to accents)\n");
  printf(" 6. Conversion TEX -> LINUX  (TeX to accents)\n");
  printf(" 7. Conversion DOS -> LINUX  (removes ^M)\n");
  printf(" 10. Exit \n");
  printf(" Enter the option you want : ");
  iopt = 0;
  scanf("%d",&iopt);
/* Read it twice, just in case ... */
  if(!iopt) scanf("%d",&iopt);
  if(iopt < 1 || iopt > 7) { 
     fprintf(stderr," Fatal error, iopt= %d\n", iopt); 
     exit(-1);
     }
  printf("Input file : \n"); scanf("%s",filein);
  printf("Output file (different from input file!): \n"); scanf("%s",fileout);
  }
else
  {
  sscanf(argv[1],"%d",&iopt);
  strcpy(filein,argv[2]);
  strcpy(fileout,argv[3]);
  }

printf(" OK iopt=%d filein=%s fileout=%s\n",iopt,filein,fileout);

/* Two possibilities: */
#if 0
status = compute_conversion(pc_in,linux_out,tex_out,&nn);
#else
status = load_conversion(pc_in,linux_out,tex_out,&nn);
if(status) exit(-1);
#endif

if((fp_in = fopen(filein,"r")) == NULL)
{
printf(" Fatal error opening input file %s \n",filein);
exit(-1);
}

if((fp_out = fopen(fileout,"w")) == NULL)
{
printf(" Fatal error opening output file %s \n",fileout);
fclose(fp_in);
exit(-1);
}

/* Scan the file and make the conversion: */
switch (iopt)
 {
/* pc_to_tex */
 case 1:
  accent_to_tex(pc_in,tex_out,fp_in,fp_out,nn);
  break;
/* linux_to_tex */
 case 2:
  accent_to_tex(linux_out,tex_out,fp_in,fp_out,nn);
  break;
/* pc_to_linux */
 case 3:
  pc_to_linux(pc_in,linux_out,fp_in,fp_out,nn);
  break;
/* linux_to_pc: the trick is just to invert the first two arguments */
 case 4:
  pc_to_linux(linux_out,pc_in,fp_in,fp_out,nn);
  break;
/* tex_to_pc */
 case 5:
  tex_to_accent(pc_in,tex_out,fp_in,fp_out,nn);
  break;
/* tex_to_linux */
 case 6:
  tex_to_accent(linux_out,tex_out,fp_in,fp_out,nn);
/* tex_to_linux */
 case 7:
  remove_CR(pc_in,fp_in,fp_out);
  break;
 default:
  printf("Invalid option \n");
  exit(-1);
  break;
 }

fclose(fp_in);
fclose(fp_out);
return(0);
}
/************************************************************
* To compute the correspondence between PC and linux with 
* the "ibmkbd.tex" file (extended IBM keyboard)
*
************************************************************/
static int compute_conversion(unsigned int *pc_in, unsigned int *linux_out,
                              char *tex_out, int *nn)
{
unsigned char c1;
char filename[60], c2, cc[5], buffer[80];
int k, kk;
FILE *fp1, *fp2;

strcpy(filename,"/d/execlnx/ibmkbd.tex");
if((fp1 = fopen(filename,"r")) == NULL)
{
printf("Fatal error opening input file %s \n",filename);
return(-1);
}

strcpy(filename,"test000.tex");
if((fp2 = fopen(filename,"w")) == NULL)
{
printf("Fatal error opening output file %s \n",filename);
return(-1);
}

k = 0;
while(!feof(fp1))
{
  if(fgets(buffer,80,fp1))
  {
    if(!strncmp(buffer,"\\def ",5))
    {
#ifdef DEBUG
    printf("%s\n",buffer);
#endif
    sscanf(buffer,"\\def %c\{%c%c%c%c%c%c",
           &c1,&c2,&cc[0],&cc[1],&cc[2],&cc[3],&cc[4]);
    pc_in[k] = (unsigned int)c1;
    kk = 6*k;
/*
    if(c2 == '\\')
*/
/* To read all symbols: */
    if(c2)
      {
      tex_out[kk] = cc[0];
      tex_out[++kk] = cc[1];
      if(cc[2] != '}') 
        {
        tex_out[++kk] = cc[2];
        if(cc[3] != '}') 
          {
          tex_out[++kk] = cc[3];
          if(cc[4] != '}') tex_out[++kk] = cc[4];
          }
        }
      tex_out[++kk] = '\0';
      printf("in=%d  tex_out=\\%s\n",pc_in[k],&tex_out[6*k]);
      fprintf(fp2," %d  \\%s \n\n",k,&tex_out[6*k]);
      k++;
      }
    }
  }
}
printf(" %d values read\n",k);
*nn = k-1;

fclose(fp1);
fclose(fp2);
return(0);
}
/******************************************************************
*
* IBM PC:
* Code ASCII étendu OEM
* Extended OEM ASCII code
*
* Linux (ISO 8859)
* Code ASCII étendu ANSI 
* Extended ANSI code 
******************************************************************/
static int load_conversion(unsigned int *pc_in, unsigned int *linux_out,
                           char *tex_out, int *nn)
{
int i;
i=-1;
pc_in[++i] = 128; linux_out[i] = 199; sprintf(&tex_out[6*i],"\\cC");  
pc_in[++i] = 129; linux_out[i] = 252; sprintf(&tex_out[6*i],"\\\"u");  
pc_in[++i] = 130; linux_out[i] = 233; sprintf(&tex_out[6*i],"\\'e");  
pc_in[++i] = 131; linux_out[i] = 226; sprintf(&tex_out[6*i],"\\^a");  
pc_in[++i] = 132; linux_out[i] = 228; sprintf(&tex_out[6*i],"\\\"a");  
pc_in[++i] = 133; linux_out[i] = 224; sprintf(&tex_out[6*i],"\\`a");  
pc_in[++i] = 134; linux_out[i] = 229; sprintf(&tex_out[6*i],"\\aa ");  
pc_in[++i] = 135; linux_out[i] = 231; sprintf(&tex_out[6*i],"\\c c");  
pc_in[++i] = 136; linux_out[i] = 234; sprintf(&tex_out[6*i],"\\^e");  
pc_in[++i] = 137; linux_out[i] = 235; sprintf(&tex_out[6*i],"\\\"e");  
pc_in[++i] = 138; linux_out[i] = 232; sprintf(&tex_out[6*i],"\\`e");  
pc_in[++i] = 139; linux_out[i] = 239; sprintf(&tex_out[6*i],"\\\"\\i ");  
pc_in[++i] = 140; linux_out[i] = 238; sprintf(&tex_out[6*i],"\\^\\i ");  
pc_in[++i] = 141; linux_out[i] = 236; sprintf(&tex_out[6*i],"\\`\\i ");  
pc_in[++i] = 142; linux_out[i] = 196; sprintf(&tex_out[6*i],"\\\"A");  
pc_in[++i] = 143; linux_out[i] = 197; sprintf(&tex_out[6*i],"\\AA");  
pc_in[++i] = 144; linux_out[i] = 201; sprintf(&tex_out[6*i],"\\'E");  
pc_in[++i] = 145; linux_out[i] = 230; sprintf(&tex_out[6*i],"\\ae ");  
pc_in[++i] = 146; linux_out[i] = 198; sprintf(&tex_out[6*i],"\\AE ");  
pc_in[++i] = 147; linux_out[i] = 244; sprintf(&tex_out[6*i],"\\^o");  
pc_in[++i] = 148; linux_out[i] = 246; sprintf(&tex_out[6*i],"\\\"o");  
pc_in[++i] = 149; linux_out[i] = 242; sprintf(&tex_out[6*i],"\\`o");  
pc_in[++i] = 150; linux_out[i] = 251; sprintf(&tex_out[6*i],"\\^u");  
pc_in[++i] = 151; linux_out[i] = 249; sprintf(&tex_out[6*i],"\\`u");  
pc_in[++i] = 152; linux_out[i] = 377; sprintf(&tex_out[6*i],"\\\"y");  
pc_in[++i] = 153; linux_out[i] = 214; sprintf(&tex_out[6*i],"\\\"O");  
pc_in[++i] = 154; linux_out[i] = 220; sprintf(&tex_out[6*i],"\\\"U");  
pc_in[++i] = 160; linux_out[i] = 225; sprintf(&tex_out[6*i],"\\'a");  
pc_in[++i] = 161; linux_out[i] = 237; sprintf(&tex_out[6*i],"\\'\\i ");  
pc_in[++i] = 162; linux_out[i] = 243; sprintf(&tex_out[6*i],"\\'o");  
pc_in[++i] = 163; linux_out[i] = 250; sprintf(&tex_out[6*i],"\\'u");  
pc_in[++i] = 164; linux_out[i] = 241; sprintf(&tex_out[6*i],"\\~n");  
pc_in[++i] = 165; linux_out[i] = 209; sprintf(&tex_out[6*i],"\\~N");  
pc_in[++i] = 168; linux_out[i] = 191; sprintf(&tex_out[6*i],"\\?'");  
pc_in[++i] = 173; linux_out[i] = 161; sprintf(&tex_out[6*i],"\\!'");  

*nn = i+1;
#ifdef DEBUG
for(i = 0; i < *nn; i++) printf("%i pc=%i tex=\\%s linux=%i \n",
                            i,pc_in[i],&tex_out[6*i],linux_out[i]);
#endif

return(0);
}
/*************************************************************************
* From IBM extended keyboard to "standard TeX" files
*************************************************************************/
static int accent_to_tex(unsigned int *pc_in, char *tex_out, FILE *fp_in,
                     FILE *fp_out, int nn)
{
char b_in[120],b_out[180];
char *pci, *pco;
int uint0;
int i;

while(!feof(fp_in))
{
  if(fgets(b_in,120,fp_in))
  {
  pci = b_in;
  pco = b_out;
  while(*pci)
    {
    *pco = *pci;
/* 2013: pb with unsigned int conversion, so I do it manually: */
     uint0 = (int)(*pco);
     if((int)uint0 < 0) uint0 += 256;
    for(i = 0; i < nn; i++)
     {
     if(uint0 == pc_in[i])
        {
         sprintf(pco,"%s",&tex_out[6*i]);
#ifdef DEBUG
printf(" pci=%s, pco=%s \n",pci,pco);
#endif
         while(*pco) pco++;
         pco--;
         break;
        }
     }
    pci++;
    pco++;
    } 
  *pco = '\0';
  fputs(b_out,fp_out);
  }
}
return (0);
}
/*************************************************************************
* From "standard TeX" to IBM extended keyboard files
* I only process the accents 
*  (\'e \`e,\`a,\^e,\`u,\^u,\^\i,\c c)
*************************************************************************/
static int tex_to_accent(unsigned int *pc_in, char *tex_out, FILE *fp_in,
                     FILE *fp_out, int nn)
{
char b_in[120],b_out[180];
char *pci, *pco;
int i;

while(!feof(fp_in))
{
  if(fgets(b_in,120,fp_in))
  {
  pci = b_in;
  pco = b_out;
  while(*pci)
    {
    *pco = *pci;
    pci++;
/* Case \'a,é,\'i,\'o,\'u */
      if(*pco == '\\')
      {
/* I only process the accents: */
        if(*pci == '\'' || *pci == '"' 
           || *pci == '^' || *pci == '`') 
        {
        for(i = 0; i < nn; i++)
         {
           if(!strncmp(pci,&tex_out[6*i],2))
           {
/* Case of \i */
           if(tex_out[6*i+2] == 'i')
             {
             if(!strncmp(pci,&tex_out[6*i],4))
               {
               *pco = (unsigned char)pc_in[i];
               pci += 4;
               break;
               }
             }
/* Simpler case of an accent on a,e,i,o,u: */
           else
             {
             *pco = (unsigned char)pc_in[i];
             pci += 2;
             break;
             }
           }
         }
        }
      }
    pco++;
    } 
  *pco = '\0';
  fputs(b_out,fp_out);
  }
}
return (0);
}
/*************************************************************************
* Keeping the accents in both systems
* Conversion from extended IBM keyboard to ISO 8859-1 ASCII accents
*************************************************************************/
static int pc_to_linux(unsigned int *pc_in, unsigned int *linux_out,
                       FILE *fp_in, FILE *fp_out, int nn)
{
char b_in[120];
char *pc;
int uint0;
int i;

while(!feof(fp_in))
{
  if(fgets(b_in,120,fp_in))
  {
  pc = b_in;
  while(*pc)
    {
/* 2013: pb with unsigned int conversion, so I do it manually: */
     uint0 = (int)(*pc);
     if((int)uint0 < 0) uint0 += 256;
    for(i = 0; i < nn; i++)
     {
     if(uint0 == pc_in[i])
        {
         *pc = (unsigned char)linux_out[i];
#ifdef DEBUG
printf(" *pc=%c, pc_in=%d linux_out=%d \n",*pc,pc_in[i],linux_out[i]);
#endif
         break;
        }
     }
// Conversion of CR to ' ':
    if(*pc == '\r') *pc = ' '; 
    pc++;
    } 
  fputs(b_in,fp_out);
  }
}
return (0);
}
/*************************************************************************
* Remove the CR at the end of the lines 
*************************************************************************/
static int remove_CR(unsigned int *pc_in, FILE *fp_in, FILE *fp_out)
{
char b_in[120];
char *pc;

while(!feof(fp_in))
{
  if(fgets(b_in,120,fp_in))
  {
  pc = b_in;
  while(*pc) {
// Conversion of CR to ' ':
    if(*pc == '\r') *pc = ' '; 
    pc++;
    } 
  fputs(b_in,fp_out);
  }
}
return (0);
}
