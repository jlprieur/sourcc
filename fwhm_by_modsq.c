/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
* Program fwhm_by_modsq 
* to compute the Full Width at Half Maximum of an equivalent long exposure
* with the power spectrum (square modulus of the FFT) 
* (by fitting a Kolmogorof law)
*
* JLP 
* Version 11-05-99
-------------------------------------------------------------------*/
/*
#define DEBUG
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <math.h>
#include <fftw.h>
#include <jlp_ftoc.h>

static int fit_kolmo_center(float *in_image, int nx, int ny,
                        float *sigx, float *sigy);
static int aris_r0calc(float *modsq, int nx, int ny, float *r0);

main(argc,argv)
int argc;
char *argv[];
{
char in_name[61], comments[81];
float *in_image, *testi;
INT_PNTR pntr_image;
INT4 istatus, nx, ny;
float sigx, sigy; 
register int i, j;

printf(" Program FWHM_BY_MODSQ to compute the Full Width at Half Maximum from the power spectrum)\n");
printf(" JLP Version 11-05-99 \n");

/* One parameters only is allowed to run the program: */
/* Carefull: 7 parameters always, using JLP "runs" */
if(argc == 7 && *argv[3]) argc = 4;
if(argc == 7 && *argv[2]) argc = 3;
if(argc == 7 && *argv[1]) argc = 2;
if(argc != 2 && argc != 1)
  {
  printf(" Syntax: fwhm_by_modsq filename\n"); 
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
/* Computing the power spectrum (for debug purpose) 
  testi = (float *)malloc(nx * ny * sizeof(float));
  for(i = 0; i < nx * ny; i++) testi[i] = 0.;

  fftw_float(in_image,testi,nx,ny,1);
  for(i = 0; i < nx * ny; i++) in_image[i] = in_image[i] * in_image[i]
                                             + testi[i] * testi[i];
*/

  fit_kolmo_center(in_image, nx, ny, &sigx, &sigy);

JLP_END();
}
/*****************************************************************
* To fit a Kolmogorof law to f1(xx,yy)
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
static int fit_kolmo_center(float *in_image, INT4 nx, INT4 ny,
                               float *sigx, float *sigy)
{
float *ff2, *gg; 
INT4 istatus, npts, ifail, nx1, ny1, istart, jstart, iw, jw;
float aa, bb, r0, fwhm; 
register int i, j;

nx1 = 20; ny1 = 20; 
istart = nx/2 - nx1/2;
jstart = ny/2 - ny1/2;
ff2 = (float *) malloc(nx1 * ny1 * sizeof(float));
gg = (float *) malloc(nx1 * ny1 * sizeof(float));
if(ff2 == NULL || gg == NULL)
  {
  printf("FWHM/Fatal error alocating memory space \n");
  exit(-1);
  }
#ifdef DEBUG
printf(" istart=%d jstart=%d nx1=%d ny1=%d\n",istart,jstart,nx1,ny1);
#endif
for(j = 0; j < ny1; j++)
  for(i = 0; i < nx1; i++)
   {
   iw = i + istart;
   jw = j + jstart;
   ff2[i + j * nx1] = (iw - nx/2) * (iw - nx/2) + (jw - ny/2) * (jw - ny/2); 
   gg[i + j * nx1] = in_image[iw + jw * nx];
   }
npts = nx1 * ny1;

jlp_fit_kolmogorof(ff2, gg, &npts, &aa, &bb, &ifail); 

/* JLP99: I notice that the ratio between long exposure determination
* and this program is about 15: */
fwhm = pow((double)(bb/6.88),-0.6)/15.;
/* fwhm = lambda /r0 */
r0 = 1./fwhm;
#ifdef DEBUG
printf(" Full Kolmogorov fit: aa=%f bb=%f r0=%f fwhm~%f (10 mm)\n",
         aa,bb,r0,fwhm); 
#else
printf(" Full Kolmogorov fit: r0=%f fwhm~%f (10 mm)\n",r0,fwhm); 
#endif

aris_r0calc(in_image, nx, ny, &r0);
/* To make it compatible with the previous fit: */
r0 *= 4.;
printf(" Eric Aristidi approximation: r0=%f fwhm=%f\n",r0,1./r0);

return(0);
}

