/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
* Program write ti ASCII format 
*
* JLP 
* Version 10-06-99
-------------------------------------------------------------------*/

/*
#define DEBUG
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <jlp_ftoc.h>


main(argc,argv)
int argc;
char *argv[];
{
char in_name[61], out_name[61], comments[81], generic_name[61], *pc;
INT_PNTR pntr_ima;
INT4 nx, ny;
float *ima;
int i;
FILE *fp;

printf(" Program rbdf to read BDF Starlink image files and convert to FITS format\n");
printf(" JLP Version 20-05-97 \n");

/* One or three parameters only are allowed to run the program: */
/* Carefull: 7 parameters always, using JLP "runs" */
if(argc == 7 && *argv[3]) argc = 4;
if(argc == 7 && *argv[2]) argc = 3;
if(argc == 7 && *argv[1]) argc = 2;
if(argc != 2 && argc != 3)
  {
  printf(" Syntax: wascii in_fname (assuming *.fits *.asc) \n"); 
  printf(" or :  wascii in_fname out_fname \n\n"); 
  printf(" Fatal: Syntax error: argc=%d\n",argc);
  exit(-1);
  }

/* Interactive input of parameters: */
if (argc == 3 )
 {
  strcpy(in_name,argv[1]);
  strcpy(out_name,argv[2]);
 }
else if (argc == 2)
 {
  strcpy(generic_name,argv[1]);
  pc = generic_name;
  while(*pc && *pc != '.') pc++;
  *pc = '\0';
  sprintf(in_name,"%s.fits",generic_name);
  sprintf(out_name,"%s.asc",generic_name);
  printf(" Input: %s    Output: %s \n",in_name,out_name);
 }
else
 { 
  printf(" Syntax: wascii in_file out_file \n\n"); 
  printf(" Input file := ");scanf("%s",in_name);
  printf(" Output file := ");scanf("%s",out_name);
 }

/**********************************************************/
JLP_BEGIN();
JLP_INQUIFMT();


#ifdef DEBUG
printf(" OK, will read >%s< \n",in_name);
#endif

/* Calling reading subroutine: */
  JLP_VM_READIMAG1(&pntr_ima,&nx,&ny,in_name,comments);
  ima = (float *)pntr_ima;

  printf("Conversion of ASCII file: %s to: %s\n",in_name,out_name);
  
  if((fp=fopen(out_name,"w")) == NULL)
   {
   printf("Error writing output file: %s \n",out_name);
   }
  for(i = 0; i < nx*ny; i++) fprintf(fp,"%14.7e ",ima[i]);
  fclose(fp);

JLP_END();
}
