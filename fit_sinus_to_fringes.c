/************************************************************
* Program fit_sinus_to_fringes
* From Eric Aristidi's pasep.c
*
* SYNTAX:
*  fringes.exe modsq_reference modsq_binary 
*              f_atm,f_tel Fourier/Direct,angle,sep nlines
*
*    input: (modsq = power spectrum)!
*    f_atm, f_tel: radii in pixels (used for boundaries for the fit)
*    F or D: Fourier or Direct domain
*    angle: angle of the central fringe relative to Ox (trigonometric sense) 
*    sep: period of the fringes (distance between two minima, or two maxima)
*    nlines: number of lines to be integrated
*
* Example:
* #%/bin/csh
* runs fringes hr933v1_m capellav_m 25,48 F,53.,35. 10
* # negative separation if input value is accurate enough
* clean:
* rm -f dist.dat erreur2d.fits erreurrot.fits
* rm -f visib2d.fits visibfit.dat visibrot.fits
* end:
*
* JLP
* Version 27/01/2003
*************************************************************/
/*
Au sujet du programme d'ajustement de sinusoides :
------------------------------------------------

Il n'ajuste pas a deux dim, mais a une dim. 

- Il faut donc prealablement connaitre l'angle de position (je fais ca
sur les franges: j'integre dans une direction et je maximise le
contraste, je le fais "a la main", en me disant a chaque fois qu'il
faudrait automatiser ca).

- Ensuite on choisit si on ajuste sur une coupe des franges ou sur la
somme de N coupes (en fait ce qui marche le mieux c'est une coupe).

- On obtient une fonction W(u) a une dim. On ajuste une sinusoide la
dessus, l'ajustement se fait entre u_min et u_max. u_min est > frequence
de coupure atm., u_max < freq. coupure telescope.

Le programme donne separation (avec erreur) et delta m (la je fais
moyennement confiance, c'est generalement assez foireux). On genere
aussi un fichier pour mon logiciel graphique.

Eric
*/
/* From Eric Aristidi's Pasep.c - Version 2.3 (24 avril 97)
JLP99: Attention, l'integration se fait le long des colonnes
                  les franges sont tournees pour etre // Oy

Modifs pour la version 2.0
--------------------------
- Nouveau calcul des barres d'erreur sur e et m (NR)
- calcul de distance sur d et arret si d_{n}-d_{n-1} < \epsilon
- la barre d'erreur sur d est prise egale a epsilon
- soustraction de la moyenne sur un rectangle : on prend la coupe 1D

Modifs pour la version 2.1
--------------------------
- il y avait une erreur sur les barres d'erreur de e et m -> correction
- calcul des barres d'erreur sur d en par la formule du fit de acos((z-e)/m)=2 PI x/d

Modifs pour la version 2.2
--------------------------
- Prise en compte de plusieurs visibilites pour des classes differentes

Modifs pour la version 2.3
--------------------------
- remise de la moyenne sur un rectangle pour la visib 1D (nlines)
- soustraction de la troncature a fc

====================================================*/

#include <stdio.h>
#include <math.h>
#include "aris_lib.c"
#include "jlp_ftoc.h"

/* #define ECH 0.0309	
(secondes par pixel)
*/
#define ECH 1	

#define TWOPI (2.*PI)
/* Defined in jlp_ftoc.h:
#define SQUARE(a) ((a)*(a))
*/

/* Contained here: */
int visibcalc(float *pwsdbl, float *pwsref, float *visib2d, float *erreur2d,
              int dim, int fc);
int calcul_em(float *visib1d, float *erreur1d, int dim, int fa, int fc, 
              float d, float *e, float *m, float *dist);
int extract_cut(float *visib2d, float *erreur2d, float *visib1d, 
                float *erreur1d, int fc, int fa, float angle, int dim, 
                int nlines);
int cosine_fit(float *visib1d, float *erreur1d, int fc, int fa, float sep, 
               int dim, float *ee, float *mm, float *dd, float *alpha);
int plot_visib1(float *visib1d, float *erreur1d, int fc, int fa,
             int dim, char *filename, char *comments, char *plotdev);
int plot_fit(float *visib1d, float *erreur1d, int fc, int fa, float sep, 
             int dim, float ee, float mm, float dd, 
             char *filename, char *comments, char *plotdev);
int jlp_calcul_em(float *visib1d, float *erreur1d, int dim, int fa, int fc, 
              float d, float *e, float *m, float *dist);

int jlp_cosine_fit(float *visib1d, float *erreur1d, int fc, int fa, float sep, 
                   int dim, float *ee, float *mm, float *dd, float *alpha);
static double jlp_cosine_func(double *xx);
static void jlp_cosine_grad(double *xx, double *dx);

/* Needed by jlp_cosine_fit */
static float *ff, *yy, xx3;
static int nn, nn1, iposit;
/* In ../dcv/dcv_cgrad.c */
int jlp_minimize(double *x, int n, int nmax, double ftol, int *iter,
                  double *fret, double (*func)(double *),
                  void (*dfunc)(double *, double *), int positive);
static int jlp_check_grad(double (*func)(double []),
                          void (*dfunc)(double[], double[]));


