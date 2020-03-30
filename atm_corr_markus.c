/*
It performs two calculations: Owens 1967 and Allen's
Astrophysical Quantities p. 262 (chapter 11.20).

 2001-DEC-19, Markus Wildi, initial version
 Calculate the chromatic aberration of the atmosphere

 Compile and link: gcc atm_c.c -lm -oatm_c

Markus Wildi <markus.wildi@datacomm.ch>
*/

#include <stdlib.h> 
#include <math.h> 


double refractive_index( double lambda, double d_s, double d_w) ;
double allen262( double lambda, double temperature, double pressure_s, double pressure_w) ;

double d_s( double pressure, double temperature) ;
double d_w( double pressure, double temperature) ;

/* Values from the paper */
/**/
/*  With P=720mBars, Hygro=30%, T=0 degrees (C), zenith_distance=70 degrees: */
/*  Atmospheric pressure is 540.18 mmHg (720.00 mBar) */
/*  Saturation water pressure is 4.583691 mmHg  */
/*  Saturation water pressure is 4.58 mmHg (6.11 mBar) */
/*  and water partial pressure is 1.38 mmHg (1.83 mBar) */

/*  (air index - 1) is 0.00020888 at 540.00 nm */
/*  (air index - 1) is 0.00020858 at 560.00 nm */
/*  Air dispersion to be corrected: 0.166617 (arcsec in focal plane)  */
/*  (In Owens'formulae, lambda is in microns and P in millibars) */

int main () {

  double d_sv, d_wv ;

  double lambda1    = .54 ; // um
  double lambda2    = .56 ; //um

  double temperature= 273.15 ; //Kelviin
  double pressure_t = 720.0  ; //mbar
  double zenith_distance= 70. *M_PI/180. ;

  double pressure_s  ;
  double pressure_w= 1.83  ; //mbar

  double index1, index2, d_index ;

  double d_z ;
  double width ;


  printf("\nDelta Z calculation Owens 1967\n\n") ;


  /* Calculate the partial pressures */

  printf("P_w: %16.15g\n", pressure_w) ;
  pressure_s= pressure_t- pressure_w ;

  
  /* Calculate the factors D_s and D_w */

  d_sv= d_s( pressure_s, temperature) ;
  d_wv= d_w( pressure_w, temperature) ; 
  printf("D_sv: %16.15g, D_wv: %16.15g\n", d_sv, d_wv) ;
   
  /* Calculate the lower refractive index */
 
  index1= refractive_index( lambda1, d_sv, d_wv) ;
  printf( "\nIndex1: %16.15g\n", index1) ;

  /* Calculate the upper refractive index */

  index2= refractive_index( lambda2, d_sv, d_wv) ;
  printf( "Index2: %16.15g\n", index2) ;

  /* Calculate the d_z */
 
  d_index= index2-index1;
  d_z= -tan( zenith_distance) * d_index/index1 ;
 
  printf( "Delta z (arcsec): %16.15g\n", d_z * 180./M_PI *3600.) ;


  /* The same calculation with the formula of Allen's book */ 

  printf("\nDelta Z calculation Allen's Astrophysical Qunatities, p. 262\n\n") ;

  index1=  allen262( lambda1, temperature, pressure_s, pressure_w) ;
  printf( "Index1: %16.15g\n", index1) ;
  index2=  allen262( lambda2, temperature, pressure_s, pressure_w) ;
  printf( "Index2: %16.15g\n", index2) ;
  
  d_index= index2-index1 ;
  d_z= - tan( zenith_distance)/ index1 * d_index ;

  
  printf( "Delta z (arcsec): %16.15g\n", d_z * 180./M_PI *3600.) ;
}


/* Allen's formula inclusive water vapor correction */

double allen262( double lambda, double temperature, double pressure_s, double pressure_w) {

  double index ;

  double p_normal= 1013.25 ;
  double t_normal= 288.15 ;
  double lambda_nm ;
  double a=    64.328  ;
  double b= 29498.1e-6;
  double c=   255.4e-6;
  double pressure ;

  pressure= pressure_s + pressure_w ;


  lambda_nm= 1000. * lambda ;

  index= pressure * t_normal/( p_normal * temperature) * 
         ((a + b/( 146.e-6- pow( lambda_nm, -2.)) + 
	   c /( 41.e-6- pow( lambda_nm, -2.)))) ;
  
  index= index- 43.49 * 
    (  1. - 7.956e3 * pow( lambda_nm, -2.)) * pressure_w/pressure_s ;

  return index *  1.e-6+ 1.;
}


/* Calculation according to Owens 1967 */

double refractive_index( double lambda, double d_s, double d_w) {

  double index ;

  index= ( d_s *( 2371.34 + 683939.7/( 130.- pow( lambda, -2.)) 
		  + 4547.3/(38.99- pow(lambda, -2.))) +
	   d_w *( 6487.31 + 58.058 * pow( lambda, -2.) 
		  -0.7115 * pow( lambda, -4.) 
		  + 0.08851* pow( lambda, -6.))) ;

 
  return index * 1.e-8 + 1. ;

}

double d_s( double pressure, double temperature) {

  double d_s ;

  d_s= pressure/temperature* 
    ( 1. + pressure * 
      ( 57.9e-8 - 9.3250e-4/ temperature + 0.25844/pow(temperature, 2.))) ;

  return d_s ;
}

double d_w( double pressure, double temperature) {

  double d_w ;

  d_w= pressure/temperature*
       (  1. + pressure*( 1.+ 3.7e-4* pressure)*
	  (-2.37321 * 10.e-3+ 2.23366/temperature- 710.792/pow(temperature,  2.) 
	   + 7.75141 * 10.e4/pow( temperature, 3.))) ;

  return d_w ;
}
