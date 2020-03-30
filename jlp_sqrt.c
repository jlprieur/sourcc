#include <stdio.h>
#include <math.h>
main(argc,argv)
char **argv;
int argc;
{
double value;

#ifdef DEBUG
  printf(" jlp_sqrt    -- Version 14-11-96 ---\n");
#endif

/* argc=7 when used with "runs" */
if(argc != 2 && argc != 7)
  {
#ifdef DEBUG
  printf(" SYNTAX: jlp_sqrt value (argc=%d)\n",argc);
#else
  printf("-2\n");
#endif
  exit(-1);
  }

sscanf(argv[1],"%lf",&value);

if(value < 0.)
 { 
#ifdef DEBUG
  printf(" negative value: %f\n",value); 
#else
  printf("-1");
#endif
  exit(-1);
 }
else
 {
#ifdef DEBUG
  printf(" sqrt[%f] = %f \n",value,sqrt(value));
#else
  printf("%f\n",sqrt(value));
#endif
 }

exit(0);
}
