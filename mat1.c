#include <math.h>
#define TINY 1.0e-20;

#define TEST
#ifdef TEST
#define NMAX 5
main()
{
float ll[NMAX][NMAX], uu[NMAX][NMAX], aa[NMAX][NMAX], dd;
int nn, indx[NMAX];
register int i, j, k;
nn = 4;
for(i = 1; i <= nn; i++) 
for(j = 1; j <= nn; j++) 
{ll[i][j] = 0.; uu[i][j] = 0.; aa[i][j] = 0.;}

/* Lower matrix: (diagonal to ones ...)*/
for(i = 1; i <= nn; i++)
for(j = 1; j <= i; j++)
     ll[i][j] = 1 + j - i;

/* Output: */
printf(" Lower matrix: \n");
for(i = 1; i <= nn; i++)
  {
  for(j = 1; j <= nn; j++)
    printf("%.5g ",ll[i][j]); 
  printf("\n"); 
  }  

/* Upper matrix: */
/*
for(i = 1; i <= nn; i++)
  for(j = i; j <= nn; j++)
     uu[i][j] = 1 + i + j;
*/
  for(j = 1; j <= nn; j++)
     uu[j][j] = 1;

/* Output: */
printf(" Upper matrix: \n");
for(i = 1; i <= nn; i++)
  {
  for(j = 1; j <= nn; j++)
    printf("%.5g ",uu[i][j]); 
  printf("\n"); 
  }  

/* Product: A = L*U */
for(i = 1; i <= nn; i++)
  for(j = 1; j <= nn; j++)
    {
    aa[i][j] = 0.;
    for(k = 1; k <= nn; k++)
     aa[i][j] += ll[i][k] * uu[k][j];  
    }

/* Output: */
printf(" Product matrix: \n");
for(i = 1; i <= nn; i++)
  {
  for(j = 1; j <= nn; j++)
    printf("%.5g ",aa[i][j]); 
  printf("\n"); 
  }  
/* Calling decomposition: */
#if 1
ludecomp(aa,nn,indx,&dd);
#else
ludcmp(aa,&nn,&nn,indx,&dd);
#endif

/* Determinant: */
for(j = 1; j <= nn; j++) dd *= aa[j][j];
printf("Determinant = %f \n",dd);

/* Output: */
printf(" Decomposed 'LU' matrix: \n");
for(i = 1; i <= nn; i++)
  {
  for(j = 1; j <= nn; j++)
    printf("%.5g ",aa[i][j]); 
  printf("\n"); 
  }  
printf(" Index: \n");
for(j = 1; j <= nn; j++)
    printf("%d ",indx[j]); 
  printf("\n"); 
/*******************************************************/
/* Check if correct: */
for(i = 1; i <= nn; i++)
  for(j = 1; j < i; j++)
     ll[i][j] = aa[i][j];
/* Fill the diagonal with ones: */
for(i = 1; i <= nn; i++) ll[i][i] = 1.;

for(j = 1; j <= nn; j++)
  for(i = j; i <= nn; i++)
     uu[j][i] = aa[j][i];

/* Product: A = L*U */
for(i = 1; i <= nn; i++)
  for(j = 1; j <= nn; j++)
    {
    aa[i][j] = 0.;
    for(k = 1; k <= nn; k++)
     aa[i][j] += ll[i][k] * uu[k][j];  
    }

/* Output: */
printf(" computed LU matrix from decomposition: \n");
for(i = 1; i <= nn; i++)
  {
  for(j = 1; j <= nn; j++)
    printf("%.5g ",aa[i][j]); 
  printf("\n"); 
  }  

}
#endif
/****************************************************************************
* LU decomposition (Numerical recipes in C p43)
* Given an n*n matrix aa, this routines replaces it by the LU decomposition
* of a rowwise permutation of itself.
*
* INPUT:
*  aa[j][i]
*  n
* OUTPUT:
*  aa[j][i]
*  indx[nn]:  vector which records the row permutation effected by the partial
*            pivoting. 
*  dd: +/-1 depending on whether the number of row interchanges 
            was even (dd=+1) or odd (dd=-1) 
****************************************************************************/
int ludecomp(aa,nn,indx,dd)
float aa[NMAX][NMAX], *dd;
int nn, *indx;
{
register int i, j, k;
int jmax;
float big, dum, sum, temp;
float *vv;

vv = (float *) malloc((nn+1) * sizeof(float));
if(!vv) {printf("ludecomp/Fatal error allocating memory space!\n");
         return(-1);}

/* No row interchanges yet: */
*dd = 1.0;

/* Loop over rows to get the implicit scale information: */
for(j = 1; j <= nn; j++)
  {
  big = 0.0;
    for(i = 1; i <= nn; i++)
       if((temp = fabs(aa[j][i])) > big) big = temp;
/* Largest element is null: */
  if(big == 0.) 
       {printf("ludecomp/Fatal error: singular matrix in row %d\n",j);
        return(-2);
       }
/* Save the scaling: */
  vv[j] = 1.0/big;
  } 

/* This is the loop over columns of Crout's method: */
for(i = 1; i <= nn; i++)
{
/* Equations: */
/* (12): beta_ij = a_ij - sum_{k=1}^{i-1} alpha_ik beta_kj */
/* (13): alpha_ij = (1/beta_jj) * (a_ij - sum_{k=1}^{i-1} alpha_ik beta_kj) */
/* This is equation (12) except for j=i: */
  if( i > 1)
  {
   for(j = 1; j < i; j++)
    {
    sum = aa[j][i];
     for(k = 1; k < i; k++) sum -= aa[j][k] * aa[k][i];
    aa[j][i] = sum;
    }
  }
/* Initialize for the search for largest pivot element: */
    big = 0.0;
/* This is equation (12) for j=i  and equation (13) for j=i+1, ..., nn: */
   for(j = i; j <= nn; j++)
    {
    sum = aa[j][i];
    if( i > 1)
    {
     for(k = 1; k < i; k++) sum -= aa[j][k] * aa[k][i];
    aa[j][i] = sum;
    }
/* Is the figure of merit for the pivot better than the best so far? */
    if((dum = vv[j] * fabs(sum)) >= big)
       {
         big = dum;
         jmax = j;
       }
    }
/* Do we need to interchange rows? */
    if( i != jmax)
     {
     printf(" interchange rows i=%d and jmax=%d\n",i,jmax);
/* Interchange: */
        for(k = 1; k <= nn; k++) 
        {
         dum = aa[jmax][k];
         aa[jmax][k] = aa[i][k];
         aa[i][k] = dum;
        }
/* Change the parity of dd: */
        *dd = -(*dd);
/* and interchange the scale factor: */
        dum = vv[jmax];
        vv[jmax] = vv[i];
        vv[i] = dum;
     }
     for(k = 1; k <= nn; k++) printf(" vv[%d] = %f",k,vv[k]);
     indx[i] = jmax;
     printf(" indx[%d] = %d\n",i, jmax);
/* Now finally divide by the pivot element: */
    if( i != nn)
     {
     if(aa[i][i] == 0.0) aa[i][i] = TINY;
     dum = 1.0 / aa[i][i];
/* If the pivot element is zero the matrix is singular (at least to the
* precision of the algorithm)
* For some applications on singular matrices it is desirable to substitute
* TINY for zero */
     for(j=i+1; j <= nn; j++) aa[j][i] *= dum;
     } 
/* Go back for the next column i in the reduction */
}
free(vv);
if(aa[nn][nn] == 0.0) aa[i][i] = TINY;
return(0);
}
