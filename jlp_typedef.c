#include "string.h"
typedef unsigned char UINT1;
typedef unsigned short UINT2;
typedef unsigned long UINT4;
typedef union{
     UINT1 i8[4];
     UINT2 i16[2];
     UINT4 i32;
     float f32;
     }
JLP_INT4;

void disp_int1(UINT1 int1, char *s);
void disp_int2(UINT2 int2, char *s);
void disp_int4(UINT4 int4, char *s);
void disp_float(float f4, char *s);

/******************************************************
*
******************************************************/
main()
{
UINT1 int1;
UINT2 int2;
UINT4 int4;
float f4;

long int j;
char s[40];

/* Display int1: */
printf(" Entrez int1: ");
gets(s);
sscanf(s,"%d",&j);
int1 = j;
disp_int1(int1, s);
printf(" int1 = %s \n",s);

/* Display int2: */
printf(" Entrez int2: ");
gets(s);
sscanf(s,"%d",&j);
int2 = j;
disp_int2(int2, s);
printf(" int2 = %s \n",s);

/* Display int4: */
printf(" Entrez int4: ");
gets(s);
sscanf(s,"%d",&j);
int4 = j;
disp_int4(int4, s);
printf(" int4 = %s \n",s);

/* Display float: */
printf(" Entrez float: ");
gets(s);
sscanf(s,"%f",&f4);
disp_float(f4, s);
printf(" float = %s \n",s);

}
/******************************************************
* INT1: 1 byte, 8 bits
******************************************************/
void disp_int1(UINT1 int1, char *s)
{
register int i;
int j;

for(i = 0; i < 8; i++)
   {
   j = (int1 << i) & (1 << 7);
   sprintf(s+i,"%d",j);
   }

s[8] = ' ';
s[9] = '\0';
}

/******************************************************
* INT2: 2 bytes, 16 bits
******************************************************/
void disp_int2(UINT2 int2, char *s)
{
register int i;
int j;
JLP_INT4 k; 

k.i16[0] = int2;
disp_int1(k.i8[0], s);
disp_int1(k.i8[1], s+9);
}

/******************************************************
* INT4: 4 bytes, 32 bits
******************************************************/
void disp_int4(UINT4 int4, char *s)
{
JLP_INT4 k; 

k.i32 = int4;
disp_int1(k.i8[0], s);
disp_int1(k.i8[1], s+9);
disp_int1(k.i8[2], s+18);
disp_int1(k.i8[3], s+27);
}

/******************************************************
* float: 4 bytes, 32 bits
******************************************************/
void disp_float(float f4, char *s)
{
JLP_INT4 k; 

k.f32 = f4;
disp_int1(k.i8[0], s);
disp_int1(k.i8[1], s+9);
disp_int1(k.i8[2], s+18);
disp_int1(k.i8[3], s+27);
}
