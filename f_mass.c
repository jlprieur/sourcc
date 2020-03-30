/**********************************************************************
* Program to plot the mass function (for spectral binaries)
*
**********************************************************************/
#include <stdio.h>
#include <string.h>
#include <math.h>
int main(int argc, char *argv[])
{
double M_1, sine_incl, mu, f_m;
FILE *fp;
char outname[60];
register int i;

strcpy(outname,"f_mass.dat");

if((fp = fopen(outname,"w")) == NULL) 
  {
  fprintf(stderr,"Fatal error opening output file %s\n", outname);
  return(-1);
  }
fprintf(fp,"mu, f_m(mu) with M_1=1 sine_incl=1 f_m \n");  
M_1 = 1.;
sine_incl = 1.;
for(i = 0; i < 100; i++) {
  mu = 0.1 * (double)i;
  f_m = M_1 * sine_incl * mu * mu * mu / ((1 + mu) * (1 + mu));
  fprintf(fp,"%f %f \n", (float)mu, (float)f_m);  
  }
fclose(fp);
return(0);
}
