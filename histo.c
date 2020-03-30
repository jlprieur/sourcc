/**************************************************************************
* Program to compute the histogram of a list contained in an ASCII file 
*
* JLP
* Version 12/03/2007
**************************************************************************/
#include "jlp_ftoc.h"

/* Contained here: */
static int read_data_from_file(double **in_data, int *npts, int icol,
                               char *infile, int log_is_wanted);
static int compute_histo(double *in_data, int npts, double *histo_val,
                         int *histo_nval, double min_val,
                         double max_val, int nbins, int *nvalues);
static int save_to_file(double *histo_val, int *histo_nval, int nbins,
                        char *outfile, char *comments);

int main(int argc, char *argv[])
{
double *in_data, *histo_val, min_val, max_val;
int *histo_nval, npts, nbins, status, icol, log_is_wanted, nvalues;
char infile[60], outfile[60], comments[100];
register int i, k;

npts = 10;
nbins = 0;
min_val = 0.;
max_val = 0.;
strcpy(infile,"");
strcpy(outfile,"tmp.dat");

/* To handle "runs histo" */
for(k = 7; k > 0; k--) if(argc == k && argv[k-1][0] == '\0') argc = k-1;
if(argc < 4 && argc != 5) {
  printf("Error, argc=%d \n Syntax is:\n", argc);
  printf("runs histo input_list output_file icol,nbins [min,max] [log]\n");
  printf("Enter 0,0 if automatic scale for min/max\n");
  return(-1);
 }
sscanf(argv[1], "%s", infile);
sscanf(argv[2], "%s", outfile);
sscanf(argv[3], "%d,%d", &icol, &nbins);
if(icol == 0 || nbins == 0 || icol > 7) {
  fprintf(stderr, "Fatal error/invalid parameters: nbins=%d icol=%d\n", 
          nbins, icol);
  return(-1);
  }
if(argc >= 5) {
  sscanf(argv[4], "%lf,%lf", &min_val, &max_val);
  printf("OK: histogram with %d bins from %f to %f\n", nbins, min_val, max_val);
  } else {
  printf("OK: histogram with %d bins from min to max\n", nbins);
  }

log_is_wanted = 0;
if(argc >= 6) {
  if(!strncmp(argv[5], "log", 3)) {
   log_is_wanted = 1;
   }
  }

/* Read the input data */
status = read_data_from_file(&in_data, &npts, icol, infile, log_is_wanted);
if(status) {
  fprintf(stderr,"Fatal error in read_data_from_file status=%d\n", status);
  return(-1);
  }

histo_val = (double *)malloc(nbins * sizeof(double));
histo_nval = (int *)malloc(nbins * sizeof(double));

if(min_val == max_val) {
  min_val = in_data[0];
  max_val = in_data[0];
  for(i = 1; i < npts; i++) {
   min_val = MINI(min_val, in_data[i]);
   max_val = MAXI(max_val, in_data[i]);
   }
  printf("Min value=%f Max value = %f\n", min_val, max_val);
 }

compute_histo(in_data, npts, histo_val, histo_nval, min_val, max_val, nbins, 
              &nvalues);

sprintf(comments,"From %.20s min=%.2f max=%.2f nbins=%d nvalues=%d\n", 
        infile, min_val, max_val, nbins, nvalues);

save_to_file(histo_val, histo_nval, nbins, outfile, comments);

free(in_data);
free(histo_val);
free(histo_nval);

printf("histo/Output to %s \n", outfile);

return(0);
}

