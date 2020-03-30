#include <stdio.h>
#include <math.h>
#include <malloc.h>
#include "../fitsio/fitsio.h"

/*
filename="mylib.c"
*/
#define MAXREAL 2e30
#define MAX(a,b) ( (a)>(b) ? (a) : (b))
#define MIN(a,b) ( (a)<(b) ? (a) : (b))
#define sgn(x)  (x==0)? 0 : (2*(x>0)-1)
#define MOINPUISS(a)  ( (((int) a) & 1)==0 ? (1) : (-1) )
#define ABS(x)  ( (x)>0 ? (x) : (-(x)) )
#define PI  3.1415926535897932384626
#define FRAC(x)  (x-(int) (x) )

void raz(float *tab,int dim);
void raz_d(double *tab, int dim);
void ascwrite(char *nomfic, int dimx, int dimy, float *tab);
void ascwrite3d(char *nomfic, int dimx, int dimy, int dimz, float *tab);
void ascopen(char *nomfic, int dimx, int dimy, float *tab);
void fitsopen(char *nomfic, int *dx, int *dy, float **tab);
void fitsopen1d(char *nomfic, int *dim, float **tab);
void ascopen(char *nomfic, int dimx, int dimy, float *tab);
void minmax(float *tab, int dimx, int dimy, 
            float *xmin, float *xmax, float *xmoy);
char *i_to_str(int a);
void iwrite(char *nomfic, int dimx, int dimy, float pmax, float *tab);
void fitswrite(char *nomfic, int dimx, int dimy, float *tab);
void rwrite(char *nomfic, int dimx, int dimy, float *tab);
void rwrite3d(char *nomfic, int dimx, int dimy, int dimz, float *tab);
void ropen(char *nomfic, int dimx, int dimy, float *tab);
void ropen3d(char *nomfic, int dimx, int dimy, int dimz, float *tab);
void lect(float *tab, char *nomfic, int dim);
void miroiry(float *im1, float *im2, unsigned int dimx, unsigned int dimy);
void scan_ext(char *nomtotfic, char *nomfic, char *ext);
void convolve(float *h, float *x, float *y, int dim, int dimcv);
void parfit(double x1, double x2, double x3, double y1, 
            double y2, double y3, double *a, double *b, double *c);
void interpol(float *tab1, float *tab2, int dim1, int dim2);
void tf1d(float *real, float *imag, int dim, int isign);
void tf2d(float *real, float *imag, int dim, int isign);
void CosSin(float *tcos,float *tsin,int m);
void RFLBTS(float *t,int n);
void RFFTH(float *Re,float *Im,float *tcos,float *tsin,int m,int n,int direct);
void tf2d_mod(float *real, float *imag, int dim, int isign);
void charwrite(char *nomfic, int dimx, int dimy, float *tab);

/* -------------------------------------------------------------------- */
/*    Mise a zero d'un tableau de reels  */
/* -------------------------------------------------------------------- */

void raz(float *tab,int dim)
{
 register int i;
 
 for (i=0;i<dim;i++) *(tab+i)=0;

}

/* -------------------------------------------------------------------- */
/*    Mise a zero d'un tableau de doubles  */
/* -------------------------------------------------------------------- */

void raz_d(double *tab, int dim)
{
 register int i;
 
 for (i=0;i<dim;i++) *(tab+i)=0;
}


/* -------------------------------------------------------------------- */
/*   Ecriture d'un fichier formatte de reels   */
/* -------------------------------------------------------------------- */

void ascwrite(char *nomfic, int dimx, int dimy, float *tab)
{
 register int i;
 FILE *fic,*fopen();
 
 if ((fic=fopen(nomfic,"w"))!=NULL)
 {
  for (i=0;i<dimx*dimy;i++) fprintf(fic,"%g\n",*(tab+i));
  fclose(fic); 
 }
 else printf("Pb ouverture fichier %s \n",nomfic);

}
/* -------------------------------------------------------------------- */
/*   Ecriture d'un fichier formatte de reels 3d  */
/* -------------------------------------------------------------------- */

void ascwrite3d(char *nomfic, int dimx, int dimy, int dimz, float *tab)
{
 register long i;
 FILE *fic,*fopen();
 
 if ((fic=fopen(nomfic,"w"))!=NULL)
 {
  for (i=0;i<(long)dimx*dimy*dimz;i++) fprintf(fic,"%g\n",*(tab+i));
  fclose(fic); 
 }
 else printf("Pb ouverture fichier %s \n",nomfic);

}
/* -------------------------------------------------------------------- */
/*   Lecture d'un fichier formatte de reels   */
/* -------------------------------------------------------------------- */

