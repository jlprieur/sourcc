/***********************************************************
* Program risl2 
* To compute chromatic aberration of the speckle camera  
* and position of Risley prisms 
*
* Version 21-04-94
* JLP
***********************************************************/
#include <stdio.h>
#include <math.h>
#include <jlp_ftoc.h>

#define DEBUG

#define PI 3.14159265
#define DEGTORAD   (PI/180.00)

main()
{
float x[4], y[4], alpha, beta, alpha0, beta0, ra_max_disp, rb_max_disp;
float ra_x, ra_y, rb_x, rb_y, disp_x, disp_y, disp_angle, disp_mod, w[4];
float chr_abx, chr_aby, chr_abx2, chr_aby2;
int i;
int ra_code[4], rb_code[4], ra_cod, rb_cod, status;
char answer[2], logfile[61];
FILE *fp;

printf("Program risl2 to compute chromatic aberration of the speckle camera\n");
printf(" and position of Risley prisms \n");
printf(" JLP - Version 21-04-94 \n");

strcpy(logfile,"risl1.log");
if ((fp = fopen(logfile,"w")) == NULL)
    {printf("Fatal error opening logfile %s \n",logfile);
     exit(-1);
    }  

fprintf(fp," Program risl2  - Version 21-04-94 - \n");
 
printf(" Enter starting RA, RB positions (between 0 and 1023) :");
scanf("%d,%d",ra_code,rb_code);
fprintf(fp," Starting position : RA = %d RB = %d \n",
                ra_code[0],rb_code[0]);

#ifdef DEBUG
printf(" OK, starting position : RA = %d RB = %d \n",
                ra_code[0],rb_code[0]);
#endif

/************************************************* 
* Compute positions to be measured:
* Position 1: (RA=alpha, RB=beta) 
* Position 2: (RA=alpha+PI, RB=beta+PI) 
* Position 3: (RA=alpha+PI, RB=beta) 
* Position 4: (RA=alpha, RB=beta+PI) 
**************************************************/
ra_code[1] = ra_code[0] + 512;
ra_code[2] = ra_code[0] + 512;
ra_code[3] = ra_code[0];
rb_code[1] = rb_code[0] + 512;
rb_code[2] = rb_code[0];
rb_code[3] = rb_code[0] + 512;

for(i = 0; i < 4; i++) if(ra_code[i] > 1023) ra_code[i] -= 1024;
for(i = 0; i < 4; i++) if(rb_code[i] > 1023) rb_code[i] -= 1024;

printf(" Procedure: measure the vector coordinates (x,y) of the shift\n");
printf(" of one detail in the image at two wavelengths at positions:\n");
printf("    1) RA = alpha          RB = beta      (Here: RA = %d RB = %d)\n",
                ra_code[0],rb_code[0]);
printf("    2) RA = alpha + PI     RB = beta + PI (Here: RA = %d RB = %d)\n",
                ra_code[1],rb_code[1]);
printf("    3) RA = alpha + PI     RB = beta      (Here: RA = %d RB = %d)\n",
                ra_code[2],rb_code[2]);
printf("    4) RA = alpha          RB = beta + PI (Here: RA = %d RB = %d)\n",
                ra_code[3],rb_code[3]);
printf(" Enter x1,y1,x2,y2,x3,y3,x4,y4 (coordinates of the spot for the 4 positions of the prisms): \n");
scanf("%f,%f,%f,%f,%f,%f,%f,%f",
              &x[0],&y[0],&x[1],&y[1],&x[2],&y[2],&x[3],&y[3]);
fprintf(fp," x1=%.1f, y1=%.1f, x2=%.1f, y2=%.1f, x3=%.1f, y3=%.1f, x4=%.1f, y4=%.1f\n",
              x[0],y[0],x[1],y[1],x[2],y[2],x[3],y[3]);

/* Chromatic aberration of the speckle camera:
*/
 chromatic_aberr(x,y,&chr_abx,&chr_aby,&chr_abx2,&chr_aby2);

/* Output of results: */
   printf(" Chromatic aberration of the camera: x = %.1f +/-%.1f, y = %.1f +/-%.1f\n",
           chr_abx, chr_abx2, chr_aby, chr_aby2); 
   fprintf(fp," Chromatic aberration of the camera: x = %.1f +/-%.1f, y = %.1f +/-%.1f\n",
           chr_abx, chr_abx2, chr_aby, chr_aby2); 

/**********************************************************
* Coordinates of dispersion vector at (RA=alpha, RB=beta)
***********************************************************/

/*********************************************************
* Principle:
* Position 1: (RA=alpha, RB=beta) 
*              M cos(alpha) + M cos(beta) + chr_abx = x[0]
*              M sin(alpha) + M sin(beta) + chr_aby = y[0]
* Position 2: (RA=alpha+PI, RB=beta+PI) 
*              -M cos(alpha) - M cos(beta) + chr_abx = x[1]
*              -M sin(alpha) - M sin(beta) + chr_aby = y[1]
* Position 3: (RA=alpha+PI, RB=beta) 
*              -M cos(alpha) + M cos(beta) + chr_abx = x[2]
*              -M sin(alpha) + M sin(beta) + chr_aby = y[2]
* Position 4: (RA=alpha, RB=beta+PI) 
*              M cos(alpha) - M cos(beta) + chr_abx = x[3]
*              M sin(alpha) - M sin(beta) + chr_aby = y[3]
*
* Combination (1,3):
*              M cos(alpha) = (x[0] - x[2]) / 2.
*              M sin(alpha) = (y[0] - y[2]) / 2.
* Combination (1,4):
*              M cos(beta) = (x[0] - x[3]) / 2.
*              M sin(beta) = (y[0] - y[3]) / 2.
* Combination (2,3):
*              M cos(beta) = (x[2] - x[1]) / 2.
*              M sin(beta) = (y[2] - y[1]) / 2.
* Combination (2,4):
*              M cos(alpha) = (x[3] - x[1]) / 2.
*              M sin(alpha) = (y[3] - y[1]) / 2.
*
* RA at alpha: (ra_x,ra_y)
* RB at beta:  (rb_x,rb_y)
**************************************************************/

/* Mean value of the two estimations: */
#ifdef DEBUG
  printf(" ra_x: %f %f \n",(x[0] - x[2]) / 2.,(x[3] - x[1]) / 2.);
  printf(" ra_y: %f %f \n",(y[0] - y[2]) / 2.,(y[3] - y[1]) / 2.);
  printf(" rb_x: %f %f \n",(x[0] - x[3]) / 2.,(x[2] - x[1]) / 2.);
  printf(" rb_y: %f %f \n",(y[0] - y[3]) / 2.,(y[2] - y[1]) / 2.);
#endif

      ra_x = (x[0] - x[2]) / 2.; 
      ra_y = (y[0] - y[2]) / 2.; 
      rb_x = (x[0] - x[3]) / 2.; 
      rb_y = (y[0] - y[3]) / 2.;
w[0] = ra_x;
w[1] = ra_y;
w[2] = rb_x;
w[3] = rb_y;
      rb_x += (x[2] - x[1]) / 2.;
      rb_y += (y[2] - y[1]) / 2.;
      ra_x += (x[3] - x[1]) / 2.;
      ra_y += (y[3] - y[1]) / 2.;
      ra_x /= 2.; ra_y /= 2.; rb_x /= 2.; rb_y /= 2.;
w[0] -= ra_x;
w[1] -= ra_y;
w[2] -= rb_x;
w[3] -= rb_y;
for(i = 0; i < 4; i++) if(w[i] < 0.) w[i] = -w[i];

  printf(" Dispersion of RA (for RA=%d) : x=%.1f +/-%.1f, y=%.1f +/-%.1f\n",
             ra_code[0],ra_x,w[0],ra_y,w[1]);
  printf(" Dispersion of RB (for RB=%d) : x=%.1f +/-%.1f, y=%.1f +/-%.1f\n",
             rb_code[0],rb_x,w[2],rb_y,w[3]);
  fprintf(fp," Dispersion of RA (for RA=%d) : x=%.1f +/-%.1f, y=%.1f +/-%.1f\n",
             ra_code[0],ra_x,w[0],ra_y,w[1]);
  fprintf(fp," Dispersion of RB (for RB=%d) : x=%.1f +/-%.1f, y=%.1f +/-%.1f\n",
             rb_code[0],rb_x,w[2],rb_y,w[3]);

/* Conversion to polar coordinates: */
 to_polar(ra_x,ra_y,&alpha,&ra_max_disp);
 to_polar(rb_x,rb_y,&beta,&rb_max_disp);

 printf(" For RA = %d alpha = %.1f (deg), for RB = %d beta = %.1f (deg) \n",
            ra_code[0],alpha/DEGTORAD,rb_code[0],beta/DEGTORAD);
 printf(" Maximum dispersions of RA and RB : %.1f  %.1f (pixels) \n",
           ra_max_disp,rb_max_disp);
 fprintf(fp," For RA = %d alpha = %.1f (deg), for RB = %d beta = %.1f (deg) \n",
            ra_code[0],alpha/DEGTORAD,rb_code[0],beta/DEGTORAD);
 fprintf(fp," Maximum dispersions of RA and RB : %.1f  %.1f (pixels) \n",
           ra_max_disp,rb_max_disp);
 
 alpha0 = alpha - ra_code[0]*PI/512.;
 beta0 = beta - rb_code[0]*PI/512.;

/* Computing dispersion at origin; */
 printf(" For RA=0 RB=0, prism dispersion angles: alpha0 = %.1f beta0 = %.1f (deg)\n",
            alpha0/DEGTORAD,beta0/DEGTORAD);
 fprintf(fp," For RA=0 RB=0, prism dispersion angles: alpha0 = %.1f beta0 = %.1f (deg)\n",
            alpha0/DEGTORAD,beta0/DEGTORAD);

/* Now trying a prediction: */
 for(i = 0; i < 3; i++)
   {
   printf(" Enter the global dispersion you need: x,y (pixels) := "); 
   scanf("%f,%f",&disp_x,&disp_y);
   to_polar(disp_x,disp_y,&disp_angle,&disp_mod);
   status = compute_risley_code(disp_angle,disp_mod,ra_max_disp,rb_max_disp,
                     alpha0,beta0,chr_abx,chr_aby,&ra_cod,&rb_cod);
   if(!status)
    {
    printf("   => Risley prisms positions: RA = %d   RB = %d\n",ra_cod,rb_cod);
    fprintf(fp," Global dispersion needed: x=%.1f, y=%.1f (pixels)\n",
            disp_x,disp_y); 
    fprintf(fp,"   => Risley prisms positions: RA = %d   RB = %d\n",ra_cod,rb_cod);
    }
   }

 printf(" Logfile in %s\n",logfile);
 fclose(fp);
 exit(0);
} 
/************************************************** 
* Conversion to polar coordinates: 
****************************************************/
int to_polar(x,y,angle,modulus)
float x, y, *angle, *modulus;
{
 *modulus = x * x + y * y;
 *modulus = sqrt((double)(*modulus));
 if(*modulus != 0.)
   {
   *angle = acos((double)(x / (*modulus)));
   if(y < 0) *angle = 2. * PI - *angle;
   }
 else
   *angle = 0.;

return(0);
}
/************************************************** 
* Compute risley code positions
* Assume same maximum dispersion for both prisms
*
* INPUT:
* disp_angle,disp_mod: global dispersion to obtain
* ra_max_disp,rb_max_disp: maximum dispersion for Risley prisms RA and RB 
* alpha0,beta0: dispersion angles for RA=0, RB=0 
* chr_abx, chr_aby: chromatic aberration of the speckle camera
*
* OUTPUT:
* ra_cod, rb_cod
****************************************************/
int compute_risley_code(disp_angle,disp_mod,ra_max_disp,rb_max_disp,
                     alpha0,beta0,chr_abx,chr_aby,ra_cod,rb_cod) 
