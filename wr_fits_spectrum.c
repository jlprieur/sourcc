/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
* wr_fits_spectrum.c 
*
* JLP
* Version 30/01/2006 
---------------------------------------------------------------------*/
#include   <stdio.h>
#include   <math.h>
#include   <jlp_ftoc.h>

#define  MAXIMUM(x,y)    (((x) > (y)) ? (x) : (y))
#define  MINIMUM(x,y)    (((x) < (y)) ? (x) : (y))
#define  POW2(x)    (x)*(x)

static int jlp_read_ascii_spect(char *infile, char *comments, float **wavel, 
                                float **flux, float **snr, long *npts);

/* italk  = 0 (no info) 
*         = 1 (some info output to screen) 
*         = 2 (all info output to screen) 
*/

/*
#define DEBUG 1
*/

int main(int argc, char *argv[])
{
 float    *wavel, *flux, *snr;
 long     npts;
 char     infile[60], outfile[60], comments[81];
 int      istatus, italk;

 wavel = NULL; flux = NULL; snr = NULL;
 *comments = '\0';

 JLP_INQUIFMT();

 printf(" wr_fits_spectrum\n");
 printf(" Version 01-02-2006\n"); 

if(argc == 7 && *argv[3] != ' ' && *argv[3]) argc = 4;
if(argc == 7 && *argv[2] != ' ' && *argv[2]) argc = 3;
if(argc == 7 && *argv[1] != ' ' && *argv[1]) argc = 2;
if(argc != 3 && argc != 4)
{
printf(" Syntax is: wr_fits_spectrum input_file output_file [talk]   with talk=0, 1 or 2\n");
exit(-1);
}
strcpy(infile,argv[1]);
strcpy(outfile,argv[2]);
if(argc == 4)
   sscanf(argv[3],"%d",&italk);
else
   italk = 1;

if(italk) printf(" wr_fits_spectrum: >%s<  talk=%d\n", infile, italk);
istatus = jlp_read_ascii_spect(infile, comments, &wavel, &flux, &snr, &npts);

printf(" Calling write to FITS table: status = %d\n", istatus);
if(istatus == 0) {
  sprintf(comments,"%s", infile);
/* Possibility of choosing between SNR output or not: */
#ifndef OUTPUT_OF_SNR_ALSO
  if(snr != NULL) free(snr);
  snr = NULL;
#endif
  istatus = jlp_write_spfits(outfile, comments, wavel, flux, snr, npts, italk);
  printf(" Output to FITS table: status = %d\n", istatus);
}

if(wavel != NULL) free(wavel);
if(flux != NULL) free(flux);
if(snr != NULL) free(snr);

JLP_END();
return 0;
}
/******************************************************************
*
* INPUT:
* infile: file name
*
* OUTPUT:
* npts: number of points
*
******************************************************************/
static int jlp_read_ascii_spect(char *infile, char *comments, float **wavel, 
                                float **flux, float **snr, long *npts)
{
char buffer[80];
long i;
FILE *fp;

if((fp = fopen(infile, "r")) == NULL ) {
   printf("jlp_read_ascii_spect/Error opening %s\n", infile);
   return(-1);
  }
/* Read number of points: */
  fgets(buffer, 80, fp);
  fgets(buffer, 80, fp);
  sscanf(buffer,"%ld", npts);

/* Allocate memory: */
if((*wavel = (float *) malloc((*npts)*sizeof(float))) == NULL) {
 printf("jlp_read_sp_bintable/Error allocating memory, npts=%ld\n",
        *npts);
 fclose(fp); return(-2);
 }
if((*flux = (float *) malloc((*npts)*sizeof(float))) == NULL) {
 printf("jlp_read_sp_bintable/Error allocating memory, npts=%ld\n",
        *npts);
 fclose(fp); return(-2);
 }

if((*snr = (float *) malloc((*npts)*sizeof(float))) == NULL) {
 printf("jlp_read_sp_bintable/Error allocating memory, npts=%ld\n",
        *npts);
 fclose(fp); return(-2);
 }

/* Read table: */
  i = 0;
  while (i < *npts) {
/* wavel: wavelength in nm, flux (relative value in ?) */
    fscanf(fp,"%f %f %f \n", &((*wavel)[i]), &((*flux)[i]), &((*snr)[i]));
    i++;
  }
  fclose(fp);

printf(" OK: %ld values read (npts=%ld) \n", i, *npts);

return(0);
}
