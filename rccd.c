/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
* Program rccd to read CCD Toulouse format 
*
* JLP 
* Version 20-01-99
-------------------------------------------------------------------*/

/************************** REMEMBER::: ***********************
 To dump a file on the screen and see Ascii and Hexa codes:
      od -cx file_name
 each line with 32 bytes
 first block of 512 bytes from first up to line starting with 1000 (Hexa)
 then from 2000 (2nd block), 3000 (3rd block), etc.
 (WARNING: all the lines are not displayed if filled with zeroes...)
*/

#define DEBUG 2
#define NBLOCKMAX 10000 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <math.h>
#include <jlp_ftoc.h>


void rccd_swap_int(unsigned short *i);
void rccd_swap_long2(unsigned long *i);
void rccd_swap_short(unsigned short *i);
void rccd_swap_long();

float vmsreal(char *bytes);
int vmsinteger(char *bytes);

main(argc,argv)
int argc;
char *argv[];
{
char in_name[61], out_name[61], generic_name[61], comments[81];
char descr_name[61], descr_value[81], *pc;
float *real_array;
long status, nx, ny, iformat, descr_length;
long in_f, out_f;
register int i;

printf(" Program rccd to read CCD Toulouse image files and convert to FITS format\n");
printf(" JLP Version 20-01-99 \n");

/* One or three parameters only are allowed to run the program: */
/* Carefull: 7 parameters always, using JLP "runs" */
if(argc == 7 && *argv[3]) argc = 4;
if(argc == 7 && *argv[2]) argc = 3;
if(argc == 7 && *argv[1]) argc = 2;
if(argc != 2 && argc != 3)
  {
  printf(" Syntax: rccd fname  (assuming fname.net -> fname.fits)\n"); 
  printf(" or :    rccd in_fname out_fname \n\n"); 
  printf(" Fatal: Syntax error: argc=%d\n",argc);
  exit(-1);
  }

/* Interactive input of parameters: */
if (argc == 3 )
 {
  strcpy(in_name,argv[1]);
  strcpy(out_name,argv[2]);
 }
else if (argc == 2)
 {
  strcpy(generic_name,argv[1]);
  pc = generic_name;
  while(*pc && *pc != '.') pc++;
  *pc = '\0';
  sprintf(in_name,"%s.net",generic_name);
  sprintf(out_name,"%s.fits",generic_name);
  printf(" Input: %s    Output: %s \n",in_name,out_name);
 }
else
 { 
  printf(" Syntax: rccd in_file out_file \n\n"); 
  printf(" Input file := ");scanf("%s",in_name);
  printf(" Output file := ");scanf("%s",out_name);
 }

/**********************************************************/
JLP_BEGIN();
/* 3=CCD int   4=CCD real */
in_f = 4; out_f = 8;
JLP_FORMAT(&in_f,&out_f);


#ifdef DEBUG
printf(" OK, will read >%s< \n",in_name);
#endif

/* Calling reading subroutine: */
  comments[0] = '\0';
  status = rd_iccd(&real_array,&nx,&ny,in_name,comments);

if(!status)
  {

#ifdef DEBUG
for(i=0; i<10; i++) printf(" a[%d]: %f ",i,real_array[i]);
printf("\n");
#endif

  printf("Conversion of CCD file: %s to: %s\n",in_name,out_name);
  
  if(!*comments) sprintf(comments," From  %s",in_name);
#ifdef DEBUG
  printf(">%s<\n",comments);
#endif
  JLP_WRITEIMAG(real_array,&nx,&ny,&nx,out_name,comments);
  }

JLP_END();
}
/****************************************************************
* Subroutine rd_iccd to read integer*2 CCD Toulouse format
* Header: char*80 ident, int*2 nx, int*2 ny 
****************************************************************/
int rd_iccd(real_array,nx,ny,in_name,comments)
float **real_array;
long *nx, *ny;
char in_name[], comments[];
{
FILE *fd;
char header[1088], *p;
long status, nbytes_to_read, nbytes, isize, nvalues, nblock, iblock, k; 
long int_image;
unsigned short *ii; 
char iarray[1088*2];
register int i, j;
union {
unsigned short is;
char c[2];
      } tmp0;

#ifdef DEBUG
printf(" \n rd_ccd/reading file : %s \n",in_name);
#endif

/* Opens the input file */
if((fd = fopen(in_name,"r")) == NULL)
  {
  printf("rd_ccd/error opening input file: >%s< \n",in_name);
  return(-1);
  }

/***************************************************************/
/* Header: char*80 ident, int*2 nx, int*2 ny */ 
/* Reads comments (80 bytes): */
  nbytes_to_read = 512;
  nbytes = fread(header,sizeof(char),nbytes_to_read,fd);
#ifdef DEBUG
  printf("        %ld bytes read \n", nbytes);
#endif
  if(nbytes != nbytes_to_read)
    {
     printf("rd_ccd/error reading header: \n");
     printf("       Only %ld bytes read \n", nbytes);
     return(-2);
    }
  strncpy(comments,header,80);
  comments[79] = '\0';
  tmp0.c[0] = header[80]; tmp0.c[1] = header[81];
/* Not necessary (LINUX on a penthium is like DEC) 
*/
#ifdef ibm
  rccd_swap_int(&(tmp0.is));
#endif
  *nx = tmp0.is;
/* JLP99 here set *nx if necessary ...*/
/*
  *nx = 682;
*/
  tmp0.c[0] = header[82]; tmp0.c[1] = header[83];
/* Not necessary (LINUX on a penthium is like DEC) 
*/
#ifdef ibm
  rccd_swap_int(&(tmp0.is));
#endif
  *ny = tmp0.is;

#ifdef DEBUG
  printf("rccd: nx=%ld ny=%ld\n",*nx,*ny);
#endif
 if(*nx > 1088) 
    {
    printf("Error, this version allows only 1088 columns\n");
    return(-3);
    }

/* Close file and re-open (since RECL=NX) */
fclose(fd);
if((fd = fopen(in_name,"r")) == NULL)
  {
  printf("rd_ccd/error opening input file: >%s< \n",in_name);
  return(-1);
  }

/***************************************************************/
/* Just to try: */
#ifdef TT
  nbytes_to_read = (*nx) * 2;
  nvalues = fread(iarray,1,nbytes_to_read,fd);
  nvalues = fread(iarray,1,nbytes_to_read,fd);
   for(i = 0; i < 30; i++) 
       {
       tmp0.c[0] = iarray[2*i];
       tmp0.c[1] = iarray[2*i+1];
       printf(" %x or %d\n",tmp0.is, tmp0.is); 
       ii = &iarray[2*i];
       rccd_swap_short(ii);
       tmp0.is = *ii;
       printf(" h[%d]: swaped: %d \n",i,tmp0.is);
       }
if(i != 1234) return(1);
#endif

/***************************************************************/
/* Allocate memory space for output image: */
printf(" Output image size: nx = %d ny = %d \n",*nx,*ny);
isize = *nx * *ny * sizeof(float);
if(!((*real_array) = (float*)malloc(isize) ) )
  {printf("rd_ccd/fatal error: no memory space available! (isize=%d)\n",isize);
   return(-1);
  }
for(i = 0; i < *nx * *ny; i++) (*real_array)[i] = 0.; 

/***************************************************************/
/* Read the data: */
  nbytes_to_read = (*nx) * 2;
/* Empty header first...
*/
  nvalues = fread(iarray,1,nbytes_to_read,fd);
k=0;
for(nblock = 0; (nblock < NBLOCKMAX) && (k < *nx * *ny); nblock++)
{
  nvalues = fread(iarray,sizeof(char),nbytes_to_read,fd);
  if(nvalues != nbytes_to_read)
  {
  printf("rccd/end of file: >%s< \n",in_name);
  printf(" Only %d values read in last block #%d \n",nvalues,nblock);
  nblock = NBLOCKMAX;
  }

/*
  printf(" block#%d ",nblock);
*/

    for(i = 0; i < nvalues; i+=2) 
      {
       if(k == *nx * *ny) break;
       tmp0.c[0] = iarray[i];
       tmp0.c[1] = iarray[i+1];
       (*real_array)[k++] = (float)tmp0.is; 
      }
/* End of current block */
}

  printf(" last block#%d\n",nblock);

/* Closes the input file */
  fclose(fd);
  return(0);
}
/****************************************************************
* Subroutine rd_rccd to read real CCD Toulouse format
* Header: char*80 ident, int*2 nx, int*2 ny 
****************************************************************/
int rd_rccd(real_array,nx,ny,in_name,comments)
float **real_array;
long *nx, *ny;
char in_name[], comments[];
{
FILE *fd;
char header[512], *p;
long status, nbytes_to_read, nbytes, isize, nvalues, nblock, iblock, k; 
long int_image;
unsigned long *ii; 
char iarray[512*4];
register int i, j;
union {
unsigned short is;
char c[2];
      } tmp0;
union {
unsigned long il;
float         ff;
char          c[4];
      } tmp;

#ifdef DEBUG
printf(" \n rd_ccd/reading file : %s \n",in_name);
#endif

/* Opens the input file */
if((fd = fopen(in_name,"r")) == NULL)
  {
  printf("rd_ccd/error opening input file: >%s< \n",in_name);
  return(-1);
  }

/***************************************************************/
/* Header: char*80 ident, int*2 nx, int*2 ny */ 
/* Reads comments (80 bytes): */
  nbytes_to_read = 512;
  nbytes = fread(header,sizeof(char),nbytes_to_read,fd);
#ifdef DEBUG
  printf("        %ld bytes read \n", nbytes);
#endif
  if(nbytes != nbytes_to_read)
    {
     printf("rd_ccd/error reading header: \n");
     printf("       Only %ld bytes read \n", nbytes);
     return(-2);
    }
  strncpy(comments,header,80);
  comments[79] = '\0';
  tmp0.c[0] = header[80]; tmp0.c[1] = header[81];
/* Not necessary (LINUX on a penthium is like DEC) 
*/
#ifdef ibm
  rccd_swap_int(&(tmp0.is));
#endif
  *nx = tmp0.is;
  tmp0.c[0] = header[82]; tmp0.c[1] = header[83];
/* Not necessary (LINUX on a penthium is like DEC) 
*/
#ifdef ibm
  rccd_swap_int(&(tmp0.is));
#endif
  *ny = tmp0.is;

#ifdef DEBUG
  printf("rccd: nx=%ld ny=%ld\n",*nx,*ny);
#endif
 if(*nx > 512) 
    {
    printf("Error, this version allows only 512 columns\n");
    return(-3);
    }

/* Close file and re-open (since RECL=NX) */
fclose(fd);
if((fd = fopen(in_name,"r")) == NULL)
  {
  printf("rd_ccd/error opening input file: >%s< \n",in_name);
  return(-1);
  }

/***************************************************************/
/* Just to try: */
#define TT
#ifdef TT
  nbytes_to_read = (*nx) * 4;
  nvalues = fread(iarray,1,nbytes_to_read,fd);
  nvalues = fread(iarray,1,nbytes_to_read,fd);
   for(i = 0; i < 30; i++) 
       {
       tmp.c[0] = iarray[4*i];
       tmp.c[1] = iarray[4*i+1];
       tmp.c[2] = iarray[4*i+2];
       tmp.c[3] = iarray[4*i+3];
       printf(" %x or %f or vmsf: %f or vmsi:%d\n",tmp.il, tmp.ff, 
                    vmsreal(tmp.c),vmsinteger(tmp.c));
       ii = &iarray[4*i];
       rccd_swap_long2(ii);
       tmp.il = *ii;
       printf(" h[%d]: (%x swap_long=%f or vms:%f) \n",i,ii,tmp.ff,
                    vmsreal(tmp.c));
       }
if(i != 1234) return(1);
#endif

/***************************************************************/
/* Allocate memory space for output image: */
printf(" Output image size: nx = %d ny = %d \n",*nx,*ny);
isize = *nx * *ny * sizeof(float);
if(!((*real_array) = (float*)malloc(isize) ) )
  {printf("rd_ccd/fatal error: no memory space available! (isize=%d)\n",isize);
   return(-1);
  }
for(i = 0; i < *nx * *ny; i++) (*real_array)[i] = 0.; 

/***************************************************************/
/* Read the data: */
  nbytes_to_read = 512;
/* Empty block first...
  nvalues = fread(iarray,sizeof(char),nbytes_to_read,fd);
*/
int_image = 0;
k=0;
for(nblock = 0; (nblock < NBLOCKMAX) && (k < *nx * *ny); nblock++)
{
  nvalues = fread(iarray,sizeof(char),nbytes_to_read,fd);
  if(nvalues != nbytes_to_read)
  {
  printf("rccd/end of file: >%s< \n",in_name);
  printf(" Only %d values read in last block #%d \n",nvalues,nblock);
  nblock = NBLOCKMAX;
  }

/*
  printf(" block#%d ",nblock);
*/

/* Reads photon coordinates and fills image array: */
/* Values are ffxx xyyy,  ffxx xyyy, ... thus: */
    for(i = 0; i < nvalues; i+=4) 
      {
       if(k == *nx * *ny) break;
       if(int_image)
           {
           (*real_array)[k++] = (float)vmsinteger( &iarray[i] );
           }
       else
           (*real_array)[k++] = vmsreal( &iarray[i] );
      }
/* End of current block */
}

  printf(" last block#%d\n",nblock);

/* Closes the input file */
  fclose(fd);
  return(0);
}
/**************************************************************
* Swap two bytes of an unsigned short integer
***************************************************************/
void rccd_swap_int(unsigned short *i)
{
union {
unsigned short ii;
char         ch[2];
      } tmp;
char ch0, cwork; 

cwork = 15;

tmp.ii = *i;

#ifdef DEBUG
printf(" swap_int/before: *i= %d ch[O,1] %d %d \n",*i,tmp.ch[0],tmp.ch[1]);
printf(" swap_int/before: tmp.ii= %d  \n",tmp.ii);
#endif

ch0 = tmp.ch[0]; 
tmp.ch[0] = tmp.ch[1]; 
tmp.ch[1] = ch0; 
/*
tmp.ch[0] = tmp.ch[1] & cwork; 
tmp.ch[1] = ch0 & cwork; 
*/
*i = tmp.ii;

#ifdef DEBUG
printf(" swap_int/after: *i= %d ch[O,1] %d %d \n",*i,tmp.ch[0],tmp.ch[1]);
#endif
}
/**************************************************************
* Inversion of an unsigned long integer
* From [0 1 2 3] to [0 1 3 2]: bad
* From [0 1 2 3] to [1 0 2 3]: not bad
***************************************************************/
void rccd_swap_long(i)
unsigned long *i;
{
union {
unsigned long int ii;
char         ch[4];
      } tmp;
char ch0, ch2; 

tmp.ii = *i;
ch2 = tmp.ch[1]; 
tmp.ch[1] = tmp.ch[0]; 
tmp.ch[0] = ch2; 
*i = tmp.ii;
}
/**************************************************************
* Inversion of an unsigned long integer
* From [0 1 2 3] to [3 2 1 0]
***************************************************************/
void rccd_swap_long2(unsigned long *i)
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
/**************************************************************
* Inversion of an unsigned short integer
* From [0 1] to [1 0]
***************************************************************/
void rccd_swap_short(unsigned short *i)
{
union {
unsigned short ii;
char         ch[2];
      } tmp;
char ch0; 

tmp.ii = *i;
ch0 = tmp.ch[0]; 
tmp.ch[0] = tmp.ch[1]; 
tmp.ch[1] = ch0; 
*i = tmp.ii;
}
/**************************************************************
 *  From  "dipconv.c"
 *  Author:
 *     David Berry (DSB):
 *     24-APR-1995 (DSB):
 */
