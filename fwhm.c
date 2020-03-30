/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
* Program fwhm to compute the Full Width at Half Maximum of a long exposure
* (from speckle data integration) 
*
* JLP 
* Version 08-03-99
-------------------------------------------------------------------*/
/*
#define DEBUG
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <math.h>
#include <jlp_ftoc.h>

main(argc,argv)
int argc;
char *argv[];
{
char in_name[61], comments[81];
float *in_image, *xx, *yy, *f1; 
INT_PNTR pntr_image;
INT4 istatus, nx, ny, npts, ifail;
float sigx, sigy, xc, yc, rho, errors[5], sky_level; 
register int i, j;

printf(" Program FWHM to compute the Full Width at Half Maximum of a long integration\n");
printf(" JLP Version 08-03-99 \n");

/* One parameters only is allowed to run the program: */
/* Carefull: 7 parameters always, using JLP "runs" */
if(argc == 7 && *argv[3]) argc = 4;
if(argc == 7 && *argv[2]) argc = 3;
if(argc == 7 && *argv[1]) argc = 2;
if(argc != 2 && argc != 1)
  {
  printf(" Syntax: fwhm filename\n"); 
  printf(" Fatal: Syntax error: argc=%d\n",argc);
  exit(-1);
  }

/* Interactive input of parameters: */
if (argc == 2 )
 { 
  strcpy(in_name,argv[1]);
 }
else
 { 
  printf(" Input file := ");scanf("%s",in_name);
 }

/**********************************************************/
  JLP_BEGIN();
  JLP_INQUIFMT();
  istatus = JLP_VM_READIMAG1(&pntr_image,&nx,&ny,in_name,comments);
  if(istatus != 0) exit(-1);

  in_image = (float *) pntr_image;

/*****************************************************************
* To fit a Gaussian to f1(xx,yy)
*
* f1: array of values f1(xx[i],yy[i])
* xx, yy: arrays of coordinates x, y
* npts: number of points
*
* Parameters:
* sigx, sigy, xc, yc, rho
* error[sigx,sigy,xc,yc,rho]
*
* ifail = 0 if correct
*         -3: all values are negative or null!
*****************************************************************/
xx = (float *) malloc(nx * ny * sizeof(float));
yy = (float *) malloc(nx * ny * sizeof(float));
f1 = (float *) malloc(nx * ny * sizeof(float));
if(xx == NULL || yy == NULL || f1 == NULL)
  {
  printf("FWHM/Fatal error alocating memory space \n");
  exit(-1);
  }
/* JLP99: I remove the background of the sky to avoid problems */
auto_sky(in_image,nx,ny,&sky_level);

for(j = 0; j < ny; j++)
  for(i = 0; i < nx; i++)
   {
   xx[i + j * nx] = i;
   yy[i + j * nx] = j;
   f1[i + j * nx] = in_image[i + j * nx] - sky_level;
   }
npts = nx * ny;

jlp_fit_gauss(xx, yy, f1, &npts, &sigx, &sigy, &xc, &yc,
                  &rho, errors, &ifail);

printf(" xc=%.2f yc=%.2f sigx=%.2f sigy=%.2f, rho=%.2f \n",
         xc, yc, sigx, sigy, rho);
/* 2.35 sigma if Gaussian x^2/(2 sig^2)
 But here only x^2 / sig^2, hence 2.35/sqrt(2)=1.67
*/
         sigx = 1.67 * (sigx + sigy)/2.;
printf(" FWHM=Mean_sigm*1.67=%.2f (in arcsec: %.2f (10mm), %.2f (20mm)\n",
          sigx, sigx * 0.0109, sigx * 0.0258);
JLP_END();
}
