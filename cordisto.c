/***************************************************************
* cordisto
* To correct distortion of CP40 camera
* Fits a 3rd order polynomial to x and y coordinates
*
*  x(ix,iy) = fx(ix,iy)
*   with fx(ix,iy) = ax ix**3 + bx ix**2 iy + cx ix iy**2 + ... 
*  y(ix,iy) = fy(ix,iy)
*   with fy(ix,iy) = ay ix**3 + by ix**2 iy + cy ix iy**2 + ... 
*
*  where:
*  x(ix,iy) and y(ix,iy) are the theoretical coordinates of a photon 
*  actually recorded in (ix,iy) 
*  The problem is to find the coefficients (ax, bx, cx, ...) and
*  (ay, by, cy, ...) which minimize the sums:
*   SUM on all patches (ix, iy) of ( x(ix,iy) - fx(xi,yi) ) **2
*   with psi_x = x(ix, iy) and  fx(ix,iy) = A * phi_x
*                 / ax \
*                 | bx |
*                 | cx |
*    with phi_x = | dx |
*                 | ex |
*                 | fx |
*                 \ gx /
*
*
* and then solves the normal equation:  A* A phi_x = A* psi_x
*
*  and the same for y:
*   SUM on all patches (ix, iy) of ( y(ix,iy) - fy(xi,yi) ) **2
*
* JLP 
* 07-05-96
*****************************************************************/
#include <stdio.h>
#include <math.h>
#include <jlp_ftoc.h>

/*
#define DEBUG
*/

