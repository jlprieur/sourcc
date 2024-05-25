/*********************************************************************
*
* Contains:
* int read_header_gif(filename,height,width,depth,datatype,cmaptype,cmaplength)
* int c_read_raster_gif(image,height,width,cmaplength,red,green,blue,filename)
* int i_read_raster_gif(image,height,width,cmaplength,red,green,blue,filename)
* int f_read_raster_gif(image,height,width,cmaplength,red,green,blue,filename)
* int gif_decode(fp,raster,height,width,top,left)
*
* From Eric Anterrieu
* JLP Version 02-03-95
**********************************************************************/
#include <stdio.h>
#include <malloc.h>
#include <string.h>

typedef unsigned char  byte;    /* characters are supposed to be unsigned */
/* JLP 97
#define MAGIC          "GIF87a" 
*/
#define MAGIC          "GIF89a" /* gif signature */
#define IMAGESEP       0x2c     /* image separator: ',' */
#define BLOCKSEP       0x21     /* block separator: '!' */
#define GIFTERM        0x3b     /* gif terminator:  ';' */
#define INTERLACEMASK  0x40
#define COLORMAPMASK   0x80
#define SEQUENTIALDATA 1
#define INTERLACEDDATA 2
#define GLOBALCOLORMAP 1
#define LOCALCOLORMAP  2

typedef struct codestruct 
           {
           struct codestruct  *prefix;
           byte               first, suffix;      
           } codetype;

static codetype CodeTab[4096];
static int      InitBits, ClearCode, EOFCode, CodeMask;
static int      AvailCode, OldCode, Code, CodeSize;
static int      gif_process();
static void     gif_outcode();

