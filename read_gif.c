/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
* Program rtiff to read GIF format 
* Uses "xv" source code
*
* JLP
* Version 14-04-97
-----------------------------------------------------------------------*/

/*
#define DEBUG
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <jlp_ftoc.h>

typedef unsigned char byte;
/* info structure filled in by the LoadXXX() image reading routines */
typedef struct { byte *pic;                  /* image data */
                 int   w, h;                 /* pic size */
                 int   type;                 /* PIC8 or PIC24 */
                 byte  r[256],g[256],b[256]; /* colormap, if PIC8 */
                 int   normw, normh;         /* 'normal size' of image file
                                                (normally eq. w,h, except when
                                                doing 'quick' load for icons */
                 int   frmType;              /* def. Format type to save in */
                 int   colType;              /* def. Color type to save in */
                 char  fullInfo[128];        /* Format: field in info box */
                 char  shrtInfo[128];        /* short format info */
                 char *comment;              /* comment text */
                 int   numpages;             /* # of page files, if >1 */
                 char  pagebname[64];        /* basename of page files */
               } PICINFO;

main(argc,argv)
int argc;
char *argv[];
{
char in_name[61], out_name[61], comments[81];
float *real_array;
int   height, width, depth, datatype, cmaptype, cmaplength;
int red, green, blue;
int nx, ny, status, iformat;
register int i;
PICINFO *pinfo;

printf(" Program read_gif to read GIF image files\n");
printf(" JLP/xv Version 14-04-97 \n");

/* One, three or four parameters only are allowed to run the program: */
/* Carefull: 7 parameters always, using JLP "runs" */
if(argc == 2)
  {
  printf(" Fatal: Syntax error: argc=%d\n",argc);
  exit(-1);
  }

/* Interactive input of parameters: */
if (argc == 1)
 { 
  printf(" Syntax: rtiff in_file out_file\n\n"); 
  printf(" Input file := ");scanf("%s",in_name);
  printf(" Output file := ");scanf("%s",out_name);
 }
else
 {
  strcpy(in_name,argv[1]);
  strcpy(out_name,argv[2]);
 }

JLP_BEGIN();
JLP_INQUIFMT();


#ifdef DEBUG
printf(" OK, will read >%s< \n",in_name);
#endif


pinfo = (PICINFO *)malloc(sizeof(PICINFO));
if(pinfo == NULL) {printf("Error allocating memory for PICINFO pinfo\n");
                   exit(-1);}

status = LoadGIF(in_name,pinfo);

height=100; width = 100;
printf("height=%d width=%d \n",height,width);

real_array = (float *)malloc(height*width*sizeof(float));

if(!status)
  {

#ifdef DEBUG
for(i=0; i<100; i++) printf(" a[%d]: %f ",i,real_array[i]);
#endif

  printf(" Conversion of GIF image:  %s    to:    %s \n",in_name,out_name);
  sscanf(comments," From  %s",in_name);
/*
  JLP_WRITEIMAG(real_array,&nx,&ny,&nx,out_name,comments);
*/
  }
else
  {
  }

free(real_array);

JLP_END();
}
