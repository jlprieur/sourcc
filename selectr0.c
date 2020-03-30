/*
Je t'envoie mon programme selectr0.
Ce n'est pas vraiment un fit, il calcule les premiers points du spectre
et fait un classement en n classes de r0 suivant les valeurs trouvees.
Il y a deux passes : la premiere, il scanne toutes les images, regarde
la gamme de r0 et sauve dans un fichier tampon le nom de chaque image
avec le r0 trouve (ce n'est pas vraiment r0 mais un nombre qui lui
ressemble : le vrai r0 depend de lambda, la focale, etc...).
Puis un deuxieme passage ouvre le fichier tampon découpe le range de r0
en n, et ventile les images en appendant au nom de fichier un "_n".
C'est assez long mais je suis surpris de voir comment ca marche bien.
Dans l'article des mesures de 94 il y avait des exemples d'images
typiques des differentes classes.

A bientot

eric

*/
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/types.h>
#include <sys/stat.h>
/*
#include "/Robert/aris/bib/mylib.c"
*/

/* ------------------------------------------------------------------ */
/* Programme principal */
/* ------------------------------------------------------------------ */

aris_r0calc(ima,tmp,dim,r0)

float	*ima,*tmp,*r0;
int	dim;
{
	int	isign=1,f12_1,f12_2;
	float	pix=206265./0.0119,lambda=5e-7,x,y1,y2,r01,r02;

	raz(tmp,dim*dim);
	
//	Calcul TF
//	---------
	tf2d_mod(ima,tmp,dim,isign);

	f12_1=4;
	x=*(ima+dim*dim/2+dim/2);
	y1=*(ima+dim*dim/2+dim/2+f12_1);
	y1+=*(ima+dim*(dim/2+f12_1)+dim/2);
	y1/=2;
	f12_2=3;
	y2=*(ima+dim*dim/2+dim/2+f12_2);
	y2+=*(ima+dim*(dim/2+f12_2)+dim/2);
	y2/=2;

//	Calcul r0
//	---------
	pix=0.0119/206265.;lambda=0.00000065;
	r01=pow(3.44/log(x/y1),0.6)*lambda*(float)f12_1/pix/dim;
	r02=pow(3.44/log(x/y2),0.6)*lambda*(float)f12_2/pix/dim;
	*r0=(r02+r01)/2.;
}

/* ------------------------------------------------------------------ */
/* Test des images pourries */
/* ------------------------------------------------------------------ */

int testfic(nom,dim,format)
char *nom;
int  dim,format;
{
	struct stat b;
	int taille,rtn=0;
	
	
	taille=dim*dim; if (format!=0) taille*=4;
	if (stat(nom,&b)) 
	{
		printf("Pb stat file %s\n",nom);
	}
	else
	{
		if (((int)(taille-b.st_size))!=0) 
		{
		printf("%s -- taille : %ld - norm=%d\t",nom,b.st_size,taille);
		rtn=1;
		}
	}
		return(rtn);
}

int test(ima,dim)

float	*ima;
int	dim;
{
	register int i;
	int	rtn=0;
	float	pmax=-MAXREAL,pmin=MAXREAL;
	
	for (i=0; i<dim*dim; i++)
	{
		pmax=MAX(pmax,*(ima+i)); 
		pmin=MIN(pmin,*(ima+i)); 
	}
	if (pmax==pmin) rtn=1;
	
	return(rtn);
}


// =================================================================
//  	Programme principal
// =================================================================

