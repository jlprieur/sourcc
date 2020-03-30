/*****************************************************************
* Program pisco_photons1
* To convert magnitude to flux in photons/second
*
* Formulae: magni = -2.5 log10 (e/e0)
*     or    e = e0 10**(magni/-2.5)
*
* JLP
* Version 30-08-2017
*****************************************************************/
#include <stdio.h>
#include <stdlib.h>  // exit()
#include <string.h>
#include <math.h>

#ifndef PI
#define PI 3.14159
#endif

int magni_to_photons(double mag, double *phot, double ee0, double lambda_eff0);
int photons_to_magni(double phot, double *mag, double ee0, double lambda_eff0);
int get_Bessel_parameters(char filter_code, double *lambda_eff0, double *ee0);

#define DEBUG

// Diameter of the telescope in meters
#define TDIAMETER 1.0

int main()
{
char buffer[81], filter_code;
double magni, photons, magni0, dmag0;
int status, ioption; 
double lambda_eff0, ee0;


printf(" Program pisco_photons1\n");
printf(" Bessel broad band filters: U, B, V, R, I, J, H, K, L \n\n");
printf(" Menu:\n");
printf("  1: Conversion from magnitude to flux in photons/sec \n");
printf("  2: Conversion of flux in photons/sec to magnitude \n");
printf(" Enter your choice: ");gets(buffer);
sscanf(buffer,"%d",&ioption);

if(ioption == 1) {
   printf(" \n Enter  Bessel_band=magnitude  [e.g. B=12.5] : ");
   gets(buffer);
   sscanf(buffer,"%c=%lf", &filter_code, &magni);
  } else if (ioption == 2) {
   printf(" \n Enter  Bessel_band=flux  [e.g. V=1230.5] : ");
   gets(buffer);
   sscanf(buffer,"%c=%lf", &filter_code, &photons);
  } else {
   printf("Error; invalid option! \n");
  }

// Get lambda_eff and e0:
 status = get_Bessel_parameters(filter_code, &lambda_eff0, &ee0);
 if(status != 0) {
  fprintf(stderr, "Error in get_Bessel_parameters: status = %d\n", status);
  return(-1);
  }

switch (ioption)
{
/*****************************************************/
/* Option 1: Conversion from magnitude to photons/sec */
  case 1:

/* Calling conversion routine: */
      magni_to_photons(magni, &photons, ee0, lambda_eff0);
      printf(" Assuming central obscuration 0.8, optical efficiency 0.4\n");
      printf(" Flux in phot/sec (D=1m, 10nm bandwidth, Q.E=1.0): %g\n",
               photons); 
      printf(" Flux in phot/20msec (D=1m, 10nm bandwidth, Q.E.=0.5): %g\n",
               photons/100.); 
      break;
/*****************************************************/
/* Option 2: Conversion from photons/sec to magnitude */
  case 2:

/* Calling conversion routine: */
      photons_to_magni(photons, &magni, ee0, lambda_eff0);
magni0 = magni;
      printf(" Assuming D=2m, central obscuration 0.8, optical efficiency 0.4\n");
// 
      printf(" If initial flux was in phot/sec (D=1m, 10nm bandwidth, Q.E.=1.0): %c=%4.2f\n",
               filter_code, magni0); 
//
dmag0 = - 2.5 * log10(0.5 * 0.020);
magni0 = magni - dmag0;
      printf(" If initial flux was in phot/20msec (D=1m, 10nm bandwidth, Q.E.=0.5, i.e. dmag=%.2f): %c=%4.2f\n",
               dmag0, filter_code,magni-7.5); 
      break;
}

return(0);
}
/****************************************************************************
* Get lambda_eff and e0:
*
****************************************************************************/
int get_Bessel_parameters(char filter_code, double *lambda_eff0, double *ee0)
{
int i, ifilter_code;
double work, N_lambda[9], h_nu_photon;
/* Nota: Indices of array e0: 0=U 1=B 2=V 3=R 4=I */
/* Cf Lena p90, in W/m2/micrometer */
double e0[] = {4.35e-8, 7.20e-8, 3.92e-8, 1.76e-8, 0.83e-8};
// Effective lambda_eff in nm: (from Bessel et al., 1998)
double Bessel_lambda_eff[] = 
 { 366., 438., 545., 641., 798., 1220., 1630., 2190., 3450.}; 
char Bessel_band[] = { 'U', 'B', 'V', 'R', 'I', 'J', 'H', 'K', 'L'};
// F_nu in 10^{-20} ergs/cm2/Hz: (from Bessel et al., 1998)
double F_nu[] = 
 { 1.790, 4.063, 3.636, 3.064, 2.416, 1.589, 1.021, 0.640, 0.285}; 
// F_lambda in 10^{-11} ergs/cm2/AA: (from Bessel et al., 1998)
double F_lambda[] = 
 { 417.5, 632.0, 363.1, 217.7, 112.6, 31.47, 11.38, 3.961, 0.708}; 

#ifdef DEBUG
// Display F_nu, F_lambda and N_lambda
 for(i = 0; i < 9; i++) {
// Conversion from ergs/s/cm2 to W/m2: (1 erg = 10^{-7} Joule)
   work = 1.E-20 * F_nu[i] * 1.E-7 / 1.E-4;
   printf("%c F_nu = %.3e (ergs/s/cm2/Hz) ou %.2e (W/m2/Hz)\n",
          Bessel_band[i], 1.E-20 * F_nu[i], work); 
// Conversion from ergs/s/cm2/AA to W/m2/nm: (1 erg = 10^{-7} Joule)
   work = 1.E-11 * F_lambda[i] * 1.E-7 / 1.E-4 / 0.1;
   printf("%c F_lambda = %.3e (ergs/s/cm2/A) ou %.2e (W/m2/nm)\n",
          Bessel_band[i], 1.E-11 * F_lambda[i], work); 
/* Conversion to photons (lambda_eff0 in nm): 
*  E = h*nu = h*c /lambda_eff0
*  h=6.626E-34 J.s
*  c=2.998E+8 m/s
*  energy_phot = 6.63e-34 * 3.e8 / (lambda_eff0 * 1e-9);
*/
//   h_nu_photon = 6.626E-34 * 2.998E+8 / (Bessel_lambda_eff[i] * 1.E-9); 
   h_nu_photon = 6.626E-17 * 2.99 / Bessel_lambda_eff[i]; 
// Conversion from ergs/s/cm2/AA to W/m2/nm: (1 erg = 10^{-7} Joule)
   work = 1.E-11 * F_lambda[i] * 1.E-7 / 1.E-4 / 0.1;
   N_lambda[i] = work / h_nu_photon;
   printf("%c N_lambda = %.3e (photons/s/m2/nm) ou %.3e (photon/s/cm2/AA)\n",
          Bessel_band[i], N_lambda[i], N_lambda[i] * 1.E-4 / 10.); 
if(i <5) printf("e0[%d]=%.3e W/m2/micrometer\n", i, e0[i]);
   }
#endif

/* Check if filter_code is correct: */
  i = 0; ifilter_code = -1;
  while(i < 8 && ifilter_code == -1)
    {if(filter_code == Bessel_band[i]) ifilter_code = i; else i++;} 

#ifdef DEBUG
      printf(" ifilter_code= %d\n", ifilter_code);
#endif

/* If not found, exit: */
   if(ifilter_code == -1)
       { printf(" Fatal error, wrong wavelength Bessel_band\n"); return(-1);}

 *lambda_eff0 =  Bessel_lambda_eff[ifilter_code];
/* old version
 *ee0 = e0[ifilter_code];
*/
// New version:
// Conversion from ergs/s/cm2/AA to W/m2/nm: (1 erg = 10^{-7} Joule)
 work = 1.E-11 * F_lambda[ifilter_code] * 1.E-7 / 1.E-4 / 0.1;
// Conversion to W/m2/micrometer
 *ee0= work * 1.E+3;

return(0);
}
/****************************************************************************
* Routine to convert magnitude to flux in photons/sec
* Formula:
*        e = e0 10**(magni/-2.5)
*        magni = 2.5 log10(e/e0) 
*                       (i.e., e0 is the flux in W/m2/micron for magni=0)
* Cf Lena p90, in V: e0=3.92e-8 W/m2/micrometer 
****************************************************************************/
int magni_to_photons(double mag, double *phot, double ee0, double lambda_eff0)
{
double energy_phot;

*phot = ee0 * pow(10.0,(double)(mag/-2.5));

/* Conversion to  from W/m2 to W/2-meter telescope:
      *3.14*0.8*0.4 m2 if 2-meter telescope
        with central obscuration of 0.8, and optical efficiency of 0.4
*/
*phot = *phot * PI * (TDIAMETER * TDIAMETER / 4.) * 0.8 *0.4;

/* Conversion from W/micron to W/10 nm:  *0.01 if 10-nm bandwidth filter */
*phot = *phot * 0.01;

/* Conversion to photons: / h*nu  if lambda_eff0 is the mean wavelength
*  E = h*nu = h*c /lambda_eff0
*  h=6.63e-34
*  energy_phot = 6.63e-34 * 3.e8 / (lambda_eff0 * 1e-9);
*/
energy_phot = 6.63e-17 * 3.0 / lambda_eff0;
#ifdef DEBUG
printf(" lambda_eff0=%g nm, energy_phot = %g J\n",lambda_eff0,energy_phot);
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
int photons_to_magni(double phot, double *mag, double ee0, double lambda_eff0)
{
double energy_phot;

/* Conversion to flux in Joules: * h*nu  if lambda_eff0 is the mean wavelength
*  E = h*nu = h*c / lambda_eff0
*  h=6.63e-34
*  energy_phot = 6.63e-34 * 3.e8 / (lambda_eff0 * 1e-9);
*/
energy_phot = 6.63e-17 * 3.0 / lambda_eff0;
#ifdef DEBUG
printf(" lambda_eff0=%g nm, energy_phot = %g J\n",lambda_eff0,energy_phot);
#endif

phot = phot * energy_phot;

/* Conversion to  from W/2-meter telescope to W/m2:
      /(3.14*0.8*0.4) m2 if 2-meter telescope
        with central obscuration of 0.8, and optical efficiency of 0.4
*/
phot = phot / (PI *  (TDIAMETER * TDIAMETER / 4.) * 0.8 *0.4);

/* Conversion from W/10nm to W/micron:  /0.01 if 10-nm bandwidth filter */
phot = phot / 0.01;

/* Conversion to magnitude: */
*mag = -2.5 * log10((double)(phot / ee0));

return(0);
}