/***************************************************************
* fit_kolmogorof
* To fit a Kolmogorof law to Log_intensity(|x|^2) as a function of
* in x^2 + y^2
*
*   g(f) = rho * exp [ -6.88 * (lambda * f / r_0)^5/3 ]
*
*   g(f) = rho * exp [ - b * f^5/3 ]    with b = 6.88 * (lambda / r_0)^5/3
*
*  Log_Intensity(x,y) 
*    G(f) = a - b f^(5/3)    with f^2 = x^2 + y^2 
*
*  where:
*  The problem is to find the coefficients (a, b)
*  which minimize the sums:
*   SUM on all the selected disk (x, y) of ( Log_intensity - G(x,y) ) **2
*
* The normal equation can be written as:  
*
*    n a - b SUM f^5/3 = SUM Log_g
*    a SUM f^5/3 - b SUM f^10/3 = SUM f^5/3 Log_g
*
* Hence:
*    a = 1/n * (SUM Log_g + b SUM f^5/3)
*    b = (SUM f^5/3 Log_g  - 1/n SUM Log_g SUM f^5/3)
*          / ( 1/n * SUM f^5/3 * SUM f^5/3 - SUM f^10/3)
*
*   where n is the number of points (SUM 1)
*
*
* gg: array of values gg(xx[i],yy[i])
*
* ff2: arrays of x^2 + y^2
* npts: number of points
*
* ifail = 0 if correct
*         -3: all values are negative or null!
*****************************************************************/
int jlp_fit_kolmogorof(float *ff2, float *gg, INT4 *npts,
                       float *aa, float *bb, INT4 *ifail)
{
/* gg measured intensities */
/* log_g log of measured intensities */
double *log_g, *ff;
double sum_g, sum_f1, sum_f2, sum_gf, w1;
register int i, j, k;

*ifail = 0;

 if((log_g = (double *) malloc(*npts * sizeof(double))) == NULL ||
     (ff = (double *) malloc(*npts * sizeof(double))) == NULL )
  {
  printf("jlp_fit_gauss/Error allocating memory for array (npts=%d)\n",*npts);
  *ifail = -1;
  return(-1);
  }

/* Transfer to double precision arrays and conversion of intensity to Log : */
i = 0;
for(k = 0; k < *npts; k++)
  {
       if(gg[k] > 0) 
         {
          log_g[i] = log((double)gg[k]); 
          ff[i] = ff2[k]; 
          i++;
         }
  }
*npts = i;

if(*npts == 0)
  {
  printf("jlp_fit_gauss/All input values are null!\n");
  *ifail = -1;
  return(-1);
  }

/* Compute the sums:  5/3 = 1.667 (but 5/6=0.833 since x^2+y^2)*/
/* Compute the sums:  10/3 = 0.833 (but 5/3=1.667 since x^2+y^2)*/
sum_f1 = 0.; sum_f2 = 0.;
sum_g = 0.; sum_gf = 0.;
for(k = 0; k < *npts; k++)
  {
  w1 = pow(ff[k],0.833);
  sum_f1 += w1; 
  sum_f2 += pow(ff[k],1.667);
  sum_g  += log_g[k];
  sum_gf += w1 * log_g[k];
  }

#ifdef DEBUG
printf("sum_f1=%f sum_f2=%f sum_g=%f sum_gf=%f npts=%d\n",
        sum_f1,sum_f2,sum_g,sum_gf,*npts);
#endif
/* Result: 
*    a = 1/n * (SUM Log_g + b SUM f^5/3)
*    b = (SUM f^5/3 Log_g  - 1/n SUM Log_g SUM f^5/3)
*          / ( 1/n * SUM f^5/3 * SUM f^5/3 - SUM f^10/3)
*/
  *bb = (sum_gf - (1. / *npts) * sum_g * sum_f1)  
           / ( (1. / *npts) * sum_f1 * sum_f1 - sum_f2);
  *aa = (1. / *npts) * (sum_g + *bb * sum_f1);

return(0);
}
/**********************************************************/
/* Eric Aristidi's fit (approximation only) */
static int aris_r0calc(float *modsq, int nx, int ny, float *r0)
{
int     f12_1, f12_2, dim, ixc, iyc;
float   pix,lambda,x,y1,y2,r01,r02;
dim = nx;
ixc = nx/2;
iyc = ny/2;

 x=modsq[ixc + iyc*dim];

 f12_1=4;
 y1=modsq[(ixc+f12_1) + iyc * dim] + modsq[ixc + (iyc+f12_1) * dim];
 y1/=2.;

 f12_2=3;
 y2=modsq[(ixc+f12_2) + iyc * dim] + modsq[ixc + (iyc+f12_2) * dim];
 y2/=2.;

//  Compute r0
//  ---------
 pix=0.0119/206265.;lambda=0.00000065;
 r01=pow(3.44/log(x/y1),0.6)*lambda*(float)f12_1/pix/dim;
 r02=pow(3.44/log(x/y2),0.6)*lambda*(float)f12_2/pix/dim;
     *r0=(r02+r01)/2.;
}

