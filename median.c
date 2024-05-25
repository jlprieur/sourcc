/*************************************************************************
* Program to compute the median of a list of files
* (used to generate the "unresolved" mean autocorrelation of power spectrum
*  for speckle processing)
*
* JLP
* Version 22/02/2007
**************************************************************************/
#include <jlp_ftoc.h>

#define DEBUG

/* Contained in jlp_sort.c : 
int JLP_MEDIAN(float *data, int npts, float *value);
*/

/* Prototypes of files contained here: */
static int median_read_data(char *list_file, float ***img, int *nfiles,
                            INT4 *nx, INT4 *ny);
static int median_compute_mean(float *mean_img, float **img, int nfiles, 
                               INT4 nx, INT4 ny, float *a0, float *a1);
static int median_compute_coeff(float *model, float *imgg, INT4 nx, INT4 ny,
                                float *a0, float *a1);
static int median_compute_median(float *median_img, float **img, 
                                 INT4 nx, INT4 ny, int nfiles);

/***************************************************************************/
int main(int argc, char *argv[]) {
float **img, *mean_img, *median_img, *a0, *a1;
int nfiles, status, isize;
INT4 nx, ny;
int iter, i, k;
char list_file[60], out_fname[60], comments[80];

/* To handle "runs median" */
for(k = 7; k > 0; k--) if(argc == k && argv[k-1][0] == '\0') argc = k-1;
if(argc != 3) {
  printf("Error, argc=%d \n Syntax is:\n", argc);
  printf("median.exe list.dat output_file\n");
  printf("or: runs median list.dat output_file\n");
  return(-1);
 }
sscanf(argv[1], "%s", list_file);
sscanf(argv[2], "%s", out_fname);

JLP_INQUIFMT();

/* Read the files, allocate memory for img
* and load the pointers into img array */
status = median_read_data(list_file, &img, &nfiles, &nx, &ny);
#ifdef DEBUG
printf("median_read_data has read %d files, nx=%d ny=%d status=%d\n", 
        nfiles, nx, ny, status);
#endif

/* Allocate memory for mean_img (mean of all the images): */
mean_img = (float *)malloc(nx * ny * sizeof(float));
median_img = (float *)malloc(nx * ny * sizeof(float));
a0 = (float *)malloc(nfiles * sizeof(float));
a1 = (float *)malloc(nfiles * sizeof(float));

/* Initialize a0 and a1 arrays to compute the first mean: */
for(k = 0; k < nfiles; k++) {
 a0[k] = 0.; a1[k] = 1.;
 }

/* First computation of the mean with dummy a0 and a1:
* and then iterate to obtain a stable solution:
* (tested in 2007: 8 iterations are enough)
*/
for(iter = 0; iter < 8; iter++) {
  median_compute_mean(mean_img, img, nfiles, nx, ny, a0, a1);

  for(k = 0; k < nfiles; k++) 
     median_compute_coeff(mean_img, img[k], nx, ny, &a0[k], &a1[k]);

#ifdef DEBUG
printf("iter=%d a0[0]=%f a1[0]=%f\n", iter, a0[0], a1[0]);
#endif
  }
/* Compute mean another time (since the center has been zeroed
* by median_compute_coeff):
*/
  median_compute_mean(mean_img, img, nfiles, nx, ny, a0, a1);

/* Normalization of the images with those coefficients: */
 isize = nx * ny;
 for(k = 0; k < nfiles; k++) 
   for(i = 0; i < isize; i++) img[k][i] = (img[k][i] - a0[k]) / a1[k]; 

/* Compute the median of all frames: */
  median_compute_median(median_img, img, nx, ny, nfiles);

sprintf(comments,"Median computed from %s",list_file);
JLP_WRITEIMAG(median_img, &nx, &ny, &nx, out_fname, comments);

/* Save mean array (to see if there are some remnants of the
* autocorrelation peaks) : */
#ifdef DEBUG
strcpy(out_fname,"test_mean");
sprintf(comments,"Mean computed from %s",list_file);
JLP_WRITEIMAG(mean_img, &nx, &ny, &nx, out_fname, comments);
#endif

/* Free memory: */
for(k = 0; k < nfiles; k++) free(img[k]);
free(mean_img);
free(median_img);
free(a0);
free(a1);
return(0);
}
/**************************************************************************
* median_read_data
* to read all the images corresponding to the list contained in list_file
* 
* INPUT:
* list_file: name of the file containing the list of images
*
* OUTPUT:
* img: array of pointers with the addresses of the arrays
* nfiles: number of image files
* nx, ny: size of the images
**************************************************************************/
static int median_read_data(char *list_file, float ***img, int *nfiles,
                            INT4 *nx, INT4 *ny)
{
INT_PNTR pntr_ima;
INT4 nx1, ny1;
int status, k;
char fname[60], comments[80];
FILE *fp;

*nfiles = 0;

if((fp = fopen(list_file,"r")) == NULL) {
  printf("median_read_data/Error reading list of files contained in %s\n",
          list_file);
  return(-1);
  }

/* First go to count the files: */
k = 0;
while(!feof(fp)) {
  if(fscanf(fp, "%s\n", fname)) k++; 
}
fclose(fp);

/* Allocate memory space for pointers: */
*img = (float **)malloc(k * sizeof(float **));

/* Second go to open the files: */
if((fp = fopen(list_file,"r")) == NULL) {
  printf("median_read_data/Error reading list of files contained in %s\n",
          list_file);
  return(-1);
  }

k = 0;
while(!feof(fp)) {
  if(fscanf(fp, "%s\n", fname)) {
     status = JLP_VM_READIMAG1(&pntr_ima, &nx1, &ny1, fname, comments);
#ifdef DEBUG
     printf("OK: reading file %s, status = %d\n", fname, status);
#endif
     if(status != 0) {
       fprintf(stderr, "median_read_data/Error reading file %s status=%d\n", 
               fname, status);
       fclose(fp);
       return(-2);
       }
     if(k == 0) {
       *nx = nx1; *ny = ny1;
     } else { 
       if(*nx != nx1 || *ny != ny1) {
       fprintf(stderr, "median_read_data/Error, incompatible sizes: nx=%d nx1=%d ny=%d ny1=%d\n", 
               *nx, nx1, *ny, ny1);
       fclose(fp);
       return(-3);
       }
     }

     (*img)[k++] = (float *)pntr_ima;
     }
  }

*nfiles = k;
fclose(fp);
return(0);
}
/**************************************************************************
* median_compute_mean
* to compute the mean of all the images contained in img array 
* 
* INPUT:
* img: array of pointers with the addresses of the arrays
* nfiles: number of image files
* nx, ny: size of the images
* a0, a1: coefficients to be applied to the input images
*         (img - a0) / a1),   since model * a1 + a0 is close to img)
*
* OUTPUT:
* mean_img: mean array
**************************************************************************/
static int median_compute_mean(float *mean_img, float **img, int nfiles, 
                               INT4 nx, INT4 ny, float *a0, float *a1)
{
float ww;
int isize;
int i, k;

isize = nx * ny;
for(i = 0; i < isize; i++) mean_img[i] = 0.;

ww = 0.;
for(k = 0; k < nfiles; k++) {
  if(a1[k] == 0) {
   fprintf(stderr,"median_compute_mean/fatal error: a1[%d] is null!\n", k);
   exit(-1);
  }
  ww += 1./a1[k];
  for(i = 0; i < isize; i++) mean_img[i] += (img[k][i] - a0[k]) / a1[k];
  }

/* Normalization (i.e. division with the sum of the weights): */
for(i = 0; i < isize; i++) mean_img[i] /= ww;

return(0);
}
/**************************************************************************
* Compute the coefficients to normalized the input image relative to
* the model.
*
* Linear regression:
*
* Minimum of Sum ( z - a1 f - a0)^2
* is reached when gradient is nul, i.e., when:
* sum_fz = a1 sum_ff + a0 sum_f
* sum_z  = a1 sum_f + a0 sum_1
*
* INPUT:
* model: model to be fitted to imgg
* imgg: image array to be processed 
* nx, ny: size of the image
*
* OUTPUT:
* model: (central part is zeroed!)
* a0, a1: coefficients found for the fit
**************************************************************************/
static int median_compute_coeff(float *model, float *imgg, INT4 nx, INT4 ny,
                                float *a0, float *a1) 
{
float *tmp, wback, wdata;
double sum_1, sum_f, sum_z, sum_fz, sum_ff, det;
int tmp_dim, ixc, iyc, ii, jj, iside;
int i, j;

ixc = nx/2; iyc = ny/2;

/* Central pixels are generally bad, so I neutralize them for the fit:
* Warning: big scatter found for the pixels 62,64 and 66,64, so iside
* has to be >= 2:
*/
iside = 2;
tmp_dim = 2 * iside + 1;
tmp = (float *)malloc(tmp_dim * tmp_dim * sizeof(float));
for(j = -iside; j <= iside; j++) {
  for(i = -iside; i <= iside; i++) {
   ii = (ixc + i) + (iyc + j) * nx;
/* First save original data */
   tmp[(i + iside) + (j + iside) * tmp_dim] = imgg[ii];
/* Then set it to zero: */
   model[ii] = 0.;
   imgg[ii] = 0.;
   }
 }

sum_1 = 0.;
sum_f = 0.;
sum_z = 0.;
sum_fz = 0.;
sum_ff = 0.;
for(j = 0; j < ny; j++) {
  jj = j * nx;
  for(i = 0; i < nx; i++) {
   wback = model[i + jj];
   wdata = imgg[i + jj];
   sum_1 += 1.;
   sum_f += wback;
   sum_z += wdata;
   sum_fz += wback * wdata;
   sum_ff += SQUARE(wback);
  }
 }

/* Resolution with the determinant of the system:
* sum_fz = a1 sum_ff + a0 sum_f
* sum_z  = a1 sum_f + a0 sum_1
*/
det = sum_ff * sum_1 - sum_f * sum_f;
*a1 = (sum_1 * sum_fz  - sum_f * sum_z) / det;
*a0 = (sum_ff * sum_z  - sum_f * sum_fz) / det;

#ifdef DEBUGG
printf("median_compute_coeff/ a1=%f a0=%f\n", *a1, *a0);
#endif

/* Restore data: */
for(j = -iside; j <= iside; j++) {
  for(i = -iside; i <= iside; i++) {
   ii = (ixc + i) + (iyc + j) * nx;
   imgg[ii] = tmp[(i + iside) + (j + iside) * tmp_dim];
   }
 }

return(0);
}
/**************************************************************************
* median_compute_median
* to compute the median of all the images contained in img array
*
* INPUT:
* img: array of pointers with the addresses of the arrays
* nfiles: number of image files
* nx, ny: size of the images
*
* OUTPUT:
* median_img: median array
**************************************************************************/
static int median_compute_median(float *median_img, float **img, 
                                 INT4 nx, INT4 ny, int nfiles)
{
float *data, value;
int isize;
int i, j, k;

data = (float *)malloc(nfiles * sizeof(float));

isize = nx * ny;
for(i = 0; i < isize; i++) { 
    j = 0;
    for(k = 0; k < nfiles; k++) data[j++] = img[k][i];
    JLP_MEDIAN(data, nfiles, &value);
    median_img[i] = value;
  }

free(data);
return(0);
}
