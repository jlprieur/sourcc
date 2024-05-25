/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
* read_spectra.c 
* To read FITS spectrum tables 
*
* Main program to test jlp_read_spfits
*
***** Info about od ***********************************************
* To dump a file to the screen and see Ascii and Hexa codes:
*     od -cx file_name
* each line with 32 bytes
* first block of 512 bytes from first up to line starting with 1000 (Hexa)
* then from 2000 (2nd block), 3000 (3rd block), etc.
* (WARNING: all the lines are not displayed if filled with zeroes...)
* for decimal address:
*      od -Ad -cx file_name
*******************************************************************
*
* JLP
* Version 02/02/2006 
---------------------------------------------------------------------*/
#include   <math.h>
#include   <jlp_ftoc.h>

/* italk  = 0 (no info) 
*         = 1 (some info output to screen) 
*         = 2 (all info output to screen) 
*/
static int jlp_write_ascii(char *outfile, float *wavelength, 
                           float *flux, long npts, char *comments);
static int jlp_check_ascii(char *in_asciifile, float *wavelength, 
                           float *flux, long npts);

/*
#define DEBUG 1
*/

/* Main program to test jlp_read_spfits */
int main(int argc, char *argv[])
{
 float    *wavelength, *flux_norm, *flux_unnorm, *snr;
 long     *order_nb, npts;
 char      infile[60], outfile[60], comments[81];
 int       istatus, italk, icorot;

 wavelength = NULL; flux_norm = NULL; flux_unnorm = NULL; snr = NULL;
 order_nb = NULL;
 *comments = '\0';

 JLP_INQUIFMT();

 printf(" Test of jlp_read_spfits routine, to read FITS spectrum tables\n");
 printf(" Version 30-01-2006\n"); 

if(argc == 7 && *argv[4] != ' ' && *argv[4]) argc = 5;
if(argc == 7 && *argv[3] != ' ' && *argv[3]) argc = 4;
if(argc == 7 && *argv[2] != ' ' && *argv[2]) argc = 3;
if(argc == 7 && *argv[1] != ' ' && *argv[1]) argc = 2;
if(argc != 4 && argc != 5)
{
printf(" Syntax is: read_spectra input_file output_file icorot [talk]    with talk=0, 1 or 2\n");
exit(-1);
}
printf(" argc=%d\n", argc);

strcpy(infile,argv[1]);
strcpy(outfile,argv[2]);
sscanf(argv[3],"%d",&icorot);
if(argc == 5)
   sscanf(argv[4],"%d",&italk);
else
   italk = 1;

if(italk) printf(" Input spectrum: %s  output: %s icorot=%d talk=%d\n", 
                 infile, outfile, icorot, italk);
/*
int jlp_read_spfits(char *infile, char *comments, float **wavelength,
                    float **flux_unnorm, float **flux_norm, float **snr,
                    long **order_nb, long *nrows, int firstrow,
                    int nelements, int icorot, int vm, int italk);
*/

/* Read only wavelength and flux_norm: */
istatus = jlp_read_spfits(infile, comments, &wavelength, &flux_unnorm, 
                          &flux_norm, &snr, &order_nb, &npts, 0, 0,  
                          icorot, 1, italk);

/******* Output spectrum to ASCII file ********************************/
if(istatus == 0) {
  printf(" OK: status = %d \n", istatus);
  jlp_write_ascii(outfile, wavelength, flux_norm, npts, comments);
/* DEBUG for Kurucz only: */
  if(icorot == 0)
      jlp_check_ascii("vfijfd.9750.45", wavelength, flux_norm, npts);
  }

if(wavelength != NULL) free(wavelength);
if(flux_unnorm != NULL) free(flux_unnorm);
if(flux_norm != NULL) free(flux_norm);
if(snr != NULL) free(snr);
if(order_nb != NULL) free(order_nb);

JLP_END();
return 0;
}
/****************************************************************
* Output spectrum to ASCII file 
* 
****************************************************************/
static int jlp_write_ascii(char *outfile, float *wavelength, 
                              float *flux, long npts, char *comments)
{
int i;
FILE *fp;

/* Save output to ASCII file: */
  if((fp = fopen(outfile,"w")) == NULL)
    {
    printf("jlp_write_ascii/Error opening file: %s\n", outfile);
    return(-1);
    }

/* Save output to ASCII file: */
   fprintf(fp,"# %s\n", comments);
   for(i = 0; i < npts; i++) 
       {
       fprintf(fp,"%f %f\n",  wavelength[i], flux[i]); 
       }
   fclose(fp);

 printf("\n jlp_write_ascii/Output to %s\n", outfile);
return(0);
}
/****************************************************************
* Input spectrum from ASCII file (to check if JLP's conversion was OK) 
* 
****************************************************************/
static int jlp_check_ascii(char *in_asciifile, float *wavelength, 
                           float *flux, long npts)
{
float wave, flx;
double sumw, sumf, sumsqw, sumsqf, www, wwf, mean, sigma;
long npts1;
int i;
FILE *fp;
char buffer[80];

/* Input from ASCII file: */
  if((fp = fopen(in_asciifile,"r")) == NULL)
    {
    printf("jlp_check_ascii/Error opening file: %s\n", in_asciifile);
    return(-1);
    }
  fgets(buffer, 80, fp);
  fgets(buffer, 80, fp);
  sscanf(buffer,"%ld", &npts1);
  if(npts1 != npts) {
    fclose(fp);
    return(-1);
    }

/* Read ASCII file: */
   sumw = 0; sumf = 0.;
   sumsqw = 0.; sumsqf = 0.;
   for(i = 0; i < npts; i++) 
       {
       fscanf(fp,"%f %f %*f\n", &wave, &flx); 
       www = (wave - wavelength[i]);
       wwf = (flx - flux[i]);
if(i > 0 && i < 10) printf("wave=%e wavelength=%e www=%e wwf = %e\n", 
                            wave, wavelength[i], www, wwf);
       sumw += www; sumsqw += www * www;
       sumf += wwf; sumsqf += wwf * wwf;
       }
   fclose(fp);

 mean = sumw / (float) npts;
 sigma = sqrt(sumsqw / (float) npts - mean * mean);
 printf("\n jlp_check_ascii/ Wavelengths: mean=%e sigma=%e (npts=%ld)\n", 
            mean, sigma, npts);

 mean = sumf / (float) npts;
 sigma = sqrt(sumsqf / (float) npts - mean * mean);
 printf("\n jlp_check_ascii/ Flux: mean=%e sigma=%e\n", mean, sigma);
return(0);
}
