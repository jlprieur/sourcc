/***********************************************************************
*                          Calcul de GRISM                            
* programme de Hans DEKKER         
*
* From version of 1993 in C by Herv� VALENTIN 
*
* JLP BC++ Version 01-10-93
* JLP Linux Version 09/12/2005 
***********************************************************************/

#include<stdio.h>
#include<math.h>

/* Prototypes: */
double Calc_alpha(double lambda_centre_a, double pas_a, double np_a);
double Calc_dispersion(double pas_d, double alpha_d, double f_d);
double Calc_theta0(double np_t0, double alpha_t0);
double Calc_pos_det(double foc, double angle);
double Calc_thetab(double np_tb, double alpha_tb, double beta_tb, double nr_tb);
double Calc_lambda_blaze(double pas_lb, double np_lb, double alpha_lb, 
                         double thetab_lb);
double Rad_deg(double x);
double Deg_rad(double x);

#define DEBUG

#define PI 3.14159
#define MAXj 9
#define MAXi 19

/***********************************************************************/
/*                            D�but du Grism()                         */
/***********************************************************************/

/* void Grism() */

main()
{

/***********************************************************************/
/*      D�but des d�clarations de variables et de fonctions()          */
/***********************************************************************/


 int i,j;                         /* compteurs     */
 int test = 1;                    /* test pour recommencer ou non */

 double np;                        /* indice prisme */
 double nr;                        /* indice r�sine */
 double lambda_centre;             /* lambda centre */
 double f;                         /* focale cam�ra */
 double pas;                       /* entre les strilles */
 double beta;                      /* angle des facettes */
 double alpha;                     /* angle du prisme    */
 double disp;                      /* dispersion         */
 double theta0;                    /* angle de l'ordre 0 % � l'axe */
 double x0;                        /* point d'impact ordre 0 sur le CCD */
 double thetab;                    /* angle de lambda blaze % � l'axe */
 double xb;                        /* point d'impact  ----- */
 double lambda_blaze;              /* lambda blaze */
 double tab_x[MAXi];               /* tableau de positions sur le CCD */
 double tab_lambda[MAXj][MAXi];    /* tableau des longueurs d'ondes */

 char ans[10];

/***********************************************************************/
/*                       Fin des d�clarations                          */
/***********************************************************************/

 while(test == 1)

 {                            /* deb de while */

/***********************************************************************/
/*                       D�but des acquisitions                        */
/***********************************************************************/

#ifdef DEBUG
  printf("Debug mode\n");
#endif

 printf("indices du prisme (1.54 ?) et de la r�sine (1.61 ?) \n");
 scanf("%lf,%lf",&np,&nr);

 printf("pas du r�seau (traits/mm) (600 ?)\n");
 scanf("%lf",&pas);
 pas = 1/pas;                 /* transforme le nb de traits/mm en pas (mm) */

 printf("Beta (en degr�s) angle des facettes. (26 ?)\n");
 scanf("%lf",&beta);
 beta = Deg_rad(beta);

 printf("lambda_centre en nm (450 ?) \n");
 scanf("%lf",&lambda_centre);
/* Conversion en mm */
 lambda_centre = lambda_centre * 1.E-6;

 printf("focale de l'objectif derri�re le GRISM en mm\n");
 scanf("%lf",&f);

#ifdef DEBUG
 printf(" OK les param�tres rentr�s sont: \n");
 printf("indice du prisme = %f\n",np);
 printf("indice de la r�sine = %f\n",nr);
 printf("pas du r�seau (traits/mm) = %f \n",1/pas);
 printf("Beta (degr�s) angle des facettes. = %f \n",Rad_deg(beta));
 printf("lambda_centre en nm = %f\n",lambda_centre* 1.E6);
 printf("focale de l'objectif derri�re le GRISM en mm = %f\n",f);
#endif

/***********************************************************************/
/*              Fin des acquisitions et d�but des calculs              */
/***********************************************************************/

/* alpha in radians
*/
 alpha = Calc_alpha(lambda_centre,pas,np);

 disp = Calc_dispersion(pas,alpha,f);

 theta0 = Calc_theta0(np,alpha);

 x0 = Calc_pos_det(f,theta0);

 thetab = Calc_thetab(np,alpha,beta,nr);

 xb = Calc_pos_det(f,thetab);

 lambda_blaze = Calc_lambda_blaze(pas,np,alpha,thetab);

 for (i=0;i<MAXi;i++)
 {
  tab_x[i]=(i - (MAXi/2));

  tab_lambda[0][i]= pas * (np * sin(alpha) + sin(atan(tab_x[i] / f) - alpha));
  tab_lambda[0][i]= tab_lambda[0][i] * pow10(6);

     for(j=1;j<MAXj;j++)
     {
      tab_lambda[j][i] = tab_lambda[0][i] / (j+1);
     }

 }


/***********************************************************************/
/*              Fin des calculs et d�but des affichages                */
/***********************************************************************/


 alpha = Rad_deg(alpha);
 printf("\nalpha = %.3f (degr�s) angle du prisme.\n",alpha);

 disp = disp * pow10(7);
 printf("dispersion = %.3f ( �/mm).\n",disp);

 theta0 = Rad_deg(theta0);
 printf("Theta_0 = %.3f (degr�s) angle de l'ordre 0 par rapport � l'axe.\n",theta0);

 printf("x0 = %.3f point d'impact (en mm) de l'ordre 0 sur le CCD.\n",x0);

 thetab = Rad_deg(thetab);
 printf("Theta_B = %.3f angle (degr�s) du rayon du blaze par rapport � l'axe.\n",thetab);

 printf("xB = %.3f point d'impact (en mm) du blaze\n",xb);

 lambda_blaze = lambda_blaze * 1.E+6;
 printf("lambda_blaze = %.3f (nm)\n",lambda_blaze);

 printf("\nPour voir la suite, tapez une touche\n");
 scanf("%c",ans);


 printf("|------------------------------------------------------------------------------|");
 printf("|  POS | ORD 1 | ORD 2 | ORD 3 | ORD 4 | ORD 5 | ORD 6 | ORD 7 | ORD 8 | ORD 9 |");
 printf("|------|-------|-------|-------|-------|-------|-------|-------|-------|-------|");

 for (i=0;i<MAXi;i++)
 {
  if(tab_x[i]<0)
  {
   printf("| %.1f ",tab_x[i]);
  }

  else
  {
   printf("|  %.1f ",tab_x[i]);
  }


     for(j=0;j<MAXj;j++)
     {
      if(tab_lambda[j][i] > 99.99 && tab_lambda[j][i] < 999.99)
      {
       printf("| %.2f",tab_lambda[j][i]);
      }
      else
      {
       if(tab_lambda[j][i] > 999.99)
       {
        printf("|%.2f",tab_lambda[j][i]);
       }
       else
       {
        printf("|  %.2f",tab_lambda[j][i]);
       }

      }
     }

  printf("|");
 }
 printf("**************************************************************\n");

 printf("Pour sortir taper sur ESC,  pour continuer taper sur CReturn ");
 scanf("%c",ans);
 printf("OK: ASCII value is %d\n",(int)ans[0]);
 if ((int)ans[0] == 27) (test = 0);
 else (test = 1);

 }                          /* fin du while */

/***********************************************************************/
/*                      Fin des affichages et du main()                */
/***********************************************************************/
}

