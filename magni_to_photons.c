/*****************************************************************
* Program magni_to_photons
* To convert magnitude to flux in photons/second
*
* Formulae: magni = -2.5 log10 (e/e0)
*     or    e = e0 10**(magni/-2.5)
* Cf Lena p90, in V: e0=3.92e-8 W/m2/micrometer 
* (previous version: 30-03-1993)
*
* JLP
* Version 30-08-2017
*****************************************************************/
#include <stdio.h>
#include <stdlib.h>  // exit()
#include <string.h>
#include <math.h>

int to_photons(double mag, double *phot, double ee0, double lambda0);
int to_magni(double phot, double *mag, double ee0, double lambda0);

#define DEBUG

int main()
{
char buffer[81], code;
float work;
double magni, photons, magni0, dmag0;
int i;
int ioption, icode;

/* Nota: Indices of array e0: 0=U 1=B 2=V 3=R 4=I */
/* Cf Lena p90, in W/m2/micrometer */
double e0[] = {4.35e-8, 7.20e-8, 3.92e-8, 1.76e-8, 8.3e-8};
/* Lambda in nm: */
double lambda[] = {360., 440., 550., 700., 900.}; 
char band[] = { 'U', 'B', 'V', 'R', 'I'};

printf(" Program magni_to_photons\n\n");
printf(" Please note that only U,B,V,R,I are avaible\n");
printf(" Menu:\n");
printf("  1: Conversion from magnitude to flux in photons/sec \n");
printf("  2: Conversion of flux in photons/sec to magnitude \n");
printf(" Enter your choice: ");gets(buffer);
sscanf(buffer,"%d",&ioption);

switch (ioption)
{
/*****************************************************/
/* Option 1: Conversion from magnitude to photons/sec */
  case 1:
/* Warning: cannot read double precision magni... */
      printf(" \n Enter  band=magnitude  [e.g. B=12.5] : ");
      gets(buffer);
      sscanf(buffer,"%c=%f", &code, &work);
      magni = (double)work;
#ifdef DEBUG
      printf(" magni= %f\n",magni);
#endif
/* Check if code is correct: */
      i = 0; icode = -1;
      while(i < 5 && icode == -1)
        {if(code == band[i]) icode = i; else i++;} 
/* If not found, exit: */
      if(icode == -1)
         { printf(" Fatal error, wrong wavelength band\n"); exit(-1);}

#ifdef DEBUG
      printf(" code= %c icode=%d\n",code, icode);
#endif

/* Calling conversion routine: */
      to_photons(magni,&photons,e0[icode],lambda[icode]);
      printf(" Assuming central obscuration 0.8, optical efficiency 0.4\n");
      printf(" Flux in phot/sec (D=2m, 10nm bandwidth, Q.E=1.0): %g\n",
               photons); 
      printf(" Flux in phot/20msec (D=2m, 10nm bandwidth, Q.E.=0.05): %g\n",
               photons/1000.); 
      printf(" Flux in phot/10msec (D=3.6m, 10nm bandwidth, Q.E.=0.05): %g\n",
               photons/617.); 
      break;
/*****************************************************/
/* Option 2: Conversion from photons/sec to magnitude */
  case 2:
/* Warning: cannot read double precision magni... */
      printf(" \n Enter  band=flux  [e.g. V=1230.5] : ");gets(buffer);
      sscanf(buffer,"%c=%f",&code,&work);
      photons = (double)work;
#ifdef DEBUG
      printf(" photons = %f\n",photons);
#endif
/* Check if code is correct: */
      i = 0; icode = -1;
      while(i < 5 && icode == -1)
        {if(code == band[i]) icode = i; else i++;} 
/* If not found, exit: */
      if(icode == -1)
         { printf(" Fatal error, wrong wavelength band\n"); exit(-1);}

#ifdef DEBUG
      printf(" code= %c icode=%d\n",code, icode);
#endif

/* Calling conversion routine: */
      to_magni(photons,&magni,e0[icode],lambda[icode]);
magni0 = magni;
      printf(" Assuming D=2m, central obscuration 0.8, optical efficiency 0.4\n");
// 
      printf(" If initial flux was in phot/sec (D=2m, 10nm bandwidth, Q.E.=1.0): %c=%4.2f\n",
               code, magni0); 
//
dmag0 = - 2.5 * log10(0.05 * 0.020);
magni0 = magni - dmag0;
      printf(" If initial flux was in phot/20msec (D=2m, 10nm bandwidth, Q.E.=0.05, i.e. dmag=%.2f): %c=%4.2f\n",
               dmag0, code,magni-7.5); 
dmag0 = - 2.5 * log10(0.05 * 0.010) - 5. * log10(3.6/2);
magni0 = magni - dmag0;
      printf(" If initial flux was in phot/10msec (D=3.6m, 10nm bandwidth, Q.E.=0.05, i.e., dmag=%.2f): %c=%4.2f\n",
               dmag0, code,magni-6.98); 
      break;
/*****************************************************/
  default:
      printf(" Sorry option not yet available\n");
      break;
}

return(0);
}