main()
{
	int	dim,format_b,format_r,flag;
	float	*ima,*tmp,r0,sigma,moyr0,sigmar0;
	float	maxr0=-1,minr0=10000;
	int 	nfic,i,j,k,nbim_b,nbim_r,class,nb_class,x,ttmp=0;
	char	nomdir_b[40],nomdir_r[40],ficname[80],nomgenfic[30],*systemcall;
	char	nomgendbl[30],nomgenref[30],nomdir[40],fichier[80];
	FILE	*fic,*fic1,*fopen();
	int	nc[100];	


	printf("[0] Calcul des r0 ou [1] classement : ");
	scanf("%d",&flag);

	if (!(flag))
	{	
/*	Input des parametres  */
	
	printf("Dimension images entree =");
	scanf("%d",&dim);
	
	printf("Directory des images binaire  =");
	scanf("%s",&nomdir_b);
	
	printf("format binaire [0=bin, 1=asc, 2=float] :");
	scanf("%d",&format_b);
	
	printf("Directory des images ref  =");
	scanf("%s",&nomdir_r);
	
	printf("format ref [0=bin, 1=asc, 2=float] :");
	scanf("%d",&format_r);
	
	
/*	Ecriture sur disque du directory des images reference et dbl */

	systemcall=(char *) malloc(80);
	sprintf(systemcall,"ls %s > /tmp/lsdbl\n",nomdir_b);
	system(systemcall);
	sprintf(systemcall,"ls %s | wc -l > /tmp/nbdbl\n",nomdir_b);
	system(systemcall);  fic=fopen("/tmp/nbdbl","r"); fscanf(fic,"%d",&nbim_b); fclose(fic);
	sprintf(systemcall,"ls %s > /tmp/lsref\n",nomdir_r);
	system(systemcall);
	sprintf(systemcall,"ls %s | wc -l > /tmp/nbref\n",nomdir_r);
	system(systemcall);  fic=fopen("/tmp/nbref","r"); fscanf(fic,"%d",&nbim_r); fclose(fic);

/*	Alloc. Memoire virtuelle */

	ima=(float *) malloc(dim*dim*sizeof(float));
	tmp=(float *) malloc(dim*dim*sizeof(float));

//	 Traitement des images binaires
//	 ------------------------------
	printf("Traitement des %d images binaires ----\n",nbim_b);
	system("date");
	fic1=fopen("binr0","w");
	fic=fopen("/tmp/lsdbl","r");
	ttmp=nbim_b;
	k=0;
	while (k<nbim_b)
	{
		fscanf(fic,"%s",&nomgenfic);
		sprintf(ficname,"%s/%s",nomdir_b,nomgenfic);
		switch(format_b)
		{
			case 0 : {lect(ima,ficname,dim*dim); break;}
			case 1 : {ascopen(ficname,dim,dim,ima);break;}
			case 2 : {ropen(ficname,dim,dim,ima); break;}
		}

		if (test(ima,dim) || testfic(ficname,dim,format_b))
		{
			printf("   Image %s pourrie \n",ficname);
			sprintf(systemcall,"rm %s",ficname);
			system(systemcall);
			k++; ttmp--;
		}
		else
		{
			r0calc(ima,tmp,dim,&r0);
			printf("   Image no %d : %s - r0=%g \n",k+1,ficname,r0);
			if (k<nbim_b)  fprintf(fic1,"%d %g %s\n",k,r0,ficname);
			k++;
		}		
	}
	fclose(fic); fclose(fic1);
	if (ttmp!=nbim_b) {fic=fopen("/tmp/nbdbl","w"); fprintf(fic,"%d\n",ttmp); fclose(fic);}



//	 Traitement des images ref
//	 -------------------------
	printf("Traitement des %d images ref ----\n",nbim_r);
	system("date");
	ttmp=nbim_r;
	k=0;
	fic1=fopen("refr0","w");
	fic=fopen("/tmp/lsref","r");
	while (k<nbim_r)
	{
		fscanf(fic,"%s",&nomgenfic);
		sprintf(ficname,"%s/%s",nomdir_r,nomgenfic);
		switch(format_r)
		{
			case 0 : {lect(ima,ficname,dim*dim); break;}
			case 1 : {ascopen(ficname,dim,dim,ima);break;}
			case 2 : {ropen(ficname,dim,dim,ima); break;}
		}


		if (test(ima,dim) || testfic(ficname,dim,format_r))
		{
			printf("   Image %s pourrie \n",ficname);
			sprintf(systemcall,"rm %s",ficname);
			system(systemcall);
			k++; ttmp--;
		}
		else
		{
			r0calc(ima,tmp,dim,&r0);
			if (k<nbim_r)  fprintf(fic1,"%d %g %s\n",k,r0,ficname);
			printf("   Image no %d : %s - r0=%g \n",k+1,ficname,r0);
			k++;
		}
	}
	fclose(fic); fclose(fic1);
	if (ttmp!=nbim_r) {fic=fopen("/tmp/nbref","w"); fprintf(fic,"%d\n",ttmp); fclose(fic);}

	}

	else
	{
	printf("Dimension images =");
	scanf("%d",&dim);
	
	printf("Directory des images binaire  =");
	scanf("%s",&nomdir_b);
	
	printf("Directory des images ref  =");
	scanf("%s",&nomdir_r);
	
	printf("format binaire [0=bin, 1=asc, 2=float] :");
	scanf("%d",&format_b);
	
	printf("format ref [0=bin, 1=asc, 2=float] :");
	scanf("%d",&format_r);
	
	printf("Nom generique des images dbl classees  =");
	scanf("%s",&nomgendbl);
	
	printf("Nom generique des images ref classees  =");
	scanf("%s",&nomgenref);
	
	printf("Directory des fichiers binr0 et refr0 =");
	scanf("%s",&nomdir);
	
	ima=(float *) malloc(dim*dim*sizeof(float));
	systemcall=(char *) malloc(80);

	fic=fopen("/tmp/nbdbl","r"); fscanf(fic,"%d",&nbim_b); fclose(fic);
	fic=fopen("/tmp/nbref","r"); fscanf(fic,"%d",&nbim_r); fclose(fic);
	
	printf("Recherche du min et du max en r0 ------\n");
	sprintf(ficname,"%s/binr0",nomdir);
	fic=fopen(ficname,"r");
	minr0=1e30;
	maxr0=-1e30;
	moyr0=0;
	sigmar0=0;
	for (k=1; k<=nbim_b; k++)
	{	
		fscanf(fic,"%d",&x); fscanf(fic,"%f",&r0);
		fscanf(fic,"%s",&fichier);
		minr0=MIN(r0,minr0);
		maxr0=MAX(r0,maxr0);
		moyr0+=r0/nbim_b;
		sigmar0+=r0*r0;
	}
	sigmar0=sqrt(sigmar0/nbim_b-moyr0*moyr0);
	fclose(fic);
	printf("minr0=%g   maxr0=%g  moyr0=%g  sigmar0=%g\n",minr0,maxr0,moyr0,sigmar0);
		

//	 Repartition en classes (classes calculees sur la double)
//	 --------------------------------------------------------
	nb_class=5;
	for (k=0; k<=nb_class; k++) nc[k]=0;
		
	/* binaire */
	printf("Repartition en classes : binaire -----\n");
	system("date");
	k=1;
	sprintf(ficname,"%s/binr0",nomdir);
	fic=fopen(ficname,"r");
	for (k=1; k<=nbim_b; k++)
	{
		fscanf(fic,"%d",&x); fscanf(fic,"%f",&r0);
		fscanf(fic,"%s",&ficname);
//		class=(int)((r0-minr0)/(maxr0-minr0)*nb_class);
//		class=MIN(class,nb_class-1);
//		class++;
		class=1+(int)(nb_class*(r0-moyr0+2.5*sigmar0)/(5*sigmar0));
		printf("   Image %s - class=%d\n",ficname,class); nc[class]++;
		sprintf(fichier,"%s/%s%d_%d",nomdir_b,nomgendbl,class,nc[class]);
		sprintf(systemcall,"mv %s %s",ficname,fichier);
		switch(format_b)
		{
			case 0 : {lect(ima,ficname,dim*dim); charwrite(fichier,dim,dim,ima); unlink(ficname); break;}
			case 1 : {system(systemcall); break;}
			case 2 : {ropen(ficname,dim,dim,ima); rwrite(fichier,dim,dim,ima); unlink(ficname); break;}
		}

	}
	fclose(fic);	
	
	/* ref */
	printf("Repartition en classes : ref -----\n");
	system("date");
	for (k=0; k<=nb_class; k++) nc[k]=0;
	k=1;
	sprintf(ficname,"%s/refr0",nomdir);
	fic=fopen(ficname,"r");
	for (k=1; k<=nbim_r; k++)
	{
		fscanf(fic,"%d",&x); fscanf(fic,"%f",&r0);
		fscanf(fic,"%s",&ficname);
//		class=(int)((r0-minr0)/(maxr0-minr0)*nb_class);
//		class=MIN(class,nb_class-1);
//		class=MAX(class,-1);
//		class++;
		class=1+(int)(nb_class*(r0-moyr0+2.5*sigmar0)/(5*sigmar0));
		printf("   Image %s - class=%d\n",ficname,class); nc[class]++;
		sprintf(fichier,"%s/%s%d_%d",nomdir_r,nomgenref,class,nc[class]);
		sprintf(systemcall,"mv %s %s",ficname,fichier);
		switch(format_r)
		{
			case 0 : {lect(ima,ficname,dim*dim); charwrite(fichier,dim,dim,ima); unlink(ficname); break;}
			case 1 : {system(systemcall); break;}
			case 2 : {ropen(ficname,dim,dim,ima); rwrite(fichier,dim,dim,ima); unlink(ficname); break;}
		}

	}	
	fclose(fic);
	}	
}

