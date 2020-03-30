/* autocorrel.h : entete pour autocorrel
11 2 92 LK */


/* --------------- protos --------------- */

void	integ_from_buffer9	(photon *, long ph_per_cycle, unsigned short *integ);
void	integ_1_im9			(photon *, long  ph_per_integ, char *buf_image);
void	correlim9			(photon *, long ph_per_im, unsigned short *ac);	
void	de_correlate9		(photon *, long ph_per_correl, long ph_per_cycle,
										long ph_per_decorr, unsigned short *de_ac); 
void	correlate9			(photon *, long ph_per_correl, long ph_per_cycle, unsigned short *ac);
void	inter_correlim9		(photon *,  long, photon *, long, unsigned short *de_ac);
void	inter_correlflux9	(photon *, long ph_per_correl, long ph_per_integ,
											unsigned short *ac_org);
void plantricorr9 (
photon		*phot_ptr,			/* pointe photon en cours */
long		ph_per_correl,		/* photons par correlation t */
long		ph_per_integ,		/* photons par integration t */
Rect		m,					/* pour le decentrage du plan de la 3C à calculer */
unsigned short	*ac_org,		/* pointe origine du tab but (stockage AC) */
unsigned short	*integ_temp);	/* pointe origine tab annexe (stockage integ courte) */

void de_plantricorr9 (
photon		*phot_ptr,			/* pointe photon en cours */
long		ph_per_correl,		/* photons par correlation t */
long		ph_per_integ,		/* photons par integration t */
long		ph_per_decorr,		/* photons par séparation ∆t */
Rect		m,					/* pour le decentrage du plan de la 3C à calculer */
unsigned short	*ac_org,		/* pointe origine du tab but (stockage AC) */
unsigned short	*integ_temp);	/* pointe origine tab annexe (stockage integ courte) */
