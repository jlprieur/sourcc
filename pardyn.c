/**************************************************************************
* To compute dynamical parallax
*
* JLP
* Version 31/07/2007
**************************************************************************/
#include <stdio.h>
#include <math.h>

int main()
{
int ioption;
double k, kk, M0, a, b, c, d;
double log10_p, alpha, aa, pp, q, mv1, Cv1, Dmag;

printf("Option: 0=Manual 1=Baize-Romani 2=classV_JLP 3=classIV_JLP: \n");
scanf("%d", &ioption);
switch (ioption) {
/* Baize-Romani: (Couteau p 163) */
 case 1:
  k = 0.1117;
  M0 = 4.77; 
  break;
/* JLP Fit to class V: */
 case 2:
  k = 0.1045;
  M0 = 4.60; 
  break;
/* JLP Fit to class IV: */
 case 3:
  k = 0.1157;
  M0 = 3.75; 
  break;
 case 0:
 default:
  printf("Enter coefficients k and M0: \n");
  scanf("%lf,%lf", &k, &M0);
  break;
 }

printf("OK: k=%.4f M0=%.2f\n", k, M0);

kk= 3 - 5 * k;
a = - k * (M0 - 5) /kk;
b = 3 / kk;
c = -1 / kk;
d = k / kk;
printf("a, b, c, d = %.3f %.3f %.3f %.3f\n", a, b, c ,d);

printf("Enter semi-major axis a (in arcsec), period (in years),");
printf(" mv1, Cv, and Delta_mV:\n");
scanf("%lf,%lf,%lf,%lf,%lf", &aa, &pp, &mv1, &Cv1, &Dmag);
printf("OK: aa, pp= %.3f %.3f\n", aa, pp);
printf("OK: mv1, Cv, and Delta_mV= %.3f %.3f %.3f\n", mv1, Cv1, Dmag);
alpha = aa / pow(pp, 0.6667);
q = pow(10.0,  (- k * Dmag));
log10_p = a + b * log10(alpha) + c * log10(1.0 + q) + d * (mv1 + Cv1);

printf("Dynamical parallax=%.6f mas  (q=%.2f)\n", 
        pow(10.0, log10_p)*1000., q);

return(0);
}
