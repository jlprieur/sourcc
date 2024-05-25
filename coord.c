/***********************************************************
* coord.c
* To compute the coordinates of asteroids.
* Read orbit parameters in file "eph_asteroids.dat1"
* -- aster. number, 
* -- tau_0 (Julian day), 
* -- omega (deg) (argument du perihelie), 
* -- big_omega (deg) (longitude du noeud ascendant), 
* -- inclination (deg) (i in Rocher's program), 
* -- eccent (e in Rocher's program), 
* -- semi-major axis (UA) (not there! in Rocher's program), 
* -- H=absol. magnitude, -- G=slope param.  (H and G for magnitude computation),
* -- diameter (km) (not there! in Rocher's program), 
* -- name \n");
*
* References: Danjon, Astronomie Generale
*             Ephemerides du Bureau des longitudes
* JLP
* Version 30-08-95
************************************************************/

#define DEBUG

#define MAXELEMTS 400 

#include <stdio.h>
#include <math.h>
#include <string.h>
#include <fcntl.h>
#include <jlp_ftoc.h>

/* Kepler 3rd law:  n**2 a**3 = k (M_sol + m) 
* if we take the following units: 
*  n = 2 pi / T    (T in days) 
*  a in Astronomical Units
*  M_sol + m = M_sol = 1
* 
* We have then:
*/
#define SQRT_K 0.017202098950000
#define PI 3.14159265
/* Angle between equator and ecliptic : 23 degrees 27'
* (in radians here): (should be corrected later, because not precise ...)*/
#define EPSILON 0.4092797 

