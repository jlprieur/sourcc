/***************************************************************
* To compute Risley prisms correction for PISCO/PISCO2 
* 
* Set of routines.
* (or main program in debug mode)
*
* Contains:
* double atm_refrac_simon(wave,tt,ff,pp,zd)
* double ind_air (wave,tt,ff,pp)
* double ind_f4(wave)              
* double ind_sk10(wave)
* double ind_bk7(wave)
* int output_curves(npts,lambda_cent,tt,ff,pp,zd,cross_angle,dlambda,beta0)
* int GET_CROSSANGLE(lambda_cent,dlambda,cross_angle,
* double atm_refrac_owens(wave,tt,ff,pp,zd)
* int GET_PW_SAT(pw_sat,tt)
* 
* In the whole program, "index" is used instead of "(index - 1)"
*
* JLP
* Version of 20-09-2011 
***************************************************************/
#include <stdio.h>
#include <math.h>
#include <jlp_ftoc.h>

#define DEBUG
/*
*/

#define IDIM 200

#define F4_SK10_PRISMS 

#ifndef F4_SK10_PRISMS 
#define SINGLE_BK7_PRISM 
#endif

/* The magnification factor of the speckle camera at the TBL is 250
   i.e. ratio of focal length of the TBL (500 cm) over focal of
   field lens (20 cm)
   250 at Pic du Midi (F=50 m f=0.20m)
   It is 144 at ESO 
   It is 358 in Nice (17.89m f=0.05)
*/
/* #define MAGNIF 250 */
#define MAGNIF 358 

#ifndef PI
#define PI 3.14159265358979323846
#endif
/* To select the refraction formula to be used:
*/
#define OWENS

/* Contained here: */
double ind_air(double wave, double tt, double ff, double pp);
double ind_f4(double wave); 
double ind_sk10(double wave); 
double ind_bk7(double wave); 
int GET_PW_SAT(double *pw_sat, double *tt);
int GET_CROSSANGLE(double *lambda_cent, double *dlambda, double *cross_angle,
                   double *resid_disp, double *beta0, double *tt, double *ff, 
                   double *pp, double *zd, int *icode1, int *icode2, int *italk);
int output_curves(int npts, double lambda_cent, double tt, double ff, 
                  double pp, double zd, double cross_angle, double dlambda, 
                  double beta0);

#ifdef OWENS
#define ATM_REFRAC atm_refrac_owens
double atm_refrac_owens(double wave, double tt, double ff, double pp, double zd);
#else
#define ATM_REFRAC atm_refrac_simon
double atm_refrac_simon(double wave, double tt, double ff, double pp, double zd);
#endif

/* Risley prisms for PISCO1 and PISCO2: */
#ifdef F4_SK10_PRISMS
static double beta1=10.0, beta2=9.92;
/* Case of a single prism: */
#else
static double beta1=15.0;
#endif

