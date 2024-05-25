/**************************************************************************
* Program to compute the absolute magnitude from m_V and parallax 
*
* JLP
* Version 11/07/2007
**************************************************************************/
#include "jlp_ftoc.h"

int main(int argc, char *argv[])
{
double m_v, parallax, parallax_error, M_v, M_v_low, M_v_high; 
int error_is_known;
int k;

/* To handle "runs histo" */
for(k = 7; k > 0; k--) if(argc == k && argv[k-1][0] == '\0') argc = k-1;
if(argc != 3 && argc != 4) {
  printf("Error, argc=%d \n Syntax is:\n", argc);
  printf("runs absolute_mag m_v parallax(mas) [parallax_error(mas)]\n");
  return(-1);
 }
sscanf(argv[1], "%lf", &m_v);
sscanf(argv[2], "%lf", &parallax);
if(argc == 4) {
  sscanf(argv[3], "%lf", &parallax_error);
  error_is_known = 1; 
  }

/* If parallax in arcsec: M_v = m_v + 5. * log10(parallax) + 5.;
*/
/* If parallax in milli-arcsec: */ 
M_v = m_v + 5. * log10(parallax) - 10.;

if(error_is_known) {
  M_v_low = m_v + 5. * log10(parallax -parallax_error) - 10.;
  M_v_high = m_v + 5. * log10(parallax + parallax_error) - 10.;
  printf("m_v=%.2f parallax=%.2f +/-%.2f (mas)  M_v=%.2f (%.2f < M_v < %.2f)\n", 
          m_v, parallax, parallax_error, M_v, M_v_low, M_v_high);
  printf("Delta/2 = %.2f\n", (M_v_high - M_v_low)/2);
  } else {
  printf("m_v=%.2f parallax=%.2f (mas)  M_v=%.2f\n", m_v, parallax, M_v);
  }

return(0);
}