float vmsreal( char *bytes ){
/*
 * Interpret the first 4 bytes in the supplied array as a VMS real
 * value and return the corresponding unix floating value.
 */

      unsigned int frac;
      int exp;
      float ans;

/*  Set up a structure which overlays the supplied 4 bytes and divides
 *  then up into the various required fields. Solaris packs bit fields
 *  from the most significant bit downwards, where as OSF packs them from 
 *  the least significant bit upwards. Therefore separate structures are 
 *  required for the two operating systems. 

 *  Solaris... */

/* For IBM ul8820, um8901, etc ... sun4_Solaris*/
/*
#define sun4_Solaris
*/
#if defined( sun4_Solaris )

      struct VMS {
         unsigned f6 : 1; /* exponent bit 0 (lsb) */
         unsigned f5 : 7; /* mantissa bits 16 to 22 (msb) */
         unsigned f4 : 1; /* sign bit */
         unsigned f3 : 7; /* exponent bits 1 to 7 (msb) */
         unsigned f2 : 8; /* mantissa bits 0 (lsb) to 7 */
         unsigned f1 : 8; /* mantissa bits 8 to 15 */
             } *vms;

/*  OSF ... */

/*
#elif defined( alpha_OSF1 ) 
*/
#else

      struct VMS {
         unsigned f5 : 7; /* mantissa bits 16 to 22 (msb) */
         unsigned f6 : 1; /* exponent bit 0 (lsb) */
         unsigned f3 : 7; /* exponent bits 1 to 7 (msb) */
         unsigned f4 : 1; /* sign bit */
         unsigned f2 : 8; /* mantissa bits 0 (lsb) to 7 */
         unsigned f1 : 8; /* mantissa bits 8 to 15 */
            } *vms;
#endif


/*  Overlay the structure on top of the supplied bytes */

      vms = (struct VMS *) bytes;


/*  Extract the mantissa as an unsigned integer value */

      frac = ( vms->f2 ) + ( vms->f1 << 8 ) + ( vms->f5 << 16 ) + 
             ( 1 << 23 );


/*  Extract the exponent as an unsigned integer value. Subtract off 128
 *  to account for the exponent bias used in VMS. Also subtract of 24
 *  to shift the mantissa to the right of the binary point. */

      exp = ( vms->f6 ) + ( vms->f3 << 1 ) - 152;


/*  Form the answer as the mantissa times two to the power of exponent */

      ans = (float) ldexp( (double) frac, exp );


/*  Invert the answer if the sign bit is set. */

      if( vms->f4 ) ans = -ans ;

      return( ans );

}

int vmsinteger( char *bytes ){
/*
 * Interpret the first 4 bytes in the supplied array as a VMS integer
 * value and return the corresponding unix integer value.
 */
      union {
         unsigned int unsgn;
         int sgn;
            } ans;

      struct VMS {
         unsigned f4 : 8; /* vms bits 0 (lsb) to 7 */
         unsigned f3 : 8; /* vms bits 8 to 15 */
         unsigned f2 : 8; /* vms bits 16 to 23 */
         unsigned f1 : 8; /* vms bits 24 to 31 (msb) */
            } *vms;

      vms = (struct VMS *) bytes;
/* << 8 shift to the left of 8 bytes */
      ans.unsgn = ( vms->f4 ) + ( vms->f3 << 8 ) + ( vms->f2 << 16 ) + 
                  ( vms->f1 << 24 );
      
      return( ans.sgn );

}