//#define MAIN_PROG
#ifdef MAIN_PROG
int main()
{
double hygro, pw_sat, dlambda, cross_angle, beta0, zd;
double lambda_cent, lambda_low, lambda_high, resid_disp, tt, pp, ff;
int npts, italk, icode1, icode2;
int i;


/*
printf("Program RISLEY: correction of the Risley prisms for PISCO\n");
*/
printf("Program RISLEY: correction of the Risley prisms for PISCO2\n");
printf(" Version 20-12-2011 \n");

/* Mean atmospheric pressure at TBL is 550 mm Hg */
pp=561.;hygro=60.;tt=6.;

#define KKPRESSURE_IN_MILLIBARS

#ifdef PRESSURE_IN_MILLIBARS
printf("Enter P (milli-Bars, atm. pressure), hygrometric degree (0-100) ");
/*
printf("and T (degree C, temperature):\n");
scanf("%lf,%lf,%lf", &pp, &hygro, &tt);
*/
pp = 1013.; hygro = 30.; tt=10.;
pp *= (760./1013.);
#else
printf("Enter P (mm Hg, atm. pressure), hygrometric degree (0-100) ");
printf("and T (degree C, temperature):\n");
scanf("%lf,%lf,%lf", &pp, &hygro, &tt);
#endif

printf(" OK: atmospheric pressure is %.2f mmHg (%.2f mBar)\n",
         pp, pp * 1013. / 760.); 

GET_PW_SAT(&pw_sat,&tt);
ff = hygro * pw_sat / 100.;
printf(" Saturation water pressure is %.2f mmHg (%.2f mBar)\n",
         pw_sat, pw_sat * 1013. / 760.); 
printf(" and water partial pressure is %.2f mmHg (%.2f mBar)\n",
         ff, ff * 1013. / 760.); 

zd = 45.; beta0 = 0.;
printf(" Enter zenith angle and beta0 (relative to North) (in degrees) :\n");
scanf("%lf,%lf",&zd,&beta0);

/* NICE: Lambda_cent=560 nm / Delta_lambda=80 nm */
lambda_cent = 560.;
dlambda = 80.;
printf(" Enter central lambda and delta lambda (nm): (Nice:560,80)\n"); 
scanf("%lf,%lf",&lambda_cent,&dlambda);

lambda_low = lambda_cent - dlambda/2.; 
lambda_high = lambda_cent + dlambda/2.; 

/* PISCO1 and PISCO2: */
#ifdef F4_SK10_PRISMS
printf("\n F4_roof_angle = %.3f deg, SK10_roof_angle = %.3f deg \n",
          beta1,beta2);

/* Case of a single prism: */
#else
  printf("\n BK7_roof_angle = %.3f deg \n", beta1);
#endif
printf("\n P = %.2f mmHg, Pwater = %.2f mmHg, T = %.2f degC \n",pp,ff,tt);
printf(" zd = %.2f deg, lambda_cent = %f nm, delta_lambda = %f nm \n\n",
      zd, lambda_cent, dlambda);

#ifdef OWENS
  printf(" Computation using Owens' formula \n");
#else
  printf(" Computation using Simon's formula \n");
#endif

/* Deriving cross_angle between prisms 
   and computing residuals at lambda_cent */
italk = 1;
GET_CROSSANGLE(&lambda_cent,&dlambda,&cross_angle,&resid_disp,
               &beta0,&tt,&ff,&pp,&zd,&icode1,&icode2,&italk);

/* Output of data files to be drawn elsewhere.
   With 66 points, the step is 10 nm: */
#ifdef DEBUG
  npts=11;
#else
  npts=66;
#endif
/* Too many messages if debug:
#ifndef DEBUG
output_curves(npts,lambda_cent,tt,ff,pp,zd,cross_angle,dlambda,beta0);
#endif
*/
/* Output table for Rene Gili: */
lambda_cent = 560.;
dlambda = 80.;
italk = 0;
beta0 = 0.;
printf("zd (deg) cross_angle (deg) delta (deg) position (deg) resid_disp (arcsec)\n");
for(i = 0; i < 15; i++){
zd = 5. * (float)i;
GET_CROSSANGLE(&lambda_cent,&dlambda,&cross_angle,&resid_disp,
               &beta0,&tt,&ff,&pp,&zd,&icode1,&icode2,&italk);
printf("%.2f & %.2f & %.2f & %.2f & %.3f \\\\ \n", 
        zd, cross_angle, 44. - zd, 180. - cross_angle/2., resid_disp);
}


return(0);
}
#endif
/* End of ifdef MAIN_PROG */
/***************************************************************
* atm_refrac_simon(wave,tt,ff,pp)
* Routine to compute atmospheric refraction versus wavelength
* From Simon, W. 1966, A.J. 71,190
*
* OUTPUT:
* atm_refrac: atmospheric refraction (in arcseconds)
***************************************************************/
double atm_refrac_simon(double wave, double tt, double ff, double pp, double zd)
{
double atm_refrac, zd_rad, cc[15];
double zz,z2,z3,z4,ss,sq,qq,yy,y2;

cc[0] = 3.45020e-03;
cc[1] = -3.34591e-04;
cc[2] = -1.60149e-05;
cc[3] = -1.54316;
cc[4] = 2.27095e-01;
cc[5] = 3.14759e-03;
cc[6] = 2.87409e-03;
cc[7] = -2.92730e-04;
cc[8] = -1.56673e-05;
cc[9] = 1.41299e-03;
cc[10] = -2.22307e-04;
cc[11] = -1.66610e-06;
cc[12] = -3.34814e-05;
cc[13] = 5.33352e-06;
cc[14] = 3.52107e-08;

qq = 0.740568;
 
#ifdef DEBUG
printf(" Computing atmospheric refraction \n");
printf(" From Simon, W. 1966, A.J. 71,190, h0=2811m, p0=547mm, T0=6degC \n");
printf(" pp,ff,tt,zd %f %f %f %f \n",pp,ff,tt,zd);
#endif
 
zd_rad = zd*PI/180.0;
zz=tan(zd_rad);
z2=zz*zz;
z3=z2*zz;
z4=z2*z2;
 
ss=pp-0.148238*ff+(1.049-0.0157*tt)*pp*pp*1.e-06;
ss=ss/(720.883*(1.+0.003661*tt));
sq=ss/qq;
 
/* Formula with lambda in microns... */
  yy=1.E06/(wave*wave);
  y2=yy*yy;

/* Atmospheric refraction in arcsec
 (From Simon, W. 1966, A.J. 71,190) */
/* Reference wavelength is 400 nm */ 
  atm_refrac=cc[0] + cc[1]*yy + cc[2]*y2 + cc[3]*zz + cc[4]*zz*yy
     + cc[5]*zz*y2 + cc[6]*z2 + cc[7]*z2*yy + cc[8]*z2*y2
     + cc[9]*z3 + cc[10]*z3*yy + cc[11]*z3*y2 + cc[12]*z4
     + cc[13]*z4*yy + cc[14]*z4*y2;
  atm_refrac=atm_refrac*sq;

#ifdef DEBUG
 printf(" Atm. refraction is %f arcsec (%f milli-radians) at %.2f nm (outside the telescope, with 0 at 400 nm)\n",
        atm_refrac, 1000. * atm_refrac*PI/(180 * 3600.), wave);
#endif

return(atm_refrac);
}
/*********************************************************************
*   ind_air
*
*   wave is wavelength in nm.
*   returns n(wave)-1 for atmosphere at TEMP, PRESS (air), WPRESS (water vapor)
*   using formulae 29-31 in Owens, J.C., Appl. Opt. v.6, p.51 (67)
*
*   PRESS, WPRESS in hectopascals (mbars), TEMP in degrees C
*   these values are defined in site.h .
*
* INPUT:
* tt: temperature (Celsius)
*
*********************************************************************/
double ind_air(double wave, double tt, double ff, double pp)
{ 
double tk, ds, dw, w, wm2, r;
double TEMP, PRESS, WPRESS;
        TEMP = tt; 
/* PRESS: atmospheric pressure in hectopascals (mbars) */ 
        PRESS = pp * 1013. / 760.; 
/* WPRESS: water pressure in hectopascals (mbars) */ 
        WPRESS = ff * 1013. /760.;
  
/* tk: temperature in Kelvins */ 
        tk = TEMP + 273.16;

        ds = PRESS / tk * 
            (1. + PRESS * (57.9E-8 - 9.325E-4 / tk + .25844 / (tk*tk)));
        dw = (-2.37321E-3 + (2.23366 / tk) - 710.792 
                    / (tk*tk) + 7.75141E-4 / (tk*tk*tk));
        dw = WPRESS / tk * (1. + WPRESS * (1. + WPRESS * 3.7E-4) * dw);

        w = wave * 1.0E-3;                      /* # convert to microns */
        wm2 = 1./(w*w);

        r = ds * (2371.34 + 683939.7 / (130.-wm2) + 4547.3 / (38.9 - wm2));
        r += dw * (6847.31 + 58.058 * wm2 
                 - .7115 * wm2*wm2 + .08851 * wm2*wm2*wm2);
        r *= 1.e-8;
#ifdef DEBUG
  printf (" (air index - 1) is %7.5g at %.2f nm\n", r, wave);
#endif
return (r);
}

