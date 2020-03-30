/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
* Program rtiff to read TIFF format (from Macintosh) 
* JLP LK EA
* Version 20-04-93
-----------------------------------------------------------------------*/

/*
#define DEBUG
*/

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
register int i;

printf(" Program rtiff to read TIFF image files from Macintosh \n");
printf(" JLP/LK/EA Version 20-04-92 \n");

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
  printf(" Syntax: rtiff in_file out_file [iformat]\n\n"); 
  printf(" Input file := ");scanf("%s",in_name);
  printf(" Output file := ");scanf("%s",out_name);
  printf(" Formats available: 1=old image files 2=tiff mac 3=tiff general\n");
  printf(" Enter format := ");scanf("%d",&iformat);
 }
else
 {
  strcpy(in_name,argv[1]);
  strcpy(out_name,argv[2]);
 }

/* Tiff format (now it is better to always use 3) */
if (argc == 4) 
  sscanf(argv[3],"%d",&iformat);
else
  iformat = 3;

if(iformat < 1 || iformat > 3) 
 {
 printf(" Fatal error/Wrong format: entered value was %d ! \n",iformat);
 exit(-1);
 }

JLP_BEGIN();
JLP_INQUIFMT();


#ifdef DEBUG
printf(" OK, will read >%s< \n",in_name);
#endif

/* Calling reading subroutine: */
switch(iformat)
{
 case 1:
  status = rdoldimage(&real_array,&nx,&ny,in_name,comments);
  break;
 case 2:
  status = rdtiff1(&real_array,&nx,&ny,in_name,comments);
  break;
 case 3:
  status = rdtiff2(&real_array,&nx,&ny,in_name,comments);
  break;
}

if(!status)
  {

#ifdef DEBUG
for(i=0; i<100; i++) printf(" a[%d]: %f ",i,real_array[i]);
#endif

  printf(" Conversion of TIFF image:  %s    to:    %s \n",in_name,out_name);
  sscanf(comments," From  %s",in_name);
  JLP_WRITEIMAG(real_array,&nx,&ny,&nx,out_name,comments);
  }