void ascopen(char *nomfic, int dimx, int dimy, float *tab)
{
 register int i;
 FILE *fic,*fopen();
  
 if ((fic=fopen(nomfic,"r"))!=NULL)
 {
  for (i=0;i<dimx*dimy;i++) fscanf(fic,"%f",&(*(tab+i)));
  fclose(fic); 
 }
 else printf("Pb ouverture fichier %s \n",nomfic);

}

/* -------------------------------------------------------------------- */
/*   Lecture d'un fichier fits 2D de reels   */
/* -------------------------------------------------------------------- */

void fitsopen(char *nomfic, int *dx, int *dy, float **tab)
{
    fitsfile *fptr;
    int naxis=2;
    long naxes[2];
    int status=0,anynul,nval;
    int bitpix=FLOAT_IMG;
    long nelements,fpixel; 
    int simple,extend;
    long pcount,gcount;
    int dimx,dimy;

    if (fits_open_file(&fptr,nomfic,READONLY, &status))
         printf("Erreur ouverture : status=%d\n",status);

    if (fits_read_imghdr(fptr,10,&simple,&bitpix,&naxis,naxes,&pcount,&gcount,&extend,&status))
          printf("Erreur lecture header: status=%d\n",status );
    else
    {
      printf("dimx=%ld\n",naxes[0]); *dx=dimx=naxes[0];
      printf("dimy=%ld\n",naxes[1]); *dy=dimy=naxes[1];
    }

   dimx=(int)naxes[0];
   dimy=(int)naxes[1];
   *tab=(float *)malloc(sizeof(float)*dimx*dimy); 
   fpixel=1;
   nelements=(long)(dimx*dimy);
   nval=-99;
   if (fits_read_img(fptr,TFLOAT,fpixel,nelements,&nval,*tab,&anynul,&status) )
          printf("Erreur lecture fichier: status=%d\n",status );
   if (fits_close_file(fptr,&status))
          printf("Erreur fermeture fichier: status=%d\n",status );


}

/* -------------------------------------------------------------------- */
/*   Lecture d'un fichier fits 1D de reels   */
/* -------------------------------------------------------------------- */

void fitsopen1d(char *nomfic, int *dim, float **tab)
{
    fitsfile *fptr;
    int naxis=1;
    long naxes[1];
    int status=0,anynul,nval;
    int bitpix=FLOAT_IMG;
    long nelements,fpixel; 
    int simple,extend;
    long pcount,gcount;
    int dimx;

    if (fits_open_file(&fptr,nomfic,READONLY, &status))
         printf("Erreur ouverture : status=%d\n",status);

    if (fits_read_imghdr(fptr,10,&simple,&bitpix,&naxis,naxes,&pcount,&gcount,&extend,&status))
          printf("Erreur lecture header: status=%d\n",status );
    else
    {
      printf("dimx=%ld\n",naxes[0]); *dim=dimx=naxes[0];
    }

   dimx=(int)naxes[0];
   *tab=(float *)malloc(sizeof(float)*dimx); 
   fpixel=1;
   nelements=(long)(dimx);
   nval=-99;
   if (fits_read_img(fptr,TFLOAT,fpixel,nelements,&nval,*tab,&anynul,&status) )
          printf("Erreur lecture fichier: status=%d\n",status );
   if (fits_close_file(fptr,&status))
          printf("Erreur fermeture fichier: status=%d\n",status );


}

/* -------------------------------------------------------------------- */
/*   Lecture d'un fichier formatte de reels   */
/* -------------------------------------------------------------------- */

void ascopen3d(nomfic,dimx,dimy,dimz,tab)

float *tab;
char nomfic[];
int dimx,dimy,dimz;

{
 register long i;
 FILE *fic,*fopen();
  
 if ((fic=fopen(nomfic,"r"))!=NULL)
 {
  for (i=0;i<(long)dimx*dimy*dimz;i++) fscanf(fic,"%f",&(*(tab+i)));
  fclose(fic); 
 }
 else printf("Pb ouverture fichier %s \n",nomfic);

}


/* -------------------------------------------------------------------- */
/*    Min et max d'un tableau de reels  */
/* -------------------------------------------------------------------- */