/* ===============================================================
	Programme principal
================================================================= */
int main(int argc, char **argv)
{
int	dim, fc, fa, nbclass, tabclass[10], nlines, istat;
float	angle, sep, ee, dd, mm, alpha;	
/* attn : alpha c'est le rapport d'intensite <> angle */
float	*pwsdbl, *pwsref, *visib2d, *erreur2d, *erreur1d, *visib1d; 
float	*visib2d1, *erreur2d1; 
INT4    nx, ny;
INT_PNTR pntr_ima;
char	nomfics[61], nomficd[61], filename[61], comments[81];
char	nomfics1[90], nomficd1[90], plotdev[32], buffer[30], domain[2];
register int i, j;
	
JLP_INQUIFMT();

/* Syntax: */
if(argc != 7 && argc != 6 && argc != 1)
 {
 printf("USAGE: fringes.exe nber_of_classes reference binary f_atm,f_tel D/F,angle,sep nlines\n");
 printf(" (Possibility of interactive input of parameters if no parameters)\n");
 printf("argc = %d\n",argc);
 exit(-1);
 }

/* ---------- Input of the parameters --------------------- */


if(argc == 1)
  {
   printf("Nber of classes [1]?");
   scanf("%d",&nbclass);
   if (nbclass > 1) { 
      for (i = 0; i < nbclass; i++) {
        printf("\tClasse no :"); 
        scanf("%d",&tabclass[i]);
        }
      }
   printf("Sq. mod. of the reference star: (generic name if many classes):= ");
   scanf("%s",nomfics);
   printf("Sq. mod of the binary star: (generic name if many classes):= ");
   scanf("%s",nomficd);
  }
else
  {
/* Nber of classes: */ 
   sscanf(argv[1],"%d",&nbclass);
      printf(" OK: nbclass =%d\n",nbclass);
   if (nbclass > 1) { 
      for (i = 0; i < nbclass; i++) {
        tabclass[i] = i+1;
        printf(" OK: class #%d added \n",tabclass[i]);
        }
      }
/* Reference star: */
   strcpy(nomfics,argv[2]);
/* Binary star: */
   strcpy(nomficd,argv[3]);
   printf("Reference star: %s \n Binary star: %s \n",nomfics,nomficd);
  }

 if (nbclass <= 1) nbclass = 1;
	
if(argc == 1)
  {
  printf("Approx. atmospheric and telescope cut-off frequencies. (pixels) = ");
  scanf("%d,%d",&fa,&fc);
  domain[0] = 'F';
  printf("FOURIER domain: Inclination of the fringes (degrees relative to OX)\n"); 
  printf("and estimate of the separation of fringes (pixels) \n");
  printf("Give a negative separation if input value is accurate enough\n");
  scanf("%f,%f",&angle,&sep); 
  }
else
  {
  sscanf(argv[4],"%d,%d",&fa,&fc);
  strcpy(buffer,argv[5]);
  domain[0] = buffer[0];
  sscanf(&buffer[2],"%f,%f",&angle,&sep);
  }

/* Conversion to Fourier parameters if necessary: */
  if(domain[0] == 'D' || domain[0] == 'd')
    {
    angle += 90.;
    sep = dim / sep;
    printf("OK: conversion of input angle and separation to Fourier domain \n");
    }
  else
    printf(" OK: input angle and separation in Fourier domain \n");

    printf(" angle = %f (deg) separation = %f (pixels)\n", angle, sep);

/* Rotation of PI/2-angle to align the fringes parallel to the Y axis: */
    angle = (PI/2.) - angle * (PI/180.);

if(argc == 1)
  {
  printf("Number of lines too integrate (even number) (0 if only a slice) =");
  scanf("%d",&nlines);
  }
else
  sscanf(argv[6],"%d",&nlines);

  nlines = (nlines + 1)/2;
  printf("OK, integration along %d lines around the central line\n",nlines);
  printf("OK, fa=%d fc=%d\n", fa, fc);
	
	
/* Reference: only for size determination*/
   if(nbclass > 1)
      sprintf(nomfics1,"%s%d",nomfics,tabclass[0]);
   else
      strcpy(nomfics1,nomfics);

   JLP_VM_READIMAG1(&pntr_ima,&nx,&ny,nomfics1,comments);
   free((float *)pntr_ima);
   dim = nx;
   if(ny != nx) { 
     printf("Fatal error: nx=%d != ny=%d \n", nx, ny); 
     exit(-1);
     }

/* Allocation of memory  ------------------------------- */
  visib2d=(float *) malloc(dim*dim*sizeof(float));
  erreur2d=(float *) malloc(dim*dim*sizeof(float));
  visib1d=(float *) malloc(dim*sizeof(float));
  erreur1d=(float *) malloc(dim*sizeof(float));

  if (nbclass>1)
  {
    visib2d1=(float *) malloc(dim*dim*sizeof(float));
    erreur2d1=(float *) malloc(dim*dim*sizeof(float));
  }

/*	Cas d'une seule classe  ------------------------------- */

  if (nbclass==1)
   {
/* Reference: */
    istat=JLP_VM_READIMAG1(&pntr_ima,&nx,&ny,nomfics,comments);
    pwsref = (float *)pntr_ima; 
    if(istat != 0) {
      printf(" Fatal error reading %s istat = %d \n",nomfics,istat);
      exit(-1);
     }
   if(nx != dim || ny != dim) { 
     printf("Fatal error: nx=%d or ny=%d != dim=%d\n", nx, ny, dim); 
     exit(-1);
     }

/* Double star: */
    istat=JLP_VM_READIMAG1(&pntr_ima,&nx,&ny,nomficd,comments);
    pwsdbl = (float *)pntr_ima;
    if(istat != 0) {
      printf(" Fatal error reading %s istat = %d \n",nomficd,istat);
      exit(-1);
     }
   if(nx != dim || ny != dim) { 
     printf("Fatal error: nx=%d or ny=%d != dim=%d\n", nx, ny, dim); 
     exit(-1);
     }


/* calcul visib et erreur a 2 d: */
/* JLP2003: I put a larger fc here, to avoid edge pb: */
     visibcalc(pwsdbl,pwsref,visib2d,erreur2d,dim,fc+2);
   }
  else
/* Cas de plusieurs classes: (on ajoute toutes les visibilites calculees pour
   chaque classe) ----------------------------------------------------*/
   {
     raz(visib2d,dim*dim);
     raz(erreur2d,dim*dim);
     for (i=0; i<nbclass; i++)
	{
	  printf("-> Calcul classe no %d\n",tabclass[i]);
	  sprintf(nomfics1,"%s%d",nomfics,tabclass[i]);
	  sprintf(nomficd1,"%s%d",nomficd,tabclass[i]);
/* Reference: */
          JLP_VM_READIMAG1(&pntr_ima,&nx,&ny,nomfics1,comments);
          pwsref = (float *)pntr_ima;
          if(nx != dim || ny != dim) { 
             printf("Fatal error: nx=%d or ny=%d != dim=%d\n", nx, ny, dim); 
             exit(-1);
             }

/* Double star: */
          JLP_VM_READIMAG1(&pntr_ima,&nx,&ny,nomficd1,comments);
          pwsdbl = (float *)pntr_ima;
          if(nx != dim || ny != dim) { 
             printf("Fatal error: nx=%d or ny=%d != dim=%d\n", nx, ny, dim); 
             exit(-1);
             }

/* calcul visib et erreur a 2 d	*/
	  visibcalc(pwsdbl,pwsref,visib2d1,erreur2d1,dim,fc);
/* On ajoute les visibilites: */
          for (j=0; j<dim*dim; j++) 
            {
            visib2d[j] += visib2d1[j]; 
            erreur2d[j] += erreur2d1[j];
            }
        }
  }

strcpy(filename,"visib2d");
JLP_WRITEIMAG(visib2d,&nx,&ny,&nx,filename,comments);

/* JLP99:
strcpy(filename,"erreur2d");
JLP_WRITEIMAG(erreur2d,&nx,&ny,&nx,filename,comments);
*/

/* Projection de visib et erreur sur les tableaux 1D visib1d et erreur1d*/	
extract_cut(visib2d,erreur2d,visib1d,erreur1d,fc,fa,angle,dim,nlines);

/* Set plotdev to "xterm", i.e. X11 output: */
strcpy(plotdev,"xterm_small");
/*
printf(" Plotting device : ");gets(plotdev);strcpy(plotdev,"square");
*/

plot_visib1(visib1d, erreur1d, fc, fa, dim, nomficd, comments, plotdev);

/* Cosine fit to these 1-D arrays */
/* The two methods seem equivalent (except that "fixed" separation has not
* been implemented in JLP's method yet)
*/
#ifndef TTT
cosine_fit(visib1d,erreur1d,fc,fa,sep,dim,&ee,&mm,&dd,&alpha);
#else
jlp_cosine_fit(visib1d,erreur1d,fc,fa,sep,dim,&ee,&mm,&dd,&alpha);
#endif

plot_fit(visib1d, erreur1d, fc, fa, sep, dim, ee, mm, dd, nomficd, comments,
         plotdev);
printf(" I copy this plot to \"pst.tmp\" \n");
strcpy(plotdev,"landscape");
plot_fit(visib1d, erreur1d, fc, fa, sep, dim, ee, mm, dd, nomficd, comments,
         plotdev);

return(0);
}