JLP_END();
}
/****************************************************************/
/*
Format tiff donne par le prog IMAGE
obtenu par "reverse ingeneering" LK 16 / 9 / 92

adresses en octets depuis le debut du fichier:

(000 - 255) Ox0000 - 0x00FF: un header non dechiffre.
(256 - 263) Ox0100 - 0x0107 : la chaine ASCII "IPICIMAG"
(264 - 265) 0x0108 - 0x0109 : le nombre de lignes, par exemple 0x01A4
(266 - 267) 0x010A - 0x010B : le nombre de colones, par exemple 0x0200
(268 - 383) 0x010C - 0x02FF : un header non dechiffre.
(384 - fin)  0x0300 - fin :  les donnees  sur un octet par pixel : 
   FF =noir, 00 =blanc.

Exemple:
Longueur totale du fichier PORTRAIT: 0x300 + 0x200 * 0x300 = 0x60300 = 393984
*/
/****************************************************************/
/* Subroutine rdtiff1 to read TIFF format (from Macintosh) */
int rdtiff1(real_array,nx,ny,in_name,comments)
float **real_array;
int *nx, *ny;
char in_name[], comments[];
{
FILE *fd;
char *char_array, header[512];
int nbytes_to_read, nbytes;
register int i, j;

#ifdef DEBUG
printf(" \n rdtiff/reading file : %s \n",in_name);
#endif

/* Opens the input file */
if((fd = fopen(in_name,"r")) == NULL)
  {
  printf("rdtiff/error opening input file: >%s< \n",in_name);
  return(-1);
  }

/***************************************************************/
/* Reads begining of header (twice 256 bytes): */
nbytes_to_read = 256;
for(i=0; i<2; i++)
  {
  nbytes = fread(header,sizeof(char),nbytes_to_read,fd);
#ifdef DEBUG
  printf("        %d bytes read \n", nbytes);
#endif
  if(nbytes != nbytes_to_read)
    {
     printf("rdtiff/error reading header: \n");
     printf("       Only %d bytes read \n", nbytes);
     return(-2);
    }
  }

/***************************************************************/
/* Decode central part of header: */
/* Look for IPICIMAG */
#ifdef DEBUG
for(i=0; i<8; i++) printf(" h[%d]: %s ",i,header[i]);
#endif

if( header[0] == 'I' && header[1] == 'P' && header[2] == 'I'
 && header[3] == 'C' && header[4] == 'I' && header[5] == 'M'
 && header[6] == 'A' && header[7] == 'G')
  printf("rdtiff/ OK: format seems correct \n");
 else
 {
  printf("rdtiff/Fatal error, wrong header: \n");
  for(i=0; i<8; i++) printf(" h[%d]: %s ",i,header[i]);
  return(-1);
 }

/* Bytes #8 and #9 of second block: ny with msb then lsb */
*ny = (int)header[8]*256 + (int)header[9];

/* Bytes #10 and #11 of second block: nx with msb then lsb */
*nx = (int)header[10]*256 + (int)header[11];

printf(" nx = %d ny = %d \n",*nx,*ny);

/***************************************************************/
/* Read end of header (256 bytes): */
nbytes_to_read = 256;
nbytes = fread(header,sizeof(char),nbytes_to_read,fd);
#ifdef DEBUG
  printf("        %d bytes read \n", nbytes);
#endif
  if(nbytes != nbytes_to_read)
    {
     printf("rdtiff/error reading header: \n");
     printf("       Only %d bytes read \n", nbytes);
     return(-2);
    }

/***************************************************************/
/* Now the data: */

/* Allocate memory space: */
nbytes_to_read = *nx * *ny * sizeof(char);
if(!( char_array = malloc(nbytes_to_read) ) )
  {printf("rtiff/fatal error: no memory space available!\n");
   return(-1);
  }

/* Read data: */
nbytes = fread(char_array,sizeof(char),nbytes_to_read,fd);
if(nbytes != nbytes_to_read)
  {
  printf("rdtiff/error reading input file: >%s< \n",in_name);
  printf("       Only %d bytes read \n", nbytes);
  return(-3);
  }

/* Transfer to real array: */
if(!(*real_array = (float *)malloc(*nx * *ny * sizeof(float))) )
  {printf("rtiff/fatal error: no memory space available!\n");
   return(-1);
  }
for(j = 0; j < *ny; j++) 
  for(i = 0; i < *nx; i++) 
    (*real_array)[i + j * *nx] = 
          (float)(255 - char_array[i + (*ny - j - 1) * *nx]);  

/* Frees memory: */
   free(*char_array);

/* Closes the input file */
  fclose(fd);
  return(0);
}
/**********************************************************/
/* Subroutine to read 'old image' format (from Macintosh) */
int rdoldimage(real_array,nx,ny,in_name,comments)
float **real_array;
int *nx, *ny;
char in_name[], comments[];
{
FILE *fd;
char *char_array, header[512];
int nbytes_to_read, nbytes;
register int i,j;

#ifdef DEBUG
printf(" \n rdoldimage/reading file : %s \n",in_name);
#endif

/* Opens the input file */
if((fd = fopen(in_name,"r")) == NULL)
  {
  printf("rdoldimage/error opening input file: >%s< \n",in_name);
  return(-1);
  }

/* Reads header: */
nbytes_to_read = 512;
nbytes = fread(header,sizeof(char),nbytes_to_read,fd);
#ifdef DEBUG
  printf("        %d bytes read \n", nbytes);
#endif
if(nbytes != nbytes_to_read)
  {
  printf("rdoldimage/error reading header: \n");
  printf("       Only %d bytes read \n", nbytes);
  return(-2);
  }

#ifdef DEBUG
printf(" header[0],[1],[2],[3]: %d %d %d %d \n",(int)header[0],(int)header[1],(int)header[2],(int)header[3]);
#endif

#ifdef DEBUG
for(i=0; i<30; i++) printf(" h[%d]: %d ",i,(int)header[i]);
#endif

/* First two bytes: ny with lsb then msb */
*ny = (int)header[0] + (int)header[1]*256;

/* Next two bytes: nx with lsb then msb */
*nx = (int)header[2] + (int)header[3]*256;

printf(" nx = %d ny = %d \n",*nx,*ny);
nbytes_to_read = *nx * *ny * sizeof(char);

/* Allocate memory space: */
if(!( char_array = malloc(nbytes_to_read) ) )
  {printf("rdoldimage/fatal error: no memory space available!\n");
   return(-1);
  }

/* Reads data: */
nbytes = fread(char_array,sizeof(char),nbytes_to_read,fd);
if(nbytes != nbytes_to_read)
  {
  printf("rdoldimage/error reading input file: >%s< \n",in_name);
  printf("       Only %d bytes read \n", nbytes);
  return(-3);
  }

/* Transfer to real array: */
if(!(*real_array = (float *)malloc(*nx * *ny*sizeof(float))) )
  {printf("rdoldimage/fatal error: no memory space available!\n");
   return(-1);
  }
for(j = 0; j < *ny; j++) 
  for(i = 0; i < *nx; i++) 
    (*real_array)[i + j * *nx] = 
          (float)(255 - char_array[i + (*ny - j - 1) * *nx]);  

/* Frees memory: */
   free(*char_array);

/* Closes the input file */
  fclose(fd);
  return(0);
}
/****************************************************************/
/*
TIFF Tag Image Format File
From La Revue de l'utilisateur PC Vol 78 pp 49-64 June 1992.
/****************************************************************/

