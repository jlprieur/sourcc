/**************************************************************************
* Program to perform Kolmogorov-Smirnov test
* from two lists contained in ASCII files
*
* JLP
* Version 23/04/2007
**************************************************************************/
#include "jlp_ftoc.h"

static int read_data_from_file(double **in_data, int *npts, int icol,
                               char *infile, int log_is_wanted);
static int compute_cumul2(double *in_data1, double *in_data2, int npts1, 
                          int npts2, double *cumul_val, double *cumul_nval1, 
                          double *cumul_nval2, double min_val, double max_val, 
                          int nbins, int *nvalues1, int *nvalues2);
static int kolmo_test(double *cumul_val, double *cumul_nval1, 
                      double *cumul_nval2, int nbins, double *Kdist);
static int save_to_file(double *cumul_val, double *cumul_nval1, 
                        double *cumul_nval2, int nbins, char *outfile, 
                        char *comments);

int main(int argc, char *argv[])
{
double *in_data1, *in_data2, *cumul_val;
double min_val, max_val, step, ww, Kdist;
double *cumul_nval1, *cumul_nval2; 
int npts1, npts2;
int nbins, status, icol1, icol2, log_is_wanted, nvalues1, nvalues2;
char infile1[60], infile2[60], outfile[60], comments[160];
int k;

min_val = 0.;
max_val = 0.;
strcpy(infile1,"");
strcpy(infile2,"");
strcpy(outfile,"tmp.dat");

/* To handle "runs kolmo_smirnov" */
for(k = 7; k > 0; k--) if(argc == k && argv[k-1][0] == '\0') argc = k-1;
if(argc < 4 && argc != 5) {
  printf("Error, argc=%d \n Syntax is:\n", argc);
  printf("runs kolmo_smirnov list1 list2 cumul_lists icol1,icol2 min,max,step\n");
  printf("Enter 0,0 if automatic scale for min/max\n");
  return(-1);
 }
sscanf(argv[1], "%s", infile1);
sscanf(argv[2], "%s", infile2);
sscanf(argv[3], "%s", outfile);
sscanf(argv[4], "%d,%d", &icol1, &icol2);
sscanf(argv[5], "%lf,%lf,%lf", &min_val, &max_val, &step);
  if(step != 0.) ww = 1. + (max_val - min_val)/step;
  else ww = 0.;
  nbins = (int)ww;
  printf("OK: cumulative functions from %f to %f with a step of %f (nbins=%d)\n", 
          min_val, max_val, step, nbins);

log_is_wanted = 0;
if(argc >= 7) {
  if(!strncmp(argv[6], "log", 3)) {
   log_is_wanted = 1;
   }
  }

if(icol1 <= 0 || icol2 <= 0 || nbins <= 0 || icol1 > 7 || icol2 > 7) {
  fprintf(stderr, "Fatal error/invalid parameters: nbins=%d icol=%d,%d\n", 
          nbins, icol1, icol2);
  return(-1);
  }

/* Read the input data */
status = read_data_from_file(&in_data1, &npts1, icol1, infile1, log_is_wanted);
if(status) {
  fprintf(stderr,"Fatal error reading %s status=%d\n", infile1, status);
  return(-1);
  }

status = read_data_from_file(&in_data2, &npts2, icol2, infile2, log_is_wanted);
if(status) {
  fprintf(stderr,"Fatal error reading %s status=%d\n", infile2, status);
  return(-1);
  }

/* Compute cumulative functions: */
cumul_val = (double *)malloc(nbins * sizeof(double));
cumul_nval1 = (double *)malloc(nbins * sizeof(double));
cumul_nval2 = (double *)malloc(nbins * sizeof(double));

compute_cumul2(in_data1, in_data2, npts1, npts2, cumul_val, cumul_nval1, 
               cumul_nval2, min_val, max_val, nbins, &nvalues1, &nvalues2);

/* Perform Kolmogorov-Smirnov test,
* i.e. computes maximum distance between the two cumulative
* functions
*/
kolmo_test(cumul_val, cumul_nval1, cumul_nval2, nbins, &Kdist);

sprintf(comments,"From %s and %s min=%.3f max=%.3f nbins=%d nvalues=%d,%d Kdist=%.3f", 
        infile1, infile2, min_val, max_val, nbins, nvalues1, nvalues2, Kdist);

save_to_file(cumul_val, cumul_nval1, cumul_nval2, nbins, outfile, comments);

free(in_data1);
free(in_data2);
free(cumul_val);
free(cumul_nval1);
free(cumul_nval2);

printf("cumul/Output to %s \n", outfile);

return(0);
}