/*********************************************************************
* ind_bk7: refractive index of BK7 glass (Borosilicate "Crown")
*
* returns n (wave) - 1 for BK7 glass
* using Sellmeier's formula 
*
* INPUT:
* wave is wavelength in nm
*********************************************************************/
double ind_bk7(double wave)
{
double bkb[3], bkc[3], w, w2, ensq;
double ind;
  
        bkb[0] =  1.03961212;
        bkb[1] = 0.231792344;
        bkb[2] = 1.01046945;
        bkc[0] = 6.00069867E-3;
        bkc[1] = 2.00179144E-2;
        bkc[2] = 1.03560653E+2;

/* Convert to microns */
        w = (double)wave * 1.E-3;                        
        w2 = w*w;
        ensq = 1 + (bkb[0] * w2 ) / (w2 - bkc[0])
              + (bkb[1] * w2 ) / (w2 - bkc[1]) + (bkb[2] * w2 ) / (w2 - bkc[2]);
        ensq = sqrt (ensq);
        ind = ensq - 1.0;

#ifdef DEBUG
  printf (" BK7 glass index -1 is %f at %5.1f nm\n",
          ind, wave);
#endif
return (ind);
}
/*********************************************************************
* ind_f4: refractive index of F4 glass
*
* returns n (wave) - 1 for f4 glass
* using power series from Schott catalog 
*
* INPUT:
* wave is wavelength in nm
*********************************************************************/
double ind_f4(double wave)              
{
double f4[6], w, w2, wm2, wm4, wm6, wm8, ensq;
double ind;
  
        f4[0] = 2.54469;
        f4[1] = -8.5925665E-3;
        f4[2] = 2.2583116E-2;
        f4[3] = 7.378991E-4;
        f4[4] = -9.5060668E-6;
        f4[5] = 3.82577E-8;

/* Convert to microns */
        w = (double)wave * 1.E-3;                        
        w2 = w*w;
        wm2 = 1./w2;
        wm4 = wm2*wm2;
        wm6 = wm4*wm2;
        wm8 = wm4*wm4;
        ensq = f4[0] + f4[1] * w2 + f4[2] * wm2 + f4[3] * wm4 
               + f4[4] * wm6 + f4[5] * wm8;
        ensq = sqrt (ensq);
        ind = ensq - 1.0;

#ifdef DEBUG
  printf (" f4 glass index -1 is %f at %5.1f nm\n",
          ind, wave);
#endif
return (ind);
}

