/***************************************************************************/
/*                                                                         */
/* PROGRAM: gifinfo                                                        */
/*                                                                         */
/* PURPOSE: Return informations about a GIF raster file.                   */
/*                                                                         */
/* INPUT:  argv[1] = input file                                            */
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

int global;                         /* Is there a global color map? */
int globalcolor;                    /* Number of global colors */
int globalbits;                     /* Number of bits of global colors */

/***************************************************************************/
void  checksignature(fp)
FILE  *fp;
{
auto  byte  buf[7];

fread(buf,1,6,fp);  /* read 6 bytes */
if (strncmp(buf,"GIF",3)) error_abort("file is not a GIF file");
if (strncmp(&buf[3],"87a",3)) error_abort("unknown GIF version number");
buf[6] = '\0';
fprintf(stderr,"signature...............: %s\n",buf);
}
/***************************************************************************/
void  readscreen(fp)
FILE  *fp;
{
auto  byte  buf[7];
auto  int   screenwidth;    /* The dimensions of the screen */
auto  int   screenheight;   /*   (not those of the image)   */
auto  byte  globalmap[256][3];  /* RGB values for global color map */


fread(buf,1,7,fp);  /* read 7 bytes */
screenwidth = buf[0] + (buf[1] << 8);  /* buf[0] + 256*buf[1] */
screenheight = buf[2] + (buf[3] << 8);  /* buf[2] + 256*buf[3] */
fprintf(stderr,"screen width............: %d\n",screenwidth);
fprintf(stderr,"screen height...........: %d\n",screenheight);
global = buf[4] & 0x80;  /* buf[4] & 10000000 */
if (global) 
  {
  globalbits = (buf[4] & 0x07) + 1;  /* 1+buff[4] & 00001111 */
  globalcolor = 1<<globalbits;  /* 2^globalbits */
  fread(globalmap,3,globalcolor,fp);  /* read globalcolor bytes */
  }
}
/***************************************************************************/
void  transfert(fp)
FILE  *fp;
{
auto      byte   buf[9];
auto      int    left, top, width, height;
auto      int    interleaved;
auto      int    local, localbits, localcolor;

fread(buf,1,9,fp);
left = buf[0] + (buf[1] << 8);
top = buf[2] + (buf[3] << 8);
width = buf[4] + (buf[5] << 8);
height = buf[6] + (buf[7] << 8);
fprintf(stderr,"image width.............: %d\n",width);
fprintf(stderr,"image height............: %d\n",height);
fprintf(stderr,"top offset..............: %d\n",top);
fprintf(stderr,"left offset.............: %d\n",left);

local = buf[8] & 0x80;
if (local) 
  {
  localbits = (buf[8] & 0x7) + 1;
  localcolor = 1<<localbits;
  fprintf(stderr,"local colormap..........: %d bits / %d colors\n",
          localbits,localcolor);
  } 
else
  fprintf(stderr,"local colormap..........: No\n");

if (global) 
  fprintf(stderr,"global colormap.........: %d bits / %d colors\n",
          globalbits,globalcolor);
else
  fprintf(stderr,"global colormap.........: No\n");

if ((!local) && (!global)) 
  error_abort("no colormap present for image");

interleaved = buf[8] & 0x40;
if (interleaved)
  fprintf(stderr,"interlaced image........: Yes\n");
else
  fprintf(stderr,"interlaced image........: No\n");
}
/***************************************************************************/
main(argc,argv)
int   argc;
char  *argv[];
{
auto  FILE  *fp;
auto  char  ch, message[100], input_extension[5];
auto  int   err;

if (argc == 1)
  {
  (void) printf("\nUSAGE:\n");
  (void) printf("gifinfo input_file\n\n");
  (void) printf("\t input_file = raster GIF file\n\n");
  (void) printf("%s\n\n",copyright);
  exit(-1);
  }
if (argc != 2)
  error_abort("Unexpected number of arguments");

decode_filename(argv[1],NULL,NULL,input_extension);
if (ISGIF(input_extension) != 0)
  {
  (void) sprintf(message,"Unknown input file format [%s]",argv[1]);
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

ch = getc(fp);
if (ch == ',') transfert(fp);
else error_abort("illegal GIF block type");

fclose(fp);
}