main(argc,argv)
int argc;
char *argv[];
{
double orb[MAXELEMTS][9]; 
double aa, time, date_obs, lambda, beta, alpha, delta;
double incl, semajaxis, eccent, big_omega, omega, tau_0;
double lambda_sun, beta_sun, planet_helio_dist, planet_to_earth_dist;
double h_1, g_1, magni, phase_angle, true_diameter, app_diameter;
int idd, mm, inumb, numb[MAXELEMTS], istart, iend, status, n_orbits;
int i;
char buffer[81], filename[61], name[MAXELEMTS][20];
FILE *fp, *fp1;

printf(" Program coord to compute the ephemerids of planets\n");
printf(" JLP Version 30-08-93 \n");
printf(" Should read \"eph_asteroids.dat1\" \n");

printf(" Enter day, month, year of observation: (Ex: 21,04,1993)\n");
gets(buffer); sscanf(buffer,"%d,%d,%lf",&idd,&mm,&aa);

time = 0.0;
/*
printf(" Enter time (decimal) of observation: \n");
gets(buffer); sscanf(buffer,"%lf",&time);
*/
strcpy(filename,"coord.dat");
if ((fp1 = fopen(filename,"w")) == NULL)
  {
  printf("coord/Fatal error, cannot open output list file \n");
  exit(-1);
  }
 
/* This file eph_asteroids.dat has been built 
* from Annuaire du Bureau des Longitudes 1993 for orbit parameters
* and Asteroids II p1093 for diameters (IRAS data) 
*/
strcpy(filename,"eph_asteroids.dat1");
if ((fp = fopen(filename,"r")) != NULL)
  {
   for(i = 0; i < MAXELEMTS; i++)
      {
       if((status = fscanf(fp,"%d %lf %lf %lf %lf %lf %lf %lf %lf %lf %s\n",
           &numb[i],&orb[i][0],&orb[i][1],&orb[i][2],&orb[i][3],
           &orb[i][4],&orb[i][5],&orb[i][6],&orb[i][7],&orb[i][8],name[i]))
          == EOF) break;
/* JLP96: the last two asteroids were missing, so I remove:
       if(feof(fp)) break;
*/
#ifdef DEBUG
       printf("%d %lf %lf %lf %lf %lf %lf %lf %lf %f %s \n",numb[i],
            orb[i][0],orb[i][1],orb[i][2],orb[i][3],
            orb[i][4],orb[i][5],orb[i][6],orb[i][7],orb[i][8],name[i]);
#endif
      }
/* JLP96: the last two asteroids were missing, so I remove:
   n_orbits = i - 1;
*/
   n_orbits = i;
   printf(" %d orbit parameters entered\n",n_orbits);
   fclose(fp);
  printf(" Enter asteroid number: (0 if all the list)\n");
  scanf("%d",&inumb); 
   if(inumb)
   {
   for(i = 0; i < n_orbits; i++)
      {
      if(inumb == numb[i]) break;
      }
   if(i == n_orbits)
      {
      printf(" Fatal error: asteroid #%d is not in the list\n",inumb);
      exit(-1);
      }
   istart = i;
   iend = istart + 1;
   }
   else 
   {
   istart = 0; iend = n_orbits;
   }
  }
else
  {
  printf(" Enter the 9 parameters defining the orbit: \n");
  printf(" tau_0 (Julian day), omega (deg), big_omega (deg), incl (deg)\n");
  printf(" eccent, semi-major axis (UA), H=absol. magnitude, G=slope param.");
  printf(" (H and G for magnitude computation)\n");
  printf(" and diameter (km)\n");
  gets(buffer); 
  i = 0;
  sscanf(buffer,"%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf",
           &orb[i][0],&orb[i][1],&orb[i][2],&orb[i][3],
           &orb[i][4],&orb[i][5],&orb[i][6],&orb[i][7],&orb[i][8]);
  strcpy(name[i],"to determine object"); numb[i] = 0;
  istart = 0; iend = 1;
  }

/************************************************************************/
/* Ceres:
w1 = 2449822.26925; w2 = 71.26039; w3 = 80.66349; w4 = 10.59923;
w5 = 0.0763422; w6 = 2.7679940; w7 = 3.31; w8 = 0.10; w9 = 913;
*/
/* Vesta:
w1 = 2448965.92703; w2 = 150.07708; w3 = 103.98238; w4 = 7.13516; 
w5 = 0.0894845; w6 = 2.3613357; w7 = 3.16; w8 = 0.34; w9 = 501;
*/

  printf(" Date: %02d-%02d-%.1f at %.4fH \n",idd,mm,aa,time);
  fprintf(fp1,"Date: %02d-%02d-%.1f at %.4fH \n",idd,mm,aa,time);
  fprintf(fp1,"1=Number  2=Name  3=R.A. (2000)  4=Dec (2000) \n");
  fprintf(fp1,"5=dist_to_earth (UA)  6=dist_to_sun (UA)  7=V_magni \n");
  fprintf(fp1,"8=Phase_angle (deg)  9=Diameter (milliarcsec)\n");
/* Compute julian day "date_obs" */
  julian(aa,mm,idd,time,&date_obs);

/************************************************************************/
/* Main loop: */
for(i = istart; i < iend; i++)
  {
   printf("\n Now computing %d %s coordinates\n",numb[i],name[i]);
#ifdef DEBUG
  printf("OK: tau_0 (Jul. day), omega (deg), big_omega (deg), incl (deg), eccent, semi-major axis (UA)\n");
  printf("%d %lf %lf %lf %lf %lf %lf %lf %lf %s \n",numb[i],
            orb[i][0],orb[i][1],orb[i][2],orb[i][3],
            orb[i][4],orb[i][5],orb[i][6],orb[i][7],name[i]);
#endif

  tau_0 = orb[i][0];
  omega = orb[i][1] * PI / 180.;
  big_omega = orb[i][2] * PI / 180.;
  incl = orb[i][3] * PI / 180.;
  eccent = orb[i][4];
  semajaxis = orb[i][5];
/* Absolute magnitude */
  h_1 = orb[i][6];
/* Slope parameter: */
  g_1 = orb[i][7];
/* Diameter: */
  true_diameter = orb[i][8];

  helio_coord(date_obs,incl,semajaxis,eccent,big_omega,omega,tau_0,
            &lambda,&beta,&planet_helio_dist);

#ifdef DEBUG
  output_coord(lambda," Heliocentric longitude :","D");
  output_coord(beta," Heliocentric latitude :","D");
#endif

/* Coordinates of the sun (ecliptic geocentric)*/
  sun_coordinates(date_obs,&lambda_sun,&beta_sun);

/* Now to geocentric coordinates (compute also phase angle): */
  helio_to_geo(lambda,beta,planet_helio_dist,
             lambda_sun,beta_sun,
             &alpha,&delta,&planet_to_earth_dist,&phase_angle);

  output_coord(alpha," Right ascension (2000):","H");
  output_coord(delta," Declination (2000):","D");
  printf(" Distance of the planet to the earth: %.3f (UA),  to the sun: %.3f (UA)\n",
         planet_to_earth_dist,planet_helio_dist);

/* Compute magnitude (Annuaire du Bureau des Longitudes) 
*/
  asteroid_magni(planet_to_earth_dist,planet_helio_dist,h_1,g_1,
               phase_angle,&magni);
  app_diameter = true_diameter/(150.e+6 * planet_to_earth_dist);
  app_diameter *= 3600. * 180. * 1000 /PI;
  printf(" V=%.1f    Diameter: %d (milliarcsec)  Phase angle: %.2f (deg)\n",
         magni,(int)app_diameter,(phase_angle * 180. / PI));

   fprintf(fp1,"\n %3d %12s",numb[i],name[i]);
   output_coord1(alpha,delta,fp1);
   fprintf(fp1," %5.2f %5.2f",planet_to_earth_dist,planet_helio_dist);
   fprintf(fp1," %4.1f %5.1f %3d",magni,(phase_angle * 180. / PI),
                                 (int)app_diameter);

  }                                  /* End of main loop */

/* End of program */
 fclose(fp1);
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
* M: mean anomaly (if starting from perihelium, same angular velocity)
* param: parameter p of the orbit p = a ( 1 - e**2)
* tau_0: instant of perihelium passage.
***********************************************************/
int helio_coord(date_obs,incl,semajaxis,eccent,big_omega,omega,tau_0,
                lambda,beta,planet_helio_dist)
double date_obs,incl, semajaxis, eccent, big_omega, omega, tau_0;
double *lambda, *beta, *planet_helio_dist;
{
double period, nn, mean_ano, eccent_ano, true_ano;
double w1, w2;

/* First compute the period: 
n**2 a**3 = k (M_sol + m)
thus, n = 2 pi / T = sqrt(k (M_sol + m))/ a**(3/2)
*/
nn = SQRT_K / pow(semajaxis, 1.5);
period = 2. * PI / nn;
#ifdef DEBUG
  printf(" helio_coord/period is %f days \n",period); 
#endif

w1 = date_obs - tau_0;
w1 = w1 - period * (double)((int)(w1/period)); if(w1 < 0) w1 += period;
mean_ano = nn * w1;
from_mean_to_eccent_anomaly(mean_ano,&eccent_ano,eccent);
#ifdef DEBUG
printf(" helio_coord/Mean anomaly = %f, eccent anomaly = %f \n",
         mean_ano,eccent_ano);
#endif

/* True anomaly, cf tan(v/2) = tan (u/2) * sqrt ( (1+e) / (1-e) ) */
w1 = tan (eccent_ano / 2.) * sqrt( ( 1. + eccent) / (1. - eccent) );
w1 = atan(w1);
/* Uncertainty w1 or w1+PI (but not important since multiplied by two,
   true_ano or true_ano+ 2*PI) */ 
true_ano = 2. * w1;
if(true_ano < 0) true_ano += 2. * PI; 

#ifdef DEBUG
printf(" helio_coord/true anomaly = %f \n",true_ano);
#endif

/* Computing heliocentric distance of the planet: 
* r = p / (1 + e cos v) = a ( 1 - e cos u)
*/
*planet_helio_dist = semajaxis * ( 1. - eccent * cos (mean_ano)); 
#ifdef DEBUG
printf(" helio_coord/Heliocentric distance of the planet = %f (A.U.)\n",
         *planet_helio_dist);
#endif

/******************************************************************/
/* Now computing heliocentric coordinates: 
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
*beta = asin(w1); 
/* Since we a priori do not know what asin does, we do this: */
if(cos(*beta) < 0) *beta = PI - *beta; 

/* JLP 93 : beta always between -PI and +PI !*/
/* Test beta !!!! beta or (PI-beta)? : always cos(beta) > 0  when incl < pi/2 */
/*
if(cos(incl) < 0) *beta = PI - *beta; 
*/

/******************************************************************/
/* Then lambda, by solving the same triangle:
* cos (beta) cos(w1) = cos (v + omega) 
* cos (beta) sin(w1) = cos(i) sin (v + omega) 
*/
w1 = cos(true_ano + omega) / cos(*beta); 
w1 = acos(w1); 
/* Since we a priori do not know what acos does, we do this: */
if(sin(w1) < 0) w1 = -w1; 

/* Test w1: w1 or -w1? 
* Check with (in the same triangle, too)
* sin(w1) = cos(i) sin(v+omega) /cos(beta)
*/
w2 = cos(incl) * sin(true_ano + omega) / cos(*beta); 
if(w2 < 0) w1 = -w1; 

/* Heliocentric longitude now: */ 
 *lambda = w1 + big_omega;

#ifdef DEBUG
 printf(" helio_coord/Heliocentric coordinates: lambda=%f, beta=%f \n",
         *lambda,*beta);
#endif

}
/***************************************************
* Conversion from heliocentric to geocentric coordinates
*
* Cf Danjon, Astronomie Generale pp.184-185
****************************************************/
int helio_to_geo(lambda,beta,planet_helio_dist,lambda_sun,beta_sun,
             alpha,delta,planet_to_earth_dist,phase_angle)
double lambda, beta, planet_helio_dist, lambda_sun, beta_sun;
double *alpha, *delta, *planet_to_earth_dist, *phase_angle;
{
double sun_dist, w1;
double x1, y1, z1;
double x2, y2, z2;
double xx, yy, zz;

/* Going to x1,y1,z1 heliocentric ecliptic coordinates: */
x1 = planet_helio_dist * cos(beta) * cos(lambda);
y1 = planet_helio_dist * cos(beta) * sin(lambda);
z1 = planet_helio_dist * sin(beta);

/* Transformation to heliocentric equatorial coordinates (rotation -epsilon) */
x2 = x1;
y2 = y1 * cos(EPSILON) - z1 * sin(EPSILON);
z2 = y1 * sin(EPSILON) + z1 * cos(EPSILON);

/* Coordinates of the sun relative to the earth in ecliptic coordinates */
sun_dist = 1.;
xx = sun_dist * cos(lambda_sun) * cos(beta_sun); 
yy = sun_dist * (cos(EPSILON) * sin(lambda_sun) * cos(beta_sun) 
                 - sin(EPSILON) * sin(beta_sun)); 
zz = sun_dist * (sin(EPSILON) * sin(lambda_sun) * cos(beta_sun) 
                 + cos(EPSILON) * sin(beta_sun)); 

/* Then the coordinates of the planet relative to the earth are: */
x1 = x2 + xx;
y1 = y2 + yy;
z1 = z2 + zz;

/* We then have to solve:
       x1 = R cos(delta) cos(alpha)
       y1 = R cos(delta) sin(alpha)
       z1 = R sin(delta)
* and infer (alpha,delta) */
  *planet_to_earth_dist = sqrt(x1*x1 + y1*y1 + z1*z1);
  *delta = z1 / sqrt( x1*x1 + y1*y1);
  *delta = atan(*delta);
  if(cos(*delta) < 0) *delta += PI;

  *alpha = y1 / x1;
  *alpha = atan(*alpha);
  if((cos(*delta) * x1) < 0) *alpha += PI;

#ifdef DEBUG
 printf("helio_to_geo/ alpha = %f, delta = %f \n",*alpha,*delta);
#endif

/* Compute phase angle: 
*  (cosinus is scalar product of (x1,y1,z1) by (x2,y2,z2) ) */
w1 = x1 * x2 + y1 * y2 + z1 * z2;
/* Normalization: */
w1 /= sqrt(x1 * x1 + y1 * y1 + z1 * z1);
w1 /= sqrt(x2 * x2 + y2 * y2 + z2 * z2);
*phase_angle = acos(w1);

#ifdef DEBUG
 printf("helio_to_geo/ phase_angle = %f (deg)\n",(*phase_angle)*180./PI);
#endif

return(0);
}
/***********************************************
* To compute sun position (equatorial geocentric coordinates) 
* beta_sun is always very small...
*************************************************/
int sun_coordinates(date_obs,lambda_sun,beta_sun)
double date_obs, *lambda_sun, *beta_sun;
{
double origin, m, w1;
double alpha_sun, delta_sun;

julian((double)1950.0,1,3,(double)2.,&origin);
#ifdef DEBUG
printf(" sun_coordinates/origin is (3 Jan 1950, 2.00H): %f \n",origin);
#endif

/* Mean anomaly: M = 3548.2" (t - t_0) */
w1 = (date_obs - origin);
w1 = w1 - 365.25 * (double) ((int)(w1/365.25));

/* Mean anomaly in degrees: */
m = (3548.2/3600.) * w1;
*lambda_sun = 282.075 + m + 1.916667 * sin((double) (m * PI / 180.)); 
*lambda_sun = *lambda_sun - 360. * (double)((int)(*lambda_sun / 360.));

/* Conversion to radians: */
*lambda_sun = *lambda_sun * PI / 180.;
*beta_sun = 0.;

#ifdef DEBUG
/* Check by computing the sun coordinates: 
*   tan(alpha) = cos(epsilon) * tan (lambda)
*/
w1 = cos(EPSILON) * tan (*lambda_sun);
alpha_sun = atan(w1);
/* Check if alpha or alpha+PI (alpha_sun and lambda should be in same
quadrant): */
if(cos(*lambda_sun)*cos(alpha_sun) < 0 ) alpha_sun += PI; 
output_coord(alpha_sun," Sun right ascension :","H");

w1 = sin(EPSILON) * sin(*lambda_sun);
delta_sun = asin(w1);
if( tan(delta_sun) * sin(alpha_sun) < 0) delta_sun = PI - delta_sun;
if(cos(delta_sun) < 0) delta_sun = delta_sun + 2. * PI; 
output_coord(delta_sun," Sun declination :","D");
#endif

return(0);
}
/*********************************************************
* Compute magnitude (Annuaire du Bureau des Longitudes) 
*
* h_1: Absolute magnitude
* g_1: Slope parameter
*********************************************************/
int asteroid_magni(planet_to_earth_dist,planet_helio_dist,h_1,g_1,
               phase_angle,magni)
double planet_to_earth_dist, planet_helio_dist, h_1, g_1, phase_angle;
double *magni;
{
double w1, w2;
w1 = exp( -3.33 * pow(tan(phase_angle/2.),0.63));
w2 = exp( -1.87 * pow(tan(phase_angle/2.),1.22));
*magni = 5. * log10 (planet_helio_dist * planet_to_earth_dist) + h_1
        - 2.5 * log10((1. - g_1) * w1 + g_1 * w2);
return(0);
}
/**********************************************************/
#include <cel_meca1.c> 
