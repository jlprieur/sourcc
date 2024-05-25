/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
* wavefront1.c 
* To analyse wavefront in Perez' experiment (gradient, laplacian...)
*
* JLP
* Version 21-04-93
---------------------------------------------------------------------*/
#include <stdio.h>
#include <math.h>
#include <jlp_ftoc.h>

#define DEBUG 1
/*
#define APODI hamming 
#define APODI_NAME "Hamming" 
*/
#define APODI blackman 
#define APODI_NAME "Blackman" 

main(argc, argv)
int argc;
char *argv[];
{
float *in1, *in2, *grad, *lapla, *delta_in_r, *delta_in_i;
float *inten_r, *inten_i, *filter;
float delta_z, pix_period, freq_width, work;
long int  nx, ny, pntr, isize, nxy, istatus, nx1, ny1, kod;
int i, j;
char image1[61], image2[61], comments[81], buffer[81];
char direction[2], gradient[61], laplacian[61], outcomments[81];

  printf(" Program wavefront1  Version 21-04-93\n");

/* Input parameters:  should be 6 */
if (argc != 6)
  {
  printf("argc = %d \n",argc);
  printf("\nUSAGE:\n");
  printf("wavefront1 image1 image2 delta_z,pix_period,freq_width,direction");
  printf(" gradient laplacian\n");
  printf("\nExample\n $EXEC/wavefront.exe p0 p20 20,16,8,X grad lapla \n");
  printf("or: $EXEC/wavefront.exe p0 p10 10,14,5,Y grad lapla \n");
  exit(-1);
  }

/* File names: */
  strcpy(image1,argv[1]);
  strcpy(image2,argv[2]);
  sscanf(argv[3],"%f,%f,%f,%c",&delta_z,&pix_period,&freq_width,direction);
  strcpy(gradient,argv[4]);
  strcpy(laplacian,argv[5]);

/* Check validity of parameters: */
  if(delta_z == 0)
    {printf(" Fatal error: delta_z = 0 !\n"); exit(-1);}
  if(pix_period < 2)
    {printf(" Fatal error: pix_period is too small (2 mini)!\n"); exit(-1);}

/*****************************************************************/
   JLP_INQUIFMT();

/* Reading the input files */
  JLP_VM_READIMAG(&pntr,&nx,&ny,image2,comments,&istatus);
  JLP_FROM_MADRID(&pntr,&in1);
  JLP_VM_READIMAG(&pntr,&nx1,&ny1,image1,comments,&istatus);
  JLP_FROM_MADRID(&pntr,&in2);

/* Check size: */
  if(nx != nx1 || ny != ny1)
     {
     printf(" Error: incompatible image size (modsq and long_int! \n");
     istatus = 1;
     }

/* Allocation of memory: */
 isize = nx * ny * sizeof(float);
 JLP_GVM(&grad,&isize);
 JLP_GVM(&lapla,&isize);
 JLP_GVM(&delta_in_r,&isize);
 JLP_GVM(&delta_in_i,&isize);
 JLP_GVM(&inten_r,&isize);
 JLP_GVM(&inten_i,&isize);
 JLP_GVM(&filter,&isize);

/* Erasing the arrays : */
    nxy = nx * ny;
    for(i = 0; i < nxy; i++) 
      {
      grad[i]=0.;
      lapla[i]=0.;
      inten_i[i]=0.;
      delta_in_i[i]=0.;
      }

/* Subtraction of the two images and normalization with delta_z:
   delta_in_r = (in2 - in1) / delta_z */
   for(i = 0; i < nxy;i++) delta_in_r[i] = (in2[i] - in1[i]) / delta_z; 

/* Set the intensity to the mean of the two input frames: */
   for(i = 0; i < nxy;i++) inten_r[i] = (in1[i] + in2[i]) / 2.; 
   
/* Fourrier Transform: */
   kod=1;
   FFT_2D(delta_in_r,delta_in_i,&nx,&ny,&nx,&kod);
   FFT_2D(inten_r,inten_i,&nx,&ny,&nx,&kod);

/* Recentre the frames: */
   RECENTRE(delta_in_r,delta_in_r,&nx,&ny,&nx);
   RECENTRE(delta_in_i,delta_in_i,&nx,&ny,&nx);
   RECENTRE(inten_r,inten_r,&nx,&ny,&nx);
   RECENTRE(inten_i,inten_i,&nx,&ny,&nx);

/* Creation of the good frequency: generation of the filter: */
   create_filter(filter,nx,ny,nx,pix_period,freq_width,direction),

#ifdef DEBUG
   strcpy(buffer,"wavefront1_filter");
   sprintf(outcomments,"%s filter, pix_period= %f freq_width=%f",
           APODI_NAME,pix_period,freq_width);
   JLP_WRITEIMAG(filter,&nx,&ny,&nx,buffer,outcomments);
#endif

/* Multiplication with filter in Fourier domain 
   (selection of the good frequency) */
   for(i = 0; i < nxy;i++) 
      {
       delta_in_r[i] = delta_in_r[i] * filter[i]; 
       delta_in_i[i] = delta_in_i[i] * filter[i]; 
       inten_r[i] = inten_r[i] * filter[i]; 
       inten_i[i] = inten_i[i] * filter[i]; 
      }

/* Inverse Fourrier Transform: */
   kod=-1;
   FFT_2D(inten_r,inten_i,&nx,&ny,&nx,&kod);
   FFT_2D(delta_in_r,delta_in_i,&nx,&ny,&nx,&kod);

/* Compute the complex division (a1 + i b1) / (a2 + i b2)
  ((a1 + i b1) * (a2 - i b2)) / ((a2 + i b2) * (a2 - i b2))
   ((a1 a2 + b1 b2) + i (b1 a2 - a1 b2) ) / (a2 a2 + b2 b2)

 grad is the imaginary part
 lapla is the real part of this division
*/
   for(i = 0; i < nxy;i++) 
      {
       work = inten_r[i] * inten_r[i] + inten_i[i] * inten_i[i];
       grad[i] = delta_in_i[i] * inten_r[i] - delta_in_r[i] * inten_i[i]; 
       lapla[i] = delta_in_r[i] * inten_r[i] + delta_in_i[i] * inten_i[i]; 
       if(work != 0.)
         {
         grad[i] = grad[i] / work;
         lapla[i] = lapla[i] / work;
         }
      else
         {
         grad[i] = 0.;
         lapla[i] = 0.;
         }
      }

/* Now output of the results : */
   sprintf(outcomments,"%s %s delta_z= fd, pix_period= %f freq_width=%f",
           image1,image2,delta_z,pix_period,freq_width);
   JLP_WRITEIMAG(grad,&nx,&ny,&nx,gradient,outcomments);
   JLP_WRITEIMAG(lapla,&nx,&ny,&nx,laplacian,outcomments);

/* End : */
  JLP_END();
}
/******************************************************
* Creates filter to select frequency in Fourier domain
*
* According to value of APODI, selects either Hamming or Blackman filter
*
* X: X direction
* Y: Y direction
* B: both X and Y
********************************************************/
int create_filter(filter,nx,ny,idim,pix_period,freq_width,direction)
float filter[], pix_period, freq_width;
int nx, ny, idim;
char *direction;
{
double argx, argy;
double PI = 3.14159;
float work;
int icent, jcent; 
int i, j;
float APODI();

/******************************************************
* Relation between direct and Fourier pixels 
*
* Step in x:   dx = 1/(2 * u_max) 
* (if frequency domain between -u_max and +u_max) 
* Step in u:   du = 1/D 
* (if domain width in direct space is D) 
* Then u_max corresponds to the N/2th pixel, i.e. N/(2*D)
* and dx = D/N
*
* Numerical application:
* For pixel period of 4 pixels and frames of 128x128 pixels,
* we have 4*dx or 4/(2 * u_max) in direct space, and u_max/2 in Fourier space.
* As u_max corresponds to the 128/2 = 64th pixel relative to the zero frequency
* we have 64/2=32 pixels in Fourier domain (or 128/4)
******************************************************/
  icent = nx / pix_period;
  jcent = ny / pix_period;
#ifdef DEBUG
  printf(" icent=%d, jcent=%d, pix_period=%f, freq_width=%f \n",
           icent, jcent,pix_period,freq_width);
#endif

  switch (*direction)
  {
   case 'X':
   case 'x':
     for(j = 0; j < ny; j++)
     {
     for(i = 0; i < nx; i++)
        {
         argx = (float)(i - icent - nx/2) / freq_width; 
         if(argx < -1. || argx > 1.)
             filter[i + j * idim] = 0.;
         else
          {
             filter[i + j * idim] = APODI(PI * argx);

#ifdef DEBUG
           if(j == ny/2) printf(" i=%d, j=%d, argx=%f, filter=%f \n",
                   i,j,argx,filter[i+j*idim]);
#endif
           }
        }
     }
     break;
   case 'Y':
   case 'y':
     for(j = 0; j < ny; j++)
     {
     argy = (float)(j - jcent - ny/2) / freq_width; 
       if(argy < -1. || argy > 1.)
           work = 0.;
       else
           work = APODI(PI * argy);
     for(i = 0; i < nx; i++)
        {
         filter[i + j * idim] = work;
        }
     }
     break;
   case 'B':
   case 'b':
     for(j = 0; j < ny; j++)
     {
     argy = (float)(j - jcent - ny/2) / freq_width; 
      if(argy < -1. || argy > 1.)
        work = 0.;
      else
        work = APODI(PI * argy);
     for(i = 0; i < nx; i++)
        {
         argx = (float)(i - icent - nx/2) / freq_width; 
          if(argx < -1. || argx > 1.)
            filter[i + j * idim] = 0.;
          else
            filter[i + j * idim] = work * APODI(PI * argx);
        }
     }
     break;
   default:
     printf(" create_filter/Fatal error, wrong option\n");
     exit(-1);
  }
return(0);
}
/******************************************
* Hamming Filter
******************************************/
float hamming(argx)
double argx;
{
return(0.54 + 0.46 * cos(argx));
}
/******************************************
* Blackman Filter
******************************************/
float blackman(argx)
double argx;
{
return(0.42 + 0.5 * cos(argx) + 0.08 * cos( 2 * argx));
}