/* ----------------------------------------------------------------------
* Projection d'une image sur une colonne
*
* INPUT:
*  visib2d, erreur2d
*
* OUTPUT:
*  visib1d, erreur1d
---------------------------------------------------------------------- */
int extract_cut(float *visib2d, float *erreur2d, float *visib1d, 
                float *erreur1d, int fc, int fa, float angle, int dim, 
                int nlines)
{
float	*erreurrot,*visibrot;
int	i, j;
FILE	*fopen();
INT4    idim;
char filename[60], comments[80];
	
visibrot=(float *)malloc(dim*dim*sizeof(float));
erreurrot=(float *)malloc(dim*dim*sizeof(float));
		
/* Rotation des 2 tableaux (err et vis)
   ------------------------------------
*/
rotimage(visib2d,visibrot,dim,dim,(double)angle);
rotimage(erreur2d,erreurrot,dim,dim,(double)angle);

/*
ascwrite("visibrot.dat",dim,dim,visibrot);
ascwrite("erreurrot.dat",dim,dim,erreurrot);
*/
idim = dim;
strcpy(filename,"visibrot");
sprintf(comments,"After rotation of %f degrees",(angle * 180 / PI));
JLP_WRITEIMAG(visibrot,&idim,&idim,&idim,filename,comments);
/* JLP99:
strcpy(filename,"erreurrot");
JLP_WRITEIMAG(erreurrot,&idim,&idim,&idim,filename,comments);
*/
	

/* Calcul des projections (err et vis) d'un rectangle de hauteur nlines (non pondere)
   -----------------------------------------------------------------
*/
raz(visib1d,dim);
raz(erreur1d,dim);

if (nlines >= 2)
{
   for (j=(dim-nlines)/2; j<=(dim+nlines)/2; j++) 
   {
        for (i=dim/2-fc; i <= dim/2+fc; i++)
	{
		visib1d[i] += visibrot[i + j*dim]/nlines;
/* variance: */
		erreur1d[i] += erreurrot[i + j*dim]/nlines;	
	}
   }
}
/* nlines=0 */
else 
{
/* Coupe de la visibilite 
   ----------------------
*/
   for (i=dim/2-fc; i <= dim/2+fc; i++)
   {
	j=dim/2;
	visib1d[i] = visibrot[i + j*dim];
/* variance */
	erreur1d[i] = erreurrot[i + j*dim];	
   }
}

return(0);
}

