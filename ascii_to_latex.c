/*************************************************************************
* Program ascii_to_latex_to_ascii
* To convert an ASCII list to a LaTeX table
*
*
* JLP
* Version 15/02/2008
*************************************************************************/
#include <stdio.h>
#include <math.h>
#include <string.h>

/* Maximum length for one line will be 170 characters: */
#define NMAX 170
 
#define DEBUG
/*
*/

static int jlp_ascii_to_latex(FILE *fp_in, FILE *fp_out, int *iy, int nn);
int read_int_array_from_string(char *buffer, int len, int *out, int nmax, int *nn);

int main(int argc, char *argv[])
{
char filein[60], fileout[60], buffer[80];
int iy[20], nmax = 20, nn = 0;
register int i;
FILE *fp_in, *fp_out;

  printf("ascii_to_latex/ JLP/ Version 15/02/2008\n");


if(argc == 7 && *argv[4]) argc = 5;
if(argc == 7 && *argv[3]) argc = 4;
if(argc == 7 && *argv[2]) argc = 3;
if(argc == 7 && *argv[1]) argc = 2;
if(argc != 4)
  {
  printf("Error: argc=%d\n\n", argc);
  printf("Syntax: ascii_to_latex in_ascii_list out_latex_table iy1,iy2,iy3,iy4,iy5,...,iyn\n");
  printf("\n(iy1,iy2, etc are the location of the separators (blanks) max=%d)\n", nmax);
  return(-1);
  }

 strcpy(filein,argv[1]);
 strcpy(fileout,argv[2]);
 strcpy(buffer,argv[3]);

read_int_array_from_string(buffer, 80, iy, nmax, &nn);
printf(" OK: filein=%s fileout=%s nn=%d\n", filein, fileout, nn);
for(i = 0; i < nn; i++) printf(" iy[%d]=%d", i, iy[i]);
printf("\n");


if((fp_in = fopen(filein,"r")) == NULL)
{
printf(" Fatal error opening input file %s \n",filein);
return(-1);
}

if((fp_out = fopen(fileout,"w")) == NULL)
{
printf(" Fatal error opening output file %s \n",fileout);
fclose(fp_in);
return(-1);
}

/* Scan the file and make the conversion: */
  jlp_ascii_to_latex(fp_in,fp_out, iy, nn);

fclose(fp_in);
fclose(fp_out);
return(0);
}
/*********************************************************************************
* read_int_array_from_string
*
* INPUT:
* buffer: string to be read
* len: length of buffer
* nmax: maximum number of values to be read
*
* OUTPUT:
* nn: number of values read
* out[nn]: integer array
*********************************************************************************/
int read_int_array_from_string(char *buffer, int len, int *out, int nmax, int *nn)
{
register int i;
int ival, nval;
char *pc;

for(i = 0; i < nmax; i++) out[i] = 0;
*nn = 0;

pc = buffer;
for(i = 0; i < len; i++){
  if(buffer[i] == '\0' || buffer[i] == ',') {
    nval = sscanf(pc,"%d", &ival);
    if(nval == 1) {
       out[*nn] = ival; 
       (*nn)++;
       if(*nn >= nmax) { 
         if(buffer[i] !='\0') 
             fprintf(stderr,"Error: max number of values =%d)\n", nmax);
         break; 
       }
    }
    if(buffer[i] == '\0') break;
/* Next step: */
    i++;
    pc = &buffer[i];
  }
}   /* EOF for i=0, i< len */

return(0);
}
/*************************************************************************
*
* INPUT:
* iy[nn]: array with the location indices of the separations (blanks)
* nn : number of separation indices
*************************************************************************/
static int jlp_ascii_to_latex(FILE *fp_in, FILE *fp_out, int *iy, int nn)
{
char b_in[NMAX], b_out[NMAX+100];
register int i, j, k, l;

while(!feof(fp_in))
{
/* Maximum length for a line will be 170 characters: */
  if(fgets(b_in,NMAX,fp_in))
  {
  k = 0;
  j = 0;
  for(i = 0; i < NMAX; i++){
      if(b_in[i] == '\n' || b_in[i] == '\0' || b_in[i] == '\r') break;
      if(i == iy[k]) {
      b_out[j++] = '&';
      b_out[j++] = ' ';
      b_out[j++] = b_in[i];
      k++;
      if(k == nn) break;
      } else {
      b_out[j++] = b_in[i];
      }
    }
/*  Completes the line with the required number of separators if needed */
  b_out[j++] = ' ';
  for(l = k+1; l < nn; l++){
    b_out[j++] = '&';
    b_out[j++] = ' ';
    }
  b_out[j++] = '\\';
  b_out[j++] = '\\';
  b_out[j++] = '\0';
printf("in=>%s<\n", b_in);
printf("out=>%s<\n", b_out);
  fprintf(fp_out,"%s\n", b_out); 
  }
} /* EOF while !feof(fp_in) */
return(0);
}
