/**********************************************************************
* reseau
* Pour calculer le figure de diffraction d'un réseau à transmission
*
* a: largeur de la fente
* b: distance entre les fentes
* N: nombre de fentes
*
*********************************************************************/
#include <stdio.h>
#include <math.h>
#define PI (2. * asin(1.))
#define SQUARE(a) ((a)*(a))
#define ABS(a) (((a) > 0) ? (a) : (-(a)))

#define NPTS 2048

static compute_psi(double *xx, double *yy1, double *yy2, double aa, double bb, 
                   double NN, int npts);
static output_psi(char *filename, char *comments, double *xx, double *yy1, 
                  double *yy2,  int npts);
int main(int argc, char *argv[])
{
double xx[NPTS], yy1[NPTS], yy2[NPTS];
double aa, bb, NN;
int npts = NPTS;
char filename[80], comments[80];

printf("OK: argc=%d argv[0] = %s\n", argc, argv[0]);
if(argc != 4) {
  printf("La syntaxe correcte est: reseau a b N \n");
  printf("avec: a=largeur des fentes, b=distance entre les fentes, N=nombre de fentes \n");
  return(-1);
}
sscanf(argv[1], "%lf", &aa);
sscanf(argv[2], "%lf", &bb);
sscanf(argv[3], "%lf", &NN);

compute_psi(xx, yy1, yy2, aa, bb, NN, npts);

strcpy(filename,"reseau.dat");
sprintf(comments,"# aa=%.3f bb=%.3f NN=%d npts=%d\n", aa , bb, (int)NN, npts);

output_psi(filename, comments, xx, yy1, yy2, npts);

return(0);
}
/***********************************************************************
* Compute psi, the irradiance of the diffraction pattern (xx, yy1)
*             and its sinc enveloppe (xx, yy2)
*
***********************************************************************/
static compute_psi(double *xx, double *yy1, double *yy2, double aa, double bb, 
                   double NN, int npts)
{
double lambda, dd, xp, xp_aa, sin1;
register int i;

dd = 1.;
lambda = 1.;
/* Pour avoir un joli graphe, il faut que xp * aa varie
*  entre -2pi et + 2pi
*  donc que xx varie entre -2/aa et +2/aa
*/

printf(" PI=%f\n", PI);

for(i = 0; i < npts; i++) {
  xx[i] = (-3. + 6. * (double)i/(double)(npts)) / aa;
  xp = PI * xx[i] / lambda * dd;
  xp_aa = xp * aa;
  if(ABS(xp_aa) < 1.e-12) xp_aa = 1.e-12;
  sin1 = sin(xp * bb);
/*
  if(ABS(sin1) < 1.e-12) sin1 = 1.e-12;
*/
/* sinc:
*/
  yy2[i] = SQUARE(sin(xp_aa) / xp_aa); 
  yy1[i] = SQUARE(sin(xp_aa) / xp_aa) 
          * SQUARE(sin(NN * xp * bb) / (NN * sin(xp * bb)));
/* Debug:
  yy1[i] = SQUARE(sin(NN * xp * bb) / (NN * sin1));
  yy1[i] = SQUARE(sin(xp_aa) / xp_aa) 
          * SQUARE(sin(NN * xp * bb) / (NN * sin1));
*/
  }

return(0);
}
/***********************************************************************
* Output psi, the irradiance of the diffraction pattern (xx, yy1)
*             and its sinc enveloppe (xx, yy2)
*
*
***********************************************************************/
static output_psi(char *filename, char *comments, double *xx, double *yy1, 
                  double *yy2,  int npts)
{
FILE *fp;
register int i;

/* Opening output file: */
if((fp = fopen(filename,"w")) == NULL) {
  printf("output_psi/Fatal error opening >%s< \n", filename);
  return(-1);
  }
fprintf(fp,"%s \n",comments);

for(i = 0; i < npts; i++) {
   fprintf(fp,"%f %f %f\n",xx[i], yy1[i], yy2[i]);
   }

/* Closing output file: */
fclose(fp);
return(0);
}