/**************************************************************************
* Compute the histogram of a double precision array
*
* INPUT:
* in_data: input array
* npts: number of points in in_data
* min_val, max_val: lower and upper values of in_data used for computing 
*                   the histogram
* nbins: number of bins of the output histogram
* 
* OUTPUT:
* histo_val: lower values of the bins
* histo_nval: number of data points in the bins 
*             histo_nval[i]: number of data points contained in the bin 
*             from histo_val[i-1] (included) to histo_val[i] (excluded)
**************************************************************************/
static int compute_histo(double *in_data, int npts, double *histo_val,
                         int *histo_nval, double min_val,
                         double max_val, int nbins, int *nvalues)
{
double step, ww;
register int i, k;

if(nbins <= 0 || max_val < min_val || npts <= 0) {
 fprintf(stderr, "compute_histo/Error computing histogram, bad input values\n");
 fprintf(stderr, "nbins=%d max_val=%f min_val=%f npts=%d\n",
         nbins, max_val, min_val, npts);
 return(-1);
 }

/* Initialization of the histogram: */
step = (max_val - min_val) / (double)nbins;
for(k = 0; k < nbins; k++) {
  histo_nval[k] = 0;
  histo_val[k] = min_val + (double)k * step;
  }

/* Computation of the histogram: */
*nvalues = 0;
for(i = 0; i < npts; i++) {
  ww = in_data[i];
  if(ww >= min_val && ww <= max_val) { 
printf("ww=%f, i=%d (min=%f max=%f)\n", ww, i, min_val, max_val);
    ww = (in_data[i] - min_val) / step;
    k = (int)ww; 
    if(k >= 0 && k < nbins) {
     histo_nval[k]++;
     (*nvalues)++;
     }
  }
 }

return(0);
}
/**************************************************************************
* Save histogram to ASCII file
*
* INPUT:
*   histo_val: lower values of the bins
*   histo_nval: number of data points in the bins 
*   nbins: number of bins of the histogram
*   outfile: output file name
*   comments: comments to be written on the first line of the file
**************************************************************************/
static int save_to_file(double *histo_val, int *histo_nval, int nbins,
                        char *outfile, char *comments)
{
FILE *fp;
register int i;

if((fp = fopen(outfile, "w")) == NULL) {
  fprintf(stderr, "save_to_file/Error opening %s \n", outfile);
  return(-1);
  }

/* First line with comments */
  fprintf(fp,"%%%% %s\n", comments);

for(i = 0; i < nbins; i++) {
  fprintf(fp,"%f %d\n", histo_val[i], histo_nval[i]);
  }

fclose(fp);
return(0);
}
/**************************************************************************
* Read data from an ASCII file
*
* INPUT:
*   infile: input ASCII file name
*   icol: number of the column to be read
*
* OUTPUT:
*   in_data: input array
*   npts: number of points in in_data
**************************************************************************/
static int read_data_from_file(double **in_data, int *npts, int icol,
                               char *infile, int log_is_wanted)
{
double value;
int ival;
register int i;
char buffer[100];
FILE *fp;

/* Open the file a first time to count the maximum number of values: */
if((fp = fopen(infile,"r")) == NULL) {
  fprintf(stderr,"read_data_from_file/Error opening %s\n", infile);
  return(-1);
  }

for(i = 0; !feof(fp); i++) fgets(buffer, 100, fp);
printf("read_data_from_file/nlines = %d\n", i);
fclose(fp);
if(i == 0) return(-2);

*in_data = (double *)malloc(i * sizeof(double));
/* Opening the file again to read the data now: */
if((fp = fopen(infile,"r")) == NULL) {
  fprintf(stderr,"read_data_from_file/Error opening %s\n", infile);
  return(-1);
  }

i = 0;
fgets(buffer, 100, fp);
while(!feof(fp)) {
   if(buffer[0] != '%' && buffer[0] != '\0' && buffer[0] != '\n') {
   switch(icol) {
     case 1:
       ival = sscanf(buffer, "%lf", &value);
       break;
     case 2:
       ival = sscanf(buffer, "%*f %lf", &value);
       break;
     case 3:
       ival = sscanf(buffer, "%*f %*f %lf", &value);
       break;
     case 4:
       ival = sscanf(buffer, "%*f %*f %*f %lf", &value);
       break;
     case 5:
       ival = sscanf(buffer, "%*f %*f %*f %*f %lf", &value);
       break;
     case 6:
       ival = sscanf(buffer, "%*f %*f %*f %*f %*f %lf", &value);
       break;
     case 7:
       ival = sscanf(buffer, "%*f %*f %*f %*f %*f %*f %lf", &value);
       break;
     default:
       fprintf(stderr, "read_data_from_file/Error: icol=%d\n", icol);
       return(-3);
     } /* EOF switch on icol */
   if(ival == 1) {
     if(log_is_wanted) {
       if(value > 0.) {
         value = log10(value);
         (*in_data)[i++] = value;
       }
     } else { 
     (*in_data)[i++] = value;
     }
     } /* EOF case ival == 1 */
   } /* EOF case buffer[0] != '%' */
 fgets(buffer, 100, fp);
 } /* EOF !feof(fp) */

fclose(fp);

*npts = i;

printf("read_data_from_file/ %d values read\n", *npts);
return(0);
}