/****************************************************************/
/* Subroutine rdtiff2 to read TIFF format */
/****************************************************************/
int rdtiff2(real_array,nx,ny,in_name,comments)
float **real_array;
int *nx, *ny;
char in_name[], comments[];
{
FILE *fd;
char *char_array, header[512];
int nbytes_to_read, nbytes, to_swap;
unsigned short version, nber_of_tags, tag_type, data_type, value1;
unsigned long first_address, value_length, value, value2, strip_offset;
register int i, j;

/* Value size according to data type: 0: 0 byte, 
    1= 1 byte, 2=ASCII (1 byte), 3=short (2 bytes), 
    4=long (4 bytes), 5=rational (8 bytes) */
int tag_size[6] = {0,1,1,2,4,8};

void swap_int(), swap_lint();

#ifdef DEBUG
printf(" \n rdtiff/reading file : %s \n",in_name);
#endif

/* Opens the input file */
if((fd = fopen(in_name,"r")) == NULL)
  {
  printf("rdtiff/error opening input file: >%s< \n",in_name);
  return(-1);
  }

/***************************************************************/
/* Read first eight bytes to know if swap is needed, version, etc */
 nbytes = fread(header,1L,8L,fd);
 if(nbytes != 8)
   {printf("rdtiff/error reading beginning of header, only %d bytes read\n", nbytes);
    return(-2);}

#ifdef DEBUG
for(i=0; i<8; i++) printf(" h[%d]: %d ",i,header[i]);
printf("\n");
#endif

/* 'M''M' if Motorola, 'I''I' if Intel */
/* swap is not needed for ibm with Motorola: */
if( header[0] == 'M' && header[1] == 'M')
  {
   printf("rdtiff/ OK: Motorola format ");
   to_swap = 0;
  }
else if( header[0] == 'I' && header[1] == 'I')
  {
   printf("rdtiff/ OK: Intel format ");
   to_swap = 1;
  }
else
 {
  printf("rdtiff/Fatal error, wrong header: \n");
  for(i=0; i<8; i++) printf(" h[%d]: %d ",i,header[i]);
  return(-1);
 }

/* Version number: (usually 42) */
  memcpy(&version,header+2,2);
  if(to_swap) swap_int(&version);

#ifdef DEBUG
   printf(" Version %u \n",version);
#endif

/* First address */
  memcpy(&first_address,header+4,4);
  if(to_swap) swap_lint(&first_address);
#ifdef DEBUG
   printf(" First address %u \n",first_address);
#endif

/* Skip to this position (relative to the beginning of the file): */
if(fseek(fd,first_address,SEEK_SET))
  {printf("rdtiff/fseek error \n"); return(-2);}
if( (nbytes = fread(header,1L,2L,fd)) != 2L)
  {printf("rdtiff/fatal error reading header (first address) \n"); return(-3);}

/* And read number of tags: */
  memcpy(&nber_of_tags,header,2);
  if(to_swap) swap_int(&nber_of_tags);

#ifdef DEBUG
     printf(" Number of tags %d \n",nber_of_tags);
#endif

/***************************************************************/
/* Decode central part of header: */

/* Main loop: */
for (i=0; i<nber_of_tags; i++)
  {
  if( (nbytes = fread(header,1L,8L,fd)) != 8L)
    {printf("rdtiff/fatal error reading header (tag #%d) \n",i); return(-4);}

/* Tag type: */
   memcpy(&tag_type,header,2);
      if(to_swap) swap_int(&tag_type);

/* Data type: */
   memcpy(&data_type,header+2,2);
      if(to_swap) swap_int(&data_type);

/* Length: */
   memcpy(&value_length,header+4,4);
      if(to_swap) swap_lint(&value_length);

/* Value size according to tag type: 0= 0 byte 
    1= 1 byte, 2=ASCII (1 byte), 3=short (2 bytes), 
    4=long (4 bytes), 5=rational (8 bytes) */

#ifdef DEBUG
   printf(" Tag #%d ; tag type: %d, data type: %d value length: %d \n",
            i,tag_type,data_type,value_length);
#endif

/* Now read value: 
 (Note that we have to read 4 bytes in all cases)*/
   switch( tag_size[data_type] )
     {
/* unsigned short: */
      case 2 :
        if( (nbytes = fread(header,1L,4L,fd)) != 4L)
        {printf("rdtiff/fatal error reading header (tag #%d)\n",i); return(-4);}
        memcpy(&value1,header,2);
        if(to_swap) swap_int(&value1);
        value = (long)value1;
        break;
/* unsigned long: */
      case 4 :
        if( (nbytes = fread(header,1L,4L,fd)) != 4L)
        {printf("rdtiff/fatal error reading header (tag #%d)\n",i); return(-4);}
        memcpy(&value,header,4);
        if(to_swap) swap_lint(&value);
        break;
/* rational: not tested yet...*/
      case 8 :
        printf(" Sorry rational numbers not tested yet...\n");
/*
        if( (nbytes = fread(header,1L,4L,fd)) != 4L)
        {printf("rdtiff/fatal error reading header (tag #%d)\n",i); return(-4);}
        memcpy(&value,header,4);
        if(to_swap) swap_lint(&value);
        memcpy(&value2,header+4,4);
        if(to_swap) swap_lint(&value2);
        break;
*/
/* Default: (0 or 1 byte)
 When only one byte, the value is in "value_length" field
*/
       default:
        value = value_length * tag_size[data_type];
        break;
     }

#ifdef DEBUG
   printf(" Tag #%d ; value: %d \n",i,value);
#endif

/* Decoding the tag number for each tag: */
   switch(tag_type)
   {
/* Width of the image: 256 or 0x100*/
      case 256 :
        *nx = value;
        break;
/* Length of the image: 257 or 0x101*/
      case 257 :
        *ny = value;
        break;
/* Photometric interpretation: 262 or 0x106*/
      case 262 :
         switch(value)
          {
            case 0:
              printf(" Grey map (0=black)\n");
              break;
            case 1:
              printf(" Grey map (0=white)\n");
              break;
            case 2:
              printf(" Color map (RVB)\n");
              break;
          }
        break;
/* Strip offset (pointer to the beginning of the image) */
      case 273 :
        strip_offset = value;
        break;
   }

  }

#ifdef DEBUG
printf(" nx = %d ny = %d strip_offset = %d\n",*nx,*ny,strip_offset);
#endif

/***************************************************************/
/* Now the data: */

/* Allocate memory space: */
nbytes_to_read = *nx * *ny * sizeof(char);
if(!( char_array = malloc(nbytes_to_read) ) )
  {printf("rtiff/fatal error: no memory space available!\n");
   return(-1);
  }

/* Go to the beginning of the data (relative to the beginning of the file) */
if(fseek(fd,strip_offset,SEEK_SET))
  {printf("rdtiff/Fatal: fseek error looking for the data.\n"); return(-6);}

/* Read data: */
nbytes = fread(char_array,sizeof(char),nbytes_to_read,fd);
if(nbytes != nbytes_to_read)
  {
  printf("rdtiff/error reading input file: >%s< \n",in_name);
  printf("       Only %d bytes read \n", nbytes);
  return(-3);
  }

/* Transfer to real array: */
if(!(*real_array = (float *)malloc(*nx * *ny * sizeof(float))) )
  {printf("rtiff/fatal error: no memory space available!\n");
   return(-1);
  }
for(j = 0; j < *ny; j++) 
  for(i = 0; i < *nx; i++) 
    (*real_array)[i + j * *nx] = 
          (float)(255 - char_array[i + (*ny - j - 1) * *nx]);  

/* Frees memory: */
   free(*char_array);

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