/***********************************************************************/
/*                      Calcul l'angle alpha du prisme                 */
/***********************************************************************/
double Calc_alpha(double lambda_centre_a, double pas_a, double np_a)
{
 double alpha_a, w1;

 w1 = lambda_centre_a / (pas_a * (np_a - 1.0));
 if(w1 < 1.0) alpha_a = asin(w1);
 else
  {
  printf("Calc_alpha/Fatal error, w1 = %f lambda=%f (nm)\n",w1, lambda_centre_a*1.e6);
  exit(-1);
  }
 return(alpha_a);
}
/***********************************************************************/
/*                   Fin calcul l'angle alpha du prisme                */
/***********************************************************************/

/***********************************************************************/
/*                       Calcul la dispersion                          */
/***********************************************************************/
double Calc_dispersion(double pas_d, double alpha_d, double f_d)
{
 double disp_d;

 disp_d = pas_d * cos(alpha_d) / f_d;
 return(disp_d);
}
/***********************************************************************/
/*                  Fin calcul la dispersion                           */
/***********************************************************************/


/***********************************************************************/
/*   Calcul de l'angle correspondant � l'ordre 0 par rapport � l'axe   */
/***********************************************************************/
double Calc_theta0(double np_t0, double alpha_t0)
{
 double theta0_t0;

 theta0_t0 = ((-np_t0) * sin(alpha_t0));

 if(fabs(theta0_t0)>1)
 {
  theta0_t0 = 0;
 }

 theta0_t0 = asin(theta0_t0) + alpha_t0;
 return(theta0_t0);
}

/***********************************************************************/
/*      Calcul le point d'impact d'un rayon ou axe sur le CCD          */
/***********************************************************************/
double Calc_pos_det(double foc, double angle)
{
 double pos_det;

 pos_det = foc * tan(angle);
 return(pos_det);
}

/***********************************************************************/
/* Calcul soit disant l'angle du lambda blaze par rapport � l'axe      */
/***********************************************************************/
double Calc_thetab(double np_tb, double alpha_tb, double beta_tb, double nr_tb)
{
 double thetab_tb;

 thetab_tb = (-np_tb)*sin(alpha_tb)*cos(beta_tb)
             + sin(beta_tb)*sqrt(pow(nr_tb,2)-pow(np_tb*sin(alpha_tb),2));

 if(fabs(thetab_tb)>1)
 {
  thetab_tb = 0;
 }

 thetab_tb = asin(thetab_tb)+(alpha_tb-beta_tb);
 return(thetab_tb);
}

/***********************************************************************/
/*              Calcul lambda blaze par rapport � l'axe                */
/***********************************************************************/
double Calc_lambda_blaze(double pas_lb, double np_lb, double alpha_lb, 
                         double thetab_lb)
{
 double lambda_blaze_lb;

 lambda_blaze_lb = pas_lb * (np_lb * sin(alpha_lb) + sin(thetab_lb - alpha_lb));
 return(lambda_blaze_lb);
}

/***********************************************************************
* Conversion radians to degrees:
***********************************************************************/
double Rad_deg(double x)
{
return(x*180./PI);
}
/***********************************************************************
* Conversion degrees to radians:
***********************************************************************/
double Deg_rad(double x)
{
return(x*PI/180.);
}