--------------AACDA184E2363D78625F5002
Content-Type: text/plain; charset=iso-8859-1; name="mylib.c"
Content-Transfer-Encoding: 8bit
Content-Disposition: inline; filename="mylib.c"

#include "/usr/local/include/fitsio.h"
#define MAXREAL 2e30
#define MAX(a,b) ( (a)>(b) ? (a) : (b))
#define MIN(a,b) ( (a)<(b) ? (a) : (b))
#define sgn(x)  (x==0)? 0 : (2*(x>0)-1)
#define MOINPUISS(a)  ( (((int) a) & 1)==0 ? (1) : (-1) )
#define ABS(x)  ( (x)>0 ? (x) : (-(x)) )
#define PI  3.1415926535897932384626
#define FRAC(x)  (x-(int) (x) )


/* -------------------------------------------------------------------- */
/*    Mise a zero d©un tableau de reels  */
/* -------------------------------------------------------------------- */

raz(tab,dim)

int dim;
float *tab;
{
 register int i;
 
 for (i=0;i<dim;i++) *(tab+i)=0;
}

/* -------------------------------------------------------------------- */
/*    Mise a zero d©un tableau de doubles  */
/* -------------------------------------------------------------------- */

raz_d(tab,dim)

int dim;
double *tab;
{
 register int i;
 
 for (i=0;i<dim;i++) *(tab+i)=0;
}


