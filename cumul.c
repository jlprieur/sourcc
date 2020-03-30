/**************************************************************************
* Program to compute the cumulative function from a list
* contained in an ASCII file
*
* JLP
* Version 20/04/2007
**************************************************************************/
#include "jlp_ftoc.h"

static int read_data_from_file(double **in_data, int *npts, int icol,
                               char *infile, int log_is_wanted);
static int compute_cumul(double *in_data, int npts, double *cumul_val,
                         int *cumul_nval, double min_val,
                         double max_val, int nbins, int *nvalues);
static int save_to_file(double *cumul_val, int *cumul_nval, int nbins,
                        char *outfile, char *comments);

int main(int argc, char *argv[])
{
double *in_data, *cumul_val, min_val, max_val, step, ww;
int *cumul_nval, npts, nbins, status, icol, log_is_wanted, nvalues;
int auto_scale = 0;
char infile[60], outfile[60], comments[100];
register int i, k;

npts = 10;
min_val = 0.;
max_val = 0.;
strcpy(infile,"");
strcpy(outfile,"tmp.dat");

/* To handle "runs cumul" */
for(k = 7; k > 0; k--) if(argc == k && argv[k-1][0] == '\0') argc = k-1;
if(argc < 4 && argc != 5) {
  printf("Error, argc=%d \n Syntax is:\n", argc);
  printf("runs cumul input_list output_file icol [min,max,step] [log]\n");
  printf("Enter 0,0 if automatic scale for min/max\n");
  return(-1);
 }
sscanf(argv[1], "%s", infile);
sscanf(argv[2], "%s", outfile);
sscanf(argv[3], "%d", &icol);
if(argc >= 5) {
  auto_scale = 0;
  sscanf(argv[4], "%lf,%lf,%lf", &min_val, &max_val, &step);
  if(step != 0.) ww = 1. + (max_val - min_val)/step;
  else ww = 0.;
  nbins = (int)ww;
  printf("OK: cumulative function from %f to %f with a step of %f (nbins=%d)\n", 
          min_val, max_val, step, nbins);
  } else {
  nbins = 10;
  auto_scale = 1;
  printf("OK: cumulative function with %d bins from min to max\n", nbins);
  }

log_is_wanted = 0;
if(argc >= 6) {
  if(!strncmp(argv[5], "log", 3)) {
   log_is_wanted = 1;
   }
  }

if(icol == 0 || nbins <= 0 || icol > 7) {
  fprintf(stderr, "Fatal error/invalid parameters: nbins=%d icol=%d\n", 
          nbins, icol);
  return(-1);
  }

/* Read the input data */
status = read_data_from_file(&in_data, &npts, icol, infile, log_is_wanted);
if(status) {
  fprintf(stderr,"Fatal error in read_data_from_file status=%d\n", status);
  return(-1);
  }

cumul_val = (double *)malloc(nbins * sizeof(double));
cumul_nval = (int *)malloc(nbins * sizeof(int));

if(auto_scale) {
  min_val = in_data[0];
  max_val = in_data[0];
  for(i = 1; i < npts; i++) {
   min_val = MINI(min_val, in_data[i]);
   max_val = MAXI(max_val, in_data[i]);
   }
  step = (max_val - min_val)/(float)(nbins - 1);
  printf("Min value=%f Max value=%f step=%f\n", min_val, max_val, step);
 }

compute_cumul(in_data, npts, cumul_val, cumul_nval, min_val, max_val, nbins, 
              &nvalues);

sprintf(comments,"From %s min=%f max=%f nbins=%d nvalues=%d\n", 
        infile, min_val, max_val, nbins, nvalues);

save_to_file(cumul_val, cumul_nval, nbins, outfile, comments);

free(in_data);
free(cumul_val);
free(cumul_nval);

printf("cumul/Output to %s \n", outfile);

return(0);
}

/**************************************************************************
* Compute the cumulative function of a double precision array
*
* INPUT:
* in_data: input array
* npts: number of points in in_data
* min_val, max_val: lower and upper values of in_data used for computing 
*                   the cumulative function
* nbins: number of bins of the output cumulative function 
* 
* OUTPUT:
* cumul_val: lower values of the bins
* cumul_nval: number of data points in the bins 
*             cumul_nval[i]: number of data points contained in the bin 
**************************************************************************/
static int compute_cumul(double *in_data, int npts, double *cumul_val,
                         int *cumul_nval, double min_val,
                         double max_val, int nbins, int *nvalues)
{
double step, ww;
register int i, k;

if(nbins <= 0 || max_val < min_val || npts <= 0) {
 fprintf(stderr, "compute_cumul/Error computing cumulative function, bad input values\n");
 fprintf(stderr, "nbins=%d max_val=%f min_val=%f npts=%d\n",
         nbins, max_val, min_val, npts);
 return(-1);
 }

/* Initialization of the cumulative function: */
step = (max_val - min_val) / (double)(nbins - 1);
printf("min=%f max=%f step=%f nbins=%d\n", min_val, max_val, step, nbins);
for(k = 0; k < nbins; k++) {
  cumul_nval[k] = 0;
  cumul_val[k] = min_val + (double)k * step;
  }

/* Computation of the cumulative function: */
*nvalues = 0;
for(i = 0; i < npts; i++) {
  ww = in_data[i];
  if(ww >= min_val && ww <= max_val) {
    (*nvalues)++;
    for(k = 0; k < nbins; k++) if(ww <= cumul_val[k]) cumul_nval[k]++;
  }
}

return(0);
}
/**************************************************************************
* Save cumulative function to ASCII file
*
* INPUT:
*   cumul_val: lower values of the bins
*   cumul_nval: number of data points in the bins 
*   nbins: number of bins of the cumulative function
*   outfile: output file name
*   comments: comments to be written on the first line of the file
**************************************************************************/
static int save_to_file(double *cumul_val, int *cumul_nval, int nbins,
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
  fprintf(fp,"%f %d\n", cumul_val[i], cumul_nval[i]);
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
     printf("OK: i=%d value=%f\n", i, value);
     } /* EOF case ival == 1 */
   } /* EOF case buffer[0] != '%' */
 fgets(buffer, 100, fp);
 } /* EOF !feof(fp) */

fclose(fp);

*npts = i;

printf("read_data_from_file/ %d values read\n", *npts);
return(0);
}