void minmax(float *tab, int dimx, int dimy, 
            float *xmin, float *xmax, float *xmoy)
{
 register int i;
 
 *xmoy=0;
 *xmin=1e30;
 *xmax=-1e30;
 for (i=0;i<dimx*dimy;i++)
 {
   *xmoy+=(*(tab+i))/dimx/dimy;
   *xmax=MAX(*(tab+i),*xmax);
   *xmin=MIN(*(tab+i),*xmin);
 }
}

/* -------------------------------------------------------------------- */
/*   Conversion entier -> chaine : i_to_str     */
/* -------------------------------------------------------------------- */

char *i_to_str(int a)

{
 char *s;
 
 s=(char *) malloc (80*sizeof(char));

 if (a==0) strcpy(s,"0"); 
 else sprintf(s,"%d",a);
 return(s);
}


/* -------------------------------------------------------------------- */
/*   Ecriture d'un fichier compact non formatte   */
/*  On ecrit sur 1 octet on doit entrer pmax si le tableau */
/*  n'est pas de 0 a 255 sinon pmax=0   */
/* -------------------------------------------------------------------- */

void iwrite(char *nomfic, int dimx, int dimy, float pmax, float *tab)

{
 register int i;
 char c;
 FILE *fics,*fopen();
 
  if ((fics=fopen(nomfic,"w"))!=NULL)
  {
   for (i=0;i<dimx*dimy;i++)
   {
    if (pmax!=0) tab[i]*=255/pmax;
    c=(int) (tab[i]+0.5); 
    putc(c,fics); 
   }
   fclose(fics);

  }
  else
  {
   puts("Pb ouverture fichier sortie");
  }
}
/* -------------------------------------------------------------------- */
/*   Ecriture d'un fichier de reels en FITS   */
/* -------------------------------------------------------------------- */

void fitswrite(char *nomfic, int dimx, int dimy, float *tab)
{
     fitsfile *fptr;
     int bitpix=FLOAT_IMG;
     int naxis =2,status=0;
     long naxes[2];
     long nelements,fpixel; 
     naxes[0] = dimx; 
     naxes[1] =dimy;

     if (fits_create_file(&fptr,nomfic,&status))
        printf("Erreur ouverture : status=%d\n",status);

     if (fits_create_img(fptr, bitpix,naxis,naxes,&status))
        printf("Erreur img : status=%d\n",status);

     fpixel=1;
     nelements = dimx*dimy;

     if (fits_write_img(fptr,TFLOAT,fpixel,nelements,tab,&status))
        printf("Erreur ecriture : status=%d\n",status);

     if (fits_close_file(fptr,&status))
        printf("Erreur fermeture : status=%d\n",status);

}
/* -------------------------------------------------------------------- */
/*   Ecriture d'un fichier de reels non formatte   */
/* -------------------------------------------------------------------- */

void rwrite(char *nomfic, int dimx, int dimy, float *tab)
{
 FILE *fics,*fopen();
 
  if ((fics=fopen(nomfic,"w"))!=NULL)
  {
   fwrite(tab,sizeof(float),dimx*dimy,fics);
   fclose(fics);

  }
  else
  {
   puts("Pb ouverture fichier sortie");
  }
}
/* -------------------------------------------------------------------- */
/*   Ecriture d'un fichier de reels non formatte 3d  */
/* -------------------------------------------------------------------- */

void rwrite3d(char *nomfic, int dimx, int dimy, int dimz, float *tab)
{
 FILE *fics,*fopen();
 
  if ((fics=fopen(nomfic,"w"))!=NULL)
  {
   fwrite(tab,sizeof(float),(long)dimx*dimy*dimz,fics);
   fclose(fics);

  }
  else
  {
   puts("Pb ouverture fichier sortie");
  }
}
/* -------------------------------------------------------------------- */
/*   Lecture d'un fichier de reels non formatte   */
/* -------------------------------------------------------------------- */

void ropen(char *nomfic, int dimx, int dimy, float *tab)
{
 FILE *fics,*fopen();
 
  if ((fics=fopen(nomfic,"r"))!=NULL)
  {
   fread(tab,sizeof(float),dimx*dimy,fics);
   fclose(fics);

  }
  else
  {
   puts("Pb ouverture fichier entree");
  }
}
/* -------------------------------------------------------------------- */
/*   Lecture d'un fichier de reels non formatte   */
/* -------------------------------------------------------------------- */

