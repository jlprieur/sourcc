/***********************************************************
* Program to_polar 
* To compute polar coordinates from cartesian coordinates 
*
* Version 18/10/2021 
* JLP
***********************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define DEBUG

#define PI 3.14159265
#define DEGTORAD   (PI/180.00)

static int to_polar(double x1, double y1, double *angle1, double *modulus1);

int main(int argc, char* argv[])
{
double x0, y0, x10, y10, x20, y20, rho0, theta0;

printf("Program to convert from cartesian to polar\n");
printf(" JLP - Version 18/10/2021 \n");

if(argc < 2) {
 printf("to_polar/good syntax is:  to_polar x10,y10 x20,y20 \n");
 exit(-1);
 }
sscanf(argv[1], "%lf,%lf", &x10, &y10);
sscanf(argv[2], "%lf,%lf", &x20, &y20);
printf(" OK: x10=%f y10=%f \n", x10, y10);
printf(" OK: x20=%f y20=%f \n", x20, y20);

/* Conversion to polar coordinates: */
 x0 = x20 - x10;
 y0 = y20 - y10;
printf(" x0=%f y0=%f \n", x0, y0);
 to_polar(x0, y0, &theta0, &rho0);

 printf(" theta0 = %.1f (deg), rho0 = %.1f \n", theta0/DEGTORAD, rho0);
 
return(0);
} 
/************************************************** 
* Conversion to polar coordinates: 
****************************************************/
static int to_polar(double x1, double y1, double *angle1, double *modulus1)
{
 *modulus1 = x1 * x1 + y1 * y1;
 *modulus1 = sqrt((double)(*modulus1));
 if(*modulus1 != 0.)
   {
   *angle1 = acos((double)(x1 / (*modulus1)));
   if(y1 < 0) *angle1 = 2. * PI - *angle1;
   }
 else
   *angle1 = 0.;

return(0);
}
