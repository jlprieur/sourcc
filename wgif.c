/***************************************************************************
*
***************************************************************************/
#include <stdio.h>
#include <malloc.h>
#include <string.h>

typedef unsigned char  byte;    /* characters are supposed to be unsigned */
#define MAGIC          "GIF87a" /* gif signature */
#define IMAGESEP       0x2c     /* image separator: ',' */
#define BLOCKSEP       0x21     /* block separator: '!' */
#define GIFTERM        0x3b     /* gif terminator:  ';' */
#define INTERLACEMASK  0x40
#define COLORMAPMASK   0x80
#define SEQUENTIALDATA 1
#define INTERLACEDDATA 2
#define GLOBALCOLORMAP 1
#define LOCALCOLORMAP  2

/*****************************************************************************
*
* FUNCTION: c_writ_raster_gif
*
* PURPOSE: Creates a gifrasterfile.
*
* INPUT:  filename = rasterfile name
*         height = number of lines
*         width = number of pixels per line
*         image[0..height-1][0..width-1] = char raster image
*         cmaplength = length of colormap
*         red[0..cmaplength-1] = red components of colormap table
*         green[0..cmaplength-1] = green components of colormap table
*         blue[0..cmaplength-1] = blue components of colormap table
*
* RETURN: > 0 (compression ratio) if everything OK
*         -1 if unable to open rasterfile
*         -2 if unable to close rasterfile
*         -3 if error encoutered while writing on stream
*         -5 if unexpected rasterfile format: no colormap passed
*         -7 if unexpected parameter: bad dimension of data or colormap
*         -8 if memory allocation failure
*         -9 if error occurred while encoding data
*
* VERSION: December 1992
*
* AUTHOR: Eric ANTERRIEU
*
*******************************************************************************/
/*******************************************************************************/
int    c_writ_raster_gif(image,height,width,cmaplength,red,green,blue,filename)
char   *filename;
int    height, width;
int    cmaplength, *red, *green, *blue;
char   **image;
/*******************************************************************************/
{
auto    FILE   *fp;
auto    byte   buf[16];
auto    char   **raster;
auto    int    datatype = SEQUENTIALDATA, cmaptype = GLOBALCOLORMAP;
auto    int    err, i, j, c = 1, left = 0, top = 0, depth = 0, ratio;
auto    float  z, zmin, zmax;
extern  void   c_matrix_free();
extern  char   **c_matrix_alloc();
extern  int    gif_encode();

if ((red == NULL) || (green == NULL) || (blue == NULL)) return(-5);

/* compute depth of data from length of colormap
*/
while (c < cmaplength) {c *= 2; depth++;}
if (c != cmaplength) return(-7);

/* open file
*/
if ((fp = fopen(filename,"w")) == NULL) return(-1);

/* write out GIF signature (6 bytes)
*/
(void) sprintf(buf,"%s",MAGIC);
err = fwrite(buf,sizeof(byte),6,fp);
if (err != 6) {fclose(fp); return(-3);}

/* write out screen descriptor (7 bytes)
*/
buf[0] = width & 0xFF;          /* width & 00001111        */
buf[1] = (width >> 8) & 0xFF;   /* (width/256) & 00001111  */
buf[2] = height & 0xFF;         /* height & 00001111       */
buf[3] = (height >> 8) & 0xFF;  /* (height/256) & 00001111 */
buf[4] = 0x00;
if (cmaptype == GLOBALCOLORMAP)  buf[4] = COLORMAPMASK;  
buf[4] |= (depth - 1) << 5; 
buf[4] |= (depth - 1);
buf[5] = 0;
buf[6] = 0;
err = fwrite(buf,sizeof(byte),7,fp);
if (err != 7) {fclose(fp); return(-3);}

/* write out the global colormap (3*cmaplength bytes)
*/
if (cmaptype == GLOBALCOLORMAP)
  {
  for (c=0; c<cmaplength; c++)
    {
    buf[0] = red[c];
    buf[1] = green[c];
    buf[2] = blue[c];
    err = fwrite(buf,sizeof(byte),3,fp);
    if (err != 3) {fclose(fp); return(-3);}
    }
  }

/* write out an image separator (1 byte)
*/
buf[0] = IMAGESEP;
err = fwrite(buf,sizeof(byte),1,fp);
if (err != 1) {fclose(fp); return(-3);}

/* write out the image descriptor (9 bytes)
*/
buf[0] = left & 0xFF;            /* left & 00001111         */
buf[1] = (left >> 8) & 0xFF;     /* (left/256) & 00001111   */
buf[2] = top & 0xFF;             /* top & 00001111          */
buf[3] = (top >> 8) & 0xFF;      /* (top/256) & 00001111    */
buf[4] = width & 0xFF;           /* width & 00001111        */
buf[5] = (width >> 8) & 0xFF;    /* (width/256) & 00001111  */
buf[6] = height & 0xFF;          /* height & 00001111       */
buf[7] = (height >> 8) & 0xFF;   /* (height/256) & 00001111 */
buf[8] = 0x00;
if (cmaptype == LOCALCOLORMAP)  buf[8] = COLORMAPMASK; 
if (datatype == INTERLACEDDATA)  buf[8] |= INTERLACEMASK;
buf[8] |= (depth - 1);
err = fwrite(buf,sizeof(byte),9,fp);
if (err != 9) {fclose(fp); return(-3);}

/* write out the local colormap (3*cmaplength bytes)
*/
if (cmaptype == LOCALCOLORMAP)
  {
  for (c=0; c<cmaplength; c++)
    {
    buf[0] = red[c];
    buf[1] = green[c];
    buf[2] = blue[c];
    err = fwrite(buf,sizeof(byte),3,fp);
    if (err != 3) {fclose(fp); return(-3);}
    }
  }

/* compress and write out the image (<< width*height bytes !)
*/
raster = c_matrix_alloc(height,width);
if (raster == NULL) {fclose(fp); return(-8);}

zmin = zmax = image[0][0];
for (i=0; i<height; i++)
for (j=0; j<width; j++)
  {
  if (image[i][j] < zmin) zmin = image[i][j];
  if (image[i][j] > zmax) zmax = image[i][j];
  }
for (i=0; i<height; i++)
for (j=0; j<width; j++)
  {
  z = (cmaplength-1)*(((float) image[i][j]) - zmin)/(zmax-zmin);
  raster[i][j] = ((char) z);
  }

ratio = gif_encode(fp,raster,height,width,depth,datatype);
c_matrix_free(raster,height);
if (ratio < 0) {fclose(fp); return(-9);}

/* write out the GIF terminator (1 byte)
*/
buf[0] = GIFTERM;
err = fwrite(buf,sizeof(byte),1,fp);
if (err != 1) {fclose(fp); return(-3);}

/* close the file
*/
if (fclose(fp) == EOF) return(-2);

return(ratio);
}
/******************************************************************************
*
* FUNCTION: i_writ_raster_gif
*
* PURPOSE: Creates a gifrasterfile.
*
* INPUT:  filename = rasterfile name
*         height = number of lines
*         width = number of pixels per line
*         image[0..height-1][0..width-1] = int raster image
*         cmaplength = length of colormap
*         red[0..cmaplength-1] = red components of colormap table
*         green[0..cmaplength-1] = green components of colormap table
*         blue[0..cmaplength-1] = blue components of colormap table
*
* RETURN: > 0 (compression ratio) if everything OK
*         -1 if unable to open rasterfile
*         -2 if unable to close rasterfile
*         -3 if error encoutered while writing on stream
*         -5 if unexpected rasterfile format: no colormap passed
*         -7 if unexpected parameter: bad dimension of data or colormap
*         -8 if memory allocation failure
*         -9 if error occurred while encoding data
*
* VERSION: December 1992
*
* AUTHOR: Eric ANTERRIEU
*
*******************************************************************************/
/*******************************************************************************/
int    i_writ_raster_gif(image,height,width,cmaplength,red,green,blue,filename)
char   *filename;
int    height, width;
int    cmaplength, *red, *green, *blue;
int    **image;
/*******************************************************************************/
{
auto    FILE   *fp;
auto    byte   buf[16];
auto    char   **raster;
auto    int    datatype = SEQUENTIALDATA, cmaptype = GLOBALCOLORMAP;
auto    int    err, i, j, c = 1, left = 0, top = 0, depth = 0, ratio;
auto    float  z, zmin, zmax;
extern  void   c_matrix_free();
extern  char   **c_matrix_alloc();
extern  int    gif_encode();

if ((red == NULL) || (green == NULL) || (blue == NULL)) return(-5);

/* compute depth of data from length of colormap
*/
while (c < cmaplength) {c *= 2; depth++;}
if (c != cmaplength) return(-7);

/* open file
*/
if ((fp = fopen(filename,"w")) == NULL) return(-1);

/* write out GIF signature (6 bytes)
*/
(void) sprintf(buf,"%s",MAGIC);
err = fwrite(buf,sizeof(byte),6,fp);
if (err != 6) {fclose(fp); return(-3);}

/* write out screen descriptor (7 bytes)
*/
buf[0] = width & 0xFF;          /* width & 00001111        */
buf[1] = (width >> 8) & 0xFF;   /* (width/256) & 00001111  */
buf[2] = height & 0xFF;         /* height & 00001111       */
buf[3] = (height >> 8) & 0xFF;  /* (height/256) & 00001111 */
buf[4] = 0x00;
if (cmaptype == GLOBALCOLORMAP)  buf[4] = COLORMAPMASK;  
buf[4] |= (depth - 1) << 5; 
buf[4] |= (depth - 1);
buf[5] = 0;
buf[6] = 0;
err = fwrite(buf,sizeof(byte),7,fp);
if (err != 7) {fclose(fp); return(-3);}

/* write out the global colormap (3*cmaplength bytes)
*/
if (cmaptype == GLOBALCOLORMAP)
  {
  for (c=0; c<cmaplength; c++)
    {
    buf[0] = red[c];
    buf[1] = green[c];
    buf[2] = blue[c];
    err = fwrite(buf,sizeof(byte),3,fp);
    if (err != 3) {fclose(fp); return(-3);}
    }
  }

/* write out an image separator (1 byte)
*/
buf[0] = IMAGESEP;
err = fwrite(buf,sizeof(byte),1,fp);
if (err != 1) {fclose(fp); return(-3);}

/* write out the image descriptor (9 bytes)
*/
buf[0] = left & 0xFF;            /* left & 00001111         */
buf[1] = (left >> 8) & 0xFF;     /* (left/256) & 00001111   */
buf[2] = top & 0xFF;             /* top & 00001111          */
buf[3] = (top >> 8) & 0xFF;      /* (top/256) & 00001111    */
buf[4] = width & 0xFF;           /* width & 00001111        */
buf[5] = (width >> 8) & 0xFF;    /* (width/256) & 00001111  */
buf[6] = height & 0xFF;          /* height & 00001111       */
buf[7] = (height >> 8) & 0xFF;   /* (height/256) & 00001111 */
buf[8] = 0x00;
if (cmaptype == LOCALCOLORMAP)  buf[8] = COLORMAPMASK; 
if (datatype == INTERLACEDDATA)  buf[8] |= INTERLACEMASK;
buf[8] |= (depth - 1);
err = fwrite(buf,sizeof(byte),9,fp);
if (err != 9) {fclose(fp); return(-3);}

/* write out the local colormap (3*cmaplength bytes)
*/
if (cmaptype == LOCALCOLORMAP)
  {
  for (c=0; c<cmaplength; c++)
    {
    buf[0] = red[c];
    buf[1] = green[c];
    buf[2] = blue[c];
    err = fwrite(buf,sizeof(byte),3,fp);
    if (err != 3) {fclose(fp); return(-3);}
    }
  }

/* compress and write out the image (<< width*height bytes !)
*/
raster = c_matrix_alloc(height,width);
if (raster == NULL) {fclose(fp); return(-8);}

zmin = zmax = image[0][0];
for (i=0; i<height; i++)
for (j=0; j<width; j++)
  {
  if (image[i][j] < zmin) zmin = image[i][j];
  if (image[i][j] > zmax) zmax = image[i][j];
  }
for (i=0; i<height; i++)
for (j=0; j<width; j++)
  {
  z = (cmaplength-1)*(((float) image[i][j]) - zmin)/(zmax-zmin);
  raster[i][j] = ((char) z);
  }

ratio = gif_encode(fp,raster,height,width,depth,datatype);
c_matrix_free(raster,height);
if (ratio < 0) {fclose(fp); return(-9);}

/* write out the GIF terminator (1 byte)
*/
buf[0] = GIFTERM;
err = fwrite(buf,sizeof(byte),1,fp);
if (err != 1) {fclose(fp); return(-3);}

/* close the file
*/
if (fclose(fp) == EOF) return(-2);

return(ratio);
}
/******************************************************************************
*
* FUNCTION: f_writ_raster_gif
*
* PURPOSE: Creates a gifrasterfile.
*
* INPUT:  filename = rasterfile name
*         height = number of lines
*         width = number of pixels per line
*         image[0..height-1][0..width-1] = float raster image
*         cmaplength = length of colormap
*         red[0..cmaplength-1] = red components of colormap table
*         green[0..cmaplength-1] = green components of colormap table
*         blue[0..cmaplength-1] = blue components of colormap table
*
* RETURN: > 0 (compression ratio) if everything OK
*         -1 if unable to open rasterfile
*         -2 if unable to close rasterfile
*         -3 if error encoutered while writing on stream
*         -5 if unexpected rasterfile format: no colormap passed
*         -7 if unexpected parameter: bad dimension of data or colormap
*         -8 if memory allocation failure
*         -9 if error occurred while encoding data
*
* VERSION: December 1992
*
* AUTHOR: Eric ANTERRIEU
*
*******************************************************************************/
/*******************************************************************************/
int    f_writ_raster_gif(image,height,width,cmaplength,red,green,blue,filename)
char   *filename;
int    height, width;
int    cmaplength, *red, *green, *blue;
float  **image;
/*******************************************************************************/
{
auto    FILE   *fp;
auto    byte   buf[16];
auto    char   **raster;
auto    int    datatype = SEQUENTIALDATA, cmaptype = GLOBALCOLORMAP;
auto    int    err, i, j, c = 1, left = 0, top = 0, depth = 0, ratio;
auto    float  z, zmin, zmax;
extern  void   c_matrix_free();
extern  char   **c_matrix_alloc();
extern  int    gif_encode();

if ((red == NULL) || (green == NULL) || (blue == NULL)) return(-5);

/* compute depth of data from length of colormap
*/
while (c < cmaplength) {c *= 2; depth++;}
if (c != cmaplength) return(-7);

/* open file
*/
if ((fp = fopen(filename,"w")) == NULL) return(-1);

/* write out GIF signature (6 bytes)
*/
(void) sprintf(buf,"%s",MAGIC);
err = fwrite(buf,sizeof(byte),6,fp);
if (err != 6) {fclose(fp); return(-3);}

/* write out screen descriptor (7 bytes)
*/
buf[0] = width & 0xFF;          /* width & 00001111        */
buf[1] = (width >> 8) & 0xFF;   /* (width/256) & 00001111  */
buf[2] = height & 0xFF;         /* height & 00001111       */
buf[3] = (height >> 8) & 0xFF;  /* (height/256) & 00001111 */
buf[4] = 0x00;
if (cmaptype == GLOBALCOLORMAP)  buf[4] = COLORMAPMASK;  
buf[4] |= (depth - 1) << 5; 
buf[4] |= (depth - 1);
buf[5] = 0;
buf[6] = 0;
err = fwrite(buf,sizeof(byte),7,fp);
if (err != 7) {fclose(fp); return(-3);}

/* write out the global colormap (3*cmaplength bytes)
*/
if (cmaptype == GLOBALCOLORMAP)
  {
  for (c=0; c<cmaplength; c++)
    {
    buf[0] = red[c];
    buf[1] = green[c];
    buf[2] = blue[c];
    err = fwrite(buf,sizeof(byte),3,fp);
    if (err != 3) {fclose(fp); return(-3);}
    }
  }

/* write out an image separator (1 byte)
*/
buf[0] = IMAGESEP;
err = fwrite(buf,sizeof(byte),1,fp);
if (err != 1) {fclose(fp); return(-3);}

/* write out the image descriptor (9 bytes)
*/
buf[0] = left & 0xFF;            /* left & 00001111         */
buf[1] = (left >> 8) & 0xFF;     /* (left/256) & 00001111   */
buf[2] = top & 0xFF;             /* top & 00001111          */
buf[3] = (top >> 8) & 0xFF;      /* (top/256) & 00001111    */
buf[4] = width & 0xFF;           /* width & 00001111        */
buf[5] = (width >> 8) & 0xFF;    /* (width/256) & 00001111  */
buf[6] = height & 0xFF;          /* height & 00001111       */
buf[7] = (height >> 8) & 0xFF;   /* (height/256) & 00001111 */
buf[8] = 0x00;
if (cmaptype == LOCALCOLORMAP)  buf[8] = COLORMAPMASK; 
if (datatype == INTERLACEDDATA)  buf[8] |= INTERLACEMASK;
buf[8] |= (depth - 1);
err = fwrite(buf,sizeof(byte),9,fp);
if (err != 9) {fclose(fp); return(-3);}

/* write out the local colormap (3*cmaplength bytes)
*/
if (cmaptype == LOCALCOLORMAP)
  {
  for (c=0; c<cmaplength; c++)
    {
    buf[0] = red[c];
    buf[1] = green[c];
    buf[2] = blue[c];
    err = fwrite(buf,sizeof(byte),3,fp);
    if (err != 3) {fclose(fp); return(-3);}
    }
  }

/* compress and write out the image (<< width*height bytes !)
*/
raster = c_matrix_alloc(height,width);
if (raster == NULL) {fclose(fp); return(-8);}

zmin = zmax = image[0][0];
for (i=0; i<height; i++)
for (j=0; j<width; j++)
  {
  if (image[i][j] < zmin) zmin = image[i][j];
  if (image[i][j] > zmax) zmax = image[i][j];
  }
for (i=0; i<height; i++)
for (j=0; j<width; j++)
  {
  z = (cmaplength-1)*(image[i][j] - zmin)/(zmax-zmin);
  raster[i][j] = ((char) z);
  }

ratio = gif_encode(fp,raster,height,width,depth,datatype);
c_matrix_free(raster,height);
if (ratio < 0) {fclose(fp); return(-9);}

/* write out the GIF terminator (1 byte)
*/
buf[0] = GIFTERM;
err = fwrite(buf,sizeof(byte),1,fp);
if (err != 1) {fclose(fp); return(-3);}

/* close the file
*/
if (fclose(fp) == EOF) return(-2);

return(ratio);
}
/*******************************************************************************
*
* FUNCTION: gif_encode
*
* PURPOSE: Compress data for a gifrasterfile.
*
* INPUT:  fp = pointer to a gif rasterfile open for binary write
*         height = number of lines
*         width = number of pixels per line
*         depth = number of bits per pixel
*         raster[0..height-1][0..width-1] = char raster image
*         datatype = type of data (sequential or interlaced)
*
* RETURN: > 0 (compression ratio) if everything OK
*         -1 if error encountered while writing on stream
*
* VERSION: December 1992
*
* AUTHOR: Eric ANTERRIEU from LZW compression algorithm based on compress and
*         from the modifications made by David Rowley for GIF.
*
*******************************************************************************/
#include <stdio.h>
#include <ctype.h>
#include <signal.h>

