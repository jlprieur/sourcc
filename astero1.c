#include<stdio.h>
#define SIGN(x) ((x == '+')?(1.0):(-1.0)) 
/*#define DEBUG*/
main()
 {

/** Declaration des variables **/

char s1[81]; 
char date[5],alpha[10],delta[11];
int n, k,JJ,MM,YYYY,HH,mm,SS,found,l;

register int i;
float date_value,alpha_hour,alpha_min,delta_deg,delta_min,date_utilisat;
float alpha_value,delta_value,old_date,old_alpha,old_delta;
float new_date, new_alpha, new_delta,true_alpha,true_delta;

char filename[40];
char buffer[40];
char car;
FILE *fp;
k=0;
found=0;

/** Nom du fichier, date et heure  **/

printf("Donner le nom du fichier:\n");
gets(filename);         /** lit la chaine de caracteres **/         
printf("Donner la date et l'heure ex:14 09 1994 22 56 45\n");
gets(buffer);  
sscanf(buffer,"%d %d %d %d %d %d",&JJ,&MM,&YYYY,&HH,&mm,&SS);
#ifdef DEBUG
printf("Date et Heure:%d %d %d %d %d %d\n",JJ,MM,YYYY,HH,mm,SS);
#endif
/** Calcul de la date en jour **/

date_utilisat = (((((float)SS/60.)+ mm)/60) + (float)HH)/24.+(float)JJ;
printf("la date est:%f\n",date_utilisat);

/** Ouverture de fichier en mode lecture **/
fp=fopen(filename,"r"); 
                        
 if (fp == NULL)
  {
  printf ("erreur fatale d'ouverture du fichier %s \n",filename);
  exit(-1);
  }  

n = 80;

/** lit sur le fichier fp la chaine de  s1 **/      
/**boucle pour lire toutes les lignes**/

while(fgets(s1,n,fp) != NULL)  
    {                        
 
#ifdef DEBUG
printf("%s\n",s1);
#endif
/** copie la chaine s1 dans la chaine date **/

strncpy(date,&s1[8],5);   
date[6]='\0';
#ifdef DEBUG
printf("date:%s \n",date);
#endif
/**for(i = 0; i < 10; i++) printf("s1[%d]:%c\n",i,s1[i]);**/
 if (s1[0] == '1')
    {
    k++;
#ifdef DEBUG 
    printf("k=%d\n",k);
#endif
    sscanf(date,"%f",&date_value);
#ifdef DEBUG
     printf("valeur:%f \n",date_value);
#endif
/** Lecture et ecriture de alpha en heure et delta en degres **/

        strncpy(alpha,&s1[14],8);
        alpha[9]='\0';
#ifdef DEBUG
        printf("alpha:%s\n",alpha);
#endif
        sscanf(alpha,"%f %f",&alpha_hour,&alpha_min );
#ifdef DEBUG
        printf("alpha=%.0f  %.2f\n",alpha_hour,alpha_min); 
#endif
        alpha_value= (alpha_min /60.) + alpha_hour;
#ifdef DEBUG
        printf("alpha_value=%f\n",alpha_value);
#endif
        
        strncpy(delta,&s1[25],9);
        delta[10]='\0';
#ifdef DEBUG
        printf("delta:%s\n",delta);
#endif
        sscanf(delta,"%c%f %f",&car,&delta_deg,&delta_min);
#ifdef DEBUG
        printf("delta=%.0f  %.2f\n",delta_deg,delta_min); 
#endif
        delta_value =SIGN(car)*((delta_min /60.) + delta_deg);
#ifdef DEBUG
        printf("delta_value=%f\n",delta_value);
#endif 

     if (date_utilisat <= date_value)
        {

        found =1;
#ifdef DEBUG
        printf("Trouve\n"); 
#endif
        break;

       }              /** fin du second if **/ 
    }                 /** fin du premier if **/ 

        old_date=date_value;
        old_alpha=alpha_value;
        old_delta=delta_value;
 
  }                   /** fin de boucle while **/
 if (found)
        {
        new_date=date_value;
        new_alpha=alpha_value;
        new_delta=delta_value;
#ifdef DEBUG
        printf("old_date=%f\n",old_date);
        printf("old_alpha=%f\n",old_alpha);
        printf("old_delta=%f\n",old_delta);
 
        printf("new_date=%f\n",new_date);
        printf("new_alpha=%f\n",new_alpha);
        printf("new_delta=%f\n",new_delta);
#endif        
        true_alpha=((date_utilisat - old_date)/ (new_date - old_date)) * (new_alpha - old_alpha) + old_alpha ;
        true_delta=((date_utilisat - old_date)/ (new_date - old_date)) * (new_delta - old_delta) + old_delta ;
        printf("true_alpha=%f\n",true_alpha);
        printf("true_delta=%f\n",true_delta);
        convert1("alpha 2000 = ",true_alpha,"H");
        convert1("delta 2000 = ",true_delta,"D");
        }
else
   printf("echec\n"); 

fclose(fp);           /** fermeture du fichier **/

}                    /** fin du programme principal **/ 
/*************************************************
* Fonction de conversion et d'affichage en H m s ou D ' "
*
*************************************************/
int convert1(string1,value,option)
char *string1, *option;
float value;
{
float work;
int ih0, ih1;
float h2;
ih0 = (int)value; 
work = 60. * (value - (float)ih0);
ih1 = (int)work;
h2 = 60. * (work - (float)ih1);
if(*option == 'H')
printf("%s %d H %d m %.2f s \n",string1,ih0,ih1,h2);
else
printf("%s %d D %d' %.1f\" \n",string1,ih0,ih1,h2);

return(0);
}