/* -------------------------------------------------------------------- */
/*   Ecriture d©un fichier formatte de reels   */
/* -------------------------------------------------------------------- */

ascwrite(nomfic,dimx,dimy,tab)

float *tab;
char nomfic[];
int dimx,dimy;

{
 register int i;
 FILE *fic,*fopen();
 float a;
 
 if ((fic=fopen(nomfic,"w"))!=NULL)
 {
  for (i=0;i<dimx*dimy;i++) fprintf(fic,"%g\n",*(tab+i));
  fclose(fic); 
 }
 else printf("Pb ouverture fichier %s \n",nomfic);

}
/* -------------------------------------------------------------------- */
/*   Ecriture d©un fichier formatte de reels 3d  */
/* -------------------------------------------------------------------- */

ascwrite3d(nomfic,dimx,dimy,dimz,tab)

float *tab;
char nomfic[];
int dimx,dimy,dimz;

{
 register long i;
 FILE *fic,*fopen();
 float a;
 
 if ((fic=fopen(nomfic,"w"))!=NULL)
 {
  for (i=0;i<(long)dimx*dimy*dimz;i++) fprintf(fic,"%g\n",*(tab+i));
  fclose(fic); 
 }
 else printf("Pb ouverture fichier %s \n",nomfic);

}
/* -------------------------------------------------------------------- */
/*   Lecture d©un fichier formatte de reels   */
/* -------------------------------------------------------------------- */

