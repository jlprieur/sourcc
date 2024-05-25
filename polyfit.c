/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
* polyfit.c 
* Main program calling routines of "polyfit_utils.c" that fit a polynomial P(x)
*
* JLP 
* Version 27-07-2007
-------------------------------------------------------------------*/
#define DEBUG

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <math.h>
#include <jlp_ftoc.h>

/* Defined in "polyfit_utils.c": */
int POLYFIT(double *xx, double *yy, int *npts, int *poly_order,
            double *xc, double *error_xc, double *rms_resid);
int CALPOLY(double *x, double *y, double *xc, int *poly_order);

/* Defined here: */
static int read_data(char *infile, int icol, double *xx, int *npts,
                     int nmax);
int jlp_read_column(char *buffer, int icol, float *value, int len);
int output_poly_and_data(char *outfile, char *comments, double *xx, 
                         double *yy, int npts, int poly_order, double *xc);

int main(int argc, char *argv[])
{
double xc[12], error_xc[12], rms_resid;
double xx[200], yy[200];
int status, nmax = 200, poly_order, icol_x, icol_y, npts_x, npts_y, npts;
char infile_x[60], infile_y[60], outfile[60], comments[200];

printf(" Program to fit a polynomial to data: Y = pol(X)\n");
printf(" JLP Version 27-07-2007 \n");

if(argc == 7 && *argv[5]) argc = 6;
if(argc == 7 && *argv[4]) argc = 5;
if(argc == 7 && *argv[3]) argc = 4;
if(argc == 7 && *argv[2]) argc = 3;
if(argc == 7 && *argv[1]) argc = 2;
if(argc != 6)
  {
  printf("Error/Bad syntax: argc=%d\n\n", argc);
  printf("Syntax:      polyfit_main infile_x icol_x infile_y icol_y order\n");
  exit(-1);
  }
else
  {
  strcpy(infile_x,argv[1]);
  sscanf(argv[2],"%d", &icol_x);
  strcpy(infile_y,argv[3]);
  sscanf(argv[4],"%d", &icol_y);
  sscanf(argv[5],"%d", &poly_order);
  }
printf("OK: infile_x=%s infile_y=%s icol_x=%d icol_y=%d order=%d\n",
        infile_x, infile_y, icol_x, icol_y, poly_order);
sprintf(outfile,"polyfit.dat");
sprintf(comments,"infile_x=%s infile_y=%s icol_x=%d icol_y=%d order=%d",
        infile_x, infile_y, icol_x, icol_y, poly_order);

/* Limitation of the order of the polynomial 
(since xc, error arrays are limited to 12)
*/
if(poly_order >= 10) printf("Fatal error, poly_order = %d\n", poly_order);

status = read_data(infile_x, icol_x, xx, &npts_x, nmax);
if(status) { 
  printf("Fatal error reading file >%s< \n", infile_x);
  return(-1);
  }
status = read_data(infile_y, icol_y, yy, &npts_y, nmax);
if(status) { 
  printf("Fatal error reading file >%s< \n", infile_y);
  return(-1);
  }
if(npts_x != npts_y){
   printf("Fatal error/inconsistent data: npts_x=%d npts_y=%d\n", 
           npts_x, npts_y);
   return(-1);
  } 
npts = npts_y;

/* Solve problem */
status = POLYFIT(xx, yy, &npts, &poly_order, xc, error_xc,
                 &rms_resid);

/* Output data points for a further plot */
if(!status) output_poly_and_data(outfile, comments, xx, yy, npts, poly_order, 
                                 xc);
return(0);
}
/*************************************************************************
*
* INPUT:
* infile_x: name of the input file with X data 
*
* OUTPUT:
* npts: number of data points 
*************************************************************************/
static int read_data(char *infile, int icol, double *xx, int *npts,
                     int nmax)
{
int status = 0, k, len=110;
float val;
FILE *fp_in;
char buffer[120];
int i;

k = 0;

if((fp_in = fopen(infile, "r")) == NULL) {
  printf("read_data/Error opening input file >%s<\n", infile);
  return(-1);
  }

/* Read new line */
for(i = 0; i < nmax; i++) {
  if(fgets(buffer,120,fp_in) == NULL) break;

status = jlp_read_column(buffer, icol, &val, len);
if(!status) xx[k++] = val;

printf("JLPPP:i=%d k=%d >%s< val =%f status=%d\n", i, k, buffer, val, status);
} /* EOF loop on i */

if(i == nmax) {
  printf("Fatal error: this program is limited to %d points\n", nmax);
  exit(-1);
  }

*npts = k;
printf("read_data/%d points successfully read from >%s<\n", *npts, infile);

fclose(fp_in);

return(0);
}
/*************************************************************************
*
* INPUT:
*  buffer[len]: string buffer to be decoded
*  icol: column number 
*  len: length of string buffer
*
* OUTPUT:
*  value: value successfully read if status=0 (0 otherwise, with status=-1)
*
*************************************************************************/
int jlp_read_column(char *buffer, int icol, float *value, int len)
{
int ival, status = -1;
int ic, i0, i1, no_blank, found = 0;
char pc[120];
float val;
int i, k;

  *value = 0;
  ival = 0;
  val = 0.;
/* Look for transitions between ' ' and .not.' ' : */
if(buffer[0] != '#' && buffer[0] != '%') {
  ic = 0; i0 = 0; i1 = 0;
  no_blank = (buffer[0] != ' ') ? 1 : 0;
  for(i = 1; i < len; i++){
/* Transition to blank (or end of string) was found: */
     if((buffer[i] == '\0') || (no_blank && buffer[i] == ' ')){ 
        ic++;
        if(icol == ic) {
           i1 = i;
           found = 1;
           break;
           }
        no_blank = 0;
        i0 = i;
        } else {
        no_blank = (buffer[i] != ' ') ? 1 : 0;
        }
     }

/* Copy isolated block between i0 and i1 indices: */
  if(found) {
  k=0;
  for(i = i0; i < i1; i++) pc[k++] = buffer[i];
  pc[k++] = '\0';

/* Decode this block */
  ival = sscanf(pc, "%f", value);
  if(ival == 1) status = 0;
  }
printf("i0=%d i1=%d ival=%d value=%f status=%d found = %d ic=%d\n", 
        i0, i1, ival, *value, status, found, ic);

} /* EOF case buffer != % and != # */

return(status);
}
/*****************************************************************************
*
*****************************************************************************/
int output_poly_and_data(char *outfile, char *comments, double *xx, 
                         double *yy, int npts, int poly_order, double *xc)
{
FILE *fp_out;
double ww;
int i;

if((fp_out = fopen(outfile,"w")) == NULL){
  printf("output_poly_and_data/Error opening output file >%s<\n", outfile);
  return(-1);
  }
  fprintf(fp_out,"%% %s\n", comments);
  fprintf(fp_out,"%% x y_O y_C O-C\n");

for(i = 0; i < npts; i++) {
  CALPOLY(&(xx[i]), &ww, xc, &poly_order);
  fprintf(fp_out,"%f %f %f %f\n", xx[i], yy[i], ww, yy[i] - ww);
  }

fclose(fp_out);
return(0);
}
