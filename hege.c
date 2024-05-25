/************************************************************
* hege.c
* To correct for detector defects, as proposed by Hege
* in Diffraction-limited imaging with Very Large Telescopes
* p 113, D.M. Alloin and J.-M. Mariotti, 1989, 
* Kluwer Academic Publishers.
*
* 1) Creates a 1D cut of the input image along that direction:
*  - rotation of the input image 
*  - computation of the mean of the central "nlines" lines
*
* 2) Then fits a Gaussian in the frequency range: f_tel,f_max
*
* SYNTAX:
*  hege.exe input_modsq f_tel cut angle,nlines
* or  hege.exe input_modsq f_tel all
*
* Example in /dt/speckle4_simu/refg7
* runs hege ref_g7_a_m 36 cut 0.,1
* or (here cut parameters are only used for graphic output):
* runs hege ref_g7_a_m 36 all 0.,1
* clean:
* if cut:
*    rm -f hege_pws_rot.fits hege_fit.dat
* else
*    rm -f hege_resi.fits hege_gauss.fits hege_fit.dat
* end:
*
*
* JLP
* Version 24/07/2002
*************************************************************/

#include <stdio.h>
#include <math.h>
#include "aris_lib.c"
#include "jlp_ftoc.h"

/* #define ECH 0.0309	
(secondes par pixel)
*/
#define ECH 1	

/* in src/sourcc/conju_grad.c : */
int JLP_CGRAD(double *aa, double *psi, double *phi, INT4 *nx1, INT4 *ny1,
              INT4 *ifail);

/* Contained here: */
int gaussian_fit_1d(float *pws_1d, int fc, INT4 nx_1d, float *gg, float *sig_x,
                    float *hh);
int gaussian_fit_1d_zero_offset(float *pws_1d, int fc, INT4 nx_1d, 
                                float *gg, float *sig_x);
int gaussian_fit_1d_seidel(float *pws_1d, int fc, INT4 nx_1d, 
                           float *gg, float *sig_x, float *hh, int iter_max);
int gaussian_fit_2d_seidel(float *pws_2d, float *gauss_2d, int fc, INT4 nx, INT4 ny, 
                           float *gg, float *sig_x, float *sig_y, float *hh,
                           char *filename, char *comments, int iter_max);
int gaussian_fit_2d(float *pws_2d, float *gauss_2d, int fc, 
                    INT4 nx, INT4 ny, float *gg, float *sig_x, float *sig_y, 
                    float *hh, char *filename, char *comments);
int gaussian_fit_2d_zero_offset(float *pws_2d, float *gauss_2d, int fc, 
                                INT4 nx, INT4 ny, float *gg, float *sig_x, 
                                float *sig_y, char *filename, char *comments);
int gauss_like_fit_2d_zero_offset(float *pws_2d, float *gauss_2d, int fc, 
                                  INT4 nx, INT4 ny, float *gg, float *sig_x, 
                                  float *sig_y, char *filename, char *comments);
int extract_cut(float *pws_2d, float *pws_1d, 
          int fc, float angle, INT4 nx, INT4 ny, INT4 nx_1d, int nlines);
int plot_pws_1d(float *pws_1d, 
                INT4 nx_1d, char *filename, char *comments, char *plotdev);
int plot_fit_1d(float *pws_1d, int fc, INT4 nx_1d, float gg, float sig_x,
                float hh, char *filename, char *comments, char *plotdev);
int plot_fit_2d(float *pws_1d, float *gauss_1d, INT4 nx_1d, 
                char *plotdev);
int output_fit_1d(float *pws_1d, int fc, INT4 nx_1d, float gg, float sig_x);
int output_fit_2d(float *pws_1d, float *gauss_1d, INT4 nx_1d);

