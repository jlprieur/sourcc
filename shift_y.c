/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
* Program shift_y to shift the columns according to the barycenter
* computed for each column. 
* Used for aligning the spectra restored with "inv_bispec2_1D"
*
* JLP 
* Version 27-01-00
-------------------------------------------------------------------*/
#define DEBUG

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <math.h>
#include <jlp_ftoc.h>

/*
#define BARYCENTER
*/
static int shift_y_edge(float *in_image, float *out_image, 
                              int nx, int ny, float x_sig);
static int shift_y_barycenter(float *in_image, float *out_image, 
                              int nx, int ny, float x_sig);
main(argc,argv)
int argc;
char *argv[];
{
char in_name[61], out_name[61], comments[81];
float *in_image, *out_image, x_sig; 
INT_PNTR pntr_image;
INT4 istatus, nx, ny, npts, ifail;
register int i, j;

#ifdef BARYCENTER 
printf(" Program shift_y to shift the columns according to the barycenter\n");
#else
printf(" Program shift_y to shift the columns according to an edge\n");
#endif
printf(" JLP Version 27-01-00 \n");

/* One parameters only is allowed to run the program: */
/* Carefull: 7 parameters always, using JLP "runs" */
if(argc == 7 && *argv[3]) argc = 4;
if(argc == 7 && *argv[2]) argc = 3;
if(argc == 7 && *argv[1]) argc = 2;
if(argc != 4 && argc != 1)
  {
  printf(" Syntax: shift_y in out x_sigma\n"); 
  printf(" Fatal: Syntax error: argc=%d\n",argc);
  exit(-1);
  }

/* Input of parameters with the command line: */
if (argc == 4 )
 { 
  strcpy(in_name,argv[1]);
  strcpy(out_name,argv[2]);
  sscanf(argv[3],"%f",&x_sig);
 }
/* Interactive input of parameters: */
else
 { 
  printf(" Input file := ");scanf("%s",in_name);
  printf(" Output file := ");scanf("%s",out_name);
  printf(" Threshold (in sigma units) := ");scanf("%f",&x_sig);
 }
printf(" Threshold (in sigma units): x_sig= %f \n",x_sig);

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
#ifdef BARYCENTER 
  shift_y_barycenter(in_image,out_image,nx,ny,x_sig);
#else
  shift_y_edge(in_image,out_image,nx,ny,x_sig);
#endif
  JLP_WRITEIMAG(out_image,&nx,&ny,&nx,out_name,comments);

JLP_END();
}
/******************************************************************
*
* Detects the first edge in each column and recentre the column 
*******************************************************************/
static int shift_y_edge(float *in_image, float *out_image, 
                        int nx, int ny, float x_sig)
{
double sum0, sum1;
float work, mean, sig, ww;
int jb, jshift, jout;
register int i, j;

for(i = 0; i < nx; i++)
  {
/* I first compute the mean and sigma */ 
  sum0 = 0.; sum1 = 0.;
  for(j = 0; j < ny; j++)
   {
   work = in_image[i + j * nx];
   sum0 += work; 
   sum1 += work*work; 
   }
  mean = sum0 / ny;
  sig = sum1 / ny - mean*mean;
  sig = sqrt((double)sig);

/* Look for an edge  at (mean + x_sig * sig) */ 
  sum0 = 0.; sum1 = 0.;
  ww = mean + x_sig * sig;
  jb = 0;
  for(j = 0; j < ny-1; j++)
   {
     work = in_image[i + j * nx];
     if(work > ww && work < in_image[i + (j+1) * nx]) {jb = j; break;}
   }

   jshift = (ny/2) - jb;
#ifdef DEBUG
  if(i < 10) printf(" Column #%d edge is at %d\n",i,jb);
#endif
  for(j = 0; j < ny; j++)
   {
   jout = j + jshift;
   if(jout < 0) jout += ny;
   if(jout >= ny) jout -= ny;
   out_image[i + jout * nx] = in_image[i + j * nx];
   }
/* End of the loop on the columns (iy) */
  }
return(0);
}
/******************************************************************
*
* Compute the barycenter for each column and recentre it 
* Not very good
*******************************************************************/
static int shift_y_barycenter(float *in_image, float *out_image, 
                              int nx, int ny, float x_sig)
{
double sum0, sum1;
float work, mean, sig, ww;
int jb, jshift, jout, nval;
register int i, j;

for(i = 0; i < nx; i++)
  {
/* I first compute the mean and sigma */ 
  sum0 = 0.; sum1 = 0.;
  for(j = 0; j < ny; j++)
   {
   work = in_image[i + j * nx];
   sum0 += work; 
   sum1 += work*work; 
   }
  mean = sum0 / ny;
  sig = sum1 / ny - mean*mean;
  sig = sqrt((double)sig);

/* I apply a threshold to get a better (?) result: */
  sum0 = 0.; sum1 = 0.;
  ww = mean + x_sig * sig;
  nval = 0;
  for(j = 0; j < ny; j++)
   {
   work = in_image[i + j * nx];
     if(work > ww)
     {
     sum0 += work; 
     sum1 += (float)j * work;
     nval++;
     }
   }
  if(sum0 == 0.) 
   jb = 0;
  else
   jb = sum1 / sum0;

   jshift = (ny/2) - jb;
#ifdef DEBUG
  if(i < 100) printf(" Barycenter of column #%d is %d, mean=%f sig=%f nval=%d\n",
                      i,jb,mean,sig,nval);
#endif
  for(j = 0; j < ny; j++)
   {
   jout = j + jshift;
   if(jout < 0) jout += ny;
   if(jout >= ny) jout -= ny;
   out_image[i + jout * nx] = in_image[i + j * nx];
   }
/* End of the loop on the columns (iy) */
  }
return(0);
}