/*********************************************************************
* ind_sk10
*  returns n(wave)-1 for sk10 glass
*  using power series from Schott catalog 
* wave is wavelength in Angstroms
*********************************************************************/
double ind_sk10(double wave)    
{
double sk10[6], w, w2, wm2, wm4, wm6, wm8, ensq;
double ind;

        sk10[0] = 2.588171;
        sk10[1] = -9.3042171E-3;
        sk10[2] = 1.6075769E-2;
        sk10[3] = 2.2083748E-4;
        sk10[4] = 3.5467529E-6;
        sk10[5] = 2.6143582E-7;

/* convert to microns */
        w = (double)wave * 1.E-3;                  
        w2 = w*w;
        wm2 = 1./w2;
        wm4 = wm2*wm2;
        wm6 = wm4*wm2;
        wm8 = wm4*wm4;
        ensq = sk10[0] + sk10[1] * w2 + sk10[2] * wm2 
                + sk10[3] * wm4 + sk10[4] * wm6 + sk10[5] * wm8;
        ensq = sqrt (ensq);
        ind = ensq - 1.0;
#ifdef DEBUG
printf (" sk10 glass index -1 is %f at %5.1f nm\n",
         ind, wave);
#endif
return (ind);
}
/**********************************************************************
* output_curves
**********************************************************************/
int output_curves(int npts, double lambda_cent, double tt, double ff, 
                  double pp, double zd, double cross_angle, double dlambda, 
                  double beta0)
{
double lambda[IDIM], ad[IDIM], residual[IDIM];
double  radtosec, sin1, sin2, cross_angle1, resid_disp1;
double step, atm_refrac_low, work, work1, work2, cross_coef, cangle;
register int i;
char outfile[41];
FILE *fd;
int italk, icode1, icode2;

step=(1000-350)/(float)(npts-1);

/* lambda will be in nanometers */
for(i=0; i<npts; i++) lambda[i]=350+step*(float)i ;

/******************************************************************/
/* Output of glass index */
#ifdef F4_SK10_PRISMS
  fd=fopen("f4_sk10.dat","w");
  for(i=0; i<npts; i++)
     fprintf(fd,"%8.4f %f %f \n",(float)lambda[i],(float)ind_f4(lambda[i]),
          (float)ind_sk10(lambda[i]));
  fclose(fd);
  printf(" Output of glass index in 'f4_sk10.dat' (f4 and sk10) \n");
#else
/* Case of a single prism: */
  fd=fopen("bk7.dat","w");
  for(i=0; i<npts; i++)
     fprintf(fd,"%8.4f %f \n",(float)lambda[i],(float)ind_bk7(lambda[i]));
  fclose(fd);
  printf(" Output of glass index in 'bk7.dat' \n");
#endif

/* Some constants: */
  radtosec=180.*3600./PI;
  cangle = (cross_angle/2.)*PI/180.;
/* Taking the magnification factor into account: */
  cross_coef = 2.*cos(cangle)*radtosec/MAGNIF;

/* Computing the atmospheric refraction in arcseconds,
  translated to the value for lambda_low */
  atm_refrac_low = ATM_REFRAC(lambda_cent - dlambda/2.,tt,ff,pp,zd);

/* Computing residual dispersion in arcsec (focal plane): */
#ifdef F4_SK10_PRISMS
  sin1 = sin((double)(beta1*PI/180.));
  sin2 = sin((double)(beta2*PI/180.));
  for(i = 0; i < npts; i++) {
   work1 = ind_f4(lambda[i])* sin1;
   work2 = ind_sk10(lambda[i])* sin2;
   ad[i] = atm_refrac_low - ATM_REFRAC(lambda[i],tt,ff,pp,zd);
   residual[i] = ad[i] + (work1-work2) * cross_coef;
  }
/* Computing residual dispersion translated to the value for lambda_low */
work = (ind_f4(lambda_cent - dlambda/2.) * sin1 
               - ind_sk10(lambda_cent - dlambda/2.) * sin2) * cross_coef;
/* Case of a single prism: */
#else
  sin1 = sin((double)(beta1*PI/180.));
  for(i = 0; i < npts; i++) {
   work1 = ind_bk7(lambda[i])* sin1;
   ad[i] = atm_refrac_low - ATM_REFRAC(lambda[i],tt,ff,pp,zd);
   residual[i] = ad[i] + work1 * cross_coef;
  }
/* Computing residual dispersion translated to the value for lambda_low */
work = ind_bk7(lambda_cent - dlambda/2.) * sin1 * cross_coef;
#endif


/* Normalisation to have a null residual at lambda_low: */
  for(i=0; i<npts; i++) residual[i] = residual[i] - work; 

/******************************************************************/
/* Output of the residual dispersion and atmospheric dispersion: */
sprintf(outfile,"risley.dat");
fd = fopen(outfile,"w");
printf(" Output of lambda, residual disp. (arcsec in focal plane), atm. disp. (arcsec) ");
printf(" in file: risley.dat \n");

fprintf(fd," %8.2f %f %f %8.2f %8.2f %8.2f %8.2f \n",
         (float)lambda[0],(float)residual[0],(float)ad[0],pp,ff,tt,zd);
  for(i=1; i<npts; i++) 
      fprintf(fd,"%8.2f %f %f \n",
         (float)lambda[i],(float)residual[i],(float)ad[i]);

fclose(fd);

/******************************************************************/
/* Loop on all the spectrum, to draw the residual dispersion at
lambda_cent with the selected bandwidth */

printf(" Output of resid_disp and cross_angle for all spectrum in file: resid_cross.dat \n");
fd = fopen("resid_cross.dat","w");

/* Deriving cross_angle between prisms 
   and computing residuals at lambda_cent */
  for(i=0; i<npts; i++) 
    {
      italk = 0;
      GET_CROSSANGLE(&lambda[i],&dlambda,&cross_angle1,
                     &resid_disp1,&beta0,&tt,&ff,&pp,&zd,
                     &icode1,&icode2,&italk);
      fprintf(fd,"%8.2f %f %f\n",
         (float)lambda[i],(float)resid_disp1,(float)cross_angle1);
    }
fclose(fd);

return(0);
}

