/***************************************************************************/
/*                                                                         */
/* PROGRAM: gif2bin                                                        */
/*                                                                         */
/* PURPOSE: Converts GIF image to BIN raster file.                         */
/*                                                                         */
/* INPUT:  argv[1] = input file                                            */
/*                                                                         */
/* OUTPUT: argv[2] = output file                                           */
/*                                                                         */
/* VERSION: August 1992                                                    */
/*                                                                         */
/* AUTHOR: Eric ANTERRIEU                                                  */
/*                                                                         */
/***************************************************************************/
#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include "LIB_OMP.h"
#include "copyright.h"

typedef unsigned char byte;
typedef struct codestruct {
        struct codestruct *prefix;
        byte first,suffix;
        } codetype;

int global;                         /* Is there a global color map? */
int globalcolor;                    /* Number of global colors */
int globalbits;                     /* Number of bits of global colors */
byte globalmap[256][3];             /* RGB values for global color map */
char colortable[256][3];            /* Hex intensity strings for an image */
byte *raster;                       /* Decoded image data */
codetype *codetable;                /* LZW compression code data */
int datasize,codesize,codemask;     /* Decoder working variables */
int clear,eoi;                      /* Special code values */

/***************************************************************************/
void  checksignature(fp)
FILE  *fp;
{
auto  byte  buf[6];

fread(buf,1,6,fp);  /* read 6 bytes */
if (strncmp(buf,"GIF",3)) error_abort("file is not a GIF file");
if (strncmp(&buf[3],"87a",3)) error_abort("unknown GIF version number");
}
/***************************************************************************/
void  readextension(fp)
FILE  *fp;
{
auto  byte  code, count;
auto  char  buf[255];

code = getc(fp);
while (count = getc(fp)) fread(buf,1,count,fp);
}
/***************************************************************************/
void  readscreen(fp)
FILE  *fp;
{
auto  byte  buf[7];
auto  int   screenwidth;    /* The dimensions of the screen */
auto  int   screenheight;   /*   (not those of the image)   */

fread(buf,1,7,fp);  /* read 7 bytes */
screenwidth = buf[0] + (buf[1] << 8);  /* buf[0] + 256*buf[1] */
screenheight = buf[2] + (buf[3] << 8);  /* buf[2] + 256*buf[3] */
global = buf[4] & 0x80;  /* buf[4] & 10000000 */
if (global) 
  {
  globalbits = (buf[4] & 0x07) + 1;  /* 1+buff[4] & 00001111 */
  globalcolor = 1<<globalbits;  /* 2^globalbits */
  fread(globalmap,3,globalcolor,fp);  /* read globalcolor bytes */
  }
}
/***************************************************************************/
void  initcolors(colortable,colormap,ncolors)
char  colortable[256][3];
byte  colormap[256][3];
int   ncolors;
{
register  int   i;
auto      int   color;
auto      char  hextab[] = {'0','1','2','3','4','5','6','7',
                            '8','9','A','B','C','D','E','F'};

for (i=0; i<ncolors; i++) 
  {
  color = 77*colormap[i][0] + 150*colormap[i][1] + 29*colormap[i][2];
  color >>= 8;
  colortable[i][0] = hextab[color >> 4];
  colortable[i][1] = hextab[color & 15];
  colortable[i][2] = '\0';
  }
}
/***************************************************************************/
void      outcode(p,fill)
codetype  *p;
byte      **fill;
{
if (p->prefix) outcode(p->prefix,fill);
*(*fill)++ = p->suffix;
}
/***************************************************************************/
int   decodehexa(string)
char  *string;
{
register  int  i, j;
auto      int  val=0, x;

for (i=0; i<strlen(string); i++)
  {
  x = 1;
  for (j=0; j<strlen(string)-1-i; j++) x *= 16;
  switch(string[i])
    {
    case '0': val +=  0*x; break;
    case '1': val +=  1*x; break;
    case '2': val +=  2*x; break;
    case '3': val +=  3*x; break;
    case '4': val +=  4*x; break;
    case '5': val +=  5*x; break;
    case '6': val +=  6*x; break;
    case '7': val +=  7*x; break;
    case '8': val +=  8*x; break;
    case '9': val +=  9*x; break;
    case 'A': val += 10*x; break;
    case 'B': val += 11*x; break;
    case 'C': val += 12*x; break;
    case 'D': val += 13*x; break;
    case 'E': val += 14*x; break;
    case 'F': val += 15*x; break;
    }
  }
return(val);
}
/***************************************************************************/
void  process(code,fill)
int   code;
byte  **fill;
{
static  int       avail, oldcode;
auto    codetype  *p;

if (code == clear) 
  {
  codesize = datasize + 1;
  codemask = (1 << codesize) - 1;
  avail = clear + 2;
  oldcode = -1;
  } 
else if (code < avail) 
  {
  outcode(&codetable[code],fill);
  if (oldcode != -1) 
    {
    p = &codetable[avail++];
    p->prefix = &codetable[oldcode];
    p->first = p->prefix->first;
    p->suffix = codetable[code].first;
    if ((avail & codemask) == 0 && avail < 4096) 
      {
      codesize++;
      codemask += avail;
      }
    }
  oldcode = code;
  } 
else if (code == avail && oldcode != -1) 
  {
  p = &codetable[avail++];
  p->prefix = &codetable[oldcode];
  p->first = p->prefix->first;
  p->suffix = p->first;
  outcode(p,fill);
  if ((avail & codemask) == 0 && avail < 4096) 
    {
    codesize++;
    codemask += avail;
    }
  oldcode = code;
  } 
else 
  {
  error_abort("illegal code in raster data");
  }
}
/***************************************************************************/
void  readraster(fp,width,height)
FILE  *fp;
int   width,height;
{
auto  byte  buf[255], *ch, *fill = raster;
auto  int   count, datum=0;
auto  int   bits=0, code;

datasize = getc(fp);
clear = 1 << datasize;
eoi = clear+1;
codesize = datasize + 1;
codemask = (1 << codesize) - 1;
codetable = (codetype*)malloc(4096*sizeof(codetype));
if (!codetable) error_abort("not enough memory for code table");
for (code = 0; code < clear; code++) 
  {
  codetable[code].prefix = (codetype*)0;
  codetable[code].first = code;
  codetable[code].suffix = code;
  }
for (count = getc(fp); count > 0; count = getc(fp)) 
  {
  fread(buf,1,count,fp);
  for (ch=buf; count-- > 0; ch++) 
    {
    datum += *ch << bits;
    bits += 8;
    while (bits >= codesize) 
      {
      code = datum & codemask;
      datum >>= codesize;
      bits -= codesize;
      if (code == eoi) goto exitloop; 
      process(code,&fill);
      }
    }
  }
exitloop:
if (fill != raster + width*height) error_abort("raster has the wrong size");
free(codetable);
}
/***************************************************************************/
void  transfert(fp,filename)
FILE  *fp;
char  *filename;
{
register  int    i, j;
auto      byte   *scanline, val, buf[9];
auto      char   localmap[256][3];
auto      int    left, top, width, height;
auto      int    interleaved, *interleavetable;
auto      int    local, localbits, localcolor;
auto      float  **rasterfloat;

fread(buf,1,9,fp);
left = buf[0] + (buf[1] << 8);
top = buf[2] + (buf[3] << 8);
width = buf[4] + (buf[5] << 8);
height = buf[6] + (buf[7] << 8);
raster = (byte*)malloc(width*height);
if (raster == NULL) error_abort("not enough memory for image");
rasterfloat = f_matrix_alloc(height,width);
if (rasterfloat == NULL) error_abort("not enough memory for image");

local = buf[8] & 0x80;
interleaved = buf[8] & 0x40;
if (local) 
  {
  localbits = (buf[8] & 0x7) + 1;
  localcolor = 1<<localbits;
  fread(localmap,3,localcolor,fp);
  initcolors(colortable,localmap,localcolor);
  } 
else if (global) 
  {
  initcolors(colortable,globalmap,globalcolor);
  } 
else 
  {
  error_abort("no colormap present for image");
  }
readraster(fp,width,height);
if (interleaved) 
  {
  interleavetable = (int*)malloc(height*sizeof(int));
  if (!interleavetable) error_abort("not enough memory for interleave table");
  j = 0;
  for (i=top;   i<top+height; i += 8) interleavetable[i] = j++;
  for (i=top+4; i<top+height; i += 8) interleavetable[i] = j++;
  for (i=top+2; i<top+height; i += 4) interleavetable[i] = j++;
  for (i=top+1; i<top+height; i += 2) interleavetable[i] = j++;
  for (i=top;   i<top+height; i++)
    {
    scanline = raster + interleavetable[i]*width;
    for (j=0; j<width; j++)
      {
      val = *scanline++;
      rasterfloat[height-1-i+top][j] = decodehexa(colortable[val]);
      }
    }
  free(interleavetable);
  } 
else 
  {
  for (i=top; i<top+height; i++)
    {
    scanline = raster + i*width;
    for (j=0; j<width; j++)
      {
      val = *scanline++;
      rasterfloat[height-1-i+top][j] = decodehexa(colortable[val]);
      }
    }
  }
free(raster);
writ_f_raster(rasterfloat,height,width,filename);
f_matrix_free(rasterfloat,height);
}
/***************************************************************************/
main(argc,argv)
int   argc;
char  *argv[];
{
auto  FILE  *fp;
auto  char  ch, message[100], input_extension[5], output_extension[5];
auto  int   err, quit = 0;

if (argc == 1)
  {
  (void) printf("\nUSAGE:\n");
  (void) printf("gif2bin input_file output_file\n\n");
  (void) printf("\t input_file = raster GIF file\n");
  (void) printf("\t output_file = raster BIN file\n\n");
  (void) printf("%s\n\n",copyright);
  exit(-1);
  }
if (argc != 3)
  error_abort("Unexpected number of arguments");

decode_filename(argv[1],NULL,NULL,input_extension);
decode_filename(argv[2],NULL,NULL,output_extension);
if (ISGIF(input_extension) != 0)
  {
  (void) sprintf(message,"Unknown input file format [%s]",argv[1]);
  error_abort(message);
  }
if (ISBIN(output_extension) != 0)
  {
  (void) sprintf(message,"Unknown output file format [%s]",argv[2]);
  error_abort(message);
  }

fp = fopen(argv[1],"r");
if (fp == NULL) 
  {
  (void) sprintf(message,"Unable to read input file [%s]",argv[1]);
  error_abort(message);
  }

checksignature(fp);
readscreen(fp);
do 
  {
  ch = getc(fp);
  switch (ch) {
    case '\0':  break;  /* this kludge for non-standard files */
    case ',':   transfert(fp,argv[2]);
                break;
    case ';':   quit = 1;
                break;
    case '!':   readextension(fp);
                break;
    default:    error_abort("illegal GIF block type");
                break;
    }
  } while (!quit);

fclose(fp);
}
