/*************************************************************************
* Program arithm_ascii
* To perform arithmetic on columns of an ASCII list 
*
*
* JLP
* Version 24/05/2007
*************************************************************************/
#include <stdio.h>
#include <math.h>
#include <string.h>

/*
#define DEBUG
*/

static int arithm_from_list(FILE *fp_in, FILE *fp_out, int icol_x, int icol_y, 
                            char *operation);

int main(int argc, char *argv[])
{
char filein[60], fileout[60], operation[8];
int icol_x, icol_y;
FILE *fp_in, *fp_out;

  printf("arithm_ascii/ JLP/ Version 24/05/2007\n");

if(argc == 7 && *argv[4]) argc = 5;
if(argc == 7 && *argv[3]) argc = 4;
if(argc == 7 && *argv[2]) argc = 3;
if(argc == 7 && *argv[1]) argc = 2;
if(argc != 5)
  {
  printf("Error/Bad syntax: argc=%d\n\n", argc);
  printf("Syntax:  arithm_ascii in_file out_file icol_x,icol_y operation\n");
  printf(" operation= + - or /, to obtain x+y x-y or x/y \n");
  return(-1);
  }
else
  {
  strcpy(filein, argv[1]);
  strcpy(fileout, argv[2]);
  sscanf(argv[3], "%d,%d", &icol_x, &icol_y);
  strcpy(operation, argv[4]);
  }
switch(*operation) {
  case '+':
  case '-':
  case '/':
  break;
  default:
    printf("Fatal error: operation %c is unknown \n", *operation);
  break;
  }

printf(" OK: filein=%s fileout=%s icol_x=%d icol_y=%d\n", filein, fileout,
       icol_x, icol_y);

if((fp_in = fopen(filein,"r")) == NULL) {
 printf(" Fatal error opening input file %s \n",filein);
 return(-1);
 } 

if((fp_out = fopen(fileout,"w")) == NULL) {
 printf(" Fatal error opening output file %s \n",fileout);
 fclose(fp_in);
 return(-1);
 } 


/* Scan the file and make the conversion: */
   arithm_from_list(fp_in, fp_out, icol_x, icol_y, operation); 

fclose(fp_in);
fclose(fp_out);
return(0);
}
/*************************************************************************
*
*************************************************************************/
static int arithm_from_list(FILE *fp_in, FILE *fp_out, int icol_x, int icol_y, 
                            char *operation)
{
int ivalx, ivaly, nn, i;
float valx, valy;
double dval;
char buffer[258], *pc;

if(icol_x < 1 || icol_x > 8) {
  printf("compute_stats_from_list/Only icol_x = 1 to 8 are allowed yet\n");
  return(-1);
  }

if(icol_y < 0 || icol_y > 8) {
  printf("compute_stats_from_list/Only icol_weights = 1 to 8 are allowed yet\n");
  return(-1);
  }

nn = 0;
i = 0;
while(1) {

/* Read new line */
  if(fgets(buffer,258,fp_in) == NULL) break;
  i++;

  ivalx = 0;
  ivaly = 0;
  valx = 0.;
  valy = 0.;
// Remove \n from buffer
  pc = buffer;
  while(*pc) {
   if(*pc == '\n') {
    *pc='\0';
    break; 
    } else {
    pc++;
    }
  }

if(buffer[0] != '#' && buffer[0] != '%') {

  switch(icol_x) { 
    case 1:
      ivalx = sscanf(buffer, "%f", &valx);
      break;
    case 2:
      ivalx = sscanf(buffer, "%*f %f", &valx);
      break;
    case 3:
      ivalx = sscanf(buffer, "%*f %*f %f", &valx);
      break;
    case 4:
      ivalx = sscanf(buffer, "%*f %*f %*f %f", &valx);
      break;
    case 5:
      ivalx = sscanf(buffer, "%*f %*f %*f %*f %f", &valx);
      break;
    case 6:
      ivalx = sscanf(buffer, "%*f %*f %*f %*f %*f %f", &valx);
      break;
    case 7:
      ivalx = sscanf(buffer, "%*f %*f %*f %*f %*f %*f %f", &valx);
      break;
    case 8:
      ivalx = sscanf(buffer, "%*f %*f %*f %*f %*f %*f %*f %f", &valx);
      break;
    default:
      fprintf(stderr,"compute_stats_from_list/Fatal error, wrong column (icol_x=%d)\n", icol_x);
      return(-1);
    }

  switch(icol_y) { 
    case 1:
      ivaly = sscanf(buffer, "%f", &valy);
      break;
    case 2:
      ivaly = sscanf(buffer, "%*f %f", &valy);
      break;
    case 3:
      ivaly = sscanf(buffer, "%*f %*f %f", &valy);
      break;
    case 4:
      ivaly = sscanf(buffer, "%*f %*f %*f %f", &valy);
      break;
    case 5:
      ivaly = sscanf(buffer, "%*f %*f %*f %*f %f", &valy);
      break;
    case 6:
      ivaly = sscanf(buffer, "%*f %*f %*f %*f %*f %f", &valy);
      break;
    case 7:
      ivaly = sscanf(buffer, "%*f %*f %*f %*f %*f %*f %f", &valy);
      break;
    case 8:
      ivaly = sscanf(buffer, "%*f %*f %*f %*f %*f %*f %*f %f", &valy);
      break;
    default:
      fprintf(stderr,"compute_stats_from_list/Fatal error, wrong column (icol_y=%d)\n", icol_y);
      return(-1);
    }


  if(ivalx != 1 && ivaly != 1) break; 
/*
  fprintf(fp_out, "%f %f %f \n", valx, valy, arithm(valx, valy, operation));
*/
//  fprintf(fp_out, "%f %f %f\n", valx, valy, valx - valy);
  dval= valx - valy;
/* DEBUG
*/
  if(dval < -180.) dval += 180.;
  if(dval < -180.) dval += 180.;
  if(dval > 180.) dval -= 180.;
  if(dval > 180.) dval -= 180.;

  if(dval > 90.) dval -= 180.;
  if(dval < -90.) dval += 180.;

  printf(" Buffer=%s\n", buffer);
  printf(" ivalx=%d ivaly=%d nn=%d valx=%f valy=%f dval=%f\n", ivalx, ivaly, 
         nn, valx, valy, dval);

  fprintf(fp_out, "%s %f\n", buffer, dval);
  nn++;
  } /* EOF test on buffer[0] */
} /* EOF while(1) loop */


printf("OK: %d couples (x,y) have been processed \n", nn);

return(0);
}