float disp_angle, disp_mod, ra_max_disp, rb_max_disp, alpha0, beta0;
float chr_abx, chr_aby;
int *ra_cod, *rb_cod;
{
float disp1_x, disp1_y, risley_mod, disp1_angle, disp1_mod;
float w1, half_cross_angle, alpha, beta;

/* Global dispersion is:
  disp_x = ra_max_disp * cos(alpha) + rb_max_disp * sin(beta) + chr_abx
  disp_y = ra_max_disp * cos(alpha) + rb_max_disp * sin(beta) + chr_abx
*/
/* First compute: (disp_x - chr_abx, disp_y - chr_aby)
*/
disp1_x = disp_mod * cos((double)(DEGTORAD * disp_angle)) - chr_abx;
disp1_y = disp_mod * sin((double)(DEGTORAD * disp_angle)) - chr_aby;
to_polar(disp1_x,disp1_y,&disp1_angle,&disp1_mod);

/* Assume same maximum dispersion for both prisms */
risley_mod = (ra_max_disp + rb_max_disp) / 2.;

/* Compute Risley prisms half cross angle: */
w1 = disp1_mod / (2. * risley_mod);
if(w1 > 1. || w1 < -1.) 
   {
   printf("compute_risley_code/Error: dispersion to be corrected is too big");
   printf(" w1 = %f \n",w1);
   return(-1);
   }
half_cross_angle = acos((double)w1); 

alpha = disp1_angle - half_cross_angle + alpha0;
beta = disp1_angle + half_cross_angle + beta0;
#ifdef DEBUG
 printf(" half cross angle = %.1f",half_cross_angle/DEGTORAD);
 printf(" alpha = %.1f, beta = %.1f (deg)\n",alpha/DEGTORAD,beta/DEGTORAD);
#endif

/* Code positions: */
*ra_cod = (int)(0.5 + alpha * 512. / PI);
if(*ra_cod < 0) *ra_cod +=1024;
if(*ra_cod > 1023) *ra_cod -=1023;
*rb_cod = (int)(0.5 + beta * 512. / PI);
if(*rb_cod < 0) *rb_cod +=1024;
if(*rb_cod > 1023) *rb_cod -=1023;

return(0);
}
/**********************************************************
* Chromatic aberration of the speckle camera:
*
* Principle:
* Position 1: (RA=alpha, RB=beta)
*              M cos(alpha) + M cos(beta) + chr_abx = x[0]
*              M sin(alpha) + M sin(beta) + chr_aby = y[0]
* Position 2: (RA=alpha+PI, RB=beta+PI)
*              -M cos(alpha) - M cos(beta) + chr_abx = x[1]
*              -M sin(alpha) - M sin(beta) + chr_aby = y[1]
* Position 3: (RA=alpha+PI, RB=beta)
*              -M cos(alpha) + M cos(beta) + chr_abx = x[2]
*              -M sin(alpha) + M sin(beta) + chr_aby = y[2]
* Position 4: (RA=alpha, RB=beta+PI)
*              M cos(alpha) - M cos(beta) + chr_abx = x[3]
*              M sin(alpha) - M sin(beta) + chr_aby = y[3]
*
* Combination (1,2):
*              chr_abx = (x[0] + x[1]) / 2.
*              chr_aby = (y[0] + y[1]) / 2.
* Combination (2,3):
*              chr_abx = (x[2] + x[3]) / 2.
*              chr_aby = (y[2] + y[3]) / 2.
*
* INPUT:
*  x[], y[]
*
* OUTPUT:
*  chr_abx_out, chr_abx_out
***********************************************************/
int chromatic_aberr(x,y,chr_abx_out,chr_aby_out,
                    chr_abx2_out,chr_aby2_out)
