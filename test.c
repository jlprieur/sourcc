#include <stdio.h>
#include <math.h>
#define PI 3.14159
int main(int argc, char *argv[])
{
double ww;
float u, h;
u = 1.;
printf("Test, PI=%f \n", PI);
printf(" Enter h:\n");
scanf("%f",&h);
ww = PI * h * u /(PI * u);
printf("u=%f h=%f ww=%f sin( pi h u)/(pi u) = %f\n", u, h, ww, sin(ww));

printf(" FLOATMAX=%f\n",getfloatmax());
return(0);
}