/* ===============================================================
	Programme principal
================================================================= */
int main(int argc, char **argv)
{
int	fc, nlines, istat, i, syntax_is_ok, cut_is_wanted, model, iter_max;
float	angle, gg, sig_x, sig_y, sig_xy, hh;
float	*pws_2d, *gauss_2d, *gauss_1d, *pws_1d; 
INT4    nx, ny, nx_1d;
INT_PNTR pntr_ima;
char	filename[61], comments[81], plotdev[32], answer[1];
	
JLP_INQUIFMT();

/* Syntax: */
if(argc == 7) {
 for(i = 6; i > 0; i--) if(!argv[i][0]) argc--;
/*
 for(i = 6; i > 0; i--) 
   printf(" argv[%d]=>%s< (%d)\n", i , argv[i], (int)(argv[i][0]));
*/
 }
syntax_is_ok = 0;
if(((argv[3][0] == 'c') || (argv[3][0] == 'C')) 
      && (argc == 6)) syntax_is_ok = 1; 
if(((argv[3][0] == 'a') || (argv[3][0] == 'A')) 
      && (argc == 6)) syntax_is_ok = 1;
if(argc == 1) syntax_is_ok = 1; 

if(!syntax_is_ok)
 {
 printf("USAGE:    runs hege modsq f_tel cut/all angle,nlines model\n");
 printf("Models: -2=Gaussian-like (zero offset) \n");
 printf("        -1=Gaussian (zero offset)\n");
 printf("         1=Gaussian (Gauss-Seidel) non-zero offset\n");
 printf("         2=Gaussian-like (Gauss-Seidel) non-zero offset\n");
 printf("         3=Gaussian (conj. gradients) non-zero offset\n\n");
 printf(" Example: \n runs hege input_modsq f_tel cut angle,nlines 1\n");
 printf(" or  runs hege input_modsq f_tel all angle,nlines model\n");
 printf(" in that case, cut parameters are only used for graphic output)\n");
 printf(" (Possibility of interactive input of parameters if no parameters on command line)\n");
 printf("argv[3][0] = %c\n",argv[3][0]);
 printf("argc = %d\n",argc);
 exit(-1);
 }

/* ---------- Input of the parameters --------------------- */

if(argc == 1)
  {
  printf("Input power spectrum := ");
  scanf("%s",filename);
  printf(" Telescope cut-off frequency (pixels) = ");
  scanf("%d",&fc);
  printf(" Do you want (a or c): \n");
  printf(" (a) = process all frame \n (c) = cut \n \n");
  scanf("%c",answer);
  if(argv[3][0] == 'c' || argv[3][0] == 'c') cut_is_wanted = 1;
  else cut_is_wanted = 0;
  }
else
  {
  strcpy(filename,argv[1]);
  sscanf(argv[2],"%d",&fc);
  if(argv[3][0] == 'c' || argv[3][0] == 'c') cut_is_wanted = 1;
  else cut_is_wanted = 0;
  }

  printf("Input modsq: %s \n", filename);
  printf(" fc = %d (pixels)\n", fc);

  if(argc == 1) {
    printf("Rotation angle for the cut (in degrees, relative to OX) and "); 
    printf("number of lines too integrate (even number) (0 if only a slice) =");
    scanf("%f,%d",&angle,&nlines); 
    printf("Model: 1=Gaussian (Gauss-Seidel method) 2=Gauss-like (conjugate gradients method)");
    printf(" 3=Gaussian (conjugate gradients method)\n");
    printf("       -1 and -2: same than 1 and 2, but assuming zero offset: \n");
    scanf("%d",&model); 
    }
  else
    {
    sscanf(argv[4],"%f,%d",&angle,&nlines);
    sscanf(argv[5],"%d",&model); 
    switch(model)
      {
      case -2:
      case -1:
      case 1:
      case 2:
      case 3:
       break;
      default:
       printf("Fatal error: unknown model (should be -2, -1, 1, 2 or 3!)\n");
       exit(-1);
      }
     if(cut_is_wanted && (ABS(model) != 1 && model != 3))
       {
       printf("Sorry, only Gaussian (model=+/-1, or 3) is allowed when 'cut' is selected\n");
       exit(-1);
       }
    }

  nlines = (nlines + 1)/2;
  printf(" Cut with angle = %.3f (deg) \n", angle);
  printf(" and integration along %d lines around the central line\n",nlines);
  printf(" model: %d (1=Gauss 2=Gauss-like)\n", model);
  if(model < 0) printf("OK: model will assume zero offset\n");
	
/* Input power spectrum: */
    istat=JLP_VM_READIMAG1(&pntr_ima,&nx,&ny,filename,comments);
    pws_2d = (float *)pntr_ima;
    nx_1d = nx;
    if(istat != 0)
     {
      printf(" Fatal error reading %s istat = %d \n",filename,istat);
      exit(-1);
     }

/* Allocation of memory  ------------------------------- */
pws_1d=(float *) malloc(nx_1d*sizeof(float));

/* Set plotdev to "xterm", i.e. X11 output: */
  strcpy(plotdev,"xterm_small");
/*
  printf(" Plotting device : ");gets(plotdev);strcpy(plotdev,"square");
*/

if(cut_is_wanted) {
/* Projection of pws_2d onto 1D array pws_1d */	
  extract_cut(pws_2d, pws_1d, fc, angle, nx, ny, nx_1d, nlines);

  plot_pws_1d(pws_1d, nx_1d, filename, comments, plotdev);

/* Gaussian fit to this 1-D array 
*  gg exp ( - x^2 / sig_x^2)
*/
  if(model == -1)
    {
    printf("1-D Gaussian fit with zero offset\n");
    gaussian_fit_1d_zero_offset(pws_1d,fc,nx_1d,&gg,&sig_x);
    hh = 0.;
    }
  else if(model == 1)
    {
    printf("1-D Gaussian fit (Gauss-Seidel) with non-zero offset\n");
    iter_max = 10;
    gaussian_fit_1d_seidel(pws_1d, fc, nx_1d, &gg, &sig_x, &hh, iter_max);
    }
  else
    {
    printf("1-D Gaussian fit (conj. gradients) with non-zero offset\n");
    gaussian_fit_1d(pws_1d, fc, nx_1d, &gg, &sig_x, &hh);
    }

/* Save data and fit to file: */
  output_fit_1d(pws_1d, fc, nx_1d, gg, sig_x);

  plot_fit_1d(pws_1d, fc, nx_1d, gg, sig_x, hh, filename, comments,
         plotdev);
  printf(" I copy this plot to \"pst.tmp\" \n");
  strcpy(plotdev,"landscape");
  plot_fit_1d(pws_1d, fc, nx_1d, gg, sig_x, hh, filename, comments,
           plotdev);
  } else {
/* Allocation of memory  ------------------------------- */
  gauss_2d=(float *) malloc(nx*ny*sizeof(float));
  gauss_1d=(float *) malloc(nx_1d*sizeof(float));

/* Gaussian fit to 2-D array 
*  gg exp ( - x^2 / sig_x^2  - y^2/ sig_y^2)
*/
switch(model)
  {
  case -1:
    printf(" Fit of a Gaussian with zero offset\n");
    gaussian_fit_2d_zero_offset(pws_2d, gauss_2d, fc, nx, ny, &gg, 
                                &sig_x, &sig_y, filename, comments);
    hh = 0.;
    break;
  case 1:
    printf(" Fit of a Gaussian with non-zero offset (Seidel)\n");
    iter_max = 30;
    gaussian_fit_2d_seidel(pws_2d, gauss_2d, fc, nx, ny, &gg, &sig_x, &sig_y, 
                           &hh, filename, comments, iter_max);
    break;
  case -2:
    printf(" Fit of a Gaussian-like function with zero offset\n");
    gauss_like_fit_2d_zero_offset(pws_2d, gauss_2d, fc, nx, ny, &gg, 
                                  &sig_x, &sig_y, filename, comments);
    hh = 0.;
    break;
  case 2:
    printf(" Fit of a Gaussian_like function with non-zero offset\n");
    printf(" Sorry not yet available\n");
    exit(-1);
    break;
  case 3:
    printf(" Fit of a Gaussian function with non-zero offset\n");
    gaussian_fit_2d(pws_2d, gauss_2d, fc, nx, ny, &gg, 
                    &sig_x, &sig_y, &hh, filename, comments);
    hh = 0.;
    break;
  }

/* Projection of pws_2d onto 1D array pws_1d */	
  extract_cut(pws_2d, pws_1d, fc, angle, nx, ny, nx_1d, nlines);
  extract_cut(gauss_2d, gauss_1d, fc, angle, nx, ny, nx_1d, nlines);

/* Output of curves: */
  printf(" angle= %.3f sig_xy = %f\n", angle, sig_xy);
  plot_fit_2d(pws_1d, gauss_1d, nx_1d, plotdev);
  printf(" I copy this plot to \"pst.tmp\" \n");
  strcpy(plotdev,"landscape");
  plot_fit_2d(pws_1d, gauss_1d, nx_1d, plotdev);

/* Save data and fit to file: */
  output_fit_2d(pws_1d, gauss_1d, nx_1d);

  free(gauss_1d);
  free(gauss_2d);
  }

free(pws_1d);
return(0);
}