#define MAIN
#ifdef MAIN
main(argc,argv)
char **argv;
int argc;
{
exit(0);
}
#endif
/*****************************************************************************
*
* FUNCTION: read_header_gif
*
* PURPOSE: Reads the header of a gifrasterfile.
*
* INPUT:  filename = rasterfile name
*
* OUTPUT: height = number of lines
*         width = number of pixels per line
*         depth = number of bits per pixel
*         datatype = type of data
*         cmaptype = type of colormap
*         cmaplength = length of colormap
*
* RETURN: 0 if everything OK
*         -1 if unable to open rasterfile
*         -2 if unable to close rasterfile
*         -3 if error encoutered while reading stream
*         -4 if unexpected rasterfile format: bad magic number
*         -5 if unexpected rasterfile format: no colormap found
*
* VERSION: December 1992
*
* AUTHOR: Eric ANTERRIEU
*
*******************************************************************************/
int   read_header_gif(filename,height,width,depth,datatype,cmaptype,cmaplength)
char  *filename;
int   *height, *width, *depth, *datatype, *cmaptype, *cmaplength;
{
auto  FILE   *fp;
auto  byte   buf[16];
auto  int    err, c;
auto  int    screenwidth, screenheight;
auto  int    imageleft, imagetop, imagewidth, imageheight, interleaved;
auto  int    cmapislocal, localbits, localcolor;
auto  int    cmapisglobal, globalbits, globalcolor;

if ((fp = fopen(filename,"r")) == NULL) return(-1);

err = fread(buf,sizeof(byte),6,fp);
if (err != 6) {fclose(fp); return(-3);}
buf[6] = '\0';
if (strncmp(buf,MAGIC,6)) 
       { printf(" buff=%6s whereas expected name is %s \n",buf,MAGIC);
        fclose(fp); return(-4);}

err = fread(buf,sizeof(byte),7,fp);
if (err != 7) {fclose(fp); return(-3);}
screenwidth  = buf[0] + (buf[1] << 8);  /* buf[0] + 256*buf[1] */
screenheight = buf[2] + (buf[3] << 8);  /* buf[2] + 256*buf[3] */
/* JLP97: */
#ifdef DEBUG
printf("screenwidth=%d screenheight=%d \n",screenwidth,screenheight);
#endif

cmapisglobal = buf[4] & COLORMAPMASK;  /* buf[4] & 10000000 */
if (cmapisglobal != 0) 
  {
  globalbits = (buf[4] & 0x07) + 1;   /* 1+buff[4] & 00001111 */
  globalcolor = 1<<globalbits;        /* 2^globalbits */
  for (c=0; c<globalcolor; c++)
    {
    err = fread(buf,sizeof(byte),3,fp);
    if (err != 3) {fclose(fp); return(-3);}
    }
  }

err = fread(buf,sizeof(byte),1,fp);
if (err != 1) {fclose(fp); return(-3);}
/* JLP97
if (buf[0] != IMAGESEP) 
*/
if (buf[0] != IMAGESEP && buf[0] != '!') 
    {printf("Wrong image separator %c (should have been %c)\n", buf[0],IMAGESEP); 
     fclose(fp); return(-4);}

err = fread(buf,sizeof(byte),9,fp);
if (err != 9) {fclose(fp); return(-3);}
imageleft   = buf[0] + (buf[1] << 8);  /* buf[0] + 256*buf[1] */
imagetop    = buf[2] + (buf[3] << 8);  /* buf[2] + 256*buf[3] */
imagewidth  = buf[4] + (buf[5] << 8);  /* buf[4] + 256*buf[5] */
imageheight = buf[6] + (buf[7] << 8);  /* buf[6] + 256*buf[7] */

/* JLP97: */
#ifdef DEBUG
printf("imagewidth=%d imageheight=%d \n",imagewidth,imageheight);
#endif

cmapislocal = buf[8] & COLORMAPMASK;
if (cmapislocal != 0) 
  {
  localbits = (buf[8] & 0x7) + 1;   /* 1+buff[8] & 00001111 */
  localcolor = 1<<localbits;        /* 2^globalbits */
  } 

if ((cmapislocal == 0) && (cmapisglobal == 0)) 
           {fclose(fp); *cmaptype = -1; return(-5);}

interleaved = buf[8] & INTERLACEMASK;

if (fclose(fp) == EOF) return(-2);

/* JLP97
if (width != NULL) *width  = imagewidth;
if (height != NULL) *height = imageheight;
*/
    *width  = screenwidth;
    *height = screenheight;

if (cmapisglobal != 0)
  {
  if (depth != NULL) *depth = globalbits;
  if (cmaplength != NULL) *cmaplength = globalcolor;
  if (cmaptype != NULL) *cmaptype = GLOBALCOLORMAP;
  }
if (cmapislocal != 0)
  {
  if (depth != NULL) *depth = localbits;
  if (cmaplength != NULL) *cmaplength = localcolor;
  if (cmaptype != NULL) *cmaptype = LOCALCOLORMAP;
  }
if (datatype != NULL) 
  {
  if (interleaved == 0) *datatype = SEQUENTIALDATA;  
      else *datatype = INTERLACEDDATA;
  }

return(0);
}
/*****************************************************************************
*
* FUNCTION: c_read_raster_gif
*
* PURPOSE: Reads a gifrasterfile.
*
* INPUT:  filename = rasterfile name
*         height = number of lines
*         width = number of pixels per line
*         cmaplength = length of colormap
*
* OUTPUT: image[0..height-1][0..width-1] = char raster image
*         red[0..cmaplength-1] = red components of colormap table
*         green[0..cmaplength-1] = green components of colormap table
*         blue[0..cmaplength-1] = blue components of colormap table
*
* RETURN: 0 if everything OK
*         -1 if unable to open rasterfile
*         -2 if unable to close rasterfile
*         -3 if error encoutered while reading stream
*         -4 if unexpected rasterfile format: bad magic number
*         -5 if unexpected rasterfile format: no colormap found
*         -6 if unexpected rasterfile format: bad data format
*         -7 if unexpected parameter: bad dimension of data or colormap
*         -8 if memory allocation failure
*
* VERSION: December 1992
*
* AUTHOR: Eric ANTERRIEU
*
*******************************************************************************/
int    c_read_raster_gif(image,height,width,cmaplength,red,green,blue,filename)
char   *filename;
int    height, width, cmaplength, *red, *green, *blue;
char   **image;
{
int    i, j, k = 0;
auto     FILE   *fp;
auto     byte   buf[16], colormap[256][3];
auto     char   *raster;
auto     int    err, c, screenwidth, screenheight, cmaptype;
auto     int    imageleft, imagetop, imagewidth, imageheight, datatype;
auto     int    cmapislocal, localbits, localcolor = 0;
auto     int    cmapisglobal, globalbits, globalcolor = 0;
/* JLP97
extern   void   c_matrix_free();
extern   char   **c_matrix_alloc();
*/
extern   int    gif_decode();

/* open file
*/
if ((fp = fopen(filename,"r")) == NULL) return(-1);

/* read GIF signature (6 bytes)
*/
err = fread(buf,sizeof(byte),6,fp);
if (err != 6) {fclose(fp); return(-3);}
buf[6] = '\0';
if (strncmp(buf,MAGIC,6)) 
       { printf(" buff=%6s whereas expected name is %s \n",buf,MAGIC);
         fclose(fp); return(-4);}

/* read screen descriptor (7 bytes)
*/
err = fread(buf,sizeof(byte),7,fp);
if (err != 7) {fclose(fp); return(-3);}
screenwidth  = buf[0] + (buf[1] << 8);  /* buf[0] + 256*buf[1] */
screenheight = buf[2] + (buf[3] << 8);  /* buf[2] + 256*buf[3] */
cmapisglobal = buf[4] & COLORMAPMASK;  /* buf[4] & 10000000 */

/* read the global colormap (3*globalcolor bytes)
*/
if (cmapisglobal != 0)
  {
  globalbits = (buf[4] & 0x07) + 1;   /* 1+buff[4] & 00001111 */
  globalcolor = 1<<globalbits;        /* 2^globalbits */
  err = fread(colormap,3*sizeof(byte),globalcolor,fp);
  if (err != globalcolor) {fclose(fp); return(-3);} 
  }

/* read an image separator (1 byte)
*/
err = fread(buf,sizeof(byte),1,fp);
if (err != 1) {fclose(fp); return(-3);}
/* JLP97
if (buf[0] != IMAGESEP) 
*/
if (buf[0] != IMAGESEP && buf[0] != '!') 
    {printf("Wrong image separator %c (should have been %c)\n", buf[0],IMAGESEP); 
     fclose(fp); return(-4);}

/* read the image descriptor (9 bytes)
*/
err = fread(buf,sizeof(byte),9,fp);
if (err != 9) {fclose(fp); return(-3);}
imageleft   = buf[0] + (buf[1] << 8);  /* buf[0] + 256*buf[1] */
imagetop    = buf[2] + (buf[3] << 8);  /* buf[2] + 256*buf[3] */
imagewidth  = buf[4] + (buf[5] << 8);  /* buf[4] + 256*buf[5] */
imageheight = buf[6] + (buf[7] << 8);  /* buf[6] + 256*buf[7] */
if (imagewidth != width) {fclose(fp); return(-7);}
if (imageheight != height) {fclose(fp); return(-7);}
cmapislocal = buf[8] & COLORMAPMASK;
if ((buf[8] & INTERLACEMASK) == 0)  datatype = SEQUENTIALDATA;  
else  datatype = INTERLACEDDATA;

/* read the local colormap (3*localcolor bytes)
*/
if (cmapislocal != 0)
  {
  localbits = (buf[8] & 0x7) + 1;   /* 1+buff[8] & 00001111 */
  localcolor = 1<<localbits;        /* 2^globalbits */
  err = fread(colormap,3*sizeof(byte),localcolor,fp);
  if (err != 3*localcolor) {fclose(fp); return(-3);} 
  }

if ((cmapislocal == 0) && (cmapisglobal == 0)) 
   {fclose(fp); return(-5);}
if ((cmaplength != globalcolor) && (cmaplength != localcolor)) 
   {fclose(fp); return(-7);}

/* store the colormap
*/
if ((red != NULL) && (green != NULL) && (blue != NULL))
  {
  for (c=0; c<cmaplength; c++)
    {
    red[c]   = colormap[c][0];
    green[c] = colormap[c][1];
    blue[c]  = colormap[c][2];
    }
  }

/* read and decompress the image (<< imagewidth*imageheight bytes!)
*/
/* JLP97
raster = c_vector_alloc(imageheight*imagewidth);
*/
raster = (char *)malloc(height*width*sizeof(char));
if (raster == NULL) 
      {printf("image height= %d width = %d\n",height,width);
       return(-8);
      }

err = gif_decode(fp,raster,height,width,imagetop,imageleft);
if (err  < 0) {
/* JLP97
              c_vector_free(raster); 
*/
              free(raster);
              fclose(fp); return(-6);}
if (datatype == SEQUENTIALDATA)
  {
  for (i=0; i<height; i++)
  for (j=0; j<width; j++) image[height-1-i][j] = raster[k++];
  }
if (datatype == INTERLACEDDATA)
  {
  for (i=0; i<height; i += 8)
  for (j=0; j<width; j++) image[height-1-i][j] = raster[k++];
  for (i=4; i<height; i += 8)
  for (j=0; j<width; j++) image[height-1-i][j] = raster[k++];
  for (i=2; i<height; i += 4)
  for (j=0; j<width; j++) image[height-1-i][j] = raster[k++];
  for (i=1; i<height; i += 2)
  for (j=0; j<width; j++) image[height-1-i][j] = raster[k++];
  }
/* JLP97
c_vector_free(raster);
*/
  free(raster);

/* close the file
*/
if (fclose(fp) == EOF) return(-2);
}

