/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
* Program rbdf to read BDF Interim STARLINK format 
*
* JLP 
* Version 20-05-97
-------------------------------------------------------------------*/

/************************** REMEMBER::: ***********************
 To dump a file on the screen and see Ascii and Hexa codes:
      od -cx file_name
 each line with 32 bytes
 first block of 512 bytes from first up to line starting with 1000 (Hexa)
 then from 2000 (2nd block), 3000 (3rd block), etc.
 (WARNING: all the lines are not displayed if filled with zeroes...)
*/

/*
#define DEBUG
*/
#define NBLOCKMAX 10000 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <math.h>
#include <jlp_ftoc.h>


void rbdf_swap_int();
void rbdf_swap_long();
void rbdf_swap_long1();
void rbdf_swap_long2();
int bdf_rdescr();

float vmsreal();
int   vmsinteger();

main(argc,argv)
int argc;
char *argv[];
{
char in_name[61], out_name[61], generic_name[61], comments[81];
char date[40], descr_name[61], descr_value[81], *pc;
float *real_array;
int status, nx, ny, iformat, descr_length;
long in_f, out_f;
register int i;

printf(" Program rbdf to read BDF Starlink image files and convert to FITS format\n");
printf(" JLP Version 20-05-97 \n");

/* One or three parameters only are allowed to run the program: */
/* Carefull: 7 parameters always, using JLP "runs" */
if(argc == 7 && *argv[3]) argc = 4;
if(argc == 7 && *argv[2]) argc = 3;
if(argc == 7 && *argv[1]) argc = 2;
if(argc != 2 && argc != 3)
  {
  printf(" Syntax: rbdf fname  (assuming fname.bdf -> fname.fits)\n"); 
  printf(" or :    rbdf in_fname out_fname \n\n"); 
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
  sprintf(in_name,"%s.bdf",generic_name);
  sprintf(out_name,"%s.fits",generic_name);
  printf(" Input: %s    Output: %s \n",in_name,out_name);
 }
else
 { 
  printf(" Syntax: rbdf in_file out_file \n\n"); 
  printf(" Input file := ");scanf("%s",in_name);
  printf(" Output file := ");scanf("%s",out_name);
 }

/**********************************************************/
JLP_BEGIN();
in_f = 2; out_f = 8;
JLP_FORMAT(&in_f,&out_f);


#ifdef DEBUG
printf(" OK, will read >%s< \n",in_name);
#endif

/* Calling reading subroutine: */
  comments[0] = '\0';
  status = rd_bdf(&real_array,&nx,&ny,in_name,date,comments);

if(!status)
  {

#ifdef DEBUG
for(i=0; i<10; i++) printf(" a[%d]: %f ",i,real_array[i]);
printf("\n");
#endif

  printf("Conversion of BDF file: %s to: %s\n",in_name,out_name);
  
  strcpy(descr_name,"DATE");
  strcpy(descr_value,date);
  descr_length=22;
  JLP_WDESCR(descr_name,descr_value,&descr_length,&status);

  if(!*comments) sprintf(comments," From  %s",in_name);
#ifdef DEBUG
  printf(">%s<\n",comments);
#endif
  JLP_WRITEIMAG(real_array,&nx,&ny,&nx,out_name,comments);
  }

JLP_END();
}
/****************************************************************
* Subroutine rd_bdf to read  BDF Starlink Interim format
****************************************************************/
int rd_bdf(real_array,nx,ny,in_name,date,comments)
float **real_array;
int *nx, *ny;
char in_name[], date[], comments[];
{
FILE *fd;
char header[512], keyword[32], cvalue[32], *p;
int status, nbytes_to_read, nbytes, isize, nvalues, nblock, iblock, k; 
int int_image;
unsigned long ii, iarray[512];
register int i, j;

#ifdef DEBUG
printf(" \n rd_bdf/reading file : %s \n",in_name);
#endif

/* Opens the input file */
if((fd = fopen(in_name,"r")) == NULL)
  {
  printf("rd_bdf/error opening input file: >%s< \n",in_name);
  return(-1);
  }

/***************************************************************/
/* Reads header (512 bytes): */
  nbytes_to_read = 512;
for(iblock = 1; iblock <= 2; iblock++)
  {
  nbytes = fread(header,sizeof(char),nbytes_to_read,fd);
#ifdef DEBUG
  printf("        %d bytes read \n", nbytes);
#endif
  if(nbytes != nbytes_to_read)
    {
     printf("rd_bdf/error reading header: \n");
     printf("       Only %d bytes read \n", nbytes);
     return(-2);
    }

/* Decode header: */

/* "IMAGE" keyword */
if(iblock == 1)
  {
   strncpy(keyword,&header[4],8);
   printf("Keyword= %.10s\n",keyword);

/* Date: 3 dates */
   strncpy(keyword,&header[16],20);
   keyword[20]='\0';
   printf("Date1= %s\n",keyword);
   strcpy(date,keyword);

   strncpy(keyword,&header[36],20);
   keyword[20]='\0';
   printf("Date2= %s\n",keyword);

   strncpy(keyword,&header[56],20);
   keyword[20]='\0';
   printf("Date3= %s\n",keyword);
  }

if(iblock == 2)
  {
/* Read descriptors: */
   status = bdf_rdescr(header,nbytes,"SIMPLE",cvalue);
/*
   int_image = 1;
   if(status) int_image = 0;
   printf("Integer image (yes=1, no=0) : answer= %d \n",int_image);
*/
/* JLP99: always float image, even if "SIMPLE"! */
   int_image = 0;
   bdf_rdescr(header,nbytes,"BITPIX",cvalue);
   bdf_rdescr(header,nbytes,"NAXIS",cvalue);
   bdf_rdescr(header,nbytes,"NAXIS1",cvalue);
   sscanf(cvalue,"%d",nx); 
   bdf_rdescr(header,nbytes,"NAXIS2",cvalue);
   sscanf(cvalue,"%d",ny); 
/* For UM8901, wrong nx, so I set it to 320! (JLP99) */
   *nx = 320;
   bdf_rdescr(header,nbytes,"OBJECT",cvalue);
   bdf_rdescr(header,nbytes,"EXPOSURE",cvalue);
   bdf_rdescr(header,nbytes,"ELAPSED",cvalue);
   bdf_rdescr(header,nbytes,"UTDATE",cvalue);
   bdf_rdescr(header,nbytes,"INSTRUME",cvalue);
   bdf_rdescr(header,nbytes,"MEANRA",cvalue);
   bdf_rdescr(header,nbytes,"MEANDEC",cvalue);
/* Data = i * bscale + bzero */
   bdf_rdescr(header,nbytes,"BSCALE",cvalue);
   bdf_rdescr(header,nbytes,"BZERO",cvalue);
/* Invalid data: */
   bdf_rdescr(header,nbytes,"INVAL",cvalue);
   bdf_rdescr(header,nbytes,"TITLE",cvalue);
/* For comments, write the value of TITLE, or 
* the date */
   if(!*cvalue || (cvalue[0] == ' ' && cvalue[1] == '\0'))
       strcpy(comments,date);
   else
       strcpy(comments,cvalue);
  }

/* End of loop on iblock */
}

/***************************************************************/
#ifdef TT
  nbytes_to_read = 512;
  nvalues = fread(iarray,sizeof(long),nbytes_to_read,fd);
   for(i = 1; i < 30; i++) 
       {
       ii = iarray[i];
       rbdf_swap_int(&ii);
/*
       printf(" h[%d]: >%c< or %x (Hexa) %d (Deci) (swap_int=%d)\n",i,iarray[i],
                   (int)iarray[i],(int)iarray[i],(int)ii);
*/
       printf(" test[%d] = %d (or compl=%d) \n",i,(int)ii,(int)(ii & 0xFFFF));
       }
#endif


/***************************************************************/
/* Allocate memory space for output image: */
printf(" Output image size: nx = %d ny = %d \n",*nx,*ny);
isize = *nx * *ny * sizeof(float);
if(!((*real_array) = (float*)malloc(isize) ) )
  {printf("rd_bdf/fatal error: no memory space available! (isize=%d)\n",isize);
   return(-1);
  }
for(i = 0; i < *nx * *ny; i++) (*real_array)[i] = 0.; 

/***************************************************************/
/* Read the data: */
k=0;
for(nblock = 0; (nblock < NBLOCKMAX) && (k < *nx * *ny); nblock++)
{
  nbytes_to_read = 512;
  nvalues = fread(iarray,sizeof(long),nbytes_to_read,fd);
  if(nvalues != nbytes_to_read)
  {
  printf("rbdf/end of file: >%s< \n",in_name);
  printf(" Only %d values read in last block #%d \n",nvalues,nblock);
  nblock = NBLOCKMAX;
  }

/*
  printf(" block#%d ",nblock);
*/

/* Reads photon coordinates and fills image array: */
/* Values are ffxx xyyy,  ffxx xyyy, ... thus: */
    for(i = 0; i < nvalues; i++) 
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
void rbdf_swap_int(i)
unsigned short *i;
{
union {
unsigned short ii;
char         ch[2];
      } tmp;
char ch0, cwork; 

cwork = 15;

tmp.ii = *i;

/*
printf(" swap_int/before: *i= %d ch[O,1] %d %d \n",*i,tmp.ch[0],tmp.ch[1]);
printf(" swap_int/before: tmp.ii= %d  \n",tmp.ii);
*/

ch0 = tmp.ch[0]; 
tmp.ch[0] = tmp.ch[1]; 
tmp.ch[1] = ch0; 
/*
tmp.ch[0] = tmp.ch[1] & cwork; 
tmp.ch[1] = ch0 & cwork; 
*/
*i = tmp.ii;

/*
printf(" swap_int/after: *i= %d ch[O,1] %d %d \n",*i,tmp.ch[0],tmp.ch[1]);
*/
}
/**************************************************************
* Inversion of an unsigned long integer
* From [0 1 2 3] to [2 3 0 1]
* -> Very bad!
***************************************************************/
void rbdf_swap_long1(i)
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
tmp.ch[0] = tmp.ch[2]; 
tmp.ch[1] = tmp.ch[3]; 
tmp.ch[2] = ch0; 
tmp.ch[3] = ch1; 
*i = tmp.ii;
}
/**************************************************************
* Inversion of an unsigned long integer
* From [0 1 2 3] to [0 1 3 2]: bad
* From [0 1 2 3] to [1 0 2 3]: not bad
***************************************************************/
void rbdf_swap_long(i)
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
* From [0 1 2 3] to [1 0 3 2]
* Not bad
***************************************************************/
void rbdf_swap_long_(i)
unsigned long *i;
{
union {
unsigned long int ii;
char         ch[4];
      } tmp;
char ch0, ch2; 

tmp.ii = *i;
ch0 = tmp.ch[0]; 
tmp.ch[0] = tmp.ch[1]; 
tmp.ch[1] = ch0; 
ch2 = tmp.ch[2]; 
tmp.ch[2] = tmp.ch[3]; 
tmp.ch[3] = ch2; 
*i = tmp.ii;
}
/**************************************************************
* Inversion of an unsigned long integer
* From [0 1 2 3] to [3 2 1 0]
* -> Very bad!
***************************************************************/
void rbdf_swap_long2(i)
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

/* For IBM ul8820, um8901, etc ...*/
#define sun4_Solaris
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

#elif defined( alpha_OSF1 ) 

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
      ans.unsgn = ( vms->f4 ) + ( vms->f3 << 8 ) + ( vms->f2 << 16 ) + 
                  ( vms->f1 << 24 );
      
      return( ans.sgn );

}
/*******************************************************************
* Read descriptors
*
*******************************************************************/
int bdf_rdescr(header,nvalues,keyword,cvalue)
char *header,*keyword,*cvalue;
int nvalues;
{
register int i, k;
int status, ilen;
char *pc1, *pc2;

status = 1;

ilen = strlen(keyword);

#ifdef DEBUG
printf(" Looking for keyword: %s (lentgh=%d) \n",keyword,ilen);
#endif
for(i = 0; i < nvalues; i++)
{
if(!strncmp(&header[i],keyword,ilen))
   {
   pc1 = &header[i];
   status = 0;
   break;
   }
}

/* Looking for the value now: */
if(!status)
  {
#ifdef DEBUG
   printf("OK, keyword >%s< has been found\n",pc1);
#endif
/* Should add one, since sometimes there is an "extra" zero at the end: */
   i += (ilen + 1);
   pc2 = &header[i];
/* Looking for first zero: */
   while(!(*pc2)) pc2++;
   strcpy(cvalue,pc2);
   printf(" %s = >%s< \n",pc1,cvalue);
  }

/* Special processing for TITLE:  TITLE\0 \0 actual comments \0\0\0... */
if(!strncmp(keyword,"TITLE",5) && cvalue[1] == '\0')
   {
   pc2++; pc2++;
   strcpy(cvalue,pc2);
   printf(" %s = >%s< \n",pc1,cvalue);
   }

return(status);
}
