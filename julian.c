/***********************************************************
* julian.c
* To compute the Julian date 
*
* JLP
* Version 09-03-99
************************************************************/

#define DEBUG

#include <stdio.h>
#include <math.h>
#include <fcntl.h>
#include <jlp_ftoc.h>

#include <cel_meca1.c>

int main(int argc, char *argv[])
{
double aa, time, date_obs, date_jan01, epoch;
int idd, mm;
int i;
char buffer[80];

printf(" Program Julian to compute the Julian date\n");

printf(" Enter day, month, year of observation: (Ex: 21,04,1993)\n");
gets(buffer); sscanf(buffer,"%d,%d,%lf",&idd,&mm,&aa);

printf(" Enter time (decimal) of observation: \n");
gets(buffer); sscanf(buffer,"%lf",&time);

/* Compute Julian date of January 1st 2000. at 12H00: */
julian(2000.,1,1,12.,&date_jan01);

for(i = 0; i < 20; i++)
  { 
/* Compute julian day "date_obs" */
  printf(" %02d/%02d/%4.0f at %.2fH ",idd,mm,aa,time);
  julian(aa,mm,idd,time,&date_obs);
  printf(" Julian date: %.4f",date_obs);

/* Approximation: */
  epoch = aa + (date_obs - date_jan01)/365.242189;
  printf(" Besselian epoch: %10.5f",epoch);

/* Real formula used in "besselian_epoch" */
  besselian_epoch(aa,mm,idd,time,&epoch);
  printf(" or also: %10.5f\n",epoch);
  time += 0.2;
  }

return(0);
}
