/***********************************************************
* cel_meca1.h
* Prototypes of routines comnatined in cel_meca.c about celestial mechanics.
*
* Version 16/12/2007
* JLP
***********************************************************/
#ifndef cel_meca1__

int input_location(double *xlat, double *xlong, int *iloc);
int local_coord__(double *aa, int *mm, int *idd, double *time, double *xlat,
                  double *xlong, double *alpha, double *delta, 
                  double *hour_angle, double *elev, double *azim);
int input_coord(char *string, double *value, char *opt);
int precess(double *alpha, double *delta, double delta_years);
int julian(double aa, int mm, int idd, double time, double *djul);
int output_coord(double val, char *label, char *opt);
int besselian_epoch(double aa, int mm, int idd, double time, double *b_date);
int output_coord1(double alpha, double delta, FILE *fp1);
int convert_coord(double coord, long *ial1, long *ial2, double *al3, char *opt);
int from_mean_to_eccent_anomaly(double mean_ano, double *eccent_ano, 
                                double eccent);

#define cel_meca1__
#endif