/*----------------------------------------------------------------------
*   calcul visib et erreur a 2 d
* --------------------------------------------------------------------- */
int visibcalc(float *pwsdbl, float *pwsref, float *visib2d, float *erreur2d,
              int dim, int fc)			
{
int	i,j,li=10,lj=10, irad2, ifc2;
float	w_ref, w_dbl, sigmadbl=0, sigmaref=0, moydbl=0, moyref=0;

/* Estimation du sigma de l'etoile double et de la reference
*  sur un petit carre (10*10 pixels) en bas au milieu de densite spectrale
*  les coins sont contamines par l'effet d'autoroute qui divise par dix le sigma
*/
for (i=0; i<li; i++) 
for (j=dim/2-lj/2; j<dim/2+lj/2; j++) 
{
	moyref+= pwsref[i*dim+j]/(li*lj);
	moydbl+= pwsdbl[i*dim+j]/(li*lj);
	sigmaref+= pwsref[i*dim+j] * pwsref[i*dim+j] / (li*lj);
	sigmadbl+= pwsdbl[i*dim+j] * pwsdbl[i*dim+j] / (li*lj);
}
sigmadbl=sqrt(sigmadbl); 
sigmaref=sqrt(sigmaref);
printf("sigma_double_star=%f  sigma_reference_star=%f\n",sigmadbl,sigmaref);

ifc2 = fc * fc;

for (i=0; i<dim; i++)
for (j=0; j<dim; j++)
{
/* JLP99: protection against division by zero:*/
  w_ref = pwsref[i*dim+j];
  if(w_ref > 0.) w_ref = 1. / w_ref; 
  else w_ref = 0.;

  w_dbl = pwsdbl[i*dim+j];
  if(w_dbl > 0.) w_dbl = 1. / w_dbl; 
  else w_dbl = 0.;

  irad2 = SQUARE(i-dim/2)+SQUARE(j-dim/2);
	if (irad2 > ifc2)
	{
	  visib2d[i*dim+j] = 0.;
	  erreur2d[i*dim+j] = 0.;
	}
	else 
	{
	  visib2d[i*dim+j] = pwsdbl[i*dim+j] * w_ref;
/* Ici erreur2d est la variance */
          erreur2d[i*dim+j] = visib2d[i*dim+j] 
                             * (sigmaref * w_ref + sigmadbl * w_dbl);
	  erreur2d[i*dim+j] = erreur2d[i*dim+j] * erreur2d[i*dim+j];
	}
}

return(0);
}

/*------------------------------------------------------------------------------
* Estimation de m et e pour d fixe
------------------------------------------------------------------------------*/

int calcul_em(float *visib1d, float *erreur1d, int dim, int fa, int fc, 
              float d, float *e, float *m, float *dist)
{
float	a,b,c,h,n,poids,deltay;
int	i,ii;

a=0; b=0; c=0; h=0; n=0;
/* calcul params */
for (i=dim/2+fa; i <= dim/2+fc; i++)		
{
	ii=i-dim/2;
	if (erreur1d[i]!=0) 
          poids = 1./erreur1d[i]; 
        else 
          poids=0;
	a += visib1d[i]*cos(2*PI*ii/d)*poids;
	b += poids*cos(2*PI*ii/d);
	c += visib1d[i]*poids;
	h += poids*cos(2.*PI*ii/d)*cos(2.*PI*ii/d);
	n += poids;
}
*e=(h*c-b*a)/(n*h-b*b);
*m=(n*a-b*c)/(n*h-b*b);

/*	 Calcul du chi2
 	---------------
*/
*dist=0;
/* calcul distance et chi2 */
for (i=dim/2+fa; i<= dim/2+fc; i++)		
{
	ii=i-dim/2;
	deltay =(visib1d[i]- (*e)-(*m)*cos(2*PI*ii/d));
	if (erreur1d[i] != 0.) 
           poids=1/erreur1d[i]; 
        else 
           poids=0;
	*dist += deltay*deltay*poids;
}

return(0);
}
/*------------------------------------------------------------------------------
* Estimation de m et e pour d fixe
* JLP99 version (actually the same as the old one...)
------------------------------------------------------------------------------*/

