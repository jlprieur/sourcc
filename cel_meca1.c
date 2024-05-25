/***********************************************************
* cel_meca1.c
* Set of routines about celestial mechanics.
*
* CONTAINS:
*   input_location(xlat,xlong,iloc)
*   local_coord(aa,mm,idd,time,xlat,xlong,alpha,delta,hour_angle,elev,azim)
*   input_coord(string,value,opt)
*   precess(alpha,delta,delta_years)
*   julian(aa,mm,idd,time,djul)
*   output_coord(val,label,opt)
*   output_coord1(alpha,delta,fp1)
*   convert_coord(coord,ial1,ial2,al3,opt)
*   from_mean_to_eccent_anomaly(mean_ano,eccent_ano,eccent)
*
* Except for input_location, all angles are in radians (internally)
*
* Version 16/08/2005
* JLP
***********************************************************/
#include <stdio.h>
#include <math.h>
#include <jlp_ftoc.h>

#ifndef PI
#define PI 3.14159265
#endif
#define DEGTORAD   (PI/180.00)

#include <cel_meca1.h>
/* Prototypes defined in cel_meca1.h :

int input_location(double *xlat, double *xlong, int *iloc);
int local_coord__(double *aa, int *mm, int *idd, double *time, double *xlat,
                  double *xlong, double *alpha, double *delta, 
                  double *hour_angle, double *elev, double *azim);
int input_coord(char *string, double *value, char *opt);
int precess(double *alpha, double *delta, double delta_years);
int julian(double aa, int mm, int idd, double time, double *djul);
int output_coord(double val, char *label, char *opt);
int besselian_epoch(double aa, int mm, int idd, double time, double *b_date);
int output_coord1(double alpha, double delta, FILE *fp1);
int convert_coord(double coord, long *ial1, long *ial2, double *al3, char *opt);
int from_mean_to_eccent_anomaly(double mean_ano, double *eccent_ano, 
                                double eccent);
*/

