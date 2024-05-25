/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
* Program normalize_modsq 
* To normalize to the central value
*
* JLP 
* Version 27-02-00
-------------------------------------------------------------------*/
#define DEBUG

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <math.h>
#include <jlp_ftoc.h>

static int normalize_center(float *in_image, float *out_image,
                            INT4 nx, INT4 ny);

main(argc,argv)
int argc;
char *argv[];
{
char in_name[61], out_name[61], comments[81];
float *in_image, *out_image, x_sig; 
INT_PNTR pntr_image;
INT4 istatus, nx, ny, npts, ifail;
int i, j;

printf(" Program normalize_modsq to normalize to the central value \n");
printf(" JLP Version 27-02-00 \n");

/* One parameters only is allowed to run the program: */
/* Carefull: 7 parameters always, using JLP "runs" */
if(argc == 7 && *argv[3]) argc = 4;
if(argc == 7 && *argv[2]) argc = 3;
if(argc == 7 && *argv[1]) argc = 2;
if(argc != 3 && argc != 1)
  {
  printf(" Syntax: normalize_modsq in out\n"); 
  printf(" Fatal: Syntax error: argc=%d\n",argc);
  exit(-1);
  }

/* Input of parameters with the command line: */
if (argc == 3 )
 { 
  strcpy(in_name,argv[1]);
  strcpy(out_name,argv[2]);
 }
/* Interactive input of parameters: */
else
 { 
  printf(" Input file := ");scanf("%s",in_name);
  printf(" Output file := ");scanf("%s",out_name);
 }

/**********************************************************/
  JLP_BEGIN();
  JLP_INQUIFMT();
  istatus = JLP_VM_READIMAG1(&pntr_image,&nx,&ny,in_name,comments);
  if(istatus != 0) exit(-1);

  in_image = (float *) pntr_image;

  out_image = (float *) malloc(nx * ny * sizeof(float));
  if(out_image == NULL)
  {
  printf("FWHM/Fatal error alocating memory space \n");
  exit(-1);
  }
  if(!normalize_center(in_image,out_image,nx,ny))
    JLP_WRITEIMAG(out_image,&nx,&ny,&nx,out_name,comments);

JLP_END();
}
/******************************************************************
*
* Detects the first edge in each column and recentre the column 
*******************************************************************/
static int normalize_center(float *in_image, float *out_image,
                            INT4 nx, INT4 ny)
{
float ww;
int i;

ww = in_image[(nx/2) + (ny/2) * nx];
printf(" Central value was: %f\n",ww);

if(ww == 0) 
     {
      printf("normalize_center/Fatal error: null value!\n");
      return(-1);
     }

for(i = 0; i < nx * ny; i++) out_image[i] = in_image[i]/ww;
   
return(0);
}