/* ----------------------------------------------------------------------
* Projection d'une image sur une colonne
*
* INPUT:
*  pws_2d 
*  angle: (degrees)
*
* OUTPUT:
*  pws_1d
---------------------------------------------------------------------- */
int extract_cut(float *pws_2d, float *pws_1d,
          int fc, float angle, INT4 nx, INT4 ny, INT4 nx_1d, int nlines)
{
float	*pws_rot;
int i, j;
FILE	*fopen();
#ifdef DEBUG
char filename[60], comments[80];
#endif
	
pws_rot=(float *)malloc(nx * ny * sizeof(float));
		
/* Rotation des 2 tableaux (err et vis)
   ------------------------------------
*/
rotimage(pws_2d,pws_rot,nx,ny,(double)(angle * PI /180.));

#ifdef DEBUG
strcpy(filename,"hege_pws_rot");
sprintf(comments,"After rotation of %.3f degrees",angle);
JLP_WRITEIMAG(pws_rot,&nx,&ny,&nx,filename,comments);
#endif

/* Calcul des projections (err et vis) d'un rectangle de hauteur nlines (non pondere)
   -----------------------------------------------------------------
*/
raz(pws_1d,nx_1d);

if (nlines >= 2)
{
   for (j=(nx_1d-nlines)/2; j<=(nx_1d+nlines)/2; j++) 
   {
        for (i=0; i<nx_1d; i++)
	{
		pws_1d[i] += pws_rot[i + j*nx_1d]/nlines;
	}
   }
}
/* nlines=0 */
else 
{
/* Cut of the power spectrum:
   ----------------------
*/
   for (i=0; i<nx_1d; i++)
   {
	j=nx_1d/2;
	pws_1d[i] = pws_rot[i + j*nx_1d];
   }
}

free(pws_rot);
return(0);
}

