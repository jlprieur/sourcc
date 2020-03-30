/* Program ctape to control a tape driver 
 JLP 
 Version 20-07-93
*/

#define DEBUG

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <jlp_ftoc.h>

#include <osudef.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <strings.h>
#include <sys/ioctl.h>
#include <sys/mtio.h>
#include <errno.h>
#include <osparms.h>
 
#define NBLOCKS_MAX 10000
#define BLOCK_SIZE 1024 

main(argc,argv)
int argc;
char *argv[];
{
FILE *out_fp;
int fd;
char dev_name[80], out_name[80], buffer[BLOCK_SIZE];
int t, stat, nbytes, size, itry;
register int k;

strcpy(dev_name,"/dev/nrmt1h");

printf(" Program ctape to control a tape driver \n");
printf(" JLP Version 20-07-93 \n");
printf(" \n Output file: ");
gets(out_name);

/* Open the output file */
if((out_fp = fopen(out_name,"w")) == NULL)
  {
  printf("ctape/Fatal error opening output file: >%s< \n",out_name);
  return(-1);
  }

/* Open tape device: */
t=O_RDONLY;
if ((fd = open(dev_name,t)) == NULL)
  {
   printf("ctape/Fatal error opening device %s \n",dev_name);
   exit(-1);
  } 
else
  {
  printf("ctape/OK device %s successfully opened \n",dev_name);
  }

/*
** Rewind tape: 
*/
  printf("ctape/Rewinding tape device \n");
  if((stat = iorew(fd)) == -1)
  {
   printf("ctape/Error rewinding tape device \n");
  } 
  else
  {
   printf("ctape/OK: tape successfully rewinded\n");
  } 


/* Read blocks from a magnetic tape.
size:  Length of bytes to be read 
.Returns bytes read / -1 if error
.Remarks 0 Bytes read, means a File Mark was detected.
*/
k = 0;
itry = 0;
while(k < NBLOCKS_MAX && itry < 10)
 {
 size = BLOCK_SIZE;
 if( (nbytes = ioread(fd,buffer,size)) != size)
  {
   printf("ctape/Error reading tape device, nbytes=%d , blocks=%d (of 1024 bytes)\n",
          nbytes,k);
/*
** if a file mark detected, then skip it.
** read, can not skip file mark by itself .
*/
   printf("ctape/ Try to skip EOF \n");
   iofsf(fd,1);
   itry++;
  } 
  else
  {
/* Writes to output file: */
   itry = 0;
  if( (nbytes = fwrite(buffer,sizeof(char),size,out_fp)) != size)
   {
   printf("ctape/Error writing to output file, blocks=%d\n",k);
   break;
   }

   if( (k % 100) == 0) printf(" Block #%d read \n",k);
  }
 k++;
 }


printf("ctape/ %d blocks read\n",k);


/**********************************************************/
close(fd);
fclose(out_fp);
}
