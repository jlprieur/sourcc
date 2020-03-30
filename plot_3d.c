/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 plot_3d.c

USAGE:
      plot_3d image_name [plotdev]

 JLP
 Version 03-09-98
---------------------------------------------------------------------*/
#include <stdio.h>
#include <math.h>
#include <jlp_ftoc.h>

#define DEBUG

int main(argc, argv)
int argc;
char **argv;
{
float *pntr_ima;
float min, max;
long int nx, ny, istatus;
char filename[60], comments[80], plotdev[40]; 
register int i;

/* Input parameters:  can be 3 or 4 (always 7 with "runs") */
if (argc != 3 && argc != 4 && argc != 7)
  {
  printf("argc = %d \n",argc);
  printf("\nUSAGE:\n");
  printf(" plot_3d filename min,max [plotdev]\n");
  return(-1);
  }

strcpy(filename,argv[1]);
sscanf(argv[2],"%f,%f",&min,&max);
strcpy(plotdev,argv[3]);
if(plotdev[0] == ' ' || !plotdev[0]) strcpy(plotdev,"xterm");

#ifdef DEBUG
printf("Filename=%s plotdev=%s min=%f max=%f\n",filename,plotdev,min,max);
#endif

  JLP_BEGIN();
  JLP_FORMAT();
  JLP_VM_READIMAG1(&pntr_ima,&nx,&ny,filename,comments,&istatus);
/* Automatic scale: */
if(min == max)
 {
   auto_scale1(pntr_ima,nx,ny,&min,&max);
   printf(" Automatic scale: min = %G max = %G \n",min,max);
 } 
  JLP_PLOT3D(pntr_ima,nx,ny,min,max,plotdev);
  JLP_END();
}
/******************************************************************
*
*
******************************************************************/
int JLP_PLOT3D(pntr_ima,nx,ny,min,max,plotdev)
float *pntr_ima, min, max;
long int nx, ny;
char *plotdev;
{
float offx1, offy1, axlen1, aylen1, xp_min, xp_max, yp_min, yp_max;
double theta, phi, ll;
int status, TeX_flag, hardcopy_device, plan;

/* Parameters of the frame:
* Boundaries of the projection plane: (xp_min,xp_max) & (yp_min, yp_max) */
/* if theta=0: plane is (y,z) */
  offx1 = 4000.; offy1 = 3500.;
  axlen1 = 27000.; aylen1 = 27000.;
  plan = 0;
  xp_min = -1; xp_max = nx+1;
  yp_min = -1; yp_max = ny+1;
  JLP_SETUP_PLOT(&offx1,&offy1,&axlen1,&aylen1,
                 &xp_min,&xp_max,&yp_min,&yp_max,&plan);

/*********************************************************************/
/* Open display: */
   TeX_flag = 1;
   if(plotdev[0] == 'x') TeX_flag = 1;
   status = jlp_device(plotdev,1,1,&TeX_flag,&hardcopy_device);
   if(status != 0)
    {
    printf(" Fatal error opening display\n");
    return(-1);
    }

   theta = 20.;
   phi=30.;
   ll=-1;
   sm_set_viewpoint(theta,phi,ll);
   sm_draw_surface(pntr_ima,1,(double)min,(double)max,nx,ny);
   jlp_box_3d("X axis","Y axis","Z axis");

   JLP_GFLUSH();
   JLP_SPCLOSE();

return(0);
}
