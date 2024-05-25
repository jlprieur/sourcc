/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
* Program rcp40tbl to read CP40 format (from "tar" written by a Macintosh) 
* JLP 
* Version 04-07-95
--------------------------------------------------------------------*/

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
#define NX_CP40  512 
#define NY_CP40  512 
/* Origin of date is 01-01-1904 at 0H TU
 which corresponds to Julian day 2416480.500 */
#define DATE_ORIGIN 2416480.50

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <jlp_ftoc.h>

/* LK format: */
#include "lk_fmt2.h"
camera *quelle_CP40;

main(argc,argv)
int argc;
char *argv[];
{
char in_name[61], out_name[61], comments[81];
float *real_array;
int isize, nx, ny, status, iformat;
int i;

printf(" Program rcar to read CP40 files \n");
printf(" JLP Version 04-07-95 \n");

/* One or three parameters only are allowed to run the program: */
/* Carefull: 7 parameters always, using JLP "runs" */
if(argc == 2)
  {
  printf(" Syntax: rcp40tbl in_file out_file \n\n"); 
  printf(" Fatal: Syntax error: argc=%d\n",argc);
  exit(-1);
  }

/* Interactive input of parameters: */
if (argc == 1)
 { 
  printf(" Syntax: rcp40tbl in_file out_file \n\n"); 
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


/* Set camera type: */
quelle_CP40 = malloc(30*sizeof(int));
set_CP4T(quelle_CP40);

/***************************************************************/
/* Set size (always the same ...) */
nx = NX_CP40; ny = NY_CP40;
printf(" Output image size: nx = %d ny = %d \n",nx,ny);

/* Allocate memory space for output image: */
isize = nx * ny * sizeof(float);
if(!(real_array = (float*)malloc(isize) ) )
  {printf("rdcar/fatal error: no memory space available! (isize=%d)\n",isize);
   return(-1);
  }
for(i = 0; i < nx * ny; i++) real_array[i] = 0.; 

#ifdef DEBUG
printf(" OK, will read >%s< \n",in_name);
#endif

/* Calling reading subroutine: */
  status = rdcp40(real_array,nx,ny,in_name,comments);

if(!status)
  {
  printf(" Conversion of CP40 file:  %s    to:    %s \n",in_name,out_name);
  JLP_WRITEIMAG(real_array,&nx,&ny,&nx,out_name,comments);
  }

JLP_END();
}
/****************************************************************/
/*
Format of CP40 data 
*/
/****************************************************************/
/* Subroutine rdcp40 to read CP40 format */
int rdcp40(real_array,nx,ny,in_name,comments)
float real_array[];
int nx, ny;
char in_name[], comments[];
{
FILE *fd;
float date, integ;
int nbytes_to_read, nbytes, nvalues, nblock, mode_test; 
long ix, iy, not_found, iphot, lk_nphot, nb_images;
char *buffer, keyword[9];
unsigned long s_date, integ_time, nphot;
int i, j, k;
long itest, istart;
char ctest[5];
phot_buf_rec *photon_buffer;

/* Header of 32 bytes */
typedef struct {
unsigned long integ_time;          /* duree en msec */
unsigned long date;                /* date of observation 
                             (in seconds starting from 01-01-1904 at 0H TU)
                             (i.e. Julian day 2416480.500) */
unsigned long nber_of_photons;     /* number of photons */
long keyword1;            /* "FORM" in ASCII */
long keyword2;            /* "CP4T" in ASCII */
long nber_of_images;      /* number of images */
short refNum;             /* always 0 */
long read_already;        /* not used */ 
short everything_read;    /* not used */
} LK_HEADER;

union{
long lg[PHOT_BUF_SIZE/sizeof(long)];
char ch[PHOT_BUF_SIZE];
} buff;

LK_HEADER *chead;
void swap_lint(), inv_lint();

strcpy(keyword,"FORMCP4T");

photon_buffer = malloc(30*sizeof(float));

#ifdef DEBUG
printf(" \n rdcar/reading file : %s \n",in_name);
#endif

/* Opens the input file */
if((fd = fopen(in_name,"r")) == NULL)
  {
  printf("rdcar/error opening input file: >%s< \n",in_name);
  return(-1);
  }

/***************************************************************/
/* Read header (file starts with "FORMCP4T", generally in the second
   block of 512 bytes): */
  nbytes_to_read = PHOT_BUF_SIZE;
  k=0;
  not_found = 1;
  while(not_found && k < 5)
  {
   nbytes = fread(buff.ch,sizeof(char),nbytes_to_read,fd);
#ifdef DEBUG
   printf("        %d bytes read \n", nbytes);
#endif
   if(nbytes != nbytes_to_read)
    {
     printf("rdcar/error reading header: \n");
     printf("       Only %d bytes read \n", nbytes);
     return(-2);
    }

/* Decode header: first look for 'F', 
   then compares the following letters to the keyword FORMCP4T */
  for(i = 0; i < nbytes; i++) 
    {
    if(buff.ch[i] == 'F')
      {
      buffer = &buff.ch[i];
      for(j = 0; j < 8; j++) 
         if(buffer[j] != keyword[j]) break; 
#ifdef DEBUG
         printf(" i=%d j=%d buffer=%s \n",i,j,buffer);
#endif
/* OK: Successfull research: */
      if(j == 8) 
        {
         not_found = 0; istart = i - 12; 
         chead = (LK_HEADER *)&(buff.ch[istart]);
#ifdef DEBUG
         printf(" i=%d, istart=%d block #%d (of %d bytes) \n",i,istart,k,PHOT_BUF_SIZE);
#endif
/* Just to exit from the loop: */
         i = nbytes + 1; break;
        }
      } 
    }

/* Goes to next block */
 k++;
 }

/*************************************************************/
/* Check if it is OK: */
 if(not_found) 
  {
  printf(" Sorry, keyword has not been found \n");
  return(-1);
  }
 else
   {
   printf(" OK, header found \n");

/*************** date ****************************************/
   s_date = chead->date;
/* When working with DEC computer, reverses long integers... */
#ifdef dec
   inv_lint(&s_date);
#endif
/* s_date was in seconds, I convert it to days: */
   date = (float)s_date / 86400.;
/* Then I subtract 2449199.500 which corresponds to the 31-07-93 */
    date = date - (2449199.500 - DATE_ORIGIN);
   printf(" date =%.4f-08-93\n",date);

/*************** integration time *******************************/
   integ_time = chead->integ_time;
#ifdef dec
   inv_lint(&integ_time);
#endif
/* integration time was in milliseconds, I convert it to seconds: */
   integ = (float)integ_time / 1000.;
   printf(" integration time =%f (sec) \n",integ);

/*************** number of photons *******************************/
   nphot =chead->nber_of_photons;
#ifdef dec
   inv_lint(&nphot);
#endif
   printf(" nphot =%u (photons)\n",nphot);
   }

/***************************************************************/
/* Shift values of array buff.ch to the origin: */ 
   nvalues = PHOT_BUF_SIZE - (istart + 32);
   for(i = 0; i < nvalues; i++) buff.ch[i] = buff.ch[i + istart + 32];  
/* Going back to long values: */
   nvalues = nvalues/sizeof(long);


/***************************************************************/
/* Read the data: */
iphot = 0;
lk_nphot = 0;
nb_images = 0;
mode_test = 0;
/* Assume that less than MAX_IMAGES images within a block: */
photon_buffer->debuts_image = malloc(MAX_IMAGES*sizeof(float));

for(nblock = 0; nblock < NBLOCKMAX; nblock++)
{
/* Set photon_buffer data pointer and number of values: */
photon_buffer->adr = buff.lg;
photon_buffer->photons = nvalues;

/* Displays current value of nblock: */
#ifdef DEBUG
  if((nblock % 500) == 1)
     printf(" Block #%d  of %d bytes\n",nblock,PHOT_BUF_SIZE);
#else
  if((nblock % 1000) == 1)
     printf(" Block #%d  of %d bytes\n",nblock,PHOT_BUF_SIZE);
#endif


CP40_vers_YX9M(photon_buffer, quelle_CP40, real_array, nx, ny, &iphot, mode_test);
lk_nphot += photon_buffer->photons;
nb_images += photon_buffer->nb_images;

/* End of current block: read next values */
  nvalues = fread(buff.lg,sizeof(long),PHOT_BUF_SIZE/sizeof(long),fd);
  if(nvalues != PHOT_BUF_SIZE/sizeof(long))
          printf(" rdcp40/Warning, only %d values read in block #%d\n",
                   nvalues,nblock);
  if(nvalues <= 0 )
  {
  printf(" rdcp40/end of file: >%s< \n",in_name);
  nblock = NBLOCKMAX;
  }
/* Going to process these data */
}

/* Exit before reaching nphot: */
printf(" Sorry only %d photons read , lk_nphot = %d (should have been %d photons)\n",
         iphot,lk_nphot, nphot);

/* Closes the input file */
my_end:
  fclose(fd);
/*
  sprintf(comments,"%s date=%.1f-08-93 integ=%.2fs nphot=%d",
          in_name,date,integ,iphot);
*/
  sprintf(comments,"%s integ=%.2fs nphot=%d",
          in_name,integ,iphot);
  printf("actual exposure=%.2fs nphot=%d nb_images=%d equiv. exposure=%.2fs \n",
          integ,iphot,nb_images,0.02*(float)nb_images);
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
* Swap two halves of a lont integer
***************************************************************/
void swap_lint(i)
unsigned long *i;
{
union {
unsigned long ii;
short         ch[2];
      } tmp;
short ch0; 

tmp.ii = *i;

ch0 = tmp.ch[0]; 
tmp.ch[0] = tmp.ch[1]; 
tmp.ch[1] = ch0; 
*i = tmp.ii;

}
/**************************************************************
* Inversion of an unsigned long integer
* From [0 1 2 3] to [3 2 1 0]
***************************************************************/
void inv_lint(i)
unsigned long *i;
{
union {
unsigned long ii;
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