ascopen(nomfic,dimx,dimy,tab)

float *tab;
char nomfic[];
int dimx,dimy;

{
 register int i;
 FILE *fic,*fopen();
 float a;
  
 if ((fic=fopen(nomfic,"r"))!=NULL)
 {
  for (i=0;i<dimx*dimy;i++) fscanf(fic,"%f",&(*(tab+i)));
  fclose(fic); 
 }
 else printf("Pb ouverture fichier %s \n",nomfic);

}

/* -------------------------------------------------------------------- */
/*   Lecture d©un fichier fits 2D de reels   */
/* -------------------------------------------------------------------- */

fitsopen(nomfic,dx,dy,tab)

float **tab;
char nomfic[];
int *dx,*dy;

{
    fitsfile *fptr;
    int naxis=2;
    long naxes[2];
    int status=0,iomode=0,anynul,nval;
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
      printf("dimx=%d\n",naxes[0]); *dx=dimx=naxes[0];
      printf("dimy=%d\n",naxes[1]); *dy=dimy=naxes[1];
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
/*   Lecture d©un fichier fits 1D de reels   */
/* -------------------------------------------------------------------- */

fitsopen1d(nomfic,dim,tab)

float **tab;
char nomfic[];
int *dim;

{
    fitsfile *fptr;
    int naxis=1;
    long naxes[1];
    int status=0,iomode=0,anynul,nval;
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
      printf("dimx=%d\n",naxes[0]); *dim=dimx=naxes[0];
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
/*   Lecture d©un fichier formatte de reels   */
/* -------------------------------------------------------------------- */

ascopen3d(nomfic,dimx,dimy,dimz,tab)

float *tab;
char nomfic[];
int dimx,dimy,dimz;

{
 register long i;
 FILE *fic,*fopen();
 float a;
  
 if ((fic=fopen(nomfic,"r"))!=NULL)
 {
  for (i=0;i<(long)dimx*dimy*dimz;i++) fscanf(fic,"%f",&(*(tab+i)));
  fclose(fic); 
 }
 else printf("Pb ouverture fichier %s \n",nomfic);

}


/* -------------------------------------------------------------------- */
/*    Min et max d©un tableau de reels  */
/* -------------------------------------------------------------------- */

minmax(tab,dimx,dimy,xmin,xmax,xmoy)

int dimx,dimy;
float *tab,*xmin,*xmax,*xmoy;
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
/*   Conversion entier -> chaine : str$     */
/* -------------------------------------------------------------------- */

char *str$(int a)

{
 char *s;
 
 s=(char *) malloc (80*sizeof(char));

 if (a==0) strcpy(s,"0"); 
 else sprintf(s,"%d",a);
 return(s);
}


/* -------------------------------------------------------------------- */
/*   Ecriture d©un fichier compact non formattÝ   */
/*  On ecrit sur 1 octet on doit entrer pmax si le tableau */
/*  n©est pas de 0 Õ 255 sinon pmax=0   */
/* -------------------------------------------------------------------- */

iwrite(nomfic,dimx,dimy,pmax,tab)

float *tab,pmax;
char nomfic[];
int dimx,dimy;

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
/*   Ecriture d©un fichier de reels en FITS   */
/* -------------------------------------------------------------------- */

fitswrite(nomfic,dimx,dimy,tab)

float *tab;
char nomfic[];
int dimx,dimy;

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
/*   Ecriture d©un fichier de reels non formattÝ   */
/* -------------------------------------------------------------------- */

rwrite(nomfic,dimx,dimy,tab)

float *tab;
char nomfic[];
int dimx,dimy;
{
 register int i;
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
/*   Ecriture d©un fichier de reels non formattÝ 3d  */
/* -------------------------------------------------------------------- */

rwrite3d(nomfic,dimx,dimy,dimz,tab)

float *tab;
char nomfic[];
int dimx,dimy,dimz;

{
 register long i;
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
/*   Lecture d©un fichier de reels non formattÝ   */
/* -------------------------------------------------------------------- */

ropen(nomfic,dimx,dimy,tab)

float *tab;
char nomfic[];
int dimx,dimy;

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
/*   Lecture d©un fichier de reels non formattÝ   */
/* -------------------------------------------------------------------- */

ropen3d(nomfic,dimx,dimy,dimz,tab)

float *tab;
char nomfic[];
int dimx,dimy,dimz;

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

lect(tab,nomfic,dim)

float *tab;
char nomfic[];
int dim;

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

miroiry(im1,im2,dimx,dimy)

float *im1,*im2;
unsigned int dimx,dimy;

{
 int i,j;
 printf("%d %d",dimx,dimy);
 for (j=0;j<dimx;j++)
 for (i=0;i<dimy;i++) *(im2+(dimy-i-1)*dimx+j) = *(im1+i*dimx+j);
}

/* -------------------------------------------------------------------------
 extraction de l©extension d©un fichier
 -------------------------------------------------------------------------  */

scan_ext(nomtotfic,nomfic,ext)

char *nomfic,*ext,*nomtotfic;
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
 ------------------------------------------------------------------------- */

convolve(h,x,y,dim,dimcv)

float h[]; /* convoluante */
float x[]; /* fonction a convoluer */
float y[]; /* fonction resultat */
int dim; /* dimension de x et y */
int dimcv; /* dimension convoluante */

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
 Ajustement d©une parabole y=a x^2 + b x + c
 a partir de 3 couples de points (x1,y1), (x2,y2) et (x3,y3)
 ------------------------------------------------------------------------- */
parfit(x1,x2,x3,y1,y2,y3,a,b,c)

double x1,x2,x3,y1,y2,y3,*a,*b,*c;

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
 Interpolation d©un tableau de points
 dim1 est la dimension de depart, dim2 celle d©arrivee.
 ------------------------------------------------------------------------- */


interpol(tab1,tab2,dim1,dim2)

float *tab1,*tab2; // tableaux de depart et d©arrivee
int dim1,dim2;


{
 int i,j;
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

tf1d(real,imag,dim,isign)

float *real,*imag;
int dim,isign;

{
  float *liRe,*liIm,*tcos,*tsin,*Re,*Im; 
  float x,y,v,min,max,min2,max2; 
  unsigned long int i,j,mn,n; 
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

tf2d(real,imag,dim,isign)

float *real,*imag;
int dim,isign;

{
  float *liRe,*liIm,*tcos,*tsin,*Re,*Im; 
  float x,y,v,min,max,min2,max2; 
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

  CosSin(float *tcos,float *tsin,int m) 
  { 
  unsigned long int i,j,lix,jlx; 
  float scl,arg; 
 
  for (i=1;i<=m;i++) 
 { 
 lix=(int)pow(2.0,(float)(m+1-i)); 
 scl=(float)(2*M_PI)/(float)lix; 
 for (j=1;j<=lix/2;j++) 
   { 
   jlx=j-1+lix/2; 
   arg=((float)(j-1))*scl; 
   tcos[jlx-1]=cos(arg); tsin[jlx-1]=sin(arg); 
 } 
    } 
  } 
 
  RFLBTS(float *t,int n) 
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
  RFLBTSH(float *t,int n) 
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
 
  RFFTH(float *Re,float *Im,float *tcos,float *tsin,int m,int n,int direct) 
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

tf2d_mod(real,imag,dim,isign)

float *real,*imag;
int dim,isign;


{
 int i;
 tf2d(real,imag,dim,isign);
 for (i=0; i<dim*dim; i++) 
 {
  *(real+i)=sqrt(*(real+i)* *(real+i) + *(imag+i)* *(imag+i));
  *(imag+i)=0;
 }
 
}
charwrite(nomfic,dimx,dimy,tab)

float   *tab;
char    nomfic[];
int     dimx,dimy;

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


--------------AACDA184E2363D78625F5002--