#define NPTX  20 
#define NPTY  20
#define NPOINTS NPTY*NPTX
/* For KMAXI=1, NCOEFFI=3 */
/* For order==KMAXI = 1,    1 x y */
/* For KMAXI=2, NCOEFFI=6 */
/* For order==KMAXI = 2,    1 x y x^2 xy y^2  */
/* For KMAXI=3, NCOEFFI=10 */
/* For order==KMAXI = 3,    1 x y x^2 xy y^2 x^3 x^2y xy^2 y^3 */  
/* For KMAXI=4, NCOEFFI=15 */
/* For order==KMAXI = 4,    1 x y x^2 xy y^2 x^3 x^2y xy^2 y^3 
**                                        x^4 x^3y x^2y^2 x^3y y^4 */  
#define KMAXI 4
#define NCOEFFI 15
/*************************************************************
*
* Main program
*
**************************************************************/
/*
#define TEST_PROGRAM
*/
#ifndef TEST_PROGRAM
main(argc,argv)
int argc;
char *argv[];
{
/* (xx1,yy1) measured coordinates */
/* (xx,yy) theoretical coordinates */
float rot_angle;
double xx[NPOINTS], yy[NPOINTS], xx1[NPOINTS], yy1[NPOINTS];
/* (xx_corr,yy_corr) new values of transformed coordinates */
float xx_corr[NPOINTS], yy_corr[NPOINTS];
double aa[NCOEFFI*NPOINTS], psi[NPOINTS], phi[NCOEFFI];
int kmax, npts, ncoeff, nx, ny, status, ifail;
float wx, wy;
double wwx, wwy;
char measured_list[61], theoretical_list[61], coeff_list[61], buffer[81];
char corr_list[61];
int i, j, k;
FILE *fp_me, *fp_th, *fp1, *fp2, *fp3;

/* Interactive input of parameters: */
/* Carefull: 7 parameters always, using JLP "runs" */
if (argc == 1)
 {
  printf(" Syntax: cordisto measured_list thoretical_list number_of_coefficients rotation_angle\n\n");
  printf(" File name of measured list := ");scanf("%s",measured_list);
  printf(" File name of theoretical list := ");scanf("%s",theoretical_list);
  printf(" Number of coefficients (1 <= ncoeff <= 10) := ");scanf("%d",&ncoeff);
  printf(" Rotation angle of theoretical list (degrees) := ");scanf("%f",&rot_angle);
 }
else
 {
  strcpy(measured_list,argv[1]);
  strcpy(theoretical_list,argv[2]);
  sscanf(argv[3],"%d",&ncoeff);
  sscanf(argv[4],"%f",&rot_angle);
 }

  if(ncoeff < 1) {printf("Error: minimum number of coefficients is 1\n"); ncoeff = 1;}
  if(ncoeff > 15) {printf("Error: maximum number of coefficients is 15\n"); ncoeff = 15;}
  printf(" OK: will read %s (measurements) and %s (theoretical list). Number of coefficients: %d\n",
          measured_list, theoretical_list, ncoeff); 

/* Open logfile: */
if ((fp1 = fopen("cordisto.log","w")) == NULL)
  {
  printf("cordisto/Fatal error, cannot open logfile cordisto.log \n");
  exit(-1);
  }
fprintf(fp1," Input files: %s (measurements) and %s (theoretical list). Number of coefficients: %d\n",
          measured_list, theoretical_list, ncoeff); 

/* Open list files: */
if ((fp_me = fopen(measured_list,"r")) == NULL)
  {
  printf("cordisto/Fatal error, cannot open input list file >%s< \n",
          measured_list);
  exit(-1);
  }
if ((fp_th = fopen(theoretical_list,"r")) == NULL)
  {
  printf("cordisto/Fatal error, cannot open input list file >%s< \n",
          theoretical_list);
  exit(-1);
  }

/* Open coeff_list file: */
  i = 0;
  while(measured_list[i] && measured_list[i] != '.') 
              {coeff_list[i] = measured_list[i]; i++;}
  strcpy(&coeff_list[i],".coef");

/* Open corr_list (corrected list) file: */
  i = 0;
  while(measured_list[i] && measured_list[i] != '.') 
              {corr_list[i] = measured_list[i]; i++;}
  strcpy(&corr_list[i],".corr");
  printf("test: %s \n",coeff_list);

if ((fp2 = fopen(coeff_list,"w")) == NULL)
  {
  printf("cordisto/Fatal error, cannot open coefficient list file: %s\n",
          coeff_list);
  exit(-1);
  }

/* Fill xx and yy arrays with theoretical coordinates of spot centroids
*  Fill xx1 and yy1 arrays with measured coordinates of spot centroids
*/
/* Theoretical centroids: */
   for(k = 0; k < NPOINTS; k++)
      {
/* Read first the full line, before decoding! */
       if(!fgets(buffer,80,fp_th)) break;
       sscanf(buffer,"%f %f",&wx,&wy);
/* Problem of divergence, so I divide theoretical data by 1000: */
       xx[k] = wx/1000.;
       yy[k] = wy/1000.;
#ifdef DEBUG
       printf(" k=%d wx=%f wy=%f \n",k,wx,wy);
#endif
      }
   fclose(fp_th);
   npts = k;

/* Measured centroids (xx1,yy1): */
   for(k = 0; k < NPOINTS; k++)
      {
       if(!fgets(buffer,80,fp_me)) break;
       sscanf(buffer,"%f %f",&wx,&wy);
#ifdef DEBUG
       printf(" k=%d wx=%f wy=%f \n",k,wx,wy);
#endif
/* Problem of divergence, so I also divide measured points by 1000: */
       xx1[k] = wx/1000.;
       yy1[k] = wy/1000.;
      }
   fclose(fp_me);

   if(npts != k)
     {
     printf("Fatal error: lists do not contain the same number of points! \n");
     printf("Measured points: %d whereas theoretical points: %d \n",npts,k); 
     fprintf(fp1,"Fatal error: lists do not contain the same number of points! \n");
     fprintf(fp1,"Measured points: %d whereas theoretical points: %d \n",npts,k); 
     exit(-1);
     }
   else
     {
     printf("OK, both lists contain %d points.\n",npts);
     fprintf(fp1,"OK, both lists contain %d points.\n",npts);
     }

/* Rotation of pattern if needed: */
/* Center of rotation is first point: */
   if(rot_angle != 0.)
     {
     fprintf(fp1,
            "Rotation of %f degrees relative to first point\n",rot_angle);
     printf("Rotation of %f degrees relative to first point\n",rot_angle);
     
    rot_angle *= 3.14159/180.;
    for(i = 1; i < npts; i++)
        {
        wwx = xx[i] - xx[0];
        wwy = yy[i] - yy[0];
        xx[i] = xx[0] + wwx * cos((double)rot_angle) 
                        - wwy * sin((double)rot_angle);
        yy[i] = yy[0] + wwx * sin((double)rot_angle) 
                        + wwy * cos((double)rot_angle);
        }
     }
  
/* For KMAXI=1, NCOEFFI=3 */
/* For order==KMAXI = 1,    1 x y */
/* For KMAXI=2, NCOEFFI=6 */
/* For order==KMAXI = 2,    1 x y x^2 xy y^2  */
/* For KMAXI=3, NCOEFFI=10 */
/* For order==KMAXI = 3,    1 x y x^2 xy y^2 x^3 x^2y xy^2 y^3 */  
/*
if(kmax == 0) ncoeff = 1;
else if(kmax == 1) ncoeff = 3;
else if(kmax == 2) ncoeff = 6;
else if(kmax == 3) ncoeff = 10;
else if(kmax == 4) ncoeff = 15;
*/
kmax = 0;
if(ncoeff > 0) kmax = 1;
if(ncoeff > 3) kmax = 2;
if(ncoeff > 6) kmax = 3;
if(ncoeff > 10) kmax = 4;

/* JLP96: I set the number of coefficients, instead: */
printf(" Order of polynomial = %d, ncoeff = %d \n",kmax,ncoeff);
fprintf(fp1," Order of polynomial = %d, ncoeff = %d \n",kmax,ncoeff);

/* NB: We use same A matrix for x and y: */
compute_aa(aa,xx1,yy1,npts,ncoeff);

/*********************************************************/
/* First solve the problem in x: */
compute_psi(psi,xx,npts);
nx = ncoeff; ny = npts;
/* Initial guess (up to NCOEFFI to allow for simple display): */
for(i = 0; i < NCOEFFI; i++) phi[i] = 0.;
phi[1] = 1.;
status = JLP_CGRAD(aa,psi,phi,&nx,&ny,&ifail);
/*
ipositiv = 0;
status = JLP_FSTEP(aa,psi,phi,&nx,&ny,&ipositiv);
*/

  printf("x solution sorted as :  1 x y x2 xy y2 x3 x2y xy2 y3 \
x4 x3y x2y2 xy3 y4\n");
  fprintf(fp1,"x solution sorted as :  1 x y x2 xy y2 x3 x2y xy2 y3 \
x4 x3y x2y2 xy3 y4\n");
  for (i = 0; i < nx; i++)
      {
      if( (i % 3) == 0) {printf("\n"); fprintf(fp1,"\n");}
      printf(" phi[%d] = %10.4g ",i,phi[i]);
      fprintf(fp1," phi[%d] = %10.4g ",i,phi[i]);
      fprintf(fp2," %12.5e ",phi[i]);
      }

   printf("\n\n");
   fprintf(fp1,"\n\n");

/* Display in "c" format: */
/*
   printf(" x_computed =%10.4g +%10.4g * xx +%10.4g * yy +%10.4g * xx^2 \n",
            phi[0],phi[1],phi[2],phi[3]); 
   printf("  +%10.4g * xx * yy +%10.4g * yy^2 ",phi[4],phi[5]); 
   printf("+%10.4g * xx^3 +%10.4g * xx^2 * yy \n",phi[6],phi[7]); 
   printf("  +%10.4g * xx * yy^2 +%10.4g * yy^3 ",phi[8],phi[9]); 
   printf("+%10.4g * xx^4 +%10.4g * xx^3 * yy \n",phi[10],phi[11]); 
   printf("+%10.4g * xx^2 * yy^2 +%10.4g * xx * yy^3 +%10.4g * yy^4 \n",
                    phi[12],phi[13],phi[14]); 
   fprintf(fp1," x_computed =%10.4g +%10.4g * xx +%10.4g * yy +%10.4g * xx * xx \n",
            phi[0],phi[1],phi[2],phi[3]); 
   fprintf(fp1,"  +%10.4g * xx * yy +%10.4g * yy^2 ",phi[4],phi[5]); 
   fprintf(fp1,"+%10.4g * xx^3 +%10.4g * xx^2 * yy \n",phi[6],phi[7]); 
   fprintf(fp1,"  +%10.4g * xx * yy^2 +%10.4g * yy^3 ",phi[8],phi[9]); 
   fprintf(fp1,"+%10.4g * xx^4 +%10.4g * xx^3 * yy \n",phi[10],phi[11]); 
   fprintf(fp1,"+%10.4g * xx^2 * yy^2 +%10.4g * xx * yy^3 +%10.4g * yy^4 \n",
                    phi[12],phi[13],phi[14]); 
*/

/* Output residuals: */
    fprintf(fp1,"x_theo, y_theo, x_meas, y_meas, x_computed, x_residual \n");
/*
    if(npts > 20) nx = 20; else nx = npts;
*/
    nx = npts;
    wwx = 0.;
    for(i = 0; i < nx; i++)
        {
        calpoly_0(xx1[i],yy1[i],phi,ncoeff,kmax,&wx);
        wy = wx - xx[i];
        xx_corr[i] = wx;
        wwx += wy*wy;
        fprintf(fp1," %8.2g  %8.2g   %8.2g  %8.2g   %10.4g  %10.4g\n",
                    xx[i],yy[i],xx1[i],yy1[i],wx,wy);
        }

/*********************************************************/
/* Then solve the problem in y: */
compute_psi(psi,yy,npts);
nx = ncoeff; ny = npts;
/* Initial guess (up to NCOEFFI to allow for simple display): */
for(i = 0; i < NCOEFFI; i++) phi[i] = 0.;
phi[2] = 1.;
status = JLP_CGRAD(aa,psi,phi,&nx,&ny,&ifail);

  printf("y solution sorted as :  1 x y x2 xy y2 x3 x2y xy2 y3 \n");
  fprintf(fp1,"y solution sorted as :  1 x y x2 xy y2 x3 x2y xy2 y3 \n");
  for (i = 0; i < nx; i++)
      {
      if( (i % 3) == 0) {printf("\n"); fprintf(fp1,"\n");}
      printf(" phi[%d] = %10.4g ",i,phi[i]);
      fprintf(fp1," phi[%d] = %10.4g ",i,phi[i]);
      fprintf(fp2," %12.5e ",phi[i]);
      }

   printf("\n\n");
   fprintf(fp1,"\n\n");

/* Display in "c" format: */
/*
   printf(" y_computed =%10.4g +%10.4g * xx +%10.4g * yy +%10.4g * xx * xx \n",
            phi[0],phi[1],phi[2],phi[3]); 
   printf("  +%10.4g * xx * yy +%10.4g * yy^2 ",phi[4],phi[5]); 
   printf("+%10.4g * xx^3 +%10.4g * xx^2 * yy \n",phi[6],phi[7]); 
   printf("  +%10.4g * xx * yy^2 +%10.4g * yy^3 ",phi[8],phi[9]); 
   printf("+%10.4g * xx^4 +%10.4g * xx^3 * yy \n",phi[10],phi[11]); 
   printf("+%10.4g * xx^2 * yy^2 +%10.4g * xx * yy^3 +%10.4g * yy^4 \n",
                    phi[12],phi[13],phi[14]); 
   fprintf(fp1," y_computed =%10.4g +%10.4g * xx +%10.4g * yy +%10.4g * xx * xx \n",
            phi[0],phi[1],phi[2],phi[3]); 
   fprintf(fp1,"  +%10.4g * xx * yy +%10.4g * yy^2 ",phi[4],phi[5]); 
   fprintf(fp1,"+%10.4g * xx^3 +%10.4g * xx^2 * yy \n",phi[6],phi[7]); 
   fprintf(fp1,"  +%10.4g * xx * yy^2 +%10.4g * yy^3 ",phi[8],phi[9]); 
   fprintf(fp1,"+%10.4g * xx^4 +%10.4g * xx^3 * yy \n",phi[10],phi[11]); 
   fprintf(fp1,"+%10.4g * xx^2 * yy^2 +%10.4g * xx * yy^3 +%10.4g * yy^4 \n",
                    phi[12],phi[13],phi[14]); 
*/

/* Output residuals: */
    fprintf(fp1," x_theo, y_theo, x_meas, y_meas, y_computed, y_residual \n");
/*
    if(npts > 20) nx = 20; else nx = npts;
*/
    nx = npts;
    wwy = 0.;
    for(i = 0; i < nx; i++)
        {
        calpoly_0(xx1[i],yy1[i],phi,ncoeff,kmax,&wx);
        wy = wx - yy[i];
        yy_corr[i] = wx;
        wwy += wy*wy;
        fprintf(fp1," %8.2g  %8.2g   %8.2g  %8.2g   %10.4g  %10.4g\n",
                    xx[i],yy[i],xx1[i],yy1[i],wx,wy);
        }
     printf(" Mean rms error in x: %10.4g, in y: %10.4g\n",
               sqrt((double)wwx)/(double)nx,sqrt((double)wwy)/(double)nx);
     fprintf(fp1," Mean rms error in x: %10.4g, in y: %10.4g\n",
               sqrt((double)wwx)/(double)nx,sqrt((double)wwy)/(double)nx);

/* Open debug/check file: */
if ((fp3 = fopen(corr_list,"w")) == NULL)
  {
  printf("cordisto/Fatal error, cannot open corrected list: %s\n",corr_list);
  exit(-1);
  }
    for(i = 0; i < nx; i++)
        fprintf(fp3," %10.4e  %10.4e\n",xx_corr[i],yy_corr[i]);
fclose(fp3);


/* End of program: */
fclose(fp1);
fclose(fp2);
printf(" Output in cordisto.log, %s and %s \n",coeff_list,corr_list);
exit(0);
}
/************************************************************
*
* Test program
* aaaa
* (define TEST_PROGRAM to activate this part)
*************************************************************/
#else
main()
{
/* (xx1,yy1) measured coordinates */
/* (xx,yy) theoretical coordinates */
double xx[NPOINTS], yy[NPOINTS], xx1[NPOINTS], yy1[NPOINTS];
double aa[NCOEFFI*NPOINTS], psi[NPOINTS], phi[NCOEFFI];
double stepx, stepy;
int kmax, npts, ncoeff, nx, ny, status, ifail;
int i, j, k;

/**aaa**/
/* Fill xx1 and yy1 arrays with measured coordinates of spot centroids
* (Here assumes regular array, but later will take measured values)
*/
/* Fill xx and yy arrays with theoretical coordinates of spot centroids
*/
k = 0;
for(j = 0; j < NPTY; j++)
  {
  for(i = 0; i < NPTX; i++)
     {
/* Theoretical centroids : */
       xx[k] = i;
       yy[k] = j;

/* Measured centroids, simulation just to see: */
       xx1[k] = 0.1 + i*1.1 + j*0.01 + 0.002 *i*j*j + j*j*j*0.001; 
       yy1[k] = 0.2 + i*0.01 + j + j*j*0.01; 

/* Increment k index: */
       k++;
     }
  }
printf(" Try with xmeas = 0.1 + 1.1 x + 0.01 y + 0.002 xy2 + 0.001 y3 \n");
printf(" and with ymeas = 0.2 + 0.01 x + y + 0.01 y2 \n");
  
/* Compute sums which will be used to compute the matrix elements (matrix A)
*/
kmax = KMAXI; npts = NPTX * NPTY; 
ncoeff = NCOEFFI;
kmax = 3; ncoeff = 10;

/* NB: We use same A matrix for x and y: */
compute_aa(aa,xx,yy,npts,ncoeff);

/*********************************************************/
/* First solve the problem in x: */
compute_psi(psi,xx1,npts);

nx = ncoeff; ny = npts;
status = JLP_CGRAD(aa,psi,phi,&nx,&ny,&ifail);
/*
ipositiv = 0;
printf(" with fixed step: \n");
status = JLP_FSTEP(aa,psi,phi,&nx,&ny,&ipositiv);
*/

  printf("sorted as :  1 x y x2 xy y2 x3 x2y xy2 y3 \n");
  for (i = 0; i < nx; i++)
      printf(" x solution: phi[%d] = %10.4g \n",i,phi[i]);

/*********************************************************/
/* Then solve the problem in y: */
compute_psi(psi,yy1,npts);
nx = ncoeff; ny = npts;
/*
ifail = 0;
printf(" with fixed step: \n");
status = JLP_FSTEP(aa,psi,phi,&nx,&ny,&ifail);
*/
status = JLP_CGRAD(aa,psi,phi,&nx,&ny,&ifail);

  printf("sorted as :  1 x y x2 xy y2 x3 x2y xy2 y3 \n");
  for (i = 0; i < nx; i++)
      printf(" y solution: phi[%d] = %10.4g \n",i,phi[i]);

}
/* End of TEST_PROGRAM */
#endif
/*********************************************************************
* To compute the second member of normal equations
*
* Is called twice: once with zz=xx1 and then with zz=yy1 
*********************************************************************/
int compute_psi(psi,zz,npts)
double *psi, *zz; 
int npts;
{
int j;


for(j = 0; j < npts; j++) psi[j] = zz[j];

return(0);
}
/*********************************************************************
* To compute the elements of matrix aa
* Sorted starting with lowest order, with x order changing more
* quickly than y's.
*********************************************************************/
int compute_aa(aa,xx,yy,npts,ncoeff)
double *aa, *xx, *yy;
int ncoeff, npts;
{
double v[10];
int i, j;

/* Loop on the lines */
   for(j = 0; j < npts; j++)
   {
/*
     aa[0 + j * ncoeff] = 1;
     aa[1 + j * ncoeff] = xx[j];
     aa[2 + j * ncoeff] = yy[j];
     aa[3 + j * ncoeff] = xx[j]*xx[j];
     aa[4 + j * ncoeff] = xx[j]*yy[j];
     aa[5 + j * ncoeff] = yy[j]*yy[j];
     aa[6 + j * ncoeff] = xx[j]*xx[j]*xx[j];
     aa[7 + j * ncoeff] = xx[j]*xx[j]*yy[j];
     aa[8 + j * ncoeff] = xx[j]*yy[j]*yy[j];
     aa[9 + j * ncoeff] = yy[j]*yy[j]*yy[j];
*/
     v[0] = 1.;
     v[1] = xx[j];
     v[2] = yy[j];
     v[3] = xx[j] * xx[j];
     v[4] = xx[j] * yy[j];
     v[5] = yy[j] * yy[j];
     v[6] = xx[j] * xx[j] * xx[j];
     v[7] = xx[j] * xx[j] * yy[j];
     v[8] = xx[j] * yy[j] * yy[j];
     v[9] = yy[j] * yy[j] * yy[j];
     for(i = 0; i < ncoeff; i++) aa[i + j * ncoeff] = v[i];
    }

return(0);
}
/*****************************************************************
*
* Compute the value given by the solution polynomial
* (to derive the residuals)
* sorted as :  1 x y x2 xy y2 x3 x2y xy2 y3
*****************************************************************/
int calpoly_0(xx,yy,phi,ncoeff,kmax,value)
double xx, yy, *phi;
int *ncoeff, kmax;
float *value;
{
int i, j, k, m;
double sum, x[KMAXI+1], y[KMAXI+1];
double work;

x[0] = 1.;
y[0] = 1.;
for(i = 1; i <= kmax; i++) x[i] = x[i-1] * xx;
for(i = 1; i <= kmax; i++) y[i] = y[i-1] * yy;

sum = 0.;
/* sorted as :  1 x y x2 xy y2 x3 x2y xy2 y3
*/
k = 0;
for(m = 0; m <= kmax && k < ncoeff; m++)
  {
    for(j = 0; j <= m && k < ncoeff; j++)
         {
/* Filling line #n1 */
         i = m - j;
         sum += x[i] * y[j] * phi[k];
         k++;
         }
    }

*value = sum;
}
