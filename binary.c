/***********************************************************
* binary.c 
* To compute the ephemerids of a double star
* Calls routines from "cel_meca.c" 
*
* References: Danjon, Astronomie Generale
*             Ephemerides du Bureau des longitudes
*             Couteau, Observation des etoiles doubles
* JLP
* Version 02-09-93
*
************************************************************/
#define DEBUG
#define MAXELEMTS 10 

#include <stdio.h>
#include <math.h>
#include <string.h>
#include <fcntl.h>
#include <jlp_ftoc.h>
#include <cel_meca1.h>

#ifndef PI
#define PI 3.14159265
#endif

/* Prototypes of functions included here: */
int binary_coord(double date_obs, double incl, double semajaxis, double eccent,
                 double big_omega, double omega, double T_0, double period,
                 double mean_motion, double *position_angle, double *separation);

int main(int argc, char *argv[])
{
double orb[MAXELEMTS][8]; 
double aa, time, date_obs, period;
double incl, semajaxis, eccent, big_omega, omega, T_0, mean_motion;
double position_angle, separation;
int numb[MAXELEMTS], istart, iend, status, n_orbits;
register int i;
char buffer[81], filename[61], name[MAXELEMTS][20];
FILE *fp, *fp1;

printf(" Program binary to compute the ephemerids of double stars\n");
printf(" JLP Version 02-09-93 \n");
printf(" Should read \"eph_binary.dat1\" \n");

printf(" Enter date of observation: (Ex: 1983.432)\n");
gets(buffer); sscanf(buffer,"%lf",&aa);

time = 0.0;
/*
printf(" Enter time (decimal) of observation: \n");
gets(buffer); sscanf(buffer,"%lf",&time);
*/
strcpy(filename,"binary.dat");
if ((fp1 = fopen(filename,"w")) == NULL)
  {
  printf("binary/Fatal error, cannot open output list file: %s \n",filename);
  exit(-1);
  }
 
/* This file eph_binary.dat has been built 
* from ... for orbit parameters
*/
strcpy(filename,"eph_binary.dat1");
if ((fp = fopen(filename,"r")) != NULL)
  {
   for(i = 0; i < MAXELEMTS; i++)
      {
       if((status = fscanf(fp,"%d %lf %lf %lf %lf %lf %lf %lf %lf %s\n",
           &numb[i],&orb[i][0],&orb[i][1],&orb[i][2],&orb[i][3],
           &orb[i][4],&orb[i][5],&orb[i][6],&orb[i][7],name[i]))
          == EOF) break;
#ifdef DEBUG
       printf("%d %lf %lf %lf %lf %lf %lf %lf %lf %s \n",numb[i],
            orb[i][0],orb[i][1],orb[i][2],orb[i][3],
            orb[i][4],orb[i][5],orb[i][6],orb[i][7],name[i]);
#endif
      }
   n_orbits = i;
   printf(" %d orbit parameters entered\n",n_orbits);
   fclose(fp);
  printf(" Enter binary list number: (0 if all the list)\n");
  scanf("%d",&istart); 
   if(istart)
   {
   istart--;
   iend = istart + 1;
   }
   else 
   {
   istart = 0; iend = n_orbits;
   }
  }
else
  {
  printf(" Enter the 7 parameters defining the orbit: \n");
  printf(" T_0 (yr), omega (deg), big_omega (deg), incl (deg)\n");
  printf(" eccent, semi-major axis (arcsec), Period (yr)");
  gets(buffer); 
  i = 0;
  sscanf(buffer,"%lf,%lf,%lf,%lf,%lf,%lf,%lf",
           &orb[i][0],&orb[i][1],&orb[i][2],&orb[i][3],
           &orb[i][4],&orb[i][5],&orb[i][6]);
/* mean motion (deg/yr) is now computed directly from the period value: */
  orb[i][7] = 360. / orb[i][6];
  strcpy(name[i],"to determine object"); numb[i] = 0;
  istart = 0; iend = 1;
  }

/************************************************************************/

  printf(" Date: %.3f at %.4fH \n",aa,time);
  fprintf(fp1," Date: %.3f at %.4fH \n",aa,time);
  fprintf(fp1,"1=Number 2=Name 3=Position angle (deg) 4=Separation (arcsec)\n");
/* Compute julian day "date_obs" */
  julian(aa,0,0,0.,&date_obs);

/************************************************************************/
/* Main loop: */
for(i = istart; i < iend; i++)
  {
   printf("\n Now computing %d %s coordinates\n",numb[i],name[i]);
#ifdef DEBUG
  printf("OK: numb, T_0 (yr), omega (deg), big_omega (deg), incl (deg), eccent, semi-major axis (arcsec)\n");
  printf("%d %lf %lf %lf %lf %lf %lf %lf %lf %s \n",numb[i],
            orb[i][0],orb[i][1],orb[i][2],orb[i][3],
            orb[i][4],orb[i][5],orb[i][6],orb[i][7],name[i]);
#endif

/* Compute julian day "T_0" */
  T_0 = orb[i][0];
  julian(T_0,0,0,0.,&T_0);
#ifdef DEBUG
  printf(" Corresponding Julian day to periastron passage: %f \n",T_0);
#endif
/* Conversion to radians: */
  omega = orb[i][1] * PI / 180.;
  big_omega = orb[i][2] * PI / 180.;
  incl = orb[i][3] * PI / 180.;
  eccent = orb[i][4];
/* Conversion from arcseconds to radians: */
  semajaxis = orb[i][5] * (PI / 180.) / 3600.;
/* Period: conversion to days: */
  period = orb[i][6] * 365.25;
/* Mean motion: conversion to radians per day: */
  mean_motion = orb[i][7] * (PI / 180.) / 365.25; 

  binary_coord(date_obs,incl,semajaxis,eccent,big_omega,omega,T_0,
            period,mean_motion,&position_angle,&separation);

  printf(" Position angle=%.3f (deg)   Angular separation = %.3f (arcsec) \n",
           position_angle*(180./PI), separation * 3600. *(180./PI)); 
   fprintf(fp1,"\n %3d %12s",numb[i],name[i]);
   fprintf(fp1," %5.2f %5.2f",
           position_angle*(180./PI), separation * 3600. *(180./PI)); 

  }                                  /* End of main loop */

/* End of program */
 fclose(fp1);
return(0);
}
/***********************************************************
*
* incl: inclination of the orbit relative to ecliptic
*        (in [0,pi/2] if prograde orbit, in [pi/2,pi] if retrograde orbit).
* semajaxis: semi-major axis a (in Astronomical Units)
* eccent: eccentricity e 
* big_omega: angle between Gamma (Vernal equinox) and the ascending node N.
* omega: angle between the ascending node N and the perihelion.
* v: true anomaly, i.e. angle perihelion-sun-planet
* u: eccentric anomaly (affinity of sqrt(1 - e**2) relative to v)
* M: mean anomaly (if starting from periastron, same angular velocity)
* param: parameter p of the orbit p = a ( 1 - e**2)
* T_0: instant of periastron passage.
* period: period in days 
* mean_motion: in radians/day 
***********************************************************/
int binary_coord(double date_obs, double incl, double semajaxis, double eccent,
                 double big_omega, double omega, double T_0, double period,
                 double mean_motion, double *position_angle, double *separation)
{
double true_separation;
double mean_ano, eccent_ano, true_ano;
double w1, w2, beta;


/* Computing mean anomaly: */
w1 = date_obs - T_0;
w1 = w1 - period * (double)((int)(w1/period)); if(w1 < 0) w1 += period;
printf(" w1 < period? %f < %f \n",w1,period);
mean_ano = mean_motion * w1;

/* Now eccentric anomaly: */
from_mean_to_eccent_anomaly(mean_ano,&eccent_ano,eccent);

#ifdef DEBUG
printf(" binary_coord/Mean anomaly = %f (deg), eccent anomaly u = %f (deg)\n",
         mean_ano * (180./PI),eccent_ano * (180./PI));
#endif

/* True anomaly, cf tan(v/2) = tan (u/2) * sqrt ( (1+e) / (1-e) ) */
w1 = tan (eccent_ano / 2.) * sqrt( ( 1. + eccent) / (1. - eccent) );
w1 = atan(w1);
/* Uncertainty w1 or w1+PI (but not important since multiplied by two,
   true_ano or true_ano+ 2*PI) */ 
true_ano = 2. * w1;
if(true_ano < 0) true_ano += 2. * PI; 

#ifdef DEBUG
printf(" binary_coord/True anomaly v = %f (deg)\n",true_ano * (180./PI));
#endif

/* Computing true separation: 
* r = p / (1 + e cos v) = a ( 1 - e cos u)
*/
true_separation = semajaxis * ( 1. - eccent * cos (mean_ano)); 
#ifdef DEBUG
printf(" binary_coord/true separation (before projection) = %f (arcsec)\n",
         true_separation * 3600. * (180./PI));
#endif

/******************************************************************/
/* Now computing projected coordinates: 
* longitude lambda and latitude beta */

/******************************************************************/
/* beta first,
* Solving the triangle i, v + omega, w1, beta
* Note that w1=(lambda-big_omega) 
* cos (beta) cos(w1) = cos (v + omega) 
* cos (beta) sin(w1) = cos(i) sin (v + omega) 
* sin(beta) = sin(i) sin( v + omega)
*/

w1 = sin(incl) * sin (true_ano + omega);
beta = asin(w1); 
/* Since we a priori do not know what asin does, we do this: */
if(cos(beta) < 0) beta = PI - beta; 

/******************************************************************/
/* Then lambda, by solving the same triangle:
* cos (beta) cos(w1) = cos (v + omega) 
* cos (beta) sin(w1) = cos(i) sin (v + omega) 
*/
w1 = cos(true_ano + omega) / cos(beta); 
w1 = acos(w1); 

/* Test w1: w1 or -w1? 
* Check with (in the same triangle, too)
* sin(w1) = cos(i) sin(v+omega) /cos(beta)
*/
w2 = cos(incl) * sin(true_ano + omega) / cos(beta); 
if(w2 * sin(w1) < 0) w1 = -w1; 
#ifdef DEBUG
printf(" binary_coord/ position angle - big omega = %f (deg)\n",
         w1 * (180./PI));
#endif

/* Position angle (longitude) now: */ 
 *position_angle = w1 + big_omega;
if(*position_angle < 0) *position_angle += 2.*PI;

/* Now projected distance: */
*separation = true_separation * cos(beta);
if(*separation < 0) *separation *= -1.;

#ifdef DEBUG
 printf(" binary_coord/Projected coordinates: position angle=%f (deg) (beta=%f (deg) orthogonal coord)\n",
         *position_angle * (180. / PI),beta * (180./PI));
#endif

return(0);
}
/***************************************************************/
