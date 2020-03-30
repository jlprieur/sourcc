/*************************************************************************
* Program txt_short_lines.c 
* To truncate long lines at 70 characters maximum 
*
*
* JLP
* Version 12/02/2007
*************************************************************************/
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <ctype.h>                   /* isprint... */

#define MAXI(a,b) (a) < (b) ? (b) : (a)
 
/* Maximum length for one line will be 160 characters: */
#define NMAX 160
 
#define DEBUG
/*
*/

static int truncate_lines(FILE *fp_in, FILE *fp_out, int imax); 

int main(int argc, char *argv[])
{
char filein[60], fileout[60];
int imax;
FILE *fp_in, *fp_out;

  printf("txt_shor_lines/ JLP - Version 01/12/2006\n");

if(argc == 7 && *argv[4]) argc = 5;
if(argc == 7 && *argv[3]) argc = 4;
if(argc == 7 && *argv[2]) argc = 3;
if(argc == 7 && *argv[1]) argc = 2;
if(argc != 3 && argc != 4)
  {
  printf("Error: argc=%d\n\n", argc);
  printf("Syntax: txt_short_lines in_txt_file out_txt_file [maxi_line_length = 72]\n");
  exit(-1);
  }
else
  {
  strcpy(filein,argv[1]);
  strcpy(fileout,argv[2]);
  if(argc == 4) sscanf(argv[3],"%d", &imax);
  else imax = 72;
  }

printf(" OK: filein=%s fileout=%s imax=%d\n", filein, fileout, imax);

if((fp_in = fopen(filein,"r")) == NULL) {
  printf(" Fatal error opening input file %s \n",filein);
  exit(-1);
  }

if((fp_out = fopen(fileout,"w")) == NULL) {
  printf(" Fatal error opening output file %s \n",fileout);
  fclose(fp_in);
  exit(-1);
  }

/* Write a first line to indicate the origin of the data: */
fprintf(fp_out,"%% From %s imax=%d\n", filein, imax);

/* Scan the file and make the conversion: */
  truncate_lines(fp_in, fp_out, imax);

fclose(fp_in);
fclose(fp_out);
return(0);
}
/*************************************************************************
*
* INPUT:
* imax: maximum number of characters in line
*************************************************************************/
static int truncate_lines(FILE *fp_in, FILE *fp_out, int imax)
{
char buffer[NMAX], cc, last_word[NMAX], line[NMAX];
int i0, line_is_full, end_was_found, buffer_len, line_len; 
register int i, k;

last_word[0] = '\0'; 
while(!feof(fp_in))
{
/* Copy last truncated word (if not equal to the full line): */
  if(strlen(last_word) != imax) {
    for(i = 0; i < strlen(last_word); i++) buffer[i] = last_word[i]; 
    i0 = i;
  } else {
    fprintf(fp_out, "%s \n", last_word); 
    i0 = 0;
  }

#ifdef DEBUG
buffer[i0] = '\0';
printf(" New line: i0=%d begin=>%s< \n", i0, buffer); 
#endif

/* Maximum length for a line will be 170 characters: */
  line_is_full = 0;
  end_was_found = 0;
/* <= since problems with long lines otherwise */
  for(i = i0; i < imax; i++) {
   if((cc = fgetc(fp_in)) != EOF ) {
    if(cc == '\n' || cc == '\0' || cc == '\r') {
       end_was_found = 1;
       break;
      }
/* Copy character: */
    else buffer[i] = cc;
   } else {
     break;
   }
  } /* EOF for loop */
  if(i == imax) line_is_full = 1;

/* Copy buffer to output file: */
  if(end_was_found || line_is_full) {
/* length of buffer: */
    buffer_len = i;
/* If last character was not \r or \n,
* look for last (and possibly troncated) word: */
    if(end_was_found) {
      line_len = buffer_len;
    } else {
      for(i = buffer_len; i >= 0; i--) if(buffer[i] == ' ') break;
/* Copy beginning of buffer to "line": */
      line_len = i;
    }
    for(i = 0; i < line_len; i++) line[i] = buffer[i];
    line[i] = '\n'; line[i+1] = '\0';
/* Copy end of buffer to "last_word": */
    k=0;
    for(i = line_len+1; i < buffer_len; i++) last_word[k++] = buffer[i];
    last_word[k] = '\0';
#ifdef DEBUG
   printf("Line: >%s< last_word =>%s<\n", line, last_word);
#endif
    fprintf(fp_out, "%s", line); 
    }
}  /* EOF while */
return(0);
}
