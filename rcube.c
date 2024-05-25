/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
* Program rcube to read FITS cubes
*
* JLP 
* Version 12-10-99
-------------------------------------------------------------------*/
/*
#define DEBUG
*/

#include <stdio.h>
#include <jlp_ftoc.h>

int main(int argc, char *argv[])
{
INT_PNTR pntr_array;
INT4 nx, ny, nz, iplane, dflag, status;
char in_name[61], out_name[61], comments[81], jlp_descr[60];
float *real_array;
INT4 in_f, out_f;
#ifdef DEBUG
int i;
#endif

printf(" Program rcube to read FITS cubes and extract a plane\n");
printf(" JLP Version 13-10-99 \n");

/* One or three parameters only are allowed to run the program: */
/* Carefull: 7 parameters always, using JLP "runs" */
if(argc == 7 && *argv[3]) argc = 4;
if(argc == 7 && *argv[2]) argc = 3;
if(argc == 7 && *argv[1]) argc = 2;
if(argc != 4)
  {
  printf(" Syntax: rcube cube_name plane_number out_image\n"); 
  printf(" Fatal: Syntax error: argc=%d\n",argc);
  exit(-1);
  }

/* Interactive input of parameters: */
if (argc == 4 )
 {
  strcpy(in_name,argv[1]);
  sscanf(argv[2],"%d",&iplane);
  strcpy(out_name,argv[3]);
 }
else
 { 
  printf(" Syntax: rbdf in_file out_file \n\n"); 
  printf(" Input file := ");scanf("%s",in_name);
  printf(" Plane to extract := ");scanf("%d",&iplane);
  printf(" Output file := ");scanf("%s",out_name);
 }

printf(" Input: %s  plane #%d  Output: %s \n",in_name,iplane,out_name);

/**********************************************************/
in_f = 8; out_f = 8;
JLP_FORMAT(&in_f,&out_f);

/* Calling reading subroutine: */
comments[0] = '\0';
/* No descriptors: */
dflag = 0;

/* iplane = 0 would extract the whole 3D array */
status = JLP_VM_RDFITS_3D(&pntr_array,&nx,&ny,&nz,&iplane,in_name,comments,
                       jlp_descr,&dflag,&status);

real_array = (float *) pntr_array;

if(!status)
  {

printf(" nx=%d ny=%d nz=%d\n",nx,ny,nz);

  sprintf(comments,"#%d from %s",iplane,in_name);
  JLP_WRITEIMAG(real_array,&nx,&ny,&nx,out_name,comments);
  }
else
  printf(" rdcube/fatal error reading file %s \n",in_name);

return(0);
}