/******************************************************************************
*
* FUNCTION: i_read_raster_gif
*
* PURPOSE: Reads a gifrasterfile.
*
* INPUT:  filename = rasterfile name
*         height = number of lines
*         width = number of pixels per line
*         cmaplength = length of colormap
*
* OUTPUT: image[0..height-1][0..width-1] = int raster image
*         red[0..cmaplength-1] = red components of colormap table
*         green[0..cmaplength-1] = green components of colormap table
*         blue[0..cmaplength-1] = blue components of colormap table
*
* RETURN: 0 if everything OK
*         -1 if unable to open rasterfile
*         -2 if unable to close rasterfile
*         -3 if error encoutered while reading stream
*         -4 if unexpected rasterfile format: bad magic number
*         -5 if unexpected rasterfile format: no colormap found
*         -6 if unexpected rasterfile format: bad data format
*         -7 if unexpected parameter: bad dimension of data or colormap
*         -8 if memory allocation failure
*
* VERSION: December 1992
*
* AUTHOR: Eric ANTERRIEU
*
*******************************************************************************/
int    i_read_raster_gif(image,height,width,cmaplength,red,green,blue,filename)
char   *filename;
int    height, width, cmaplength, *red, *green, *blue;
int    **image;
{
int    i, j, k = 0;
auto     FILE   *fp;
auto     byte   buf[16], colormap[256][3];
auto     char   *raster;
auto     int    err, c, screenwidth, screenheight, cmaptype;
auto     int    imageleft, imagetop, imagewidth, imageheight, datatype;
auto     int    cmapislocal, localbits, localcolor = 0;
auto     int    cmapisglobal, globalbits, globalcolor = 0;
/* JLP97
extern   void   c_matrix_free();
extern   char   **c_matrix_alloc();
*/
extern   int    gif_decode();

/* open file
*/
if ((fp = fopen(filename,"r")) == NULL) return(-1);

/* read GIF signature (6 bytes)
*/
err = fread(buf,sizeof(byte),6,fp);
if (err != 6) {fclose(fp); return(-3);}
buf[6] = '\0';
if (strncmp(buf,MAGIC,6)) 
       { printf(" buff=%6s whereas expected name is %s \n",buf,MAGIC);
         fclose(fp); return(-4);}

/* read screen descriptor (7 bytes)
*/
err = fread(buf,sizeof(byte),7,fp);
if (err != 7) {fclose(fp); return(-3);}
screenwidth  = buf[0] + (buf[1] << 8);  /* buf[0] + 256*buf[1] */
screenheight = buf[2] + (buf[3] << 8);  /* buf[2] + 256*buf[3] */
cmapisglobal = buf[4] & COLORMAPMASK;  /* buf[4] & 10000000 */

/* read the global colormap (3*globalcolor bytes)
*/
if (cmapisglobal != 0)
  {
  globalbits = (buf[4] & 0x07) + 1;   /* 1+buff[4] & 00001111 */
  globalcolor = 1<<globalbits;        /* 2^globalbits */
  err = fread(colormap,3*sizeof(byte),globalcolor,fp);
  if (err != globalcolor) {fclose(fp); return(-3);} 
  }

/* read an image separator (1 byte)
*/
err = fread(buf,sizeof(byte),1,fp);
if (err != 1) {fclose(fp); return(-3);}
/* JLP97
if (buf[0] != IMAGESEP) 
*/
if (buf[0] != IMAGESEP && buf[0] != '!') 
    {printf("Wrong image separator %c (should have been %c)\n", buf[0],IMAGESEP); 
     fclose(fp); return(-4);}

/* read the image descriptor (9 bytes)
*/
err = fread(buf,sizeof(byte),9,fp);
if (err != 9) {fclose(fp); return(-3);}
imageleft   = buf[0] + (buf[1] << 8);  /* buf[0] + 256*buf[1] */
imagetop    = buf[2] + (buf[3] << 8);  /* buf[2] + 256*buf[3] */
imagewidth  = buf[4] + (buf[5] << 8);  /* buf[4] + 256*buf[5] */
imageheight = buf[6] + (buf[7] << 8);  /* buf[6] + 256*buf[7] */
if (imagewidth != width) {fclose(fp); return(-7);}
if (imageheight != height) {fclose(fp); return(-7);}
cmapislocal = buf[8] & COLORMAPMASK;
if ((buf[8] & INTERLACEMASK) == 0)  datatype = SEQUENTIALDATA;  
else  datatype = INTERLACEDDATA;

/* read the local colormap (3*localcolor bytes)
*/
if (cmapislocal != 0)
  {
  localbits = (buf[8] & 0x7) + 1;   /* 1+buff[8] & 00001111 */
  localcolor = 1<<localbits;        /* 2^globalbits */
  err = fread(colormap,3*sizeof(byte),localcolor,fp);
  if (err != 3*localcolor) {fclose(fp); return(-3);} 
  }

if ((cmapislocal == 0) && (cmapisglobal == 0)) {fclose(fp); return(-5);}
if ((cmaplength != globalcolor) && (cmaplength != localcolor)) 
   {fclose(fp); return(-7);}

/* store the colormap
*/
if ((red != NULL) && (green != NULL) && (blue != NULL))
  {
  for (c=0; c<cmaplength; c++)
    {
    red[c]   = colormap[c][0];
    green[c] = colormap[c][1];
    blue[c]  = colormap[c][2];
    }
  }

/* read and decompress the image (<< imagewidth*imageheight bytes!)
*/
/* JLP97
raster = c_vector_alloc(imageheight*imagewidth);
*/
raster = malloc(height*width*sizeof(char));
if (raster == NULL) 
     {printf("height=%d width=%d \n",height,width);
     return(-8);
     }

err = gif_decode(fp,raster,height,width,imagetop,imageleft);
if (err  < 0) { 
/* JLP97
              c_vector_free(raster); 
*/
              free(raster);
              fclose(fp); return(-6);}
if (datatype == SEQUENTIALDATA)
  {
  for (i=0; i<height; i++)
  for (j=0; j<width; j++) image[height-1-i][j] = ((int) raster[k++]);
  }
if (datatype == INTERLACEDDATA)
  {
  for (i=0; i<height; i += 8)
  for (j=0; j<width; j++) image[height-1-i][j] = ((int) raster[k++]);
  for (i=4; i<height; i += 8)
  for (j=0; j<width; j++) image[height-1-i][j] = ((int) raster[k++]);
  for (i=2; i<height; i += 4)
  for (j=0; j<width; j++) image[height-1-i][j] = ((int) raster[k++]);
  for (i=1; i<height; i += 2)
  for (j=0; j<width; j++) image[height-1-i][j] = ((int) raster[k++]);
  }
/* JLP97
c_vector_free(raster);
*/
free(raster);

/* close the file
*/
if (fclose(fp) == EOF) return(-2);
}
/*****************************************************************************
*
* FUNCTION: f_read_raster_gif
*
* PURPOSE: Reads a gifrasterfile.
*
* INPUT:  filename = rasterfile name
*         height = number of lines
*         width = number of pixels per line
*         cmaplength = length of colormap
*
* OUTPUT: image[0..height-1][0..width-1] = float raster image
*         red[0..cmaplength-1] = red components of colormap table
*         green[0..cmaplength-1] = green components of colormap table
*         blue[0..cmaplength-1] = blue components of colormap table
*
* RETURN: 0 if everything OK
*         -1 if unable to open rasterfile
*         -2 if unable to close rasterfile
*         -3 if error encoutered while reading stream
*         -4 if unexpected rasterfile format: bad magic number
*         -5 if unexpected rasterfile format: no colormap found
*         -6 if unexpected rasterfile format: bad data format
*         -7 if unexpected parameter: bad dimension of data or colormap
*         -8 if memory allocation failure
*
* VERSION: December 1992
*
* AUTHOR: Eric ANTERRIEU
*
*******************************************************************************/
int    f_read_raster_gif(image,height,width,cmaplength,red,green,blue,filename)
char   *filename;
int    height, width, cmaplength, *red, *green, *blue;
float  *image;
{
int    i, j, k = 0;
auto     FILE   *fp;
auto     byte   buf[16], colormap[256][3];
auto     char   *raster;
auto     int    err, c, screenwidth, screenheight, cmaptype;
auto     int    imageleft, imagetop, imagewidth, imageheight, datatype;
auto     int    cmapislocal, localbits, localcolor = 0;
auto     int    cmapisglobal, globalbits, globalcolor = 0;
/*
extern   void   c_matrix_free();
extern   char   **c_matrix_alloc();
*/
extern   int    gif_decode();

/* open file
*/
if ((fp = fopen(filename,"r")) == NULL) return(-1);

/* read GIF signature (6 bytes)
*/
err = fread(buf,sizeof(byte),6,fp);
if (err != 6) {fclose(fp); return(-3);}
buf[6] = '\0';
if (strncmp(buf,MAGIC,6)) 
       { printf(" buff=%6s whereas expected name is %s \n",buf,MAGIC);
         fclose(fp); return(-4);}

/* read screen descriptor (7 bytes)
*/
err = fread(buf,sizeof(byte),7,fp);
if (err != 7) {fclose(fp); return(-3);}
screenwidth  = buf[0] + (buf[1] << 8);  /* buf[0] + 256*buf[1] */
screenheight = buf[2] + (buf[3] << 8);  /* buf[2] + 256*buf[3] */
cmapisglobal = buf[4] & COLORMAPMASK;  /* buf[4] & 10000000 */

/* read the global colormap (3*globalcolor bytes)
*/
if (cmapisglobal != 0)
  {
  globalbits = (buf[4] & 0x07) + 1;   /* 1+buff[4] & 00001111 */
  globalcolor = 1<<globalbits;        /* 2^globalbits */
  err = fread(colormap,3*sizeof(byte),globalcolor,fp);
  if (err != globalcolor) {fclose(fp); return(-3);} 
  }

/* read an image separator (1 byte)
*/
err = fread(buf,sizeof(byte),1,fp);
if (err != 1) {fclose(fp); return(-3);}
/* JLP97
if (buf[0] != IMAGESEP) 
*/
if (buf[0] != IMAGESEP && buf[0] != '!') 
    {printf("Wrong image separator %c (should have been %c)\n", buf[0],IMAGESEP); 
     fclose(fp); return(-4);}

/* read the image descriptor (9 bytes)
*/
err = fread(buf,sizeof(byte),9,fp);
if (err != 9) {fclose(fp); return(-3);}
imageleft   = buf[0] + (buf[1] << 8);  /* buf[0] + 256*buf[1] */
imagetop    = buf[2] + (buf[3] << 8);  /* buf[2] + 256*buf[3] */
imagewidth  = buf[4] + (buf[5] << 8);  /* buf[4] + 256*buf[5] */
imageheight = buf[6] + (buf[7] << 8);  /* buf[6] + 256*buf[7] */
/* JLP97
if (imagewidth != width) {fclose(fp); return(-7);}
if (imageheight != height) {fclose(fp); return(-7);}
*/
cmapislocal = buf[8] & COLORMAPMASK;
if ((buf[8] & INTERLACEMASK) == 0)  datatype = SEQUENTIALDATA;  
else  datatype = INTERLACEDDATA;

/* read the local colormap (3*localcolor bytes)
*/
if (cmapislocal != 0)
  {
  localbits = (buf[8] & 0x7) + 1;   /* 1+buff[8] & 00001111 */
  localcolor = 1<<localbits;        /* 2^globalbits */
  err = fread(colormap,3*sizeof(byte),localcolor,fp);
  if (err != 3*localcolor) {fclose(fp); return(-3);} 
  }

if ((cmapislocal == 0) && (cmapisglobal == 0)) {fclose(fp); return(-5);}
if ((cmaplength != globalcolor) && (cmaplength != localcolor)) {fclose(fp); return(-7);}

/* store the colormap
*/
if ((red != NULL) && (green != NULL) && (blue != NULL))
  {
  for (c=0; c<cmaplength; c++)
    {
    red[c]   = colormap[c][0];
    green[c] = colormap[c][1];
    blue[c]  = colormap[c][2];
    }
  }

/* read and decompress the image (<< imagewidth*imageheight bytes!)
*/
/* JLP97
raster = c_vector_alloc(imageheight*imagewidth);
*/
raster = (char *)malloc(height*width*sizeof(char));
if (raster == NULL) 
     {printf("height=%d width=%d \n",height,width);
     return(-8);
     }

err = gif_decode(fp,raster,height,width,imagetop,imageleft);
if (err  < 0) {
/* JLP97
               c_vector_free(raster); 
*/
               free(raster);
               fclose(fp); return(-6);}
if (datatype == SEQUENTIALDATA)
  {
  for (i=0; i<height; i++)
  for (j=0; j<width; j++) 
/*
         image[imageheight-1-i][j] = ((float) raster[k++]);
*/
       image[width * (height-1-i) + j] = ((float) raster[k++]);
  }
if (datatype == INTERLACEDDATA)
  {
  for (i=0; i<height; i += 8)
    for (j=0; j<width; j++) 
/*
         image[height-1-i][j] = ((float) raster[k++]);
*/
       image[width * (height-1-i) + j] = ((float) raster[k++]);
  for (i=4; i<height; i += 8)
  for (j=0; j<width; j++) 
/*
       image[imageheight-1-i][j] = ((float) raster[k++]);
*/
       image[width * (height-1-i) + j] = ((float) raster[k++]);
  for (i=2; i<height; i += 4)
  for (j=0; j<width; j++) 
/*
       image[height-1-i][j] = ((float) raster[k++]);
*/
       image[width * (height-1-i) + j] = ((float) raster[k++]);
  for (i=1; i<height; i += 2)
  for (j=0; j<width; j++) 
/*
       image[height-1-i][j] = ((float) raster[k++]);
*/
       image[width * (height-1-i) + j] = ((float) raster[k++]);
  }
/* JLP97
c_vector_free(raster);
*/
free(raster);

/* close the file
*/
if (fclose(fp) == EOF) return(-2);
}
/*******************************************************************************
*                                                                             *
* FUNCTION: gif_decode                                                        *
*                                                                             *
* PURPOSE: Decompress data from a gifrasterfile.                              *
*                                                                             *
* INPUT:  fp = pointer to a gif rasterfile open for binary read               *
*         height = number of lines                                            *
*         width = number of pixels per line                                   *
*         top, left = top and left offsets in the screen                      *
*                                                                             *
* OUTPUT: raster[0..height*width-1] = char raster image                       *
*                                                                             *
* RETURN: 0 if everything OK                                                  *
*         -1 if error encoutered while reading stream                         *
*                                                                             *
* VERSION: December 1992                                                      *
*                                                                             *
* AUTHOR: Eric ANTERRIEU from LZW compression algorithm based on compress and *
*         from the SUN<-->GIF converter written by Patrick J. Naughton.       *
*                                                                             *
*******************************************************************************/
int   gif_decode(fp,raster,height,width,top,left)
FILE  *fp;
char  *raster;
int   width, height, top, left;
{
auto      byte   buf[256], *ch, *data;
auto      int    err, Count, datum = 0, Bits = 0;

data = raster;

InitBits  = getc(fp);
ClearCode = 1 << InitBits;
EOFCode   = ClearCode + 1;
CodeSize  = InitBits + 1;
CodeMask  = (1 << CodeSize) - 1;

for (Code=0; Code<ClearCode; Code++) 
  {
  CodeTab[Code].prefix = (codetype*)0;
  CodeTab[Code].first  = Code;
  CodeTab[Code].suffix = Code;
  }

for (Count=getc(fp); Count>0; Count=getc(fp)) 
  {
  err = fread(buf,sizeof(byte),Count,fp);
  if (err != Count) return(-1);

  for (ch=buf; (Count--)>0; ch++) 
    {
    datum += *ch << Bits;
    Bits  += 8;
    while (Bits >= CodeSize) 
      {
      Code  = datum & CodeMask;
      datum >>= CodeSize;
      Bits  -= CodeSize;
      if (Code == EOFCode) goto exitloop; 
      if (gif_process(&data) < 0) return(-1);
      }
    }
  }
exitloop:
if (data != raster + width*height) return(-1);

return(0);
}