int jlp_calcul_em(float *visib1d, float *erreur1d, int dim, int fa, int fc, 
              float d, float *e, float *m, float *dist)
{
float	a,b,c,h,n,poids,deltay;
int	i,ii;

a=0; b=0; c=0; h=0; n=0;
/* calcul params */
for (i=dim/2+fa; i<= dim/2+fc; i++)		
{
	ii=i-dim/2;
	if (erreur1d[i]!=0) 
          poids = 1./erreur1d[i]; 
        else 
          poids=0;
	a += visib1d[i]*cos(2*PI*ii/d)*poids;
	b += poids*cos(2*PI*ii/d);
	c += visib1d[i]*poids;
	h += poids*cos(2.*PI*ii/d)*cos(2.*PI*ii/d);
	n += poids;
}
*m = ( n * a - c * b) / (n * h - b * b);
*e = (c - (*m) * b ) / n;

/* Old: (identical...!) 
*e=(h*c-b*a)/(n*h-b*b);
*m=(n*a-b*c)/(n*h-b*b);
*/
/*	 Calcul du chi2
 	---------------
*/
*dist=0;
/* calcul distance et chi2 */
for (i=dim/2+fa; i<=dim/2+fc; i++)		
{
	ii=i-dim/2;
	deltay =(visib1d[i]- (*e)-(*m)*cos(2*PI*ii/d));
	if (erreur1d[i] != 0.) 
           poids=1/erreur1d[i]; 
        else 
           poids=0;
	*dist += deltay*deltay*poids;
}

return(0);
}

/* ---------------------------------------------------------------------
/ To display the visibility data
/
----------------------------------------------------------------------*/
int plot_visib1(float *visib1d, float *erreur1d, int fc, int fa,
             int dim, char *filename, char *comments, char *plotdev)
{
float *xplot, *yplot;
float errx[1],erry[1],xout[20],yout[20];
float xmin, xmax, ymin, ymax;
INT4 ncurves, nout, error_bars, npts[2], plan, full_caption;
INT4 width_frame, height_frame, devnum;
int status;
char xlabel[41], ylabel[41], title[81], nchar[8], pcolor[60];
register int i;

/* Allocation of memory: */
xplot = (float*) malloc(dim * sizeof(float));
yplot = (float*) malloc(dim * sizeof(float));
if(xplot == NULL || yplot == NULL)
  {
  printf("plot_fit/error allocating memory: dim=%d\n", dim);
  return(-1);
  }

for(i = 0; i < dim; i++)
   {
   xplot[i] = i;
   yplot[i] = visib1d[i];
   }

ymin = yplot[0];
ymax = yplot[0];
for(i = 1; i < dim; i++) 
   {if(ymin > yplot[i]) ymin = yplot[i]; 
   if(ymax < yplot[i]) ymax = yplot[i];} 
xmin = xplot[0];
xmax = xplot[dim-1];
strcpy(nchar,"L0");
strcpy(pcolor,"Default");
strcpy(xlabel," ");
strcpy(ylabel," ");
strcpy(title," ");

plan = 0;
ncurves = 1;
error_bars = 0;
strcpy(&nchar[4],"L");
strcpy(&pcolor[30],"Default");

/* Initialize plotting device: */
width_frame = 1;
height_frame = 1;
 status = JLP_DEVICE(plotdev,&width_frame,&height_frame,
                     &xmin,&xmax,&ymin,&ymax,&plan,filename,&devnum);
if(status) {
   printf(" Fatal error opening graphic device: %s \n", plotdev);
   exit(-1);
   }

/* Display the curve: */
npts[0] = dim;
full_caption = 0;
newplot22(xplot,yplot,errx,erry,npts,&dim,&ncurves,
           xlabel,ylabel,title,nchar,pcolor,
           xout,yout,&nout,&error_bars,filename,comments,&full_caption);

/* Close display device: */
JLP_SPCLOSE();

return(0);
}
/* ---------------------------------------------------------------------
/ To display the data and the fitted cosine function
/
----------------------------------------------------------------------*/
int plot_fit(float *visib1d, float *erreur1d, int fc, int fa, float sep, 
             int dim, float ee, float mm, float dd, 
             char *filename, char *comments, char *plotdev)
{
float *xplot, *yplot;
float errx[1],erry[1],xout[20],yout[20], xrange, yrange;
float xmin, xmax, ymin, ymax;
INT4 ncurves, nout, error_bars, npts[3], plan, full_caption;
INT4 width_frame, height_frame, devnum;
int status;
char xlabel[41], ylabel[41], title[81], nchar[12], pcolor[90];
register int i, k;

/* Allocation of memory: */
xplot = (float*) malloc(3 * dim * sizeof(float));
yplot = (float*) malloc(3* dim * sizeof(float));
if(xplot == NULL || yplot == NULL)
  {
  printf("plot_fit/error allocating memory: dim=%d\n", dim);
  return(-1);
  }

k = 0;
for(i = dim/2 - fc; i <= dim/2 + fc; i++) {
/*
   xplot[i] = (i-dim/2)/((float)dim*ECH);
*/
   xplot[k] = i - dim/2;
   yplot[k] = ee + mm*cos(2*PI*(i-dim/2)/dd);
   k++;
   }
npts[0] = k;

k = 0;
for(i = dim/2 - fc; i <= dim/2; i++) {
   if(i <= dim/2-fa) { 
     xplot[k + dim] = i - dim/2;
     yplot[k + dim] = visib1d[i];
     k++;
     }
   }
npts[1] = k;

k = 0;
for(i = dim/2; i <= dim/2 + fc; i++) {
   if(i >= dim/2+fa) { 
     xplot[k + 2 * dim] = i - dim/2;
     yplot[k + 2 * dim] = visib1d[i];
     k++;
     }
   }
npts[2] = k;

/* Scale computed only with cosine function: */
ymin = yplot[0];
ymax = yplot[0];
for(k = 1; k < npts[0]; k++) {
   if(ymin > yplot[k]) ymin = yplot[k]; 
   if(ymax < yplot[k]) ymax = yplot[k];
   }
yrange = ymax - ymin;
ymin -= yrange/10.;
ymax += yrange/10.;

xmin = xplot[0];
xmax = xplot[2*fc];
xrange = xmax - xmin;
xmin -= xrange/10.;
xmax += xrange/10.;
strcpy(nchar,"L0");
strcpy(&nchar[4],"L1");
strcpy(&nchar[8],"L1");
strcpy(pcolor,"Default");
strcpy(&pcolor[30],"Default");
strcpy(&pcolor[60],"Default");
strcpy(xlabel," ");
strcpy(ylabel," ");
strcpy(title," ");

plan = 0;
ncurves = 3;
error_bars = 0;

width_frame = 1;
height_frame = 1;
 status = JLP_DEVICE(plotdev,&width_frame,&height_frame,
                     &xmin,&xmax,&ymin,&ymax,&plan,filename,&devnum);
if(status) {
   printf(" Fatal error opening graphic device: %s \n", plotdev);
   exit(-1);
   }

/* Display the curve: */
full_caption = 0;
newplot22(xplot,yplot,errx,erry,npts,&dim,&ncurves,
           xlabel,ylabel,title,nchar,pcolor,
           xout,yout,&nout,&error_bars,filename,comments,&full_caption);

/* Close display device: */
JLP_SPCLOSE();

return(0);
}
/* ----------------------------------------------------------------------
*   calcul separation et rapport d'intensite
* On fitte Z(f)=e + m.cos(2 PI f d) par moindres carres
* ref : stage erwan et thierry, p 16
* If sep is negative use the value provided by the user only and fits
* the magnitude difference only.
* 
* sep sert d'estimation initiale de la separation
*--------------------------------------------------------------------- */

