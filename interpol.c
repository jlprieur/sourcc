/************************************************************************
* routines for interpolation 
*
************************************************************************/
#include <stdio.h>
#include <math.h>

int interpol_decr(double *tab_x, double *tab_y, int nn, double xx, double *yy);

int interpol_parfit(double x1, double x2, double x3, double y1, double y2, 
                    double y3, double *aa, double *bb, double *cc);
/************************************************************************
*
* INPUT:
* tab_x[nn]: abscissa sorted in decreasing order
* tab_y[nn]: data (function of tab_x) to be read
* nn : size of tab_x, tab_y
* xx: x value of abscissa
*
* OUTPUT:
*  yy: y value corresponding to xx, interpolated in tab_y using
*      interpolated value from xx in tab_x
*
*************************************************************************/
int interpol_decr(double *tab_x, double *tab_y, int nn, double xx, double *yy)
{
int i0, found;
double x1, x2, x3, y1, y2, y3, aa, bb, cc;
register int i;

*yy = 0.;

 i0 = 0; found = 0;
 for(i = nn-1; i >= 1; i--){
    if(xx < tab_x[i]) {
      i0 = i;
      found = 1;
      break;
    }
  }

 if(!found) {
   printf("interpol_decr/Fatal error: xx=%f is not in array\n", xx);
   return(-1);
   }

/* Case of upper edge: 
*/
   if(i0 == nn-1) i0--;

   x1 = tab_x[i0 - 1];
   x2 = tab_x[i0];
   x3 = tab_x[i0 + 1];
   y1 = tab_y[i0 - 1];
   y2 = tab_y[i0];
   y3 = tab_y[i0 + 1];
   interpol_parfit(x1, x2, x3, y1, y2, y3, &aa, &bb, &cc);
   *yy = aa * xx * xx + bb * xx + cc;

return(0);
}
/***************************************************************************
* Least-squares fit of a parabol: y=a x^2 + b x + c
* from 3 couples of points (x1,y1), (x2,y2) et (x3,y3)
* (From Eric Aristidi)
***************************************************************************/
int interpol_parfit(double x1, double x2, double x3, double y1, double y2, 
                    double y3, double *aa, double *bb, double *cc)
{
double dd, da, db;

  dd = (x3*x3-x1*x1)*(x2-x1)-(x2*x2-x1*x1)*(x3-x1);
  da = (y3-y1)*(x2-x1)-(y2-y1)*(x3-x1);
  db = (x3*x3-x1*x1)*(y2-y1)-(x2*x2-x1*x1)*(y3-y1);
  *aa = da/dd;
  *bb = db/dd;
  *cc = y1 - (*aa) * x1 * x1 -(*bb) * x1;

return(0);
}