/**********************************************************************
* GET_CROSSANGLE
* Input:
*  airdisp (arcsec): air dispersion between lambda_low and lambda_high
**********************************************************************/
int GET_CROSSANGLE(double *lambda_cent, double *dlambda, double *cross_angle,
                   double *resid_disp, double *beta0, double *tt, double *ff, 
                   double *pp, double *zd, int *icode1, int *icode2, int *italk)
{
double lambda_low, lambda_high, airdisp;
double sin1, sin2, cross_coef, work, cangle;
double prism_disp, mean_dev, radtosec;

lambda_low = *lambda_cent - *dlambda/2.; 
lambda_high = *lambda_cent + *dlambda/2.; 

/* Conversion to arcsec: */
radtosec = 180.*3600./PI;

/* Computing air dispersion between lambda_low and lambda_high
*/
#ifdef DEBUG
 printf(" ************ Lambda_low= %f nm high=%f nm \n", lambda_low, lambda_high);
#endif
airdisp = ATM_REFRAC(lambda_low,*tt,*ff,*pp,*zd) -
          ATM_REFRAC(lambda_high,*tt,*ff,*pp,*zd);
if(airdisp < 0) airdisp = -airdisp;

/* Computing prism dispersion: */
#ifdef F4_SK10_PRISMS
  sin1 = sin((double)(beta1*PI/180.));
  sin2 = sin((double)(beta2*PI/180.));
  prism_disp = (ind_f4(lambda_low) - ind_f4(lambda_high)) * sin1 
               - (ind_sk10(lambda_low) - ind_sk10(lambda_high)) * sin2 ;
/* Case of a single prism: */
#else
  sin1 = sin((double)(beta1*PI/180.));
  prism_disp = (ind_bk7(lambda_low) - ind_bk7(lambda_high)) * sin1;
#endif


#ifdef DEBUG
if(*italk)
  {
printf(" dispersion of individual prism = %f arcsec (%f mrad) in parallel beam) \n",
        prism_disp * radtosec, prism_disp * 1000.);
printf(" (between %f and %f nm)  \n", lambda_low, lambda_high);
printf(" air dispersion=%f \" (%f mrad) (Cassegrain focus) or %f \" (%f mrad) (parallel beam) \n",
        airdisp, 1000. * airdisp * PI / (180. * 3600. ), 
        airdisp * MAGNIF, MAGNIF * 1000. * airdisp * PI / (180. * 3600. )); 
  }
#endif

/* Taking the magnification factor into account: */
  work = (airdisp * MAGNIF / radtosec) / (2. * prism_disp) ;
  if(work > -1. && work < 1.)
     {
     work = 2. * acos(work);
/* Conversion to degrees: */
     *cross_angle = work * 180./PI;
     }
  else
    {
    printf("get_crossangle/Dispersion cannot be fully corrected: ");
    printf("ratio=%.2f for %.2f nm\n",work,*lambda_cent);
    *cross_angle = 0.;
    }


if(*italk)
  {
   printf(" Air dispersion to be corrected: %f (arcsec in focal plane) \n",
      airdisp);
   printf(" Optimum cross_angle is %f (deg) \n",*cross_angle);
  }

/* Theoretical code positions assuming 0 offset and counterclockwise... 
(see 10 lines further down for real conversion...) */
*icode1 = (int)(1024. * (*beta0 - (*cross_angle)/2.0) /360.);
*icode2 = *icode1 + (int)(1024. * (*cross_angle) /360.);
*cross_angle = (float)(*icode2 - *icode1) * 360. / 1024.;

/* offset in X and Y for the prisms; */
/* Position of April 94 to compensate for the  dispersion RB-RA=181,
RA and RB aligned to North: 40, 371
Bonnette at 90 degrees (code 166 , 76) 
CP40, channel 4, X axis to South, Y axis (downwards) to West.  
*/
#define OFFSET1 371 
#define OFFSET2 40 
/* (Channel 3, August 1993: increasing code number to the West (North on top))*/
/* Channel 4, April 1994: increasing code number to the East (North on top) */
/* And both prisms are orientated in the same way */
*icode1 = OFFSET1 + *icode1; 
*icode2 = OFFSET2 + *icode2; 

if(*icode1 < 0) *icode1 += 1024;
if(*icode2 < 0) *icode2 += 1024;

if(*italk)
  {
 printf(" Code positions: RA=%d, RB=%d (1024 steps), cross_angle: %f (deg) \n",
        *icode1,*icode2,*cross_angle);
  }

cangle = (*cross_angle/2.) * PI/180.;
cross_coef = 2. * cos(cangle) * radtosec / MAGNIF;

/* Computing mean deviation */
#ifdef F4_SK10_PRISMS
mean_dev = ind_f4(*lambda_cent) * sin1 - ind_sk10(*lambda_cent) * sin2;
/* Case of a single prism: */
#else
mean_dev = ind_bk7(*lambda_cent) * sin1;
#endif
mean_dev = mean_dev * cross_coef;

/* Residual dispersion: */
#ifdef F4_SK10_PRISMS
*resid_disp = (ind_f4(lambda_low) - ind_f4(lambda_high)) * sin1 
             - (ind_sk10(lambda_low) - ind_sk10(lambda_high)) * sin2; 
/* Case of a single prism: */
#else
*resid_disp = (ind_bk7(lambda_low) - ind_bk7(lambda_high)) * sin1;
#endif
*resid_disp = ATM_REFRAC(lambda_low,*tt,*ff,*pp,*zd) 
              - ATM_REFRAC(lambda_high,*tt,*ff,*pp,*zd) 
              - *resid_disp * cross_coef;

if(*italk)
  {
  printf(" Atmospheric dispersion = %f arcsec in focal plane\n",
ATM_REFRAC(lambda_low,*tt,*ff,*pp,*zd) - ATM_REFRAC(lambda_high,*tt,*ff,*pp,*zd));
  printf(" Mean deviation at central wavelength = %f arcsec in focal plane\n",
      mean_dev);
  printf(" Residual dispersion (between top and bottom) = %f arcsec in focal plane\n",
      *resid_disp);
  }

return(0);
}
/***************************************************************
* atm_refrac_owens(wave,tt,ff,pp,zd)
* Routine to compute atmospheric refraction versus wavelength
* From index formulae from Owens, J.C., Appl. Opt. v.6, p.51 (67)
*
* OUTPUT:
* atm_refrac: atmospheric dispersion (in arcseconds)
***************************************************************/
double atm_refrac_owens(double wave, double tt, double ff, double pp, double zd)
{
double atm_refrac, work, zd_rad;

   zd_rad = zd*PI/180.0;

#ifdef DEBUG
printf(" Computing atmospheric refraction with Owens'formulae\n");
printf(" pp,ff,tt,zd %f %f %f %f \n",pp,ff,tt,zd);
#endif

/* Basic equation is n1 sin(i1) = n2 sin(i2)   with n1 = 1. (void). 
   Small variations of n2 induce small variations of i2:
   0 = n2 cos(i2) di2 + dn2 sin(i2), and thus:
   di2 = - tan(i2) dn2 / n2
   Since n2 - 1 is very small:
   di2 = - dn2 * tan(i2)
*/
    
/* Reference wave is 400 nm, 
   and minus sign is used to be compatible with Simon's formula */
   work = ind_air(400.,tt,ff,pp) - ind_air(wave,tt,ff,pp);
   work = -work * tan(zd_rad); 

/* Conversion to arcseconds,*/
   atm_refrac = work * 180. * 3600. / PI;

#ifdef DEBUG
 printf(" Atm. refraction is %f arcsec (%f milli-radians) at %.2f nm (outside the telescope, with 0 at 400 nm)\n",
        atm_refrac, 1000. * atm_refrac*PI/(180 * 3600), wave);
#endif

return(atm_refrac);
}
/***************************************************************
* GET_PW_SAT 
* Routine to compute saturation water pressure 
***************************************************************/
int GET_PW_SAT(double *pw_sat, double *tt)
{
double work, work1, work2, tt1;

tt1 = (*tt + 273.16) / 273.16;

work1 = -8.29692 * (tt1 - 1.);
work2 = 4.76955 * ( 1. - 1./tt1);
work = 10.79586 * ( 1. - 1./tt1) - 5.02808 * log(tt1)
   + 1.50474E-04 * (1. - pow(10.,work1))
   + 0.42873E-03 * (pow(10.,work2) - 1.) - 2.2195983 ;

*pw_sat = 760. * pow(10.,work) ;
#ifdef DEBUG
printf(" Saturation water pressure is %f mmHg \n",*pw_sat); 
#endif

return(0);
}