/**************************************************************************
* Compute the cumulative functions of two double precision arrays
*
* INPUT:
* in_data1, in_data2: input arrays
* npts1: number of points in in_data1 a,d in_data2
* min_val, max_val: lower and upper values of in_data1 and in_data2 used 
*                   for computing the cumulative function
* nbins: number of bins of the output cumulative function 
* 
* OUTPUT:
* cumul_val: lower values of the bins
* cumul_nval1: number of data points in the bins (from in_data1 array) 
*              normalized to nvalues1. 
* cumul_nval2: number of data points in the bins (from in_data2 array) 
*              normalized to nvalues2. 
*             cumul_nval[i]: number of data points contained in the bin 
**************************************************************************/
static int compute_cumul2(double *in_data1, double *in_data2, int npts1, 
                          int npts2, double *cumul_val, double *cumul_nval1, 
                          double *cumul_nval2, double min_val, double max_val, 
                          int nbins, int *nvalues1, int *nvalues2)
{
double step, ww;
int i, k;

if(nbins <= 0 || max_val < min_val || npts1 <= 0 || npts2 <= 0) {
 fprintf(stderr, "compute_cumul2/Error computing cumulative function, bad input values\n");
 fprintf(stderr, "nbins=%d max_val=%f min_val=%f npts=%d,%d\n",
         nbins, max_val, min_val, npts1, npts2);
 return(-1);
 }

/* Initialization of the cumulative function: */
step = (max_val - min_val) / (double)(nbins - 1);
printf("min=%f max=%f step=%f nbins=%d\n", min_val, max_val, step, nbins);
for(k = 0; k < nbins; k++) {
  cumul_nval1[k] = 0.;
  cumul_nval2[k] = 0.;
  cumul_val[k] = min_val + (double)k * step;
  }

/* Computation of the cumulative function: */
*nvalues1 = 0;
for(i = 0; i < npts1; i++) {
  ww = in_data1[i];
  if(ww >= min_val && ww <= max_val) {
    (*nvalues1)++;
    for(k = 0; k < nbins; k++) if(ww <= cumul_val[k]) (cumul_nval1[k])++;
  }
}

*nvalues2 = 0;
for(i = 0; i < npts2; i++) {
  ww = in_data2[i];
  if(ww >= min_val && ww <= max_val) {
    (*nvalues2)++;
    for(k = 0; k < nbins; k++) if(ww <= cumul_val[k]) (cumul_nval2[k])++;
  }
}

/* Normalization to nvalues1 and nvalues2:
*/
ww = (double)*nvalues1;
for(k = 0; k < nbins; k++) cumul_nval1[k] /= ww;

ww = (double)*nvalues2;
for(k = 0; k < nbins; k++) cumul_nval2[k] /= ww;

return(0);
}
/**************************************************************************
* Save cumulative function to ASCII file
*
* INPUT:
*   cumul_val: lower values of the bins
*   cumul_nval1: relative number of data points in the bins 
*   cumul_nval2: relative number of data points in the bins 
*   nbins: number of bins of the cumulative function
*   outfile: output file name
*   comments: comments to be written on the first line of the file
**************************************************************************/
static int save_to_file(double *cumul_val, double *cumul_nval1, 
                        double *cumul_nval2, int nbins, char *outfile, 
                        char *comments)
{
FILE *fp;
int i;

if((fp = fopen(outfile, "w")) == NULL) {
  fprintf(stderr, "save_to_file/Error opening %s \n", outfile);
  return(-1);
  }

/* First line with comments */
  fprintf(fp,"%% %s\n", comments);

for(i = 0; i < nbins; i++) {
  fprintf(fp,"%f %f %f\n", cumul_val[i], cumul_nval1[i], cumul_nval2[i]);
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
int i;
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
   if(buffer[0] != '%' && buffer[0] != '\0') {
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
/*
     printf("OK: i=%d value=%f\n", i, value);
*/
     } /* EOF case ival == 1 */
   } /* EOF case buffer[0] != '%' */
 fgets(buffer, 100, fp);
 } /* EOF !feof(fp) */

fclose(fp);

*npts = i;

printf("read_data_from_file/ %d values read\n", *npts);
return(0);
}
/*************************************************************************
*
*************************************************************************/
static int kolmo_test(double *cumul_val, double *cumul_nval1, 
                      double *cumul_nval2, int nbins, double *Kdist)
{
double ww;
int kmax = 0;
int k;

*Kdist = 0.;

for(k = 0; k < nbins; k++) {
  ww = ABS(cumul_nval1[k] - cumul_nval2[k]);
  if(ww > *Kdist) {
    *Kdist = ww;
    kmax = k;
    }
  }

printf("kolmo_test/ maxi_distance = %f at k=%d (x=%f)\n", 
        *Kdist, kmax, cumul_val[kmax]);

return(0);
}