float x[], y[], *chr_abx_out, *chr_aby_out, *chr_abx2_out, *chr_aby2_out;
{
float chr_abx, chr_aby, chr_abx2, chr_aby2; 
float chr_ab_x[2], chr_ab_y[2];
int i;

chr_abx = 0.; chr_aby = 0.;
chr_abx2 = 0.; chr_aby2 = 0.;
for(i = 0; i < 2; i++)
   {
    chr_ab_x[i] = (x[2*i] + x[2*i+1] ) / 2.;
    chr_abx += chr_ab_x[i];
    chr_abx2 += chr_ab_x[i] * chr_ab_x[i];
    chr_ab_y[i] = (y[2*i] + y[2*i+1] ) / 2.;
    chr_aby += chr_ab_y[i];
    chr_aby2 += chr_ab_y[i] * chr_ab_y[i];
#ifdef DEBUG
    printf(" chr_ab_x[%d]=%.1f chr_ab_y[%d]=%.1f \n",
           i,chr_ab_x[i],i,chr_ab_y[i]);
#endif
   }

/* Mean: */
   chr_abx /= 2.;
   chr_aby /= 2.;

/* Standard deviation: */
   chr_abx2 = chr_abx2 / 2. - chr_abx * chr_abx ;
   chr_abx2 = sqrt((double)chr_abx2);
   chr_aby2 = chr_aby2 / 2. - chr_aby * chr_aby ;
   chr_aby2 = sqrt((double)chr_aby2);

  *chr_abx_out = chr_abx;
  *chr_aby_out = chr_aby;
  *chr_abx2_out = chr_abx2;
  *chr_aby2_out = chr_aby2;

return(0);
}
