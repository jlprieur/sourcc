/****************************************************************************
* Example of command with arguments:
* a.out -in toto -out titi -cv
* a.out -h
*
* JLP
* Version 20/07/2006
*****************************************************************************/
#include <stdio.h>

/* program name (used for error messages): */
char *pname; 
int Verbose=0, Compressed=0;

void usage(pname)
{
   fprintf(stderr, "usage: %s -[coh] [-in fname] [-out ffname]\n\
  where:  c=compressed v=verbose, h=help \n", pname);
    exit(1);
}

int main(int argc, char *argv[])
{
char inf[60], outf[60];
int i;

inf[0] = '\0';
outf[0] = '\0';
setbuf(stderr, NULL);
pname = argv[0];
printf("pname=%s argc=%d\n", pname, argc);

i = 1;
while(i < argc)
    {
printf("i=%d\n", i);
        if (argv[i][0] == '-') {
            switch (argv[i][1]) {
/* verbose: */
            case 'v':
                Verbose = 1;
                printf("OK: Verbose = %d\n", Verbose);
                switch (argv[i][2]) {
                  case 'c': 
                    Compressed = 1;
                    break;
                  case 'h':
                    usage(pname);
                    break;
                  default :
                    break;
                  }
                break;
/* compressed: */
            case 'c':
                Compressed = 1;
                printf("OK: Compressed = %d\n", Compressed);
                switch (argv[i][2]) {
                  case 'v': 
                    Verbose = 1;
                    break;
                  case 'h':
                    usage(pname);
                    break;
                  default :
                    break;
                  }
                break;
/* help: */
            case 'h':
                printf("OK: Help wanted\n");
                usage(pname);
            case 'i':
               printf("argv[i]=%s\n", argv[i]);
               i++;
               strcpy(inf,argv[i]);
               break;
            case 'o':
               printf("argv[i]=%s\n", argv[i]);
               i++;
               strcpy(outf,argv[i]);
               break;
            default:
                fprintf(stderr, "%s: illegal option -%c.\n", pname,
                        argv[i][1]);
                exit(1);
            }
        }
        else
            usage(pname);
/* Examine next argument: */
     i++;
    }

printf("OK: %s inf=%s outf=%s\n", pname, inf, outf);
printf("OK: Verbose = %d Compressed = %d\n", Verbose, Compressed);

return 0;
}
