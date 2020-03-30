/*************************************************************************
* Program stat_ascii
* To compute statistics on a ASCII list 
*
*
* JLP
* Version 06/09/2006
*************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#define MINI(a,b) ((a) < (b)) ? (a) : (b)
#define MAXI(a,b) ((a) < (b)) ? (b) : (a)
/*
#define DEBUG
*/

static int compute_stats_from_list(FILE *fp_in, int icol_data, int icol_weights,
                                   float *mean, float *sigma, float* mini,
                                   float *maxi, int ireject);

int main(int argc, char *argv[])
{
char filein[128];
float mean, sigma, mini, maxi;
int icol_data, icol_weights, ireject;
FILE *fp_in;
register int i;

  printf("stat_ascii/ JLP/ Version 06/09/2006\n");

if(argc == 7 && *argv[4]) argc = 5;
if(argc == 7 && *argv[3]) argc = 4;
if(argc == 7 && *argv[2]) argc = 3;
if(argc == 7 && *argv[1]) argc = 2;
if(argc != 3 && argc != 4)
  {
  printf("Error/Bad syntax: argc=%d\n\n", argc);
  printf("Syntax:        stat_ascii in_file icol_data [icol_weights]\n");
  exit(-1);
  }
else
  {
  strcpy(filein,argv[1]);
  sscanf(argv[2],"%d", &icol_data);
  if(argc == 4)
      sscanf(argv[3],"%d", &icol_weights);
  else
      icol_weights = 0;
  }

printf(" OK: filein=%s icol_data=%d\n", filein, icol_data);

if((fp_in = fopen(filein,"r")) == NULL)
{
printf(" Fatal error opening input file %s \n",filein);
return(-1);
}
else
{
/* Scan the file and make the conversion: */
  ireject = 0;
  compute_stats_from_list(fp_in, icol_data, icol_weights, &mean, &sigma, 
                          &mini, &maxi, ireject);

/* Series of iterations with rejection of outliers: */
for (i = 0; i < 3; i++) {
  if((fp_in = fopen(filein,"r")) != NULL) {
    ireject = 1;
    compute_stats_from_list(fp_in, icol_data, icol_weights, &mean, &sigma, 
                            &mini, &maxi, ireject);
    }
 }
}

return(0);
}
/*************************************************************************
*
* INPUT:
*  icol_data : column number of data (in the range [1, 8])
*  icol_weights : column number of weights (in the range [1, 8])
*************************************************************************/
static int compute_stats_from_list(FILE *fp_in, int icol_data, int icol_weights,
                                   float *mean, float *sigma, float* mini,
                                   float *maxi, int ireject)
{
double sum, sumsq, sumw;
int ival, iweight, nn, i;
float val, weight, lowcut, highcut, bad_value;
char buffer[120];

bad_value = 0.;
printf("DEBUG 2012: bad_value=%f\n", bad_value);

*mini = 1.e+12;
*maxi = -1.e+12;
lowcut = 0.; highcut = 0.;

if(ireject) {
  lowcut = *mean - 2.5 * (*sigma);
  highcut = *mean + 2.5 * (*sigma);
  printf(" Now rejecting data points out of interval [%f %f]   (2.5 sigma)\n", 
          lowcut, highcut);
  }

if(icol_data < 1 || icol_data > 8) {
  printf("compute_stats_from_list/Only icol_data = 1 to 8 are allowed yet\n");
  fclose(fp_in);
  return(-1);
  }

if(icol_weights < 0 || icol_weights > 8) {
  printf("compute_stats_from_list/Only icol_weights = 1 to 8 are allowed yet\n");
  fclose(fp_in);
  return(-1);
  }

sumw = 0.;
sum = 0.; sumsq = 0.;
nn = 0;
i = 0;
while(1) {

/* Read new line */
  if(fgets(buffer,120,fp_in) == NULL) break;
  i++;

  ival = 0;
  val = 0.;
  weight = 1.; 

if(buffer[0] != '#' && buffer[0] != '%') {

  switch(icol_data) { 
    case 1:
      ival = sscanf(buffer, "%f", &val);
      break;
    case 2:
      ival = sscanf(buffer, "%*f %f", &val);
      break;
    case 3:
      ival = sscanf(buffer, "%*f %*f %f", &val);
      break;
    case 4:
      ival = sscanf(buffer, "%*f %*f %*f %f", &val);
      break;
    case 5:
      ival = sscanf(buffer, "%*f %*f %*f %*f %f", &val);
      break;
    case 6:
      ival = sscanf(buffer, "%*f %*f %*f %*f %*f %f", &val);
      break;
    case 7:
      ival = sscanf(buffer, "%*f %*f %*f %*f %*f %*f %f", &val);
      break;
    case 8:
      ival = sscanf(buffer, "%*f %*f %*f %*f %*f %*f %*f %f", &val);
      break;
    default:
      fprintf(stderr,"compute_stats_from_list/Fatal error, wrong column (icol_data=%d)\n", icol_data);
    exit(-1);
    }

  switch(icol_weights) { 
    case 1:
      iweight = sscanf(buffer, "%f", &weight);
      break;
    case 2:
      iweight = sscanf(buffer, "%*f %f", &weight);
      break;
    case 3:
      iweight = sscanf(buffer, "%*f %*f %f", &weight);
      break;
    case 4:
      iweight = sscanf(buffer, "%*f %*f %*f %f", &weight);
      break;
    case 5:
      iweight = sscanf(buffer, "%*f %*f %*f %*f %f", &weight);
      break;
    case 6:
      iweight = sscanf(buffer, "%*f %*f %*f %*f %*f %f", &weight);
      break;
    case 7:
      iweight = sscanf(buffer, "%*f %*f %*f %*f %*f %*f %f", &weight);
      break;
    case 8:
      iweight = sscanf(buffer, "%*f %*f %*f %*f %*f %*f %*f %f", &weight);
      break;
    default:
      weight = 1.;
      iweight = 1;
      break;
    }

/* DEBUG
  printf(" Buffer=%s\n", buffer);
  printf(" ival=%d iweight=%d nn=%d val=%f weight=%f\n", ival, iweight,
         nn, val, weight);
*/
  if(ival != 1 || iweight != 1) break; 
  if(ireject) {
    if((val > lowcut) && (val < highcut)) {
     *mini = MINI(val, *mini);
     *maxi = MAXI(val, *maxi);
     sumw += weight;
     sum += val * weight;
     sumsq += val * val * weight;
     nn++;
     } else {
#ifdef DEBUG
     printf("Rejecting line #%d \n", i);
#endif
     }
  } else if(val != bad_value) {
     *mini = MINI(val, *mini);
     *maxi = MAXI(val, *maxi);
     sumw += weight;
     sum += val * weight;
     sumsq += val * val * weight;
     nn++;
    }
  } /* EOF test on buffer[0] */
} /* EOF while(1) loop */

if(nn > 3) {
  sum = sum / sumw; 
  *mean = sum;
  sumsq = sumsq / sumw - sum * sum;
  *sigma = sqrt(sumsq); 
  printf("compute_stats_from_list/OK: nvalues=%d mean=%f sigma=%f (min=%f max=%f)\n", nn,
        *mean, *sigma, *mini, *maxi);
  }
else {
  fprintf(stderr," Error: too few points for computing statistics, nvalues= %d\n",
          nn);
  *sigma = 0.;
  *mean = 0.;
  }

fclose(fp_in);
return(0);
}
