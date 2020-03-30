/***************************************************************
* Program to work with optical matrices 
*
* Contains:
* int input_thin_lens()
* int define_object()
* int draw_rays()
* int plot_lenses(npts, metafile)
*
* JLP
* Version of 08-12-92
***************************************************************/
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <jlp_ftoc.h>

/*
#define DEBUG
*/
#define MAX_OPTICS 20
#define NRAYS_MAX  50
#define PI 3.14159265358979323846

static char buffer[91], lens_name[MAX_OPTICS][30];
static float global_m[4], object_size, xxmax, zmax; 
static float lens_m[MAX_OPTICS][4];
static float z_in[MAX_OPTICS], z_out[MAX_OPTICS], phi_diaph[MAX_OPTICS];

main()
{
float zstart, xc, yc; 
int ioption, i, nrays, nmatrix, status, in_frame, plot_opened, metafile;
float xstart[NRAYS_MAX], aper[NRAYS_MAX];

/* Called functions: */
int input_thin_lens(), define_object(), draw_rays(), transfer_matrix();
int input_matrix();

printf("Program optic_matrix to work with optical matrices \n"); 
printf(" Version 05-01-93 (max matrices = %d) (max rays = %d)\n",
         MAX_OPTICS, NRAYS_MAX); 

/* Fills global matrix 11 12 21 22  (with identity) */
global_m[0] = 1.;
global_m[1] = 0.;
global_m[2] = 1.;
global_m[3] = 0.;

nmatrix = -1;
nrays = -1;

/****************************************************************/
/* Menu (changing according to parameters already in) */
plot_opened = 0;
ioption = 1;
while(ioption > 0 && ioption < 5)
{
  printf("\n Main menu: (successive lenses should be added with increasing Z) \n");
  printf(" 1: Add a thin lens\n");
  printf(" 2: Add a full system with its matrix\n");
  if(nmatrix >= 0) printf(" 3: Draw optical rays \n");
  if(plot_opened)  printf(" 4: Save plot to metafile \n");
  printf(" 10: Exit \n");
  printf("\n Enter your choice: ");
  gets(buffer); sscanf(buffer,"%d",&ioption);
/****************************************************************/
   i = 0;
   switch (ioption)
   {
/****************************************************************/
/* Thin lens or matrix: */
      case 1 : 
      case 2 : 

/* Closes plot if already opened: */
	if(plot_opened) {JLP_SPCLOSE(); plot_opened = 0; nrays = -1;}

        i = nmatrix + 2;
/* Taking last transfer matrix  and last point for drawing into account here: */
        if(i >= MAX_OPTICS-2)
          {printf(" Maximum number of matrices has been reached: %d \n",
                  MAX_OPTICS);
           break;
          }
        zstart = (i-2 > 0) ? z_out[i-2] : 0.;

/* Name of lens: */
        printf(" Enter lens name: "); gets(lens_name[i]);
/* Switch is thin lens or full matrix: */
        if(ioption == 1)
           {
           status = input_thin_lens(lens_m[i],&z_in[i],&phi_diaph[i],zstart); 
/* Thin lens: z_out = z_in */
           z_out[i] = z_in[i];
           }
        else
           status = input_matrix(lens_m[i],&z_in[i],&z_out[i],
                                  &phi_diaph[i],zstart); 

/* If OK, add transfer matrix from previous to new lens */
        if(!status) 
         {
          nmatrix++;
          transfer_matrix(lens_m[nmatrix],zstart,z_in[i]); 
          z_in[nmatrix] = zstart;
          z_out[nmatrix] = z_in[i];
          phi_diaph[nmatrix] = 0.;
/* Taking lens matrix into account: */
          nmatrix++;
         }
        break;
/****************************************************************/
/* Draw rays: */
      case 3 : 
        if(!plot_opened) 
          {metafile = 0; status = plot_lenses(nmatrix,metafile);
           if(status) break;
           plot_opened = 1;} 
        nrays++;
        if(nrays < NRAYS_MAX)
           draw_rays(nmatrix,metafile,&xstart[nrays],&aper[nrays]); 
        else printf(" Sorry capacity has been exceeded \n");
        break;
/****************************************************************/
/* Save plot to metafile: */
      case 4 : 
        JLP_SPCLOSE();
        metafile = 1;
        status = plot_lenses(nmatrix,metafile);
        if(status) break;
        for(i = 0; i <= nrays; i++)
           draw_rays(nmatrix,metafile,&xstart[i],&aper[i]); 
/* Reset parameters: */
        JLP_SPCLOSE(); nrays = -1;
        plot_opened = 0;
        break;
      default: 
        break;

/* End of switch (ioption) */ 
   }

/* End of input loop (while ioption is valid...) */
}

/* End of program: */
/*
status = JLP_WHERE(&xc,&yc,&in_frame);
*/
if(plot_opened) JLP_SPCLOSE();
}
/****************************************************************
* Routine input_thin_lens
****************************************************************/
int input_thin_lens(lens,z,phi,zmin)
float *lens, *z, *phi, zmin;
{
float flength;

/* Enter focal length and Z position: */
  printf("\n Enter focal length, diameter and position ");
  printf("(along Z axis, origin is at object location) \n [F,phi,z]: ");
  gets(buffer); 
  sscanf(buffer,"%f,%f,%f",&flength,phi,z);
  if(flength == 0 || *z <= 0 || *phi <= 0) 
    {printf(" Wrong parameters: F=%f phi=%f z=%f \n",flength,*phi,*z);
    return(-1);
    }
/* Allows only lenses in increasing Z location */
  if(*z <= zmin ) 
    {printf(" Wrong location: z_lens=%f whereas previous lens was at: z=%f \n",
            *z,zmin);
    return(-1);
    }
 
/* Fills lens matrix (11 12 21 22) and z location */
lens[0] = 1.;
lens[1] = 0.;
lens[2] = -1./flength;
lens[3] = 1.;

#ifdef DEBUG
  printf(" Input_thin_lens/ matrix is   %f   %f \n",lens[0],lens[1]); 
  printf("                              %f   %f \n",lens[2],lens[3]); 
#endif

return(0);
}
/****************************************************************
* Routine input_matrix
****************************************************************/
int input_matrix(lens,zin,zout,phi,zmin)
float *lens, *zin, *zout, *phi, zmin;
{

/* Enter location of input/output planes: */
  printf("\n Enter Z location of input/output planes and diaphragm ");
  printf("(along Z axis, origin is at object location) \n [z_in,z_out,phi]: ");
  gets(buffer); 
  sscanf(buffer,"%f,%f,%f",zin,zout,phi);
  if(*zin > *zout || *phi <= 0) 
    {printf(" Wrong parameters: z_in=%f z_out=%f phi=%f \n",*zin,*zout,*phi);
    return(-1);
    }
/* Allows only lenses in increasing Z location */
  if(*zin <= zmin ) 
    {printf(" Wrong location: z_in=%f whereas previous lens was at: z=%f \n",
            *zin,zmin);
    return(-1);
    }
 
/* Fills lens matrix (11 12 21 22) */
  printf(" Enter matrix coefficients [T_11,T_12,T_21,T_22]: ");
  gets(buffer); 
  sscanf(buffer,"%f,%f,%f,%f",&lens[0],&lens[1],&lens[2],&lens[3]);

#ifdef DEBUG
  printf(" Input_matrix/ matrix is   %f   %f \n",lens[0],lens[1]); 
  printf("                           %f   %f \n",lens[2],lens[3]); 
#endif
return(0);
}
/****************************************************************
* Routine draw_rays
****************************************************************/
int draw_rays(nmatrix,metafile,xstart,aper)
int nmatrix, metafile;
float *xstart, *aper;
{
register int i;
float xx[MAX_OPTICS], alpha[MAX_OPTICS];
int nm, status;
FILE *fp;


/* Set up last transfer matrix: */ 
   nm = nmatrix + 1;
   transfer_matrix(lens_m[nm],z_out[nm-1],zmax); 
   phi_diaph[nm] = 0.;
   z_in[nm] = z_out[nm-1];
   z_out[nm] = zmax;
 
/* Enter input ray: */
  if(!metafile)
  {
    printf("\n Enter X position in object plane and aperture (F/D): ");
    gets(buffer); sscanf(buffer,"%f,%f",xstart,aper);
    if(*aper == 0)
    { printf("draw_rays/Error F/D=%f !\n",*aper); return(-1);}
  }

xx[0] = *xstart;
/* Take half of F/D (assuming entrance pupil at infinity) */
alpha[0] = 0.5/ *aper;

/* Loop on the lenses: */
for(i = 0; i <= nm; i++)
 {
  xx[i+1] = lens_m[i][0] * xx[i] + lens_m[i][1] * alpha[i];
  alpha[i+1] = lens_m[i][2] * xx[i] + lens_m[i][3] * alpha[i];
 }

/* Last point: */
nm++;
z_in[nm] = zmax;
xx[nm] = xx[nm - 1] + alpha[nm - 1] * (zmax - z_in[nm - 1]);
alpha[nm] = alpha[nm -1];

/* Display raw results: */ 
 for(i = 0; i <= nm; i++)
    {
    printf(" z=%f xx[%d]=%f alpha[%d]=%f \n",z_in[i],i,xx[i],i,alpha[i]); 
#ifdef DEBUG
    printf(" z_out[%d] = %f  phi_diaph[%d] = %f\n",i,z_out[i],i,phi_diaph[i]); 
#endif
    }

/* Opening output file: */
printf(" draw_rays/ Storing data to file: optic_matrix.dat \n");
if((fp = fopen("optic_matrix.dat","w")) == NULL)
 {
 printf(" draw_rays/ Cannot open output file\n");
 return(-1);
 }

/* Write coordinates to ouput file: */ 
for(i = 0; i <= nm; i++)
 {
  fprintf(fp," %f %f \n",z_in[i],xx[i]); 
 }

fclose(fp);

nm++;
status = optic_draw(z_in,xx,nm);

return(0);
}
/****************************************************************
* Routine multi_matrix 
****************************************************************/
int multi_matrix(out,in1,in2) 
float *out, *in1, *in2;
{
return(0);
}
/****************************************************************
* Routine transfer_matrix 
****************************************************************/
int transfer_matrix(lens,zstart,zend)
float *lens, zstart, zend;
{
/* Fills transfer matrix 11 12 21 22  (index n = 1) */
     lens[0] = 1.;
     lens[1] = zend-zstart;
     lens[2] = 0.;
     lens[3] = 1.;

#ifdef DEBUG
  printf(" Transfer_matrix/ matrix is   %f   %f \n",lens[0],lens[1]); 
  printf("                              %f   %f \n",lens[2],lens[3]); 
#endif

return(0);
}
/***************************************************************
* optic_draw 
*
***************************************************************/
int optic_draw(zz,xx,npts)
float *zz, *xx;
int npts;
{
#define MAX_CURVE 3
#define NMAX 100
float    xplot[NMAX*MAX_CURVE], yplot[NMAX*MAX_CURVE]; 
float    errx[NMAX*MAX_CURVE], erry[NMAX*MAX_CURVE];
char     nchar[MAX_CURVE*4];
register int i, k;
int      status, npoints[MAX_CURVE], nmax = NMAX, ncurves, error_bars;

/*********************************************************************/
/* First curve: */
  npoints[0] = npts;
  for (i=0; i<npoints[0]; ++i) 
     {
      xplot[i] = zz[i]; 
      yplot[i] = xx[i]; 
     }
  strcpy(nchar,"L1");

/* Second curve: (symmetrical relative to the first)*/
  npoints[1] = npts;
  for (i=0; i<npoints[0]; ++i) 
     {
      xplot[i + nmax] = zz[i]; 
      yplot[i + nmax] = -xx[i]; 
     }
  strcpy(nchar+4,"L2");

  printf(" nchar1 %s\n",nchar);
  printf(" nchar2 %s\n",nchar+4);

   ncurves = 2; error_bars = 0;
   for(k=0; k < ncurves; k++)
   {
    JLP_CURVE(&xplot[nmax * k],&yplot[nmax * k],&errx[nmax * k],
              &erry[nmax * k], &npoints[k],nchar+4*k,&error_bars);
   }


   JLP_GFLUSH();


return(0);
}
/****************************************************************
* Routine plot_lenses 
****************************************************************/
int plot_lenses(npts,metafile)
int npts, metafile;
{
float    offx1, offy1, axlen1, aylen1, xmin, xmax, ymin, ymax;
float    x1, x2, y1, y2, xrange, errx[2], erry[2], xplot[2], yplot[2];
char     xlabel[20], ylabel[20], title[40], nchar[MAX_CURVE*4], plotdev[32];
register int i, k;
int      status, npoints[1], ncurves, error_bars, plan;
INT4     TeX_flag, hardcopy_device;
int      isize, isymbol;

/* Limits for the plot: */
  if(!metafile)
  {
    printf("\n Enter Object size, X hight and Z length for the plot: ");
    gets(buffer); sscanf(buffer,"%f,%f,%f",&object_size,&xxmax,&zmax);
    object_size = object_size/2.;
    xxmax = xxmax/2.;
    if(xxmax <= 0 || zmax <= 0)
      {
       printf("plot_lenses/Error: wrong limits, xmax=%f and zmax=%f \n",
               xxmax,zmax);
       return(-1);
      }
  }

/*********************************************************************/
/* Parameters of the frame: */
  xmax = zmax; xmin = 0.;
  xrange = xmax - xmin;
  xmax = xrange * 1.05 + xmin; xmin = xmin - 0.05 * xrange;
  ymax = xxmax * 1.2 ; ymin = -ymax;
  offx1 = 4000.; offy1 = 3500.;
  axlen1 = 27000.; aylen1 = 27000.;
  plan = 0;
  JLP_SETUP_PLOT(&offx1,&offy1,&axlen1,&aylen1,&xmin,&xmax,&ymin,&ymax,&plan);

  printf(" Graphic device? (xterm, fullpage (postscript),...)\n");
  gets(plotdev);
  if(plotdev[0] != 'x') hardcopy_device = 0;
  else hardcopy_device = 1;

/*********************************************************************/
/* Open display: */
   TeX_flag = 0;

   status = jlp_device(plotdev,1,1,&TeX_flag, &hardcopy_device, " ");
   if(status != 0)
    {printf(" plot_lenses/Error opening display\n");
    return(-1);}

/* Labels and title: */
   if(metafile)
   {
     printf(" Zlabel :\n"); gets(xlabel);
     printf(" Xlabel :\n"); gets(ylabel);
     printf(" Title :\n"); gets(title);
   }
   else
   {
     strcpy(xlabel," ");
     strcpy(ylabel," ");
     strcpy(title," ");
   }
   jlp_box(xlabel,ylabel,title,0,1,TeX_flag,hardcopy_device," "," ",0);

#ifdef DEBUG
   printf(" npts = %d \n",npts);
#endif

/* Draws the lenses: */
   for(k=0; k <= npts; k++)
   {
#ifdef DEBUG
     printf(" phi_diaph[%d] = %f \n",k,phi_diaph[k]);
#endif
    if(phi_diaph[k] > 0.)
    {
/* Entrance plane: */
     x1 = z_in[k];
     y1 = -0.5 * phi_diaph[k];
     y2 = +0.5 * phi_diaph[k];
     JLP_LINE1(&x1,&y1,&x1,&y2);

/* Output plane: (case of thick lens)*/
     if(z_out[k] != z_in[k])
     {
       x2 = z_out[k];
       JLP_LINE1(&x2,&y1,&x2,&y2);
/* Links the two ends with a box: */
       JLP_LINE1(&x1,&y1,&x2,&y1);
       JLP_LINE1(&x1,&y2,&x2,&y2);
/* Lens name: */
       x1 = (z_in[k] + z_out[k]) /2.;
       y1 = -0.5 * phi_diaph[k] - ymax/8.;
       jlp_label1(lens_name[k],x1,y1,0.,1.,1);
     }
/* Else draws arrows on ends (since it is a thin lens) */
/* Case of divergent lens: */
     else if(lens_m[k][2] > 0.)
       {
/* symbol = 3: base down */
         isize = 4; isymbol = 3;
         JLP_SYMBOL1(&x1,&y1,&isize,&isymbol);
/* symbol = 13: base up */
         isymbol = 13;
         JLP_SYMBOL1(&x1,&y2,&isize,&isymbol);
/* Lens name: */
         x1 = z_in[k];
         y1 = -0.5 * phi_diaph[k] - ymax/8.;
         jlp_label1(lens_name[k],x1,y1,0.,1.,1);
       }
/* Case of convergent lens: */
     else if(lens_m[k][2] < 0.)
       {
       if(lens_m[k][2] < 0.)
/* symbol = 13: base up */
         isize = 4; isymbol = 13;
         JLP_SYMBOL1(&x1,&y1,&isize,&isymbol);
/* symbol = 3: base down */
         isymbol = 3;
         JLP_SYMBOL1(&x1,&y2,&isize,&isymbol);
/* Lens name: */
         x1 = z_in[k];
         y1 = -0.5 * phi_diaph[k] - ymax/8.;
         jlp_label1(lens_name[k],x1,y1,0.,1.,1);
       }
/* End if phi_diaph = 0: */
    }
/* End of k loop on lenses: */
   }

/* Draws optical axis: */
   xplot[0] = xmin + xrange * 0.02;
   xplot[1] = xmax - xrange * 0.02;
   yplot[0] = 0.; yplot[1] = 0.;
   npoints[0] = 2;
   strcpy(nchar,"L4");
   error_bars = 0;
   JLP_CURVE(xplot,yplot,errx,erry,npoints,nchar,&error_bars);
/*
or    JLP_LINE1(&xplot[0],&yplot[0],&xplot[1],&yplot[0]);
*/

/* Draws object plane: */
   xplot[0] = 0.; xplot[1] = 0.;
   yplot[0] = -object_size; yplot[1] = object_size;
   npoints[0] = 2;
   strcpy(nchar,"L3");
   JLP_CURVE(xplot,yplot,errx,erry,npoints,nchar,&error_bars);

return(0);
}