#define BITS	           12
#define HSIZE              5003 /* 80% occupancy */
#define SEQUENTIALDATA     1
#define INTERLACEDDATA     2
#define MAXCODE(n)         ((1 << (n)) - 1)

static char           Accum[256];
static int            Width, Height, Posx = 0, Posy = 0, CountDown, Pass = 0;
static int            ClearCode, EOFCode, MaxCode, MaxMaxCode = 1 << BITS;
static int            ClearFlag = 0, FreeFlag;
static int            CurBits = 0, NbrBits, InitBits, Count = 0;
static int            HashShift, HashSize = HSIZE;
static long int       HashTab[HSIZE], InCount = 0, OutCount = 0;
static unsigned short CodeTab[HSIZE];
static unsigned long  CurAccum = 0;
static int            gif_getnextpixel();
static int            gif_writecode();
static void           gif_clearhashtab();
/*******************************************************************************/
int    gif_encode(fp,raster,height,width,depth,datatype)
FILE   *fp;
char   **raster;
int    height, width, depth, datatype;
/*******************************************************************************/
{
auto long    Code;
auto int     i, j, pixel1, pixel2, hashsize;

Width = width;
Height = height;
CountDown = ((long int) width)*((long int) height);
InitBits = depth; if (depth <= 1) InitBits = 2;
fputc(InitBits,fp); InitBits++; OutCount++;
MaxCode = MAXCODE(NbrBits = InitBits);
ClearCode = (1 << (InitBits - 1));
EOFCode = ClearCode + 1;
FreeFlag = ClearCode + 2;

pixel2 = gif_getnextpixel(raster,datatype);

HashShift = 0;
for (Code = ((long) HashSize);  Code < 65536L; Code *= 2L) HashShift++;
HashShift = 8 - HashShift;
hashsize = HashSize;
gif_clearhashtab((long int) hashsize);

if (gif_writecode(ClearCode,fp) < 0) return(-1);
    
while ((pixel1 = gif_getnextpixel(raster,datatype)) != EOF)
  {
  Code = (long) (((long) pixel1 << BITS) + pixel2);
  i = ((pixel1 << HashShift) ^ pixel2);
  if (HashTab[i] == Code) 
     {
     pixel2 = CodeTab[i];
     continue;
     } 
  else if ((long) HashTab[i] < 0)  goto nomatch;
  j = hashsize - i;
  if (i == 0)  j = 1;
probe:
  if ((i -= j) < 0)  i += hashsize;
  if (HashTab[i] == Code) 
    {
    pixel2 = CodeTab[i];
    continue;
    }
  if ((long) HashTab[i] > 0)  goto probe;
nomatch:
  if (gif_writecode(pixel2,fp) < 0) return(-1);
  pixel2 = pixel1;
  if (FreeFlag < MaxMaxCode)
    {
    CodeTab[i] = FreeFlag++;
    HashTab[i] = Code;
    } 
  else
    {
    gif_clearhashtab((long int) HashSize);
    FreeFlag = ClearCode + 2;
    ClearFlag = 1;
    if (gif_writecode(ClearCode,fp) < 0) return(-1);
    }
  }

if (gif_writecode(pixel2,fp) < 0) return(-1);
if (gif_writecode(EOFCode,fp) < 0) return(-1);
fputc(0,fp);

return(100-(OutCount*100)/InCount);
}