/****************************************************************
* Subroutine INPUT_LOCATION
* Prompt for the latitude and longitude of the observatory
*
* Output:
* XLAT (degrees) : latitude
* XLONG (hours) : longitude
*****************************************************************/
int input_location(double *xlat, double *xlong, int *iloc)
{

if(*iloc < 1 || *iloc > 5)
  { 
   printf(" Location : \n 1: Pic du Midi \n 2: Mount Stromlo \n");
   printf(" 3: CFHT (Hawaii) \n 4: La Silla (ESO) \n 5: O.H.P. \n"); 
   printf(" 6: Other \n Enter the code number of the location :");
   scanf("%d",iloc);
  }
 
 switch (*iloc)
    {
/* Pic du Midi: LW=+00 DEG 08' 42" lLAT=42 DEG  56' 12"*/
      case 1:
        {
          *xlong = 0.0096667;
          *xlat = 42.9366667;
          break;
        }
/* Mount Stromlo: LW=-149 DEG 00' 30" LAT=-35 DEG 19.2' */
      case 2:
        {
          *xlong = -9.9338889;
          *xlat = -35.320;
          break;
        }
/* CFHT, Hawaii:  LW=+155 DEG 28' 18" LAT=+19 DEG 49.6' */
      case 3:
        {
          *xlong = 10.364778;
          *xlat = 19.826667;
          break;
        }
/* La Silla (ESO): LW=+70 DEG 43' 48"  LAT=-29 DEG 15.4' */
      case 4:
        {
          *xlong = 4.7153333;
          *xlat = -29.256667;
          break;
        }
 
/* OHP (Haute Provence): LW=-5 DEG 42' 48" */
      case 5:
        {
          *xlong = -.3808889;
          *xlat = 43.926667;
          break;
        }
 
/************* Prompt here the coordinates of the observatory: */
     default:
        {
/* Latitude (in degrees):  */
         input_coord(" Latitude ? (DD,MM,SS)",xlat,"D");
/* Longitude: */
         input_coord(" Longitude ? (HH,MM,SS)",xlong,"H");
         break;
        }

/* End of switch: */
   }
 
return(0);
}
/******************************************************************
* Subroutine LOCAL_COORD to compute the azimuth and elevation
* of a star, at a given time
*
* Input:
* AA, MM, IDD, TIME : year,month, day, time of the observation
* XLAT (radians), XLONG (radians): coordinates of the observatory
* ALPHA (radians), DELTA (radians): coordinates of the star
*
* Other parameters:
* DJUL : Julian day
*
* Output:
* HOUR_ANGLE: hour angle
* ELEV: elevation (radians)
* AZIM: azimuth (radians)
***************************************************************/
int local_coord__(double *aa, int *mm, int *idd, double *time, double *xlat,
                  double *xlong, double *alpha, double *delta, 
                  double *hour_angle, double *elev, double *azim)
{
double tt1, gst0h, sin_elev, djul, sin_azim, cos_azim;
int i;
 
/* Julian day of the observation (at 0H UT): */
    julian(*aa,*mm,*idd,0.,&djul);

#ifdef DEBUG
    printf(" Julian day (at 0H U.T.) :%.3f \n",djul);
#endif
 
/* Greenwich Sidereal time at 0:00:00 (U.T.) */
    tt1 = (djul - 2415020.0) / 36525.;
    gst0h = 24. * (0.276919398 + 100.0021359 * tt1 + 0.000001075*tt1*tt1);
/* Reduction to a time smaller than 24 hours: */
    i = gst0h/24.;
    gst0h = gst0h - i * 24.;

/* Conversion to radians */
    gst0h *= PI / 12.;
 
/* Display of TSG0H in hour, min, sec */
    output_coord(gst0h," G.S.T. at 0 H (U.T.) :","H");
 
/* Calculation of the hour angle in hours: */
    *hour_angle = gst0h * 12. / PI + *time * 1.002737908 
                   - (*alpha + *xlong) * 12. / PI;
    *hour_angle = *hour_angle - 24. * (double)((int)(*hour_angle /24.));

/*
printf("alpha=%f (H) xlong=%f (H) gst0h=%f (H)\n",*alpha*12./PI,
       *xlong*12./PI, gst0h * 12./PI);
*/

/* Conversion of the hour angle to radians */
    *hour_angle *= PI / 12.;
    output_coord(*hour_angle," Hour angle :","H");
 
/* Calculation of the elevation ELEV
*/
    sin_elev =  sin(*delta) * sin(*xlat) 
                + cos(*delta) * cos(*xlat) * cos(*hour_angle);
    *elev = asin(sin_elev);

/* Calculation of the azimuth: */
   if(cos(*elev) == 0.)
     {
      *azim = 0.;
     }
   else
     {
      sin_azim = sin(*hour_angle) * cos(*delta) / cos(*elev);
      cos_azim = ( cos(*hour_angle) * sin(*xlat) * cos(*delta)
                  - sin(*delta) * cos(*xlat)) / cos(*elev);
      *azim = acos(cos_azim);
      if(sin_azim < 0) *azim *= -1.;
     }
 
return(0);
}
/******************************************************************
* Subroutine INPUT_COORD
*
* Input:
* STRING
*
* Output:
* VALUE in radians
*
*******************************************************************/
int input_coord(char *string, double *value, char *opt)
{
double al3;
int ial1, ial2;
char buffer[81];
 
  printf("%s",string);
  gets(buffer); 
/* Just to correct some bugs, read it again if empty character string: */
if(!*buffer) gets(buffer);

/* Decode it now: */
  sscanf(buffer,"%d,%d,%lf",&ial1,&ial2,&al3);
 
/* Check if negative input: */
  if(buffer[0] == '-' || buffer[1] == '-')
      *value = (float)ial1 - ((float)ial2)/60. - al3 / 3600.; 
  else
      *value = (float)ial1 + ((float)ial2)/60. + al3 / 3600.; 

/* Conversion to radians: */
  if(*opt == 'H' || *opt == 'h')
     *value *= PI / 12.;
  else
     *value *= DEGTORAD;
    
  return(0);
}
/************************************************************
*       SUBROUTINE PRECESS
* From "Numerical Ephemerides" ?
*
* INPUT : 
* alpha and delta in radians 
* delta_years : (epoch of observation - epoch of the catalogue)
*
* OUTPUT : 
* corrected alpha and delta in radians 
*************************************************************/
int precess(double *alpha, double *delta, double delta_years)
{
double palpha, pdelta;

/* PALPHA and PDELTA : correction per year in hours and degrees */
  palpha = (3.075 + 1.336 * sin(*alpha) * tan(*delta) ) / 3600.;
  pdelta = (20.04 * cos(*alpha)) / 3600.;
 
  palpha *= delta_years;
  pdelta *= delta_years;
#ifdef DEBUG
  printf(" precess/Correction in alpha: %f (min), delta: %f (arcmin)\n",
           palpha * 60., pdelta * 60.);
#endif

  *alpha += (palpha * PI / 12.);
  *delta += (pdelta * DEGTORAD);
  printf(" precess/Coordinates corrected for precession: \n");
/*
  printf(" alpha=%f , delta: %f (radians)\n",*alpha,*delta);
*/
  output_coord(*alpha," Right ascension (corr) :","H");
  output_coord(*delta," Declination (corr) :","D");

return(0);
}
/*********************************************************************
* Subroutine JULIAN to compute the Julian day of an observation:
*
* The Julian day begins at Greenwich mean noon (at 12 U.T.)
*
* Here also the Gregorian calendar reform is taken into account.
* Thus the day following 1582 October 4 is 1582 October 15.
*
* The B.C. years are counted astronomically. Thus the year
* before the year +1 is the year 0.
*
* Input:
* AA, MM, IDD, TIME : year,month, day, time of the observation
* DJUL : Julian day
**********************************************************************/
int julian(double aa, int mm, int idd, double time, double *djul)
{
double day1, year1, date_obs, date_reform;
long month1, ia1, ib1;

  day1 = time/24. + (double)idd;
/* First the year after the 1st March ... */
  if(mm > 2)
    {
     year1 = aa;
     month1 = mm;
    }
   else
    {
     year1 = aa - 1;
     month1 = mm + 12;
    }
 
/* Then check if after the Gregorian reform: */
    date_obs = aa + ((int)(275 * mm / 9) 
               - 2. * (int) ((mm + 9) / 12) + idd - 30 ) / 365.; 
    date_reform = 1582. + 289./365.;
    if(date_obs >= date_reform)
       {
         ia1 = (int) (year1 / 100.);
         ib1 = 2 - ia1 + (int) (((float)ia1)/4.);
       }
    else
         ib1 = 0;
 
/* Now final formula: */
      *djul = (int)(365.25 * year1) + (int)(30.6001 * (month1 + 1))
              + day1 + 1720994.5 + ib1;
 
return(0);
}
/******************************************************************
* Subroutine OUTPUT_COORD
*
* Input:
* VALUE in degrees or radians
*  ...  (to be implemented: VALUE in radians 
*
* Print VALUE in hour (if OPT=H) or degrees (if OPT=D)
*
*******************************************************************/
int output_coord(double val, char *label, char *opt)
{
double val1, h2;
long ih0, ih1;

/* Conversion to hours (if 'H') or degrees: */
if (*opt == 'H')
   val1 = val * 12. / PI;
else
   val1 = val * 180. / PI;

convert_coord(val1,&ih0,&ih1,&h2,opt);
 
/* Output on a file, and on the screen: */
if (*opt == 'H')
   printf("%30s %3d H %2d m %4.2f s \n",label,(int)ih0,(int)ih1,h2); 
else
   printf("%30s %3d D %2d' %4.2f\" \n",label,(int)ih0,(int)ih1,h2); 
 
return(0);
}
/*******************************************************************
* Same as output_coord but for file 
*******************************************************************/
int output_coord1(double alpha, double delta, FILE *fp1)
{
double alpha1, delta1, h2;
long ih0, ih1;

/* Conversion to hours and degrees: */
   alpha1 = alpha * 12. / PI;
   delta1 = delta * 180. / PI;

convert_coord(alpha1,&ih0,&ih1,&h2,"H");

/* Output to a file: */
   fprintf(fp1," %2d %2d %4.1f",(int)ih0,(int)ih1,h2);

convert_coord(delta1,&ih0,&ih1,&h2,"D");
   fprintf(fp1," %3d %2d %2d",(int)ih0,(int)ih1,(int)h2);

return(0);
}
/**************************************************************
* Subroutine CONVERT_COORD
* Input: COORD in degrees or in hour
* Output: IAL1 degrees IAL2 ' IAL3 " (when O='D')
* Output: IAL1 hour IAL2 mn IAL3 sec (when O='H')
***************************************************************/
int convert_coord(double coord, long *ial1, long *ial2, double *al3, char *opt)
{
double coord1, al2;
long isign;

coord1 = coord;

/* Translating the values between 0H and 24H or 0.deg and 360.deg */
  isign = 1;
  if(*opt == 'H')
    {
     coord1 = coord1 - 24. * (float)((int)(coord1/24.));
     if(coord1 < 0) coord1 += 24.;
    }
   else
    {
      if(coord1 < 0)
         {
          isign = -1; 
          coord1 = -coord1; 
         }
    }

/*  Converting the values in mn, sec or ' " */
    *ial1 = (int)coord1;
    al2 = (coord1 - (double)(*ial1)) * 60.;
    *ial2 = (int)al2; 
    *al3 = (al2 - (double)(*ial2)) * 60.;
 
/* When negative ' or negative ", we add 60 : */
    if(*opt == 'D') 
     {
       *ial1 *= isign;
       if(*al3 < 0.)
         { *al3 += 60.;
           (*ial2)--;
         }

       if(*ial2 < 0.)
         { *ial2 += 60;
           (*ial1)--;
         }
      }
return(0);
}
/************************************************************
* Solving M = u - e sin u
*
************************************************************/
int from_mean_to_eccent_anomaly(double mean_ano, double *eccent_ano, 
                                double eccent)
{
double u, u1, test;
int i;

/* First guess: */
u = mean_ano;
i = 0;

/* Main loop (less than 1000 iterations*/
do {
    u1 = mean_ano + eccent * sin(u);
#ifdef DEBUG
    printf(" u=%f ",u1);
#endif
    test = u1 - u;
    if(test < 0) test = -test;
    u = u1;
    i++;
    } while ( test > 1.e-6 && i < 1000);

#ifdef DEBUG
    printf("\n ");
#endif

*eccent_ano = u1;
if(i == 1000) return(-1);
return(0);
}
/*********************************************************************
* Subroutine BESSELIAN to compute the Besselian epoch of an observation:
* Besselian year is a tropical year
* which starts when the mean sun has an ecliptic longitude of 280 degrees
*
* Input:
* aa, mm, idd, time : year,month, day, time of the observation
* b_date : Besselian epoch 
**********************************************************************/
int besselian_epoch(double aa, int mm, int idd, double time, double *b_date)
{
double djul;

julian(aa, mm, idd, time, &djul);

*b_date = 2000. + (djul - 2451544.53)/365.242189;

return(0);
}
