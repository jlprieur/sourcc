/***********************************************************
* Program CAV1
* To compute the position (hour) angle of the telescope
* for a given time of observation. 
*
* All angles in radians (internally)
*
* From "absp1.c" (version 19-12-93)
*
* Version 10-12-96
* JLP
***********************************************************/
#include <stdio.h>
#include <math.h>
#include <jlp_ftoc.h>

/* French or English langage for comments and user interface: */
/*
#define ENGLISH 
*/

/*
#define DEBUG
*/

#define PI 3.14159265
#define DEGTORAD   (PI/180.00)

int main()
{
double xlong, xlat, equin, aa, time, dyear, hour_angle, elev, azim;
double alpha, delta;
double dyears, zen_dist, rho, h3;
int idd, mm, iopt, ih1, ih2, italk, istat;
char answer[2], logfile[61];
FILE *fp;

strcpy(logfile,"cav1.log");

if ((fp = fopen(logfile,"w")) == NULL)
    {
#ifdef ENGLISH
     printf("Fatal error opening logfile %s \n",logfile);
#else
     printf("Error fatale lors de l'ouverture du fichier %s \n",logfile);
#endif
     exit(-1);
    }  

#ifdef ENGLISH
fprintf(fp," Program cav1  - Version of 19-12-93 - \n");
#else
fprintf(fp," Programme cav1  - Version du 19-12-93 - \n");
#endif
 
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

/******************************************************************
* Entering the date of observation:
*/
#ifdef ENGLISH
   printf(" Date of observation: (DD,MM,YYYY) (Ex: 10,09,1993, CR=today) := ");
#else
   printf(" Date de l'observation: (DD,MM,YYYY) (Ex: 10,09,1993, RC=aujourd'hui) := ");
#endif
   scanf("%d,%d,%lf",&idd,&mm,&aa);

if(aa < 100) 
  {
#ifdef ENGLISH
  printf(" ERROR(?): observation in AD 00%f (First century) I add 1900...\n",aa);
#else
  printf(" Erreur(?): observation en 00%f (premier siecle) J'ajoute 1900...\n",aa);
#endif
  aa += 1900.;
  }
 
/* Entering ALPHA (in hours)
*/
#ifdef ENGLISH
 input_coord(" Right ascension of the object (HH,MM,SS.SS) := ",&alpha,"H");
#else
 input_coord(" Ascension droite de l'objet (HH,MM,SS.SS) := ",&alpha,"H");
#endif
 
/* Entering DELTA (in degrees)
*/
#ifdef ENGLISH
 input_coord(" Declination ? (DD,MM,SS.SS) := ",&delta,"D");
#else
 input_coord(" Declinaison ? (DD,MM,SS.SS) := ",&delta,"D");
#endif
 
/* Entering equinox
*/
#ifdef ENGLISH
  printf(" Equinox ? (AAAA.AA) (enter 0 if no correction) := ");
#else
  printf(" Equinoxe ? (AAAA.AA) (enter 0 if no correction) := ");
#endif
  scanf("%lf",&equin);

/* Logfile: */
#ifdef ENGLISH
  fprintf(fp," Alpha: %f  Delta: %f (rad) Equinox: %f \n",alpha,delta,equin);
#else
  fprintf(fp," Alpha: %f  Delta: %f (rad) Equinoxe: %f \n",alpha,delta,equin);
#endif
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
#ifdef ENGLISH
  printf(" Observation time (U.T.) ? (HH,MM,SS.SS) := ");
#else
  printf(" Heure d'observation (T.U.) ? (HH,MM,SS.SS) := ");
#endif
  scanf("%d,%d,%lf",&ih1,&ih2,&h3);
  time = (float)ih1 + (float)ih2 / 60. + h3 / 3600.;
 
#ifdef ENGLISH
  fprintf(fp," Time: %d-%d-%f \n",ih1,ih2,h3);
#else
  fprintf(fp," Heure: %d-%d-%f \n",ih1,ih2,h3);
#endif

/* Get the local coordinates of the star, for the given time and location:
*/
   local_coord(&aa,&mm,&idd,&time,&xlat,&xlong,&alpha,&delta,
               &hour_angle,&elev,&azim);
 
/* Output of ELEV and AZIM:
*/
#ifdef ENGLISH
    output_coord(elev," Elevation :","D");
#else
    output_coord(elev," Hauteur :","D");
#endif
    output_coord(azim," Azimuth :","D");
 
/* Get the zenith distance:
*/
  zen_dist = PI/2. - elev;
 
/* Get the air mass (sec z) :
*/
   rho = 1./cos(zen_dist);

#ifdef ENGLISH
   printf("%30s %.3f \n"," Air mass :",rho);
   fprintf(fp," Air mass: %.3f \n",rho);
#else
   printf("%30s %.3f \n"," Mass d'air :",rho);
   fprintf(fp," Masse d'air: %.3f \n",rho);
#endif
 
/********************************************************
* Prompts for another go :
*/
#ifdef ENGLISH
  printf(" Do you want to change the time of observation [Y] : ");
#else
  printf(" Voulez-vous simplement changer l'heure d'observation [O] : ");
#endif
  gets(answer); gets(answer);  
 } while (answer[0] != 'N' && answer[0] != 'n');

#ifdef ENGLISH
 printf(" Logfile in %s\n",logfile);
#else
 printf(" Fichier compte-rendu : %s\n",logfile);
#endif
 fclose(fp);
 exit(0);
return(0);
} 
#include "cel_meca1.c"
#undef MAIN_PROG
#include "risley.c"