/*******************************************************************************/
static int    gif_writecode(code,fp)
int           code;
FILE          *fp;
/*******************************************************************************/
{
auto unsigned long  masks[] = {0x0000, 0x0001, 0x0003, 0x0007, 0x000F, 0x001F,
                               0x003F, 0x007F, 0x00FF, 0x01FF, 0x03FF, 0x07FF,
                               0x0FFF, 0x1FFF, 0x3FFF, 0x7FFF, 0xFFFF };

CurAccum &= masks[CurBits];

if (CurBits > 0)  CurAccum |= ((long)code << CurBits);
else  CurAccum = code;
	
CurBits += NbrBits;

while (CurBits >= 8) 
  {
  Accum[Count++] = (unsigned int) (CurAccum & 0xff);
  if (Count >= 254)
    {
    fputc(Count,fp);
    if (fwrite(Accum,1,Count,fp) != Count) return(-1);
    OutCount += Count+1;
    Count = 0;
    }
  CurAccum >>= 8;
  CurBits -= 8;
  }

if (FreeFlag > MaxCode || ClearFlag) 
  {
  if (ClearFlag) 
    {
    MaxCode = MAXCODE (NbrBits = InitBits);
    ClearFlag = 0;
    } 
  else 
    {
    NbrBits++;
    if (NbrBits == BITS)  MaxCode = MaxMaxCode;
    else  MaxCode = MAXCODE(NbrBits);
    }
  }
	
if (code == EOFCode) 
  {
  while (CurBits > 0) 
    {
    Accum[Count++] = (unsigned int) (CurAccum & 0xff);
    if (Count >= 254)
      {
      fputc(Count,fp);
      if (fwrite(Accum,1,Count,fp) != Count) return(-1);
      OutCount += Count+1;
      Count = 0;
      }
    CurAccum >>= 8;
    CurBits -= 8;
    }
  if (Count > 0)
    {
    fputc(Count,fp);
    if (fwrite(Accum,1,Count,fp) != Count) return(-1);
    OutCount += Count+1;
    Count = 0;
    }
  fflush(fp);
  if (ferror(fp)) return(-1);
  }

return(0);
}