/***************************************************************************
* gif_outcode
***************************************************************************/
static void      gif_outcode(CodeTab_p,data)
codetype         *CodeTab_p;
byte             **data;
{
if (CodeTab_p->prefix) gif_outcode(CodeTab_p->prefix,data);
*(*data)++ = CodeTab_p->suffix;
}

/***************************************************************************
* gif_process
***************************************************************************/
static int       gif_process(data)
byte             **data;
{
auto codetype    *CodeTab_p;

if (Code == ClearCode) 
  {
  CodeSize = InitBits + 1;
  CodeMask = (1 << CodeSize) - 1;
  AvailCode = ClearCode + 2;
  OldCode = -1;
  } 
else if (Code < AvailCode) 
  {
  gif_outcode(&CodeTab[Code],data);
  if (OldCode != -1) 
    {
    CodeTab_p = &CodeTab[AvailCode++];
    CodeTab_p->prefix = &CodeTab[OldCode];
    CodeTab_p->first  = CodeTab_p->prefix->first;
    CodeTab_p->suffix = CodeTab[Code].first;
    if ((AvailCode & CodeMask) == 0 && AvailCode < 4096) 
             {CodeSize++; CodeMask += AvailCode;}
    }
  OldCode = Code;
  } 
else if ((Code == AvailCode) && (OldCode != -1)) 
  {
  CodeTab_p = &CodeTab[AvailCode++];
  CodeTab_p->prefix = &CodeTab[OldCode];
  CodeTab_p->first  = CodeTab_p->prefix->first;
  CodeTab_p->suffix = CodeTab_p->first;
  gif_outcode(CodeTab_p,data);
  if ((AvailCode & CodeMask) == 0 && AvailCode < 4096) 
            {CodeSize++; CodeMask += AvailCode;}
  OldCode = Code;
  } 
else return(-1);

return(0);
}