void ropen3d(char *nomfic, int dimx, int dimy, int dimz, float *tab)
{
 FILE *fics,*fopen();
 
  if ((fics=fopen(nomfic,"r"))!=NULL)
  {
   fread(tab,sizeof(float),(long)dimx*dimy*dimz,fics);
   fclose(fics);

  }
  else
  {
   puts("Pb ouverture fichier entree");
  }
}

/* -----------------------------------*/
/* Lecture des fichiers non formattes */
/* ----------------------------------*/

void lect(float *tab, char *nomfic, int dim)
{
 register int i;
 FILE *fic,*fopen();
 
 if ((fic=fopen(nomfic,"r"))!=NULL)
 {
  for (i=0;i<dim;i++) *tab++=(float) getc(fic); 
  fclose(fic); 
 }
 else puts("Pb ouverture fichier ");

}

/* ------------------------------------------------------------------------- 
  Miroir Y d une image     
 ------------------------------------------------------------------------- */

void miroiry(float *im1, float *im2, unsigned int dimx, unsigned int dimy)
{
 int i,j;
 printf("%d %d",dimx,dimy);
 for (j=0;j<dimx;j++)
 for (i=0;i<dimy;i++) *(im2+(dimy-i-1)*dimx+j) = *(im1+i*dimx+j);
}

/* -------------------------------------------------------------------------
 extraction de l'extension d'un fichier
 -------------------------------------------------------------------------  */

void scan_ext(char *nomtotfic, char *nomfic, char *ext)
{
 unsigned int l,i,j;
 
 l=strlen(nomtotfic);
 for (i=l-1;(((unsigned char) *(nomtotfic+i))!=46)&&(i>0);i--);
 if (i!=0)
 {
  for (j=0;j<i;j++) *(nomfic+j)=*(nomtotfic+j); *(nomfic+i)='\0';
  for (j=i+1;j<l;j++) {*(ext+j-i-1)=*(nomtotfic+j); *(ext+l-i-1)='\0';}
 }
 else {strcpy(nomfic,nomtotfic); *ext='\0';}
} 

/* -------------------------------------------------------------------------
 convolution 1D
float h[]:  convoluante
float x[]: fonction a convoluer
float y[]: fonction resultat
int dim: dimension de x et y
int dimcv: dimension convoluante
 ------------------------------------------------------------------------- */

void convolve(float *h, float *x, float *y, int dim, int dimcv)
{
 register int i,j;
 
 for (i=0; i<dimcv/2; i++) y[i]=x[i];
 for (i=dim-dimcv/2; i<dim; i++) y[i]=x[i];
 
 for (i=dimcv/2; i<dim-dimcv/2; i++)
 {
  y[i]=0;
  for (j=0; j<dimcv; j++) y[i]+=h[j]*x[i-j+dimcv/2];
 }
}

/* -------------------------------------------------------------------------
 Ajustement d'une parabole y=a x^2 + b x + c
 a partir de 3 couples de points (x1,y1), (x2,y2) et (x3,y3)
 ------------------------------------------------------------------------- */
void parfit(double x1, double x2, double x3, double y1, 
            double y2, double y3, double *a, double *b, double *c)

{
 double d,da,db;
 
  d=(x3*x3-x1*x1)*(x2-x1)-(x2*x2-x1*x1)*(x3-x1);
  da=(y3-y1)*(x2-x1)-(y2-y1)*(x3-x1);
  db=(x3*x3-x1*x1)*(y2-y1)-(x2*x2-x1*x1)*(y3-y1);
  *a=da/d ;
  *b=db/d;
  *c=y1-(*a)*x1*x1-(*b)*x1;
}

/* -------------------------------------------------------------------------
 Interpolation d'un tableau de points
 float *tab1,*tab2: tableaux de depart et d'arrivee
 dim1 est la dimension de depart, dim2 celle d'arrivee.
 ------------------------------------------------------------------------- */

void interpol(float *tab1, float *tab2, int dim1, int dim2)