/*******************************************************************************/
static void   gif_clearhashtab(HashSize)
long int      HashSize;
/*******************************************************************************/
{
auto long int *hashtab_p = HashTab+HashSize;
auto long int i, j = -1;

i = HashSize - 16;
do 
  {
  *(hashtab_p-16) = j;
  *(hashtab_p-15) = j;
  *(hashtab_p-14) = j;
  *(hashtab_p-13) = j;
  *(hashtab_p-12) = j;
  *(hashtab_p-11) = j;
  *(hashtab_p-10) = j;
  *(hashtab_p-9)  = j;
  *(hashtab_p-8)  = j;
  *(hashtab_p-7)  = j;
  *(hashtab_p-6)  = j;
  *(hashtab_p-5)  = j;
  *(hashtab_p-4)  = j;
  *(hashtab_p-3)  = j;
  *(hashtab_p-2)  = j;
  *(hashtab_p-1)  = j;
  hashtab_p -= 16;
  } while ((i -= 16) >= 0);

for (i += 16; i > 0; i--)  *--hashtab_p = j;
}

/*******************************************************************************/
static int   gif_getnextpixel(raster,datatype)
char         **raster;
int          datatype;
/*******************************************************************************/
{
auto int     val;

if (CountDown == 0) return(EOF);
CountDown--;
InCount++;

val = raster[Height-1-Posy][Posx];

Posx++;
if (Posx == Width)
  {
  Posx = 0;
  if (datatype == SEQUENTIALDATA) Posy++;
  if (datatype == INTERLACEDDATA)
    {
    switch(Pass)
      {
      case 0:
        Posy += 8;
        if ( Posy >= Height ) {Pass++; Posy = 4;}
        break;
      case 1:
        Posy += 8;
        if ( Posy >= Height ) {Pass++; Posy = 2;}
        break;
      case 2:
        Posy += 4;
        if ( Posy >= Height ) {Pass++; Posy = 1;}
        break;
      case 3:
        Posy += 2;
        break;
      }
    }
  }

return(val);
}
