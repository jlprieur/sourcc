/***********************************************************
* Program ABSP1
* To compute atmospheric absorption
* and position of Risley prisms
*
* Corrected from a bug with negative coordinates (01-08-93)
* All angles in radians (internally)
* Version 19-12-93
* JLP
***********************************************************/
#include <stdio.h>
#include <math.h>
#include <jlp_ftoc.h>

/*
#define DEBUG
*/

#define PI 3.14159265
#define DEGTORAD   (PI/180.00)

main()
{
double xlong, xlat, equin, aa, time, dyear, hour_angle, elev, azim;
double alpha, delta, sin_beta, beta;
double dyears, zen_dist, rho;
double atmpp, hygro, temper, pw_sat, ff;
double lambdac, dlambda, cross_angle, resid_disp, h3;
int idd, mm, iopt, ih1, ih2, italk, icode1, icode2, istat;
char answer[2], logfile[61];
FILE *fp;

strcpy(logfile,"absp1.log");

if ((fp = fopen(logfile,"w")) == NULL)
    {printf("Fatal error opening logfile %s \n",logfile);
     exit(-1);
    }  

fprintf(fp," Program absp1  - Version of 19-12-93 - \n");
 
/* We also set the mean atmospheric conditions:
*  550 mm Hg 50% of hygrometry 0.C (Temp) */
/*
 jlp_get_dble(&atmpp,"Atmospheric pressure",atmpp);
 jlp_get_dble(&hygro,"Atmospheric pressure",atmpp);
 jlp_get_dble(&temper,"Atmospheric pressure",atmpp);
*/
 atmpp = 561.0;
 hygro = 60.;
 temper = 6.0;
 printf("Atm. Pres.(mm Hg), Hygro.(%%), Temp.(Celsius)");
 printf(" (560.,50.,-5.) := ");
 scanf("%lf,%lf,%lf",&atmpp,&hygro,&temper);
 printf(" Atm. pressure = %.1f mm. Hygro. degree: %.1f%%. Temp. = %.1f C\n",
       atmpp,hygro,temper);

/* Saturation water pressure: */
 GET_PW_SAT(&pw_sat,&temper);
 ff = hygro * pw_sat /100.;
 
/* Input of latitude and longitude :
* Here set it to 1 (TBL application) */
iopt = 1;
input_location(&xlat,&xlong,&iopt);

/***** Write to logfile the coordinates of the observatory: */
  printf(" Latitude: %f (deg) Longitude: %f (hours) \n",xlat,xlong);
  fprintf(fp," Latitude: %f (deg) Longitude: %f (hours) \n",xlat,xlong);
 
/* Conversion to radians: */
xlat *= DEGTORAD;
xlong *= PI / 12.;

/***** Wavelength and bandwidth: */ 
  printf(" Central wavelength and bandwidth (nm) := ");
  scanf("%lf,%lf",&lambdac,&dlambda);

#ifdef DEBUG
  printf(" Central wavelength: %f Bandwidth: %f \n",lambdac,dlambda);
#endif
  fprintf(fp," Central wavelength: %f Bandwidth: %f \n",lambdac,dlambda);
 
/******************************************************************
* Entering the date of observation:
*/
   printf(" Date of observation: (DD,MM,YYYY) (Ex: 10,09,1993) := ");
   scanf("%d,%d,%lf",&idd,&mm,&aa);

if(aa < 100) 
  printf(" WARNING: observation in AD 00%f (First century) \n",aa);
 
/* Entering ALPHA (in hours)
*/
 input_coord(" Right ascension of the object (HH,MM,SS.SS) := ",&alpha,"H");
 
/* Entering DELTA (in degrees)
*/
 input_coord(" Declination ? (DD,MM,SS.SS) := ",&delta,"D");
 
/* Entering equinox
*/
  printf(" Equinox ? (AAAA.AA) (enter 0 if no correction) := ");
  scanf("%lf",&equin);

/* Logfile: */
  fprintf(fp," Alpha: %f  Delta: %f (rad) Equinox: %f \n",alpha,delta,equin);
  fprintf(fp," Date: %d-%d-%d  ",idd,mm,(int)aa);

/* Correction for the precession */
  if(equin)
    {
     dyears = aa - equin;
     precess(&alpha,&delta,dyears);
    }
 
/******************************************************************
* Entry point:
******************************************************************/
do{
  printf(" Observation time (U.T.) ? (HH,MM,SS.SS) := ");
  scanf("%d,%d,%lf",&ih1,&ih2,&h3);
  time = (float)ih1 + (float)ih2 / 60. + h3 / 3600.;
 
  fprintf(fp," Time: %d-%d-%f \n",ih1,ih2,h3);

/* Get the local coordinates of the star, for the given time and location:
*/
   local_coord__(&aa,&mm,&idd,&time,&xlat,&xlong,&alpha,&delta,
               &hour_angle,&elev,&azim);
 
/* Output of ELEV and AZIM:
*/
    output_coord(elev," Elevation :","D");
    output_coord(azim," Azimuth :","D");
 
/* Get the zenith distance:
*/
  zen_dist = PI/2. - elev;
  output_coord(zen_dist," Zenith distance :","D");
 
/* Get the air mass (sec z) :
*/
   rho = 1./cos(zen_dist);

   printf("%30s %.3f \n"," Air mass :",rho);
   fprintf(fp," Air mass: %.3f \n",rho);
 
/*
 Get Beta angle, i.e., angle between the sides linking the star to the pole
 and the star to the zenith (usefull for correction of atmospheric dispersion) 
*/
   sin_beta = cos(xlat) * sin(hour_angle) / sin(zen_dist);
   beta = asin(sin_beta);
   printf(" Beta angle (for atm. disp.) : %.3f (deg) (North=0, East=90)\n",
            beta/DEGTORAD);
   fprintf(fp," Beta angle (for atm. disp.): %.3f (deg) (North=0, East=90)\n",
            beta/DEGTORAD);
/*
   printf(" %30s %.3f (deg) \n"," Zenith distance :", zen_dist/DEGTORAD);
   fprintf(fp," %30s %.3f (deg) \n"," Zenith distance :", zen_dist/DEGTORAD);
*/

/* WARNING: beta, zen_dist should be in degrees for GET_CROSSANGLE:
*/
   italk = 1;
   zen_dist /= DEGTORAD;
   beta /= DEGTORAD;
   istat = GET_CROSSANGLE(&lambdac,&dlambda,&cross_angle,&resid_disp,
                  &beta,&temper,&ff,&atmpp,&zen_dist,&icode1,&icode2,&italk);
   if(!istat)
      {
      printf(" Risley prisms positions: RA = %d RB = %d \n",icode1,icode2);
      }
 
/********************************************************
* Prompts for another go :
*/
  printf(" Do you want to change the time of observation [Y] : ");
  gets(answer); gets(answer);  
 } while (answer[0] != 'N' && answer[0] != 'n');

 printf(" Logfile in %s\n",logfile);
 fclose(fp);
 exit(0);
} 
#include <cel_meca1.c>
#undef MAIN_PROG
#include <risley.c>
