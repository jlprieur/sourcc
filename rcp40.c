/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
* Program rcp40 to read CP40 format 
* (G. Lelievre format, from streamer cartridge...)
*
* JLP 
* Version 20-07-93
-------------------------------------------------------------------*/

/************************** REMEMBER::: ***********************
 To dump a file on the screen and see Ascii and Hexa codes:
      od -cx file_name
 each line with 32 bytes
 first block of 512 bytes from first up to line starting with 1000 (Hexa)
 then from 2000 (2nd block), 3000 (3rd block), etc.
 (WARNING: all the lines are not displayed if filled with zeroes...)
*/

#define DEBUG
#define NBLOCKMAX 10000 
#define NX_CP40  640 
#define NY_CP40  512 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <jlp_ftoc.h>

main(argc,argv)
int argc;
char *argv[];
{
char in_name[61], out_name[61], comments[81];
float *real_array;
int nx, ny, status, iformat;
int i;

printf(" Program rcp40 to read CP40 files \n");
printf(" (G. Lelievre format: streamer cartridge...) \n");
printf(" JLP Version 20-07-93 \n");

/* One or three parameters only are allowed to run the program: */
/* Carefull: 7 parameters always, using JLP "runs" */
if(argc == 2)
  {
  printf(" Syntax: rcp40 in_file out_file \n\n"); 
  printf(" Fatal: Syntax error: argc=%d\n",argc);
  exit(-1);
  }

/* Interactive input of parameters: */
if (argc == 1)
 { 
  printf(" Syntax: rcp40 in_file out_file \n\n"); 
  printf(" Input file := ");scanf("%s",in_name);
  printf(" Output file := ");scanf("%s",out_name);
 }
else
 {
  strcpy(in_name,argv[1]);
  strcpy(out_name,argv[2]);
 }

/**********************************************************/
JLP_BEGIN();
JLP_INQUIFMT();


#ifdef DEBUG
printf(" OK, will read >%s< \n",in_name);
#endif

/* Calling reading subroutine: */
  status = rdcp40(&real_array,&nx,&ny,in_name,comments);

if(!status)
  {

#ifdef DEBUG
for(i=0; i<10; i++) printf(" a[%d]: %f ",i,real_array[i]);
#endif

  printf(" Conversion of CP40 file:  %s    to:    %s \n",in_name,out_name);
  sscanf(comments," From  %s",in_name);
  JLP_WRITEIMAG(real_array,&nx,&ny,&nx,out_name,comments);
  }

JLP_END();
}
/****************************************************************/
/*
Format CP40 data 
*/
/****************************************************************/
/* Subroutine rdcp40 to read CP40 format */
int rdcp40(real_array,nx,ny,in_name,comments)
float **real_array;
int *nx, *ny;
char in_name[], comments[];
{
FILE *fd;
char header[1024];
int nbytes_to_read, nbytes, isize, nvalues, nblock; 
unsigned long iarray[256], ix, iy;
int i, j;

#ifdef DEBUG
printf(" \n rdcp40/reading file : %s \n",in_name);
#endif

/* Opens the input file */
if((fd = fopen(in_name,"r")) == NULL)
  {
  printf("rdcp40/error opening input file: >%s< \n",in_name);
  return(-1);
  }

/***************************************************************/
/* Reads header (1024 bytes): */
  nbytes_to_read = 1024;
  nbytes = fread(header,sizeof(char),nbytes_to_read,fd);
#ifdef DEBUG
  printf("        %d bytes read \n", nbytes);
#endif
  if(nbytes != nbytes_to_read)
    {
     printf("rdcp40/error reading header: \n");
     printf("       Only %d bytes read \n", nbytes);
     return(-2);
    }

/* Decode header: */
#ifdef DEBUG
for(i = 0; i < 40; i++) 
   {
   if( (int)header[i] != 0 && (int)header[i] != 32)
       printf(" h[%d]: >%c< or %x (Hexa) ",i,header[i],(int)header[i]);
   }
printf("\n");
#endif
printf(" Header: %s \n",header);
/***************************************************************/

/* Set size (always the same ...) */
*nx = NX_CP40; *ny = NY_CP40;
printf(" Output image size: nx = %d ny = %d \n",*nx,*ny);

/***************************************************************/
/* Allocate memory space for output image: */
isize = *nx * *ny * sizeof(float);
if(!(*real_array = (float*)malloc(isize) ) )
  {printf("rdcp40/fatal error: no memory space available! (isize=%d)\n",isize);
   return(-1);
  }
for(i = 0; i < *nx * * ny; i++) (*real_array)[i] = 0.; 

/***************************************************************/
/* Read the data: */
for(nblock = 0; nblock < NBLOCKMAX; nblock++)
{
  nvalues = fread(iarray,sizeof(long),128,fd);
  if(nvalues != 128)
  {
  printf("rdcp40/end of file: >%s< \n",in_name);
  printf(" Only %d values read in last block #%d (throwed away)\n",nvalues,nblock);
  nblock = NBLOCKMAX;
  }
  else
  {
#ifdef DEBUG
  if((nblock % 500) == 1)
     printf(" Block #%d  of 512 bytes\n",nblock);
#else
  if((nblock % 1000) == 1)
     printf(" Block #%d  of 512 bytes\n",nblock);
#endif

/* Reads photon coordinates and fills image array: */
/* Values are ffxx xyyy,  ffxx xyyy, ... thus: */
for(i = 0; i < nvalues; i++) 
  {
    iarray[i] = iarray[i] << 8;
    ix = iarray[i] >> 20; 
    iy = (iarray[i] << 12) >> 20; 

/* Division by 4 (as we doubt of the 1/4th pixel accuracy...)
   to reduce size of output images 
   For the third quadrant: offset (512,120) (like for the run of June 1993)
    ix = ix >> 2; iy = iy >> 2;
    ix = ix - 512; iy = iy - 120; */

/* Division by 2 (as we doubt of the 1/4th pixel accuracy...)
   to reduce size of output images 
   For the third quadrant: offset (1024,240) (like for the run of June 1993) */
    ix = ix >> 1; iy = iy >> 1;
    ix = ix - 1024; iy = iy - 240;

#ifdef DEBUG
     if(i < 10 && nblock < 2)
       {
/*
       printf(" i=%d x = %x, y=%x (Hexa) ",i,ix,iy);
*/
       printf(" i=%d x = %d, y=%d \n",i,ix,iy);
       }
#endif
/* Store photon at the coordinates location: */ 
   if(ix > 0 && ix < *nx && iy > 0 && iy < *ny)
                   (*real_array)[ix + iy * *nx]++;
  }
}
/* End of current block */
}

/* Closes the input file */
  fclose(fd);
  return(0);
}
/**************************************************************
* Swap two bytes of an unsigned short integer
***************************************************************/
void swap_int(i)
unsigned short *i;
{
union {
unsigned short ii;
char         ch[2];
      } tmp;
char ch0; 

tmp.ii = *i;

#ifdef DEBUG
printf(" swap_int/before: *i= %d ch[O,1] %d %d \n",*i,tmp.ch[0],tmp.ch[1]);
printf(" swap_int/before: tmp.ii= %d  \n",tmp.ii);
#endif

ch0 = tmp.ch[0]; 
tmp.ch[0] = tmp.ch[1]; 
tmp.ch[1] = ch0; 
*i = tmp.ii;

#ifdef DEBUG
printf(" swap_int/after: *i= %d ch[O,1] %d %d \n",*i,tmp.ch[0],tmp.ch[1]);
#endif
}
/**************************************************************
* Inversion of an unsigned long integer
* From [0 1 2 3] to [3 2 1 0]
***************************************************************/
void swap_lint(i)
unsigned long *i;
{
union {
unsigned long int ii;
char         ch[4];
      } tmp;
char ch0, ch1; 

tmp.ii = *i;
ch0 = tmp.ch[0]; 
ch1 = tmp.ch[1]; 
tmp.ch[0] = tmp.ch[3]; 
tmp.ch[1] = tmp.ch[2]; 
tmp.ch[2] = ch1; 
tmp.ch[3] = ch0; 
*i = tmp.ii;
}