/********************************************************
* Routine to convert magnitude to flux in photons/sec
* Formula:
*        e = e0 10**(magni/-2.5)
* Cf Lena p90, in V: e0=3.92e-8 W/m2/micrometer 
********************************************************/
int to_photons(double mag, double *phot, double ee0, double lambda0)
{
double energy_phot;

*phot = ee0 * pow(10.0,(double)(mag/-2.5));

/* Conversion to  from W/m2 to W/2-meter telescope:
      *3.14*0.8*0.4 m2 if 2-meter telescope
        with central obscuration of 0.8, and optical efficiency of 0.4
*/
*phot = *phot * 3.14 * 0.8 *0.4;

/* Conversion from W/micron to W/10 nm:  *0.01 if 10-nm bandwidth filter */
*phot = *phot * 0.01;

/* Conversion to photons: / h*nu  if lambda0 is the mean wavelength
*  E = h*nu = h*c /lambda
*  h=6.63e-34
*  energy_phot = 6.63e-34 * 3.e8 / (lambda0 * 1e-9);
*/
energy_phot = 6.63e-17 * 3.0 / lambda0;
#ifdef DEBUG
printf(" lambda0=%g nm, energy_phot = %g J\n",lambda0,energy_phot);
#endif

*phot = *phot / energy_phot;

return(0);
}

/********************************************************
* Routine to convert flux in photons/sec to magnitude
* Formula: 
*      magni = -2.5 log10 (e/e0)
* Cf Lena p90, in V: e0=3.92e-8 W/m2/micrometer 
********************************************************/
int to_magni(double phot, double *mag, double ee0, double lambda0)
{
double energy_phot;

/* Conversion to flux in Joules: * h*nu  if lambda0 is the mean wavelength
*  E = h*nu = h*c /lambda
*  h=6.63e-34
*  energy_phot = 6.63e-34 * 3.e8 / (lambda0 * 1e-9);
*/
energy_phot = 6.63e-17 * 3.0 / lambda0;
#ifdef DEBUG
printf(" lambda0=%g nm, energy_phot = %g J\n",lambda0,energy_phot);
#endif

phot = phot * energy_phot;

/* Conversion to  from W/2-meter telescope to W/m2:
      /(3.14*0.8*0.4) m2 if 2-meter telescope
        with central obscuration of 0.8, and optical efficiency of 0.4
*/
phot = phot / (3.14 * 0.8 *0.4);

/* Conversion from W/10nm to W/micron:  /0.01 if 10-nm bandwidth filter */
phot = phot / 0.01;

/* Conversion to magnitude: */
*mag = -2.5 * log10((double)(phot / ee0));

return(0);
}