int cosine_fit(float *visib1d, float *erreur1d, int fc, int fa, float sep, 
               int dim, float *ee, float *mm, float *dd, float *alpha)
{
int	i,ii;
float	grain=0.5,olddist;
FILE	*fic,*fopen();
/* Variables pour l'estimation de e et m */
float	mag1,mag2,mag,dmag,b,c,h,n,e,m,poids,sigmae,sigmam;
float	dist,contr,dqvb,eqvb,mqvb;
float	deltacontr,dalpha,deltadist;
float	d,oldd;

printf(" Now fitting:  Z(f)=e + m.cos(2 PI f d) \n");
/* Boucle de recherche du min de distance en faisant varier d
   ----------------------------------------------------------
*/
/*
* If sep is negative use the value provided by the user only and fits
* the magnitude difference only.
*/
 if(sep < 0.)
   {
    d= -1. * sep;
    printf("------\nFitting only e and m\n");
   }
 else
   {
	printf("------\nFitting all parameters: d, e and m\n");
	fic=fopen("dist.dat","w");
/* periode de la sinusoide */
	d=sep;
	calcul_em(visib1d,erreur1d,dim,fa,fc,d,&e,&m,&dist);	
	do
	{
		do
		{
		calcul_em(visib1d,erreur1d,dim,fa,fc,d,&e,&m,&olddist);
		calcul_em(visib1d,erreur1d,dim,fa,fc,d+grain,&e,&m,&dist);
		printf("dist=%g  -  grain=%g   - d=%g  - deltadist=%e\n",
                         dist,grain,d,dist-olddist);
		d+=grain;
		} while (dist<olddist);
	fprintf(fic,"%e\n",dist);
	grain*=-0.5;
	}
	while(ABS(dist-olddist)>0.0001);
	fclose(fic);
	d-=grain; 
   }
   calcul_em(visib1d,erreur1d,dim,fa,fc,d,&e,&m,&dist);
   dqvb=d; eqvb=e; mqvb=m;
   if(m < 0.) m *= -1.;
   printf(" Result of the fit: e=%g  m=%g  (m/e=%g)  d=%g\n",e,m,m/e,d);

	oldd=d;

/* Estimation de e a d fixe
(JLP99: taking the mean as an approximation of e)
  -----------------------------
*/
	c=0; n=0;
/* calcul params */
	for (i=dim/2+fa; i<=dim/2+fc; i++)		
	{
		ii=i-dim/2;
		if (erreur1d[i]!=0) 
                  poids=1./erreur1d[i]; 
                else 
                  poids=0.;
		c += visib1d[i]*poids;
	        b += poids*cos(2*PI*ii/d);
	        h += poids*cos(2.*PI*ii/d)*cos(2.*PI*ii/d);
		n+=poids;
	}
	e=c/n;
	printf("After fixing the value of d, I found: e=%g\n",e);

/* Calcul de l'erreur sur e et m 
   -----------------------------
*/
	e=eqvb;
	m=mqvb;
	sigmae=0; sigmam=0;
	sigmae=n/(n*h-b*b); sigmam=h/(n*h-b*b);
	sigmae=sqrt(sigmae); sigmam=sqrt(sigmam);
	printf("\t sigma e=%g  -- sigma m=%g\n",sigmae,sigmam);		

/* Estimation de l'erreur sur d et affichage
   -----------------------------------------
*/
	deltadist=0;
	for (i=dim/2+fa; i<=dim/2+fc; i++)
	{
		ii=i-dim/2;
		if ((*(erreur1d+i))!=0) 
                  poids=ABS(m*m-SQUARE((*(visib1d+i)-e)))/(*(erreur1d+i)); 
                else 
                  poids=0;
		deltadist+=ii*ii*poids;
	}

	deltadist = (1./deltadist); 
        deltadist *= (d*d*d*d / (4.*PI*PI)); 
        deltadist = sqrt(deltadist);
	printf("Fourier domain:   d=%g +/- %g (pixels)\n",d,deltadist);
	printf("Direct space:     d=%g +/- %g (pixels)\n",
                dim/d,(deltadist/d)*(dim/d));
        if(m < 0.) m *= -1.;
	eqvb=e; mqvb=m;
/*
Quand on calcule la visibilite des franges, on fait la TF de l'image
dont on prend le module au carre.
si l'etoile double s'ecrit delta(x)+a delta(x-d) alors le module carre
de la TF s'ecrit: 
     1+a^2+2a cos(2 pi f d) 
en posant:
     e=1+a^2 et  m=2a
le contraste m/e est egal a: m/e = 2/(a+1/a). Il est bien insensible au
changement de a en 1/a (inversion des positions des deux etoiles).
*/
        if(eqvb != 0.)
           {
/* Contrast: */
           contr=mqvb/eqvb;
/* Luminosity ratio: */
	   *alpha = 1./contr + sqrt(1./(contr*contr) - 1.);
	   deltacontr=contr*(sigmae/eqvb+sigmam/mqvb);
/* JLP99: original formula is bad
	   dalpha=2 * (*alpha) * (*alpha) / 
                  (contr * contr *((*alpha) * (*alpha) -1)*deltacontr);
I propose:
*/
           dalpha = (eqvb+0.5*sigmae)/(mqvb-0.5*sigmam)-
                      (eqvb-0.5*sigmae)/(mqvb+0.5*sigmam);
	   printf("alpha=%g +/- %g\n",*alpha,dalpha);
           mag = 2.5 * log10((double)(*alpha));
           mag1 = 2.5 * log10((double)(*alpha - dalpha));
           mag2 = 2.5 * log10((double)(*alpha + dalpha));
           dmag = (mag2 - mag1)/2.;
           if(dmag < 0.) dmag *= -1.;
	   printf(" Hence: Delta_mag=%.2f +/- %.2f\n", mag, dmag);
           }

/* Sauvegarde de la visibilite avec barres d'erreurs (utiliser avec abscissa)
   -------------------------------------------------------------------------
*/
/* JLP99
	if ((fic=fopen("visibfit.dat","w"))!=NULL)
 	{
   	   for (i=0;i<dim;i++) 
	      {
		if ((ABS(i-dim/2)>fa)) 
                  fprintf(fic,"%g %g %g %g\n",(i-dim/2)/((float)dim*ECH),
                          eqvb+mqvb*cos(2*PI*(i-dim/2)/dqvb), 
                          visib1d[i],sqrt(erreur1d[i]));
		else 
                  fprintf(fic,"%g %g %g 0.00\n",(i-dim/2)/((float)dim*ECH),
                          eqvb+mqvb*cos(2*PI*(i-dim/2)/dqvb), 
                          visib1d[i]);
	      }
  	   fclose(fic); 
 	}
*/

*ee = eqvb;
*mm = mqvb;
*dd = dqvb;

return(0);
}