{
 int i;
 float step;
 double x1,x2,x3,y1,y2,y3,a,b,c,x,y; 

 step=((float) dim1-1)/(dim2-1);
  
/* Traitement des premiers points  */

 for (i=0;(int) (i*step)<(dim1-2);i++)
 {
  x=(double) i*step;
  x1=(double) ((int) x);
  x2=(double) ((int) x+1);
  x3=(double) ((int) x+2);
  y1=(double) *(tab1+(int) x1);
  y2=(double) *(tab1+(int) x2);
  y3=(double) *(tab1+(int) x3);
  parfit(x1,x2,x3,y1,y2,y3,&a,&b,&c);
  y=a*x*x+b*x+c;
  *(tab2+i)=y;
 }

/* Traitement des derniers points  */

 for (i=dim2-2;(int) (i*step)>=(dim1-2);i--)
 {
  x=(double) i*step;
  y=a*x*x+b*x+c;
  *(tab2+i)=y;
 }
 *(tab2+dim2-1)=*(tab2+dim1-1); // Dernier point
 
}


/* -------------------------------------------------------------------- */
/*   TF a 1 dim (celle de GRABER)     */
/* -------------------------------------------------------------------- */

void tf1d(float *real, float *imag, int dim, int isign)
{
  float *liRe,*liIm,*tcos,*tsin,*Re,*Im; 
  float x,y,min,max,min2,max2; 
  unsigned long int i,mn,n; 
  int direct=isign,size=dim;

  Im=(float *)calloc(size,sizeof(float)) ;
  Re=(float *)calloc(size,sizeof(float)) ;
  liRe=(float *)calloc(size,sizeof(float)) ; 
  liIm=(float *)calloc(size,sizeof(float)) ; 
  tcos=(float *)malloc(size*sizeof(float)) ; 
  tsin=(float *)malloc(size*sizeof(float)) ; 

  mn=(int)(log((float)size+1)/log(2.));  n=size; 
  CosSin(tcos,tsin,mn); 
  for (i=0;i<size;i++) 
  {
   	liRe[i]=MOINPUISS(i)*real[i]; 
    	liIm[i]=MOINPUISS(i)*imag[i]; 
  }
  
   RFFTH(liRe,liIm,tcos,tsin,mn,size,direct) ; 
   RFLBTS(liRe,size);  RFLBTS(liIm,size) ; 
   for (i=0;i<size;i++) 
   {
    	Re[i]=liRe[i]; Im[i]=liIm[i]; 
   } 

  min=MAXREAL; max=-MAXREAL;
  min2=MAXREAL; max2=-MAXREAL;

  for (i=0;i<size;i++) 
  { 
   	x=MOINPUISS(i)*liRe[i]; y=MOINPUISS(i)*liIm[i];
     if (min>x) min=x;
     if (max<x) max=x;
     if (min2>y) min2=y;
     if (max2<y) max2=y;
     real[i]=x;
     imag[i]=y;
   }
    
   free(liRe); free(liIm); free(tcos); free(tsin);
   free(Re); free(Im); 

   return;
}


/* -------------------------------------------------------------------- */
/*   TF a 2 dim (celle de GRABER)     */
/* -------------------------------------------------------------------- */

void tf2d(float *real, float *imag, int dim, int isign)
{
  float *liRe,*liIm,*tcos,*tsin,*Re,*Im; 
  float x,y,min,max,min2,max2; 
  unsigned long int i,j,mn,n; 
  int direct=isign,size=dim;

  Im=(float *)calloc(size*size,sizeof(float)) ;
  Re=(float *)calloc(size*size,sizeof(float)) ;
  liRe=(float *)calloc(size,sizeof(float)) ; 
  liIm=(float *)calloc(size,sizeof(float)) ; 
  tcos=(float *)malloc(size*sizeof(float)) ; 
  tsin=(float *)malloc(size*sizeof(float)) ; 

  mn=(int)(log((float)size+1)/log(2.));  n=size; 
  CosSin(tcos,tsin,mn); 
    for (j=0;j<size;j++) 
       { 
          for (i=0;i<size;i++) 
           {
     liRe[i]=MOINPUISS(i+j)*real[(j*size)+i]; 
    liIm[i]=MOINPUISS(i+j)*imag[(j*size)+i]; 
           }
   RFFTH(liRe,liIm,tcos,tsin,mn,size,direct) ; 
   RFLBTS(liRe,size);  RFLBTS(liIm,size) ; 
          for (i=0;i<size;i++) 
           {
    Re[(j*size)+i]=liRe[i]; Im[(j*size)+i]=liIm[i]; 
           } 
    } 
 

  min=MAXREAL; max=-MAXREAL;
  min2=MAXREAL; max2=-MAXREAL;
   for (i=0;i<size;i++) 
       { 
      for (j=0;j<size;j++) 
           { 
           liRe[j]=Re[size*j+i]; liIm[j]=Im[size*j+i]; 
           } 
        RFFTH(liRe,liIm,tcos,tsin,mn,size,direct); 
        RFLBTS(liRe,size);  RFLBTS(liIm,size);

        for (j=0;j<size;j++) 
           { 
           x=MOINPUISS(i+j)*liRe[j]; y=MOINPUISS(i+j)*liIm[j];
     if (min>x) min=x;
     if (max<x) max=x;
     if (min2>y) min2=y;
     if (max2<y) max2=y;
     real[i+j*size]=x;
     imag[i+j*size]=y;
           }
    }
    
   free(liRe); free(liIm); free(tcos); free(tsin);
   free(Re); free(Im); 

   return;
}