/* ---------------------------------------------------------------------
/ To display the 1D data
/
----------------------------------------------------------------------*/
int plot_pws_1d(float *pws_1d, 
                INT4 nx_1d, char *filename, char *comments, char *plotdev)
{
float *xplot, *yplot;
float errx[1],erry[1],xout[20],yout[20];
float offx1, offy1, axlen1, aylen1, xmin, xmax, ymin, ymax;
INT4 ncurves, nout, error_bars, npts[2], plan, full_caption;
char xlabel[41], ylabel[41], title[81], nchar[8], pcolor[60];
int i;

/* Allocation of memory: */
xplot = (float*) malloc(nx_1d * sizeof(float));
yplot = (float*) malloc(nx_1d * sizeof(float));
if(xplot == NULL || yplot == NULL)
  {
  printf("plot_pws_1d/error allocating memory: nx_1d=%d\n", nx_1d);
  return(-1);
  }

for(i = 0; i < nx_1d; i++)
   {
   xplot[i] = i;
   yplot[i] = pws_1d[i];
   }

ymin = yplot[0];
ymax = yplot[0];
for(i = 1; i < nx_1d; i++) 
   {if(ymin > yplot[i]) ymin = yplot[i]; 
   if(ymax < yplot[i]) ymax = yplot[i];} 
xmin = xplot[0];
xmax = xplot[nx_1d-1];
strcpy(nchar,"L0");
strcpy(pcolor,"Default");
strcpy(xlabel," ");
strcpy(ylabel," ");
strcpy(title," ");

offx1 = 4000.; offy1 = 3500.;
axlen1 = 27000.; aylen1 = 27000.;
plan = 0;
ncurves = 1;
error_bars = 0;
strcpy(&nchar[4],"L");
strcpy(&pcolor[30],"Default");

/* Initialize plotting device: */
JLP_SETUP_PLOT(&offx1,&offy1,&axlen1,&aylen1,&xmin,&xmax,&ymin,&ymax,&plan);

/* Display the curve: */
npts[0] = nx_1d;
full_caption = 0;
newplot22(xplot,yplot,errx,erry,npts,&nx_1d,&ncurves,
           xlabel,ylabel,title,nchar,pcolor,plotdev,
           xout,yout,&nout,&error_bars,filename,comments,&full_caption);

/* Close display device: */
JLP_SPCLOSE();

free(xplot);
free(yplot);
return(0);
}
/* ---------------------------------------------------------------------
/ To display the data and the Gaussian function
/
----------------------------------------------------------------------*/
int plot_fit_1d(float *pws_1d, int fc, INT4 nx_1d, float gg, float sig_x,
                float hh, char *filename, char *comments, char *plotdev)
{
float *xplot, *yplot;
float errx[1],erry[1],xout[20],yout[20], xrange, yrange;
float offx1, offy1, axlen1, aylen1, xmin, xmax, ymin, ymax;
int margin, ix;
INT4 ncurves, nout, error_bars, npts[2], plan, full_caption;
char xlabel[41], ylabel[41], title[81], nchar[8], pcolor[60];
int i, k;

/* Allocation of memory: */
xplot = (float*) malloc(2 * nx_1d * sizeof(float));
yplot = (float*) malloc( 2* nx_1d * sizeof(float));
if(xplot == NULL || yplot == NULL)
  {
  printf("plot_fit_1d/error allocating memory: nx_1d=%d\n", nx_1d);
  return(-1);
  }

/* First curve: data points */
k = 0;
margin = 3;
for(i = margin; i < nx_1d - margin; i++) {
   ix = i - nx_1d/2;
   if((ix < -fc) || (ix > fc)) {
     xplot[k] = ix;
     yplot[k] = pws_1d[i];
     k++;
     }
   }
npts[0] = k;
/* Second curve: Gaussian function */
k = 0;
for(i = margin; i < nx_1d - margin; i++) {
   ix = i - nx_1d/2;
   xplot[k + nx_1d] = ix;
   yplot[k + nx_1d] = gg * exp(-SQUARE(ix)/SQUARE(sig_x)) + hh;
   k++;
   }
npts[1] = k;

ymin = yplot[0];
ymax = yplot[0];
for(k = 1; k < npts[0]; k++) 
   {if(ymin > yplot[k]) ymin = yplot[k]; 
   if(ymax < yplot[k]) ymax = yplot[k];} 
for(k = 0; k < npts[1]; k++) 
   {if(ymin > yplot[k + nx_1d]) ymin = yplot[k + nx_1d]; 
   if(ymax < yplot[k + nx_1d]) ymax = yplot[k + nx_1d];} 
yrange = ymax - ymin;
ymin -= yrange/10.;
ymax += yrange/10.;

xmin = xplot[0 + nx_1d];
xmax = xplot[npts[1]-1 + nx_1d];
xrange = xmax - xmin;
xmin -= xrange/10.;
xmax += xrange/10.;
strcpy(nchar,"L1");
strcpy(&nchar[4],"L0");
strcpy(pcolor,"Default");
strcpy(&pcolor[30],"Default");
strcpy(xlabel,"Angular frequency ");
strcpy(ylabel,"Power spectrum");
strcpy(title," Gaussian fit ");

offx1 = 4000.; offy1 = 3500.;
axlen1 = 27000.; aylen1 = 27000.;
plan = 0;
ncurves = 2;
error_bars = 0;

/* Initialize plotting device: */
JLP_SETUP_PLOT(&offx1,&offy1,&axlen1,&aylen1,&xmin,&xmax,&ymin,&ymax,&plan);

/* Display the curve: */
full_caption = 0;
newplot22(xplot,yplot,errx,erry,npts,&nx_1d,&ncurves,
           xlabel,ylabel,title,nchar,pcolor,plotdev,
           xout,yout,&nout,&error_bars,filename,comments,&full_caption);

/* Close display device: */
JLP_SPCLOSE();

free(xplot);
free(yplot);
return(0);
}
/* ---------------------------------------------------------------------
/ To display the data and the Gaussian function
/
----------------------------------------------------------------------*/
int plot_fit_2d(float *pws_1d, float *gauss_1d, INT4 nx_1d, 
                char *plotdev)
{
float *xplot, *yplot;
float errx[1],erry[1],xout[20],yout[20], xrange, yrange;
float offx1, offy1, axlen1, aylen1, xmin, xmax, ymin, ymax;
int margin, ix;
INT4 ncurves, nout, error_bars, npts[2], plan, full_caption;
char xlabel[41], ylabel[41], title[81], nchar[8], pcolor[60];
char filename[60], comments[80];
int i, k;

/* Allocation of memory: */
xplot = (float*) malloc(2 * nx_1d * sizeof(float));
yplot = (float*) malloc( 2* nx_1d * sizeof(float));
if(xplot == NULL || yplot == NULL)
  {
  printf("plot_fit_2d/error allocating memory: nx_1d=%d\n", nx_1d);
  return(-1);
  }

/* First curve: data points */
/* Second curve: Gaussian function */
k = 0;
margin = 3;
for(i = margin; i < nx_1d - margin; i++) {
     ix = i - nx_1d/2;
     xplot[k] = ix;
     yplot[k] = pws_1d[i];
     xplot[k + nx_1d] = ix;
     yplot[k + nx_1d] = gauss_1d[i]; 
     k++;
   }
npts[0] = k;
npts[1] = k;

/* Don't use the data for the automatic scale, since 
* there is always a big seeing peak at the center...
ymin = yplot[0];
ymax = yplot[0];
for(k = 1; k < npts[0]; k++) 
   {if(ymin > yplot[k]) ymin = yplot[k]; 
   if(ymax < yplot[k]) ymax = yplot[k];} 
*/
ymin = yplot[nx_1d];
ymax = yplot[nx_1d];
for(k = 1; k < npts[1]; k++) 
   {if(ymin > yplot[k + nx_1d]) ymin = yplot[k + nx_1d]; 
   if(ymax < yplot[k + nx_1d]) ymax = yplot[k + nx_1d];} 
yrange = ymax - ymin;
ymin -= yrange/10.;
ymax += yrange/10.;

xmin = xplot[0 + nx_1d];
xmax = xplot[npts[1]-1 + nx_1d];
xrange = xmax - xmin;
xmin -= xrange/10.;
xmax += xrange/10.;
strcpy(nchar,"L1");
strcpy(&nchar[4],"L0");
strcpy(pcolor,"Default");
strcpy(&pcolor[30],"Default");
strcpy(xlabel,"Angular frequency ");
strcpy(ylabel,"Power spectrum");
strcpy(title," Gaussian fit ");

offx1 = 4000.; offy1 = 3500.;
axlen1 = 27000.; aylen1 = 27000.;
plan = 0;
ncurves = 2;
error_bars = 0;

/* Initialize plotting device: */
JLP_SETUP_PLOT(&offx1,&offy1,&axlen1,&aylen1,&xmin,&xmax,&ymin,&ymax,&plan);

/* Display the curve: */
full_caption = 0;
newplot22(xplot,yplot,errx,erry,npts,&nx_1d,&ncurves,
           xlabel,ylabel,title,nchar,pcolor,plotdev,
           xout,yout,&nout,&error_bars,filename,comments,&full_caption);

/* Close display device: */
JLP_SPCLOSE();

free(xplot);
free(yplot);
return(0);
}
/* ---------------------------------------------------------------------
* To output the data and the Gaussian function
*
*---------------------------------------------------------------------*/
int output_fit_1d(float *pws_1d, int fc, INT4 nx_1d, float gg, float sig_x)
{
float fx;
int margin, ix;
int i;
FILE *fp;

if((fp = fopen("hege_fit.dat","w")) == NULL)
  {
  printf("Sorry cannot open \"hege_fit.dat\" \n"); 
  return(-1);
  }

fprintf(fp,"# Gaussian fit: %.3f %f \n", gg, sig_x); 
/* First curve: data points 
* second curve: fitted Gaussian 
* third curve: residuals */
margin = 3;
for(i = nx_1d/2 + 1; i < nx_1d - margin; i++) {
   ix = i - nx_1d/2;
   fx = gg*exp(-SQUARE(ix)/SQUARE(sig_x));
   fprintf(fp,"%d %f %f %f\n", ix, pws_1d[i], fx, pws_1d[i] - fx);
   }

fclose(fp);
return(0);
}
/* ---------------------------------------------------------------------
* To output the data and the Gaussian function
*
*---------------------------------------------------------------------*/
int output_fit_2d(float *pws_1d, float *gauss_1d, INT4 nx_1d)
{
int margin, ix;
int i;
FILE *fp;

if((fp = fopen("hege_fit.dat","w")) == NULL)
  {
  printf("Sorry cannot open \"hege_fit.dat\" \n"); 
  return(-1);
  }

fprintf(fp,"# Gaussian 2-D fit \n"); 
/* First curve: data points 
* second curve: fitted Gaussian 
* third curve: residuals */
margin = 3;
for(i = nx_1d/2 + 1; i < nx_1d - margin; i++) {
   ix = i - nx_1d/2;
   fprintf(fp,"%d %f %f %f\n", ix, pws_1d[i], gauss_1d[i], 
           pws_1d[i] - gauss_1d[i]);
   }

fclose(fp);
return(0);
}
/***********************************************************************
* Gaussian fit to 1-D array with successive linearizations 
*  f(x) =  gg exp ( - x^2 / sig_x^2) + hh
*  df/dgg(x) = exp ( - x^2 / sig_x^2) 
*  df/dsig_x(x) = (gg exp ( - x^2 / sig_x^2))  * ( +2 x^2 / sig_x^3) 
*
***********************************************************************/
int gaussian_fit_1d(float *pws_1d, int fc, INT4 nx_1d, 
                    float *gg, float *sig_x, float *hh)
{
int np = 3, nn, iter_max;
double phi[np], *aa, *psi, res;
INT4 ifail;
int kk, ix, status, margin;
int iter, i;

/* 3 pixels as margin, since bad pixels in the edges with rotation */
  margin = 3;
/* Initial solution: */
  printf("\n *********** Compute first the initial solution with zero offset \n");
  printf(" Gauss-Seidel with 10 iterations ...\n");
  iter_max = 10;
  status = gaussian_fit_1d_seidel(pws_1d, fc, nx_1d, gg, sig_x, hh, iter_max);
  if(status){ 
    printf(" gaussian_fit_1d/error with initial solution: status=%d\n", status);
    exit(-1);
    }
  psi = (double *)malloc(nx_1d * sizeof(double));
  aa = (double *)malloc(nx_1d * np * sizeof(double));

  printf("\n *********** Now iterate on linearized problem ************\n");
for(iter = 0; iter < 10; iter++) {
/* Compute linearized problem: 
*  minimize sum_k ( y_k - f(gg,sig_x, hh)(x_k) )^2 
*  minimize sum_k ( y_k - f(gg0, sig_x0, hh0)(x_k) 
*                  - (df/dgg)(x_k) * dgg - (df/sig_x)(x_k) * dsig_x 
*                  - (df/dhh)(x_k) * dhh )^2 
*
*  f(x) = gg exp ( - x^2 / sig_x^2) + hh
*  (df/dgg)(x) = exp ( - x^2 / sig_x^2) 
*  (df/sig_x)(x) = gg * exp ( - x^2 / sig_x^2) * (+ 2 x^2 / sig_x^3) 
*  (df/dhh)(x) = 1 
*/

/* Solve linearized problem: 
*  the problem is to find the optimal phi = variation of the parameters
*  psi = aa phi
*/
/* First compute aa matrix: */
    kk = 0;
    for(i = margin; i < nx_1d - margin; i++) {
     ix = (i - nx_1d/2);
     if((ix < -fc) || (ix > fc)) {
       aa[0 + kk * np] = exp(-SQUARE(ix)/SQUARE(*sig_x));
       aa[1 + kk * np] = (*gg) * exp(-SQUARE(ix)/SQUARE(*sig_x)) 
                         *  2 * SQUARE(ix) / ((*sig_x) * (*sig_x) * (*sig_x));
       aa[2 + kk * np] = 1.;
       psi[kk] = pws_1d[i] - (*gg)*exp(-SQUARE(ix)/SQUARE(*sig_x)) - (*hh);
       kk++;
       }
    }
    nn = kk;

    phi[0] = 0.;
    phi[1] = 0.;
    phi[2] = 0.;
/* Least square minimization: */
    status = JLP_CGRAD(aa,psi,phi,&np,&nn,&ifail);

    *gg += phi[0];
    *sig_x += phi[1];
    *hh += phi[2];

/* Compute residuals: */
  res = 0; kk = 0;
  for(i = margin; i < nx_1d - margin; i++) {
      ix = i - nx_1d/2;
      if((ix < -fc) || (ix > fc)) {
        res += SQUARE(pws_1d[i] - (*gg)*exp(-SQUARE(ix)/SQUARE(*sig_x)) - (*hh));
        kk++;
        }
      }
  if(kk == 0) res = 0.;
     else res /= kk; 
  printf("iter=%d gg=%g sig_x=%g hh=%g mean xhi2=%g (npts=%d)\n", 
          iter, *gg, *sig_x, *hh, res, kk);
/* End of iteration #iter */
  }

free(aa);
free(psi);

return(0);
}
/***********************************************************************
* Gaussian fit to 1-D array with Gauss-Seidel method 
* It works rather well if the data are close to a Gaussian (less than 10 iterations).
*
*  f(x) =  gg exp ( - x^2 / sig_x^2) + hh
*  df/dgg(x) = exp ( - x^2 / sig_x^2) 
*  df/dsig_x(x) = (gg exp ( - x^2 / sig_x^2))  * ( +2 x^2 / sig_x^3) 
*
***********************************************************************/
int gaussian_fit_1d_seidel(float *pws_1d, int fc, INT4 nx_1d, 
                           float *gg, float *sig_x, float *hh, int iter_max)
{
float hh_old; 
float *pwss;
int kk, ix, status, margin;
int iter, i;

/* Gauss-Seidel method: 
* i.e. separate fit on hh and then on the other parameters:
*/
pwss = (float *)malloc(nx_1d * sizeof(float));
if(pwss == NULL) {
    printf(" gaussian_fit_1d/Sorry: fatal error allocating memory: nx=%d\n", 
             nx_1d);
    exit(-1);
 }

margin = 3;
hh_old = 0.;
for(iter = 0; iter < iter_max; iter++) {
  for(i = 0; i <= nx_1d; i++) pwss[i] = pws_1d[i] - hh_old; 
  status = gaussian_fit_1d_zero_offset(pwss, fc, nx_1d, gg, sig_x);
  if(status){ 
    printf(" gaussian_fit_1d/Sorry: status = %d\n", status);
    exit(-1);
    }
  *hh = 0; kk = 0;
  for(i = margin; i < nx_1d - margin; i++) {
    ix = i - nx_1d/2;
    if(((pws_1d[i] - hh_old) > 0.) && ((ix < -fc) || (ix > fc))) { 
      kk++; 
      *hh += (pws_1d[i] - (*gg)*exp(-SQUARE(ix)/SQUARE(*sig_x)));
      }
    }
  if(kk == 0) *hh = 0.;
     else *hh /= (float)kk; 
  printf("iter=%d gg=%g sig_x=%g hh=%g (kk=%d)\n", iter, *gg, *sig_x, *hh, kk);
  hh_old = *hh;
  }

free(pwss);
return(0);
}
/***********************************************************************
* Gaussian fit to 1-D array assuming zero offset 
*  f(x) =  gg exp ( - x^2 / sig_x^2)
* Linearized with:
*  log(f(x)) = log(gg) -  x^2 / sig_x^2
*
***********************************************************************/
int gaussian_fit_1d_zero_offset(float *pws_1d, int fc, INT4 nx_1d, 
                                float *gg, float *sig_x)
{
double phi[2], *aa, *psi;
int ix, np, nn, kk, status, margin;
INT4 ifail;
int i;

/*
printf(" Enter initial guess for gg and sig_x := ");
scanf("%f,%f", gg, sig_x);
*/
*sig_x = nx_1d/10.;
*gg = pws_1d[nx_1d/2+10];

/* Linearize the problem: 
*    log(f(x)) = log(gg) - x^2/sig_x^2 
* or psi(x) = phi[0] - x^2 * phi[1]
*    with 
*      phi[0] = log(gg) 
*      phi[1] = 1/sig_x^2
*      psi[x] = log(f(x))
*        
*/
np = 2;
phi[0] = log(*gg);
phi[1] = 1./(*sig_x);
psi = (double *)malloc(nx_1d * sizeof(double));
aa = (double *)malloc(nx_1d * np * sizeof(double));
/* Compute aa matrix: */
kk = 0;
/* 3 pixels as margin, since bad pixels in the edges with rotation */
margin = 3;
  for(i = margin; i < nx_1d - margin; i++) {
   ix = (i - nx_1d/2);
   if(((ix < -fc) || (ix > fc))
     && (pws_1d[i] > 0.)) {
     aa[0 + kk * np] = 1;
     aa[1 + kk * np] = - SQUARE(ix);
     psi[kk] = log(pws_1d[i]);
     kk++;
     }
   }
nn = kk;
printf(" Number of input data points: %d\n",nn);

/* Least square minimization: */
status = JLP_CGRAD(aa,psi,phi,&np,&nn,&ifail);

#ifdef DEBUG
printf("Solution: phi[0]=%.3f   phi[1]= %f\n", phi[0], phi[1]);
#endif
*gg = exp(phi[0]);
if(phi[1] != 0.) *sig_x = sqrt(1./phi[1]);
else *sig_x = 0.;
printf(" gg=%.3f   sig_x= %.3f\n", *gg, *sig_x);

free(aa);
free(psi);
return(0);
}
/***********************************************************************
* Gaussian fit to 2-D array 
*  f(x) =  gg exp ( - x^2 / sig_x^2 - y^2 / sig_y^2) + hh
* with Gauss-Seidel method.
* It is not very good in 2-D...
*
***********************************************************************/
int gaussian_fit_2d_seidel(float *pws_2d, float *gauss_2d, int fc, INT4 nx, INT4 ny, 
                           float *gg, float *sig_x, float *sig_y, float *hh,
                           char *filename, char *comments, int iter_max)
{
float hh_old;
float *pwss, rad2_x, rad2_y, fc2;
int kk, status, margin;
int iter, i, j;

/* Gauss-Seidel method:
* i.e. separate fit on hh and then on the other parameters:
*/
pwss = (float *)malloc(nx * ny * sizeof(float));
if(pwss == NULL) {
    printf(" gaussian_fit_2d_seidel/Sorry: fatal error allocating memory: nx=%d ny= %d\n",
             nx, ny);
    exit(-1);
 }

fc2 = SQUARE(fc);
margin = 3;
hh_old = 0.;
for(iter = 0; iter < iter_max; iter++) {
  for(i = 0; i <= nx * ny; i++) pwss[i] = pws_2d[i] - hh_old;
  status = gaussian_fit_2d_zero_offset(pwss, gauss_2d, fc, nx, ny, 
                                       gg, sig_x, sig_y, filename, comments);
  if(status){
    printf(" gaussian_fit_2d/Sorry: status = %d\n", status);
    exit(-1);
    }
  *hh = 0; kk = 0;
  for(j = margin; j < ny - margin; j++) {
   rad2_y = SQUARE(j - ny/2);
     for(i = margin; i < nx - margin; i++) {
       rad2_x = SQUARE(i - nx/2);
       if((rad2_x + rad2_y > fc2)
        && (pwss[i + j * nx] > 0.)) {
          kk++;
          *hh += pws_2d[i + j * nx] - (*gg) * exp(- rad2_x/SQUARE(*sig_x)
                                          - rad2_y/SQUARE(*sig_y));
       }
     }
   }
  if(kk == 0) *hh = 0.;
     else *hh /= kk;
  printf("iter=%d gg=%g sig_x=%g sig_y=%g hh=%g (kk=%d)\n", 
          iter, *gg, *sig_x, *sig_y, *hh, kk);
  hh_old = *hh;
  } 

free(pwss);
return(0);
}
/***********************************************************************
* Gaussian fit to 2-D array assuming zero offset 
*  f(x) =  gg exp ( - x^2 / sig_x^2 - y^2 / sig_y^2)
*
* Linearized with:
*  log(f(x)) =  log(gg) - x^2 / sig_x^2 - y^2 / sig_y^2
***********************************************************************/
int gaussian_fit_2d_zero_offset(float *pws_2d, float *gauss_2d, int fc, 
                                INT4 nx, INT4 ny, float *gg, float *sig_x, 
                                float *sig_y, char *filename, char *comments)
{
double phi[3], *aa, *psi;
float rad2_x, rad2_y, fc2;
int np, nn, kk, status, margin;
INT4 ifail;
int i, j;

/*
printf(" Enter initial guess for gg, sig_x  and sig_y:= ");
scanf("%f,%f", gg, sig_x, sig_y);
*/
*sig_x = nx/10.;
*sig_y = ny/10.;
*gg = pws_2d[(nx/2)+10 + (ny/2) * nx];

/* Linearize the problem: 
*    log(f(x,y)) = log(gg) - x^2/sig_x^2 - y^2 / sig_y^2 
* or psi(x,y) = phi[0] - x^2 * phi[1] - y^2 * phi[2]
*    with 
*      phi[0] = log(gg) 
*      phi[1] = 1/sig_x^2
*      phi[2] = 1/sig_y^2
*      psi[x,y] = log(f(x,y))
*        
*/
np = 3;
phi[0] = log(*gg);
phi[1] = 1./SQUARE(*sig_x);
phi[2] = 1./SQUARE(*sig_y);
psi = (double *)malloc(nx * ny *sizeof(double));
if(psi == NULL) {
  printf(" gauss_2d/Fatal error allocating memory: nx=%d ny=%d\n",
           nx, ny);
  exit(-1);
  }
aa = (double *)malloc(nx * ny * np * sizeof(double));
if(aa == NULL) {
  printf(" gauss_2d/Fatal error allocating memory: nx=%d ny=%d\n",
           nx, ny);
  exit(-1);
  }
/* Compute aa matrix: */
kk = 0;
fc2 = SQUARE(fc);
/* Just in case... */
margin = 3;
  for(j = margin; j < ny - margin; j++) {
   rad2_y = SQUARE(j - ny/2);
     for(i = margin; i < nx - margin; i++) {
       rad2_x = SQUARE(i - nx/2);
       if((rad2_x + rad2_y > fc2)
        && (pws_2d[i + j * nx] > 0.)) {
        aa[0 + kk * np] = 1;
        aa[1 + kk * np] = - rad2_x;
        aa[2 + kk * np] = - rad2_y;
        psi[kk] = log(pws_2d[i + j * nx]);
        kk++;
        }
      }
   } 
nn = kk;
#ifdef DEBUG
printf(" Number of input data points: %d\n",nn);
#endif

/* Least square minimization: */
status = JLP_CGRAD(aa,psi,phi,&np,&nn,&ifail);

#ifdef DEBUG
printf("Solution: phi[0]=%.3f phi[1]=%g phi[2]=%g \n", 
        phi[0], phi[1], phi[2]);
#endif

*gg = exp(phi[0]);
if(phi[1] != 0.) *sig_x = sqrt(1./phi[1]);
else *sig_x = 0.;
if(phi[2] != 0.) *sig_y = sqrt(1./phi[2]);
else *sig_y = 0.;

#ifdef DEBUG
printf(" gg=%.3f sig_x=%.3f sig_y=%.3f \n", *gg, *sig_x, *sig_y);
#endif

/* Remove Gaussian fit from input array: */
  for(j = 0; j < ny; j++) {
     rad2_y = SQUARE(j - ny/2);
     for(i = 0; i < nx; i++) {
       rad2_x = SQUARE(i - nx/2);
       gauss_2d[i + j * nx] = (*gg) * exp(- rad2_x/SQUARE(*sig_x)
                                          - rad2_y/SQUARE(*sig_y));
       psi[i + j * nx] = pws_2d[i + j * nx] - gauss_2d[i + j * nx];
       }
     }

strcpy(filename,"hege_resi");
sprintf(comments,"Residuals of Gaussian fit");
JLP_D_WRITEIMAG(psi,&nx,&ny,&nx,filename,comments);

strcpy(filename,"hege_gauss");
sprintf(comments,"Fitted Gaussian photon response");
JLP_WRITEIMAG(gauss_2d,&nx,&ny,&nx,filename,comments);

free(aa);
free(psi);
return(0);
}
/***********************************************************************
* Gaussian fit to 2-D array with non-zero offset 
*  f(x) =  gg exp ( - x^2 / sig_x^2 - y^2 / sig_y^2) + hh
*
***********************************************************************/
int gaussian_fit_2d(float *pws_2d, float *gauss_2d, int fc, 
                    INT4 nx, INT4 ny, float *gg, float *sig_x, float *sig_y, 
                    float *hh, char *filename, char *comments)
{
int np = 4;
double phi[np], *aa, *psi, res;
float rad2_x, rad2_y, fc2;
int nn, kk, status, margin, iter, iter_max;
INT4 ifail;
int i, j;

/* 3 pixels as margin, since bad pixels in the edges with rotation */
  margin = 3;
/* Initial solution: */
  printf("\n *********** Compute first the initial solution with zero offset \n");
  printf(" Gauss-Seidel with 1 iteration ...\n");
  iter_max = 1;
  status = gaussian_fit_2d_seidel(pws_2d, gauss_2d, fc, nx, ny, gg, sig_x, sig_y, 
                                  hh, filename, comments, iter_max);
  if(status){
    printf(" gaussian_fit_2d/error with initial solution: status=%d\n", status);
    exit(-1);
    }

psi = (double *)malloc(nx * ny *sizeof(double));
if(psi == NULL) {
  printf(" gauss_2d/Fatal error allocating memory: nx=%d ny=%d\n",
           nx, ny);
  exit(-1);
  }
aa = (double *)malloc(nx * ny * np * sizeof(double));
if(aa == NULL) {
  printf(" gauss_2d/Fatal error allocating memory: nx=%d ny=%d\n",
           nx, ny);
  exit(-1);
  }

  printf("\n *********** Now iterate on linearized problem ************\n");
for(iter = 0; iter < 10; iter++) {

/* Compute linearized problem:
*  minimize sum_k ( y_k - f(gg,sig_x,sigy,hh)(x_k) )^2
*  minimize sum_k ( y_k - f(gg0, sig_x0, sig_y0, hh0)(x_k)
*                  - (df/dgg)(x,y) * dgg - (df/sig_x)(x,y) * dsig_x
*                  - (df/sig_y)(x,y) * dsig_x  - (df/dhh)(x,y) * dhh )^2
*
*  f(x,y) = gg exp ( - x^2 / sig_x^2 - y^2 / sig_y^2) + hh
*  (df/dgg)(x,y) = exp ( - x^2 / sig_x^2 - y^2 / sig_y^2)
*  (df/sig_x)(x,y) = gg * exp ( - x^2 / sig_x^2 - y^2 / sig_y^2) * (+ 2 x^2 / sig_x^3)
*  (df/sig_y)(x,y) = gg * exp ( - y^2 / sig_y^2 - y^2 / sig_y^2) * (+ 2 y^2 / sig_y^3)
*  (df/dhh)(x,y) = 1
*/

/* Solve linearized problem:
*  the problem is to find the optimal phi = variation of the parameters
*  psi = aa phi
*/
/* First compute aa matrix: */
   kk = 0;
   fc2 = SQUARE(fc);
   for(j = margin; j < ny - margin; j++) {
      rad2_y = SQUARE(j - ny/2);
        for(i = margin; i < nx - margin; i++) {
          rad2_x = SQUARE(i - nx/2);
          if(rad2_x + rad2_y > fc2) {
          aa[0 + kk * np] = exp(-rad2_x/SQUARE(*sig_x) - rad2_y/SQUARE(*sig_y));
          aa[1 + kk * np] = (*gg) * exp(-rad2_x/SQUARE(*sig_x) - rad2_y/SQUARE(*sig_y))
                            *  2 * rad2_x / ((*sig_x) * (*sig_x) * (*sig_x));
          aa[2 + kk * np] = (*gg) * exp(-rad2_x/SQUARE(*sig_x) - rad2_y/SQUARE(*sig_y))
                            *  2 * rad2_y / ((*sig_y) * (*sig_y) * (*sig_y));
          aa[3 + kk * np] = 1.;
          psi[kk] = pws_2d[i + j*nx] - (*gg) * exp(-rad2_x/SQUARE(*sig_x) 
                                 - rad2_y/SQUARE(*sig_y)) - (*hh);
          kk++;
          }
       }
  }
  nn = kk;

#ifdef DEBUG
    printf(" Number of input data points: %d\n",nn);
#endif

  phi[0] = 0.;
  phi[1] = 0.;
  phi[2] = 0.;
  phi[3] = 0.;
/* Least square minimization: */
  status = JLP_CGRAD(aa,psi,phi,&np,&nn,&ifail);

  *gg += phi[0];
  *sig_x += phi[1];
  *sig_y += phi[2];
  *hh += phi[3];

/* Remove Gaussian fit from input array 
* and compute the residuals: 
*/
  res = 0.; kk =0;
  for(j = 0; j < ny; j++) {
     rad2_y = SQUARE(j - ny/2);
     for(i = 0; i < nx; i++) {
       rad2_x = SQUARE(i - nx/2);
       gauss_2d[i + j * nx] = (*gg) * exp(- rad2_x/SQUARE(*sig_x)
                                          - rad2_y/SQUARE(*sig_y)) + (*hh);
       psi[i + j * nx] = pws_2d[i + j * nx] - gauss_2d[i + j * nx];
          if(rad2_x + rad2_y > fc2) {
            res += SQUARE(psi[i + j * nx]);
            kk++;
          }
       }
     }

  if(kk == 0) res = 0.;
     else res /= kk;
  printf("iter=%d gg=%g sig_x=%g hh=%g mean xhi2=%g (npts=%d)\n",
          iter, *gg, *sig_x, *hh, res, kk);
/* End of iteration #iter */
  }

strcpy(filename,"hege_resi");
sprintf(comments,"Residuals of Gaussian fit");
JLP_D_WRITEIMAG(psi,&nx,&ny,&nx,filename,comments);

strcpy(filename,"hege_gauss");
sprintf(comments,"Fitted Gaussian photon response");
JLP_WRITEIMAG(gauss_2d,&nx,&ny,&nx,filename,comments);

free(aa);
free(psi);
return(0);
}
/***********************************************************************
* Gaussian_like fit to 2-D array assuming zero offset: 
*  f(x) =  gg exp ( - x^2 / sig_x^2 - y^2 / sig_y^2 + ....)
*
* Linearized with:
*  log(f(x)) =  log(gg) - x^2 / sig_x^2 - y^2 / sig_y^2 + ...
***********************************************************************/
int gauss_like_fit_2d_zero_offset(float *pws_2d, float *gauss_2d, int fc, 
                                  INT4 nx, INT4 ny, float *gg, float *sig_x, 
                                  float *sig_y, char *filename, char *comments)
{
double phi[10], *aa, *psi;
float rad_x, rad_y;
int np, nn, kk, status, margin;
INT4 ifail;
int i, j;

/*
printf(" Enter initial guess for gg, sig_x  and sig_y:= ");
scanf("%f,%f", gg, sig_x, sig_y);
*/
*sig_x = nx/10.;
*sig_y = ny/10.;
*gg = pws_2d[(nx/2)+10 + (ny/2) * nx];

/* Linearize the problem: 
*    log(f(x,y)) = log(gg) - x^2/sig_x^2 - y^2 / sig_y^2 - a4 * |y|^1.5  
* or psi(x,y) = phi[0] - x^2 * phi[1] - y^2 * phi[2] - y^1.5 * phi[3]
*    with 
*      phi[0] = log(gg) 
*      phi[1] = 1/sig_x^2
*      phi[2] = 1/sig_y^2
*      psi[x,y] = log(f(x,y))
*        
*/
np = 9;
phi[0] = log(*gg);
phi[1] = 1./SQUARE(*sig_x);
phi[2] = 1./SQUARE(*sig_y);
for(i = 3; i < np; i++) phi[i] = 1.e-4;

psi = (double *)malloc(nx * ny *sizeof(double));
if(psi == NULL) {
  printf(" gauss_2d/Fatal error allocating memory: nx=%d ny=%d\n",
           nx, ny);
  exit(-1);
  }
aa = (double *)malloc(nx * ny * np * sizeof(double));
if(aa == NULL) {
  printf(" gauss_2d/Fatal error allocating memory: nx=%d ny=%d\n",
           nx, ny);
  exit(-1);
  }
/* Compute aa matrix: */
kk = 0;
/* Just in case... */
margin = 3;
  for(j = margin; j < ny - margin; j++) {
   rad_y = ABS(j - ny/2);
     for(i = margin; i < nx - margin; i++) {
       rad_x = ABS(i - nx/2);
       if((rad_x + rad_y > fc)
        && (pws_2d[i + j * nx] > 0.)) {
        aa[0 + kk * np] = 1;
        aa[1 + kk * np] = - SQUARE(rad_x);
        aa[2 + kk * np] = - SQUARE(rad_y);
        aa[3 + kk * np] = - rad_x * rad_y;
/* All next terms are really needed for fitting phot_gv7_128.fits: */
        aa[4 + kk * np] = - rad_x * rad_x * rad_x;
        aa[5 + kk * np] = - rad_y * rad_y * rad_y;
        aa[6 + kk * np] = - rad_x * rad_y * rad_y;
        aa[7 + kk * np] = - rad_x * rad_x * rad_y;
        aa[8 + kk * np] = - SQUARE(rad_x) * SQUARE(rad_x);
        psi[kk] = log(pws_2d[i + j * nx]);
        kk++;
        }
      }
   } 
nn = kk;
printf(" Number of input data points: %d\n",nn);

/* Least square minimization: */
status = JLP_CGRAD(aa,psi,phi,&np,&nn,&ifail);

printf("Solution: \n"); 
for(i = 0; i < np; i++) printf("phi[%d]=%g\n", i, phi[i]);

/* Remove Gaussian fit from input array: */
  for(j = 0; j < ny; j++) {
     rad_y = ABS(j - ny/2);
     for(i = 0; i < nx; i++) {
       rad_x = ABS(i - nx/2);
       gauss_2d[i + j * nx] = exp(phi[0]
                                  - phi[1] * SQUARE(rad_x)
                                  - phi[2] * SQUARE(rad_y)
                                  - phi[3] * rad_x * rad_y 
                                  - phi[4] * rad_x * rad_x * rad_x
                                  - phi[5] * rad_y * rad_y * rad_y
                                  - phi[6] * rad_x * SQUARE(rad_y)
                                  - phi[7] * rad_y * SQUARE(rad_x)
                                  - phi[8] * SQUARE(rad_x) * SQUARE(rad_x) 
                                  );
       psi[i + j * nx] = pws_2d[i + j * nx] - gauss_2d[i + j * nx];
       }
     }
strcpy(filename,"hege_resi");
sprintf(comments,"Residuals of Gaussian-like fit");
JLP_D_WRITEIMAG(psi,&nx,&ny,&nx,filename,comments);

strcpy(filename,"hege_gauss");
sprintf(comments,"Fitted Gaussian-like photon response");
JLP_WRITEIMAG(gauss_2d,&nx,&ny,&nx,filename,comments);

free(aa);
free(psi);
return(0);
}