/***************************************************************************
*
***************************************************************************/
int jlp_cosine_fit(float *visib1d, float *erreur1d, int fc, int fa, float sep, 
                   int dim, float *ee, float *mm, float *dd, float *alpha)
{
double xx[4], ww;
double ftol, fret;
int iter, positive;
register int i, k;

nn = 2 * (fc + 1);
yy = (float *)malloc(nn * sizeof(float));
ff = (float *)malloc(nn * sizeof(float));

printf(" jlp_cosine_fit/Now fitting:  Z(f)=e + m.cos(2 PI f d) \n");

/* Transfer to yy: */
   k = 0;
   for (i=dim/2-fc; i<=dim/2+fc; i++) {
       if(i < dim/2-fa || i > dim/2+fa) { 
       yy[k] = visib1d[i];
       ff[k] = i - dim/2;
       k++;
       }
     }
 printf(" DEBUG/%d values, whereas nn=%d\n", k, nn);
 nn = k;

/* Starting point: ee is taken as the mean value*/
ww = 0.;
   for (k=0; k < nn; k++) ww += yy[k];
xx[1] = ww/(float)nn;

/* Starting point: mm is taken equal to the semi-amplitude */
ww = 0.;
   for (k=0; k < nn; k++) ww = MAXI(ww, yy[k] - xx[1]);
xx[2] = ww;

xx[3] = 1./abs(sep);

  printf(" Initial values: ee=%f mm=%f dd=%f\n",
           xx[1],xx[2],(float)abs((double)sep));

/* When fitting ee, mm, and dd: nn1=3,
   when fitting ee and mm only: nn1=2
*/
if(sep < 0) {
   printf("------\nFitting only e and m\n");
   nn1 = 2;
/* JLP2003/I found it necessary to use and set this variable: */
   xx3 = xx[3];
   }
 else {
   printf("------\nFitting all parameters: e, m and d\n");
   nn1 = 3;
   }

/* JLP/DEBUG: */
 jlp_check_grad(jlp_cosine_func, jlp_cosine_grad);

ftol = 1.e-5;
positive = 0;
/* Transfer to static variable: */
iposit = positive;


  jlp_minimize(xx, nn1, nn1, ftol, &iter, &fret,
               jlp_cosine_func, jlp_cosine_grad, positive);

  printf(" Number of iterations: %d \n",iter);
  printf(" Value of the minimum: %.5g\n",fret);

/* Transfer to initial variables: */
*ee = xx[1];
*mm = xx[2];
if(xx[3] != 0.) *dd = 1./xx[3];
else *dd = 0.;

  printf("Solution z=ee+mmcos(2*PI*(i-dim/2)/dd): ee=%.3f mm=%.3f dd=%.3f\n",
           *ee,*mm,*dd);

free(yy);
free(ff);
return 0;
}
/**************************************************************
* Function E to be minimized 
* E : value of the criterium in X
*            
* E = |Y(f_i)- Z(X, f_i)|^2
*
* with Z(f_i) = e + m cos(TWOPI f_i d)
* and xx[1] = e, xx[2] = m, xx[3] = d
*
**************************************************************/
static double jlp_cosine_func(double *xx)
{
double ww;
register int i;

/* JLP2003/I found it necessary to use and set this variable: */
if(nn1 == 3) xx3 = xx[3];

ww = 0.;
  for(i = 1; i <= nn; i++) {
    ww += SQUARE( yy[i-1] - (xx[1] + xx[2] * cos(TWOPI * ff[i-1] * xx3))); 
    }

if(iposit && (xx[1] <= 0 || xx[2] <= 0 || xx3 <= 0)) ww *= 100.; 

return(ww);
}
/**************************************************************
* Gradient of function to be minimized 
* E : value of the criterium in X
*            
* E = |Y(f_i)- Z(X, f_i)|^2
*
* with Z(f_i) = e + m cos(TWOPI f_i d)
* and xx[1] = e, xx[2] = m, xx[3] = d
*
**************************************************************/
static void jlp_cosine_grad(double *xx, double *dx)
{
double ww;
register int i;

/* JLP2003/I found it necessary to use and set this variable: */
if(nn1 == 3) xx3 = xx[3];

/* dZ/de = 1 */
  ww = 0.;
  for(i = 1; i <= nn; i++) {
    ww += ( yy[i-1] - (xx[1] + xx[2] * cos(TWOPI * ff[i-1] * xx3))); 
    }
  dx[1] = -2. * ww;

if(iposit && xx[1] <= 0) dx[1] = -100. * abs(dx[1]); 

/* dZ/dm = cos (TWOPI f_i d) */
  ww = 0.;
  for(i = 1; i <= nn; i++) {
    ww += ( yy[i-1] - (xx[1] + xx[2] * cos(TWOPI * ff[i-1] * xx3)))
          * cos(TWOPI * ff[i-1] * xx3); 
    }
  dx[2] = -2. * ww;

if(iposit && xx[2] <= 0) dx[2] = -100. * abs(dx[2]); 

/* dZ/dd = - m * (TWOPI * f_i) sin (TWOPI f_i d) */
  if(nn1 == 3) {
  ww = 0.;
  for(i = 1; i <= nn; i++) {
    ww += ( yy[i-1] - (xx[1] + xx[2] * cos(TWOPI * ff[i-1] * xx3)))
          * ff[i-1] * sin(TWOPI * ff[i-1] * xx3); 
    }
  dx[3] = +2. * TWOPI * xx[2] * ww;
  if(iposit && xx3 <= 0) dx[3] = -100. * abs(dx[3]); 
  }
}
/**************************************************************
* Check the validity of the gradient
* and compare f(x+dx)-f(x)/dx with grad_f(x)
*
* x1: work space of dimension nn
**************************************************************/
static int jlp_check_grad(double (*func)(double []),
                          void (*dfunc)(double[], double[]))
{
double x1[4], xx1[4], xx2[4];
register int i, j;
double eps=1.e-4, tolerance=1.e-4;
double f_xx1,f_x1,error;

/* Generate random vector (between 0 and 1) */
/*
for(i = 1; i <= nn1; i++) x1[i] = 0.;
*/
for(i = 1; i <= nn1; i++) x1[i] = (double)rand() / (double)RAND_MAX;

f_x1 = (*func)(x1);

/* Perturbation of all components: 
* hence loop on all components: */
for(i = 1; i <= nn1; i++)
   {
   for(j = 1; j <= nn1; j++) xx1[j] = x1[j];
/* Small variation of component #i: */
   xx1[i] = x1[i] + eps;
   f_xx1 = (*func)(xx1);
/* Gradient stored in xx2: */
   (*dfunc)(xx1,xx2);
   error = (f_xx1 - f_x1)/eps - xx2[i];
   error = error / (ABS(f_xx1 - f_x1)/eps + 1.e-12);
#ifdef DEBUG
   if(i < 10)
   printf(" f_x1=%e f_xx1=%e (f_xx1-f_x1)/eps = %e dx[%d] = %e error=%e\n",
             f_x1, f_xx1, (f_xx1 - f_x1)/eps, i, xx2[i], error);
#endif
    if(error > tolerance) {
      printf("AAAAAAAAAAAAA jlp_check_grad/Error! \n");
      printf("component #i=%d:  relative error =%.4e\n", i, error);
      }
   }

printf("jlp_check_grad/End: gradient has been checked.\n");
return(0);
}