void CosSin(float *tcos,float *tsin,int m) 
{ 
unsigned long int i,j,lix,jlx; 
float scl,arg; 
 
 for (i=1;i<=m;i++) { 
   lix=(int)pow(2.0,(float)(m+1-i)); 
   scl=(float)(2*M_PI)/(float)lix; 
   for (j=1;j<=lix/2;j++) { 
     jlx=j-1+lix/2; 
     arg=((float)(j-1))*scl; 
     tcos[jlx-1]=cos(arg); 
     tsin[jlx-1]=sin(arg); 
   } 
 } 
} 
 
void RFLBTS(float *t,int n) 
  { 
  unsigned long int n2,i,j,k; 
  float val; 
 
  n2=n/2; j=1; 
  for (i=1;i<n;i++) 
    { 
    if (i<j)  {  val=t[j-1]; t[j-1]=t[i-1]; t[i-1]=val;  }; 
    k=n2; 
    while (k<j)  { j=j-k; k=k/2; } 
    j=j+k; 
    } 
  } 

/*
void RFLBTSH(float *t,int n) 
  { 
  int n2,i,j,k; 
  float val,inter ; 
 
  n2=n/2; j=1; 
  for (i=1;i<n;i++) 
    { 
    if (i<j)  {  val=t[j-1]; inter=t[i-1]; t[j-1]=inter; t[i-1]=val;  }; 
    k=n2; 
    while (k<j) {  j=j-k; k=k/2;  } 
    j=j+k; 
    } 
  }  
  */
 
void RFFTH(float *Re,float *Im,float *tcos,float *tsin,int m,int n,int direct) 
    { 
 int i,j,k,j1,j2,lix,lmx,jlx; 
 float t1,t2,c,s; 
 
 for (i=1;i<=m;i++) 
   { 
   lmx=(int)(pow(2.0,(float)(m-i)));  lix=2*lmx; 
   for (j=1;j<=lmx;j++) 
    { 
    jlx=lmx+j-1;  c=tcos[jlx-1];  s=-direct*tsin[jlx-1]; 
    for(k=lix;k<=n;k+=lix) 
   { 
   j1=k-lix+j;     j2=j1+lmx; 
   t1=Re[j1-1]-Re[j2-1];    t2=Im[j1-1]-Im[j2-1]; 
   Re[j1-1]=Re[j1-1]+Re[j2-1];   Im[j1-1]=Im[j1-1]+Im[j2-1]; 
   Re[j2-1]=c*t1+s*t2;    Im[j2-1]=c*t2-s*t1;  
    } 
    } 
  } 
   } 

/* -------------------------------------------------------------------- */
/*   TF en module         */
/* -------------------------------------------------------------------- */

void tf2d_mod(float *real, float *imag, int dim, int isign)

{
 int i;
 tf2d(real,imag,dim,isign);
 for (i=0; i<dim*dim; i++) 
 {
  *(real+i)=sqrt(*(real+i)* *(real+i) + *(imag+i)* *(imag+i));
  *(imag+i)=0;
 }
 
}
void charwrite(char *nomfic, int dimx, int dimy, float *tab)
{
        register int i;
        char c;
        FILE *fics,*fopen();

                if ((fics=fopen(nomfic,"w"))!=NULL)
                {
                        for (i=0;i<dimx*dimy;i++)
                        {
                                c=(int) (tab[i]+0.5); 
                                putc(c,fics); 
                        }
                        fclose(fics);

                }
                else
                {
                        puts("Pb ouverture fichier sortie");
                }
}
