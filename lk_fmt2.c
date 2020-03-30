/*******************************************************************************
* lk_fmt2.c 
*
* traduit les coordonnees CP40 en coordonnees format YX9M:
* 9 bits pour X et 9 pour Y. Le M signifie "photons groupes par images". 
*
* From Laurent Koechlin ("photons_photons.c", version 04-07-95) 
* Version 04-07-1995
*******************************************************************************/
/* Contains:
* ------- routines de definition des CP40 --------------
* void set_CP4A (camera *quelle_CP40)	* CP40 Alain GI2T direct,    format 'CP4A'
* void set_CP4N (camera *quelle_CP40)	* CP40 Alain GI2T via eltec,   format 'CP4N'
* void set_CP4T (camera *quelle_CP40)	* CP40 INSU TBL direct 1 canal,  format 'CP4T'
* void set_CQ40 (camera *quelle_CP40)	* CP40 Alain ancien,  format 'CQ40'
* ------ routines changeant le format des coordonnees de photons ------
* void CP40_vers_YX9M                   * Traduction des CP40 vers YX sur 9 bits
* void CP40_distor_vers_YX9M            * Traduction des CP40 vers YX sur 9 bits 
*                                       * avec correction de distorsion.
* void PAPA_vers_YX9                    * Traduction photons PAPA sur 16 bits -> photons 
*                                       * YX9 sur 32 bits.
* void YCAR_vers_YX9                    * Traduction photons YCAR sur 32 bits (YX10) 
*                                       *    -> YX9 sur 32 bits.
* ---- routines changeant l'echelle ou la resolution des coordonnees de photons ----
* void prepa9_ac                        * Masque le lsb des coordonnees en preparation de l'AC.
* void zoom_Y9
* void zoom_X9
* void zoom_YX9
* void reduc_YX9                        * divise par 2 les echelles en X et Y dans le buffer
* void filtre                           * filtrage a travers une fenetre
* ------- preparation a l'ic Delta_x en flux -------
* void marque_et_centre                 * Marquer le et/ou les msb des coors 
*                                       * qui sont dans les fenetre i et/ou j, recentrer.
* void dump_coor                        * Dump des coordonnees de photons d'un buffer
*******************************************************************************/

#include <math.h>			/* pour correction de distorsion CP40 */
#include <stdio.h>

/* ???
#include "params.h"
#include "utilk.h"
#include "PhotTypes.h"
#include "affiche.h"
*/

/* Prototypes and LK definitions: */
#include "lk_fmt2.h"


/* ------- routines de definition des CP40 -------------- */

void	set_CP4A (camera *quelle_CP40)		/*  CP40 Alain GI2T direct,    format 'CP4A' */
{
	quelle_CP40->debut_im		= 0x0;		/* marqueur de debut d'image */
	quelle_CP40->masc_debut_im	= 0x80000000;	/* zone (bits) marquante */
	
	quelle_CP40->h_c1 =   0	- 50;			/* origine x du canal 1 */
	quelle_CP40->v_c1 =   0;			/* origine y du canal 1 */
	
	quelle_CP40->h_c2 = 384	-108;
	quelle_CP40->v_c2 =   0	  -8;
	
	quelle_CP40->h_c3 = 384;
	quelle_CP40->v_c3 = 576;
	
	quelle_CP40->h_c4 = 768;
	quelle_CP40->v_c4 = 576;
	
	quelle_CP40->masc_c2_4 = masc_c_pair;
	quelle_CP40->masc_c1_3 = masc_c_impair;
}


void	set_CP4N (camera *quelle_CP40)		/*  CP40 Alain GI2T via eltec,   format 'CP4N' */
{
	quelle_CP40->debut_im		= 0x0;
	quelle_CP40->masc_debut_im	= 0x80000000;
	
	quelle_CP40->h_c1 = 0	-110;
	quelle_CP40->v_c1 = 0;
	
	quelle_CP40->h_c2 = 384	-140;
	quelle_CP40->v_c2 = 0;
	
	quelle_CP40->h_c3 = 384;
	quelle_CP40->v_c3 = 576;
	
	quelle_CP40->h_c4 = 768	+9;
	quelle_CP40->v_c4 = 576;
	
	quelle_CP40->masc_c2_4 = masc_c_pair;
	quelle_CP40->masc_c1_3 = masc_c_impair;
}


void	set_CP4T (camera *quelle_CP40)		/* CP40 INSU TBL direct 1 canal,  format 'CP4T' */
{
	quelle_CP40->debut_im		= 0xBD000000;
	quelle_CP40->masc_debut_im	= 0xFF000000;
	
	quelle_CP40->h_c1 = 0;
	quelle_CP40->v_c1 = 0;
	
	quelle_CP40->h_c2 = 0;
	quelle_CP40->v_c2 = 0;
	
	quelle_CP40->h_c3 = 0;
	quelle_CP40->v_c3 = 0;
	
	quelle_CP40->h_c4 = 0;
	quelle_CP40->v_c4 = 0;

	quelle_CP40->masc_c2_4 = 0;		/* on force tout au canal 1 */
	quelle_CP40->masc_c1_3 = 0;		/* en mettant a 0 les bits de canal avant test */
}


void	set_CQ40 (camera *quelle_CP40)		/* CP40 Alain ancien,  format 'CQ40' */
{
	quelle_CP40->debut_im		= 0x0;
	quelle_CP40->masc_debut_im	= 0x007FF7FF;
	
	quelle_CP40->h_c1 = 0	-10;		/* origine x du canal 1 */
	quelle_CP40->v_c1 = 0;			/* origine y du canal 1 */
	
	quelle_CP40->h_c2 = 384	-35;
	quelle_CP40->v_c2 = 0;
	
	quelle_CP40->h_c3 = 384;
	quelle_CP40->v_c3 = 576;
	
	quelle_CP40->h_c4 = 768;
	quelle_CP40->v_c4 = 576;
	
	quelle_CP40->masc_c2_4 = masc_c_pair;
	quelle_CP40->masc_c1_3 = masc_c_impair;
}



/* ------ routines changeant le format des coordonnees de photons ------ */

/**************************************************************************** 
* --- traduction des CP40 vers YX sur 9 bits --- 
* obsA_ptr: pointeur de structure phot_buf  on y traduira les coors. de photons
*            on y actualisera nb d'images et la duree 
*****************************************************************************/
void	CP40_vers_YX9M ( phot_buf_rec *obsA_ptr, camera	*quelle_CP40,
        float *image1, int nx1, int ny1, long *iphot, int mode_test)
{
long 	lcount;		      /* nb de photons a traduire */
photon	*CP40_phot_ptr;	      /* pointe le photon a traduire */
unsigned long	CP40_phot;    /* le photon pointe au format CP40 */
unsigned long	x;	      /* coordonnees du photon en pixels */
unsigned long	y;
photon	*dest_ptr;	/* destination du photon traduit */
long 	nb_images;	/* trouvees en detectant les debuts d'ims. */
int	im_nouv, premiere_im;
	
/* Set CP40 characteristics: */
short recal_horiz_c1 = quelle_CP40->h_c1;
short recal_vert_c1  = quelle_CP40->v_c1;

short recal_horiz_c2 = quelle_CP40->h_c2;
short recal_vert_c2  = quelle_CP40->v_c2;

short recal_horiz_c3 = quelle_CP40->h_c3;
short recal_vert_c3  = quelle_CP40->v_c3;

short recal_horiz_c4 = quelle_CP40->h_c4;
short recal_vert_c4  = quelle_CP40->v_c4;

unsigned long debut_im	= quelle_CP40->debut_im;
unsigned long masc_debut_im = quelle_CP40->masc_debut_im;

unsigned long masc_c2_4 = quelle_CP40->masc_c2_4;
unsigned long masc_c1_3 = quelle_CP40->masc_c1_3;


/*
Format coor CP40 :
 31             24 23             16 15              8 7               0
| -l-l-l-l-l-l-l- | -l-l-l-l-l-l-l- | -l-l-l-l-l-l-l- | -l-l-l-l-l-l-l- |
|	F	 |	F	  |**lylylylylylyly | ylyl.l.l*lxlxlx | xlxlxlxlxlxl.l. |
bit 23 : ** : a 1 pour les 2 canaux du bas		(canal 3 ou 4)
bit 11 : *  : a 1 pour les 2 canaux de droite	(canal 2 ou 4).
Les axes y et x sont orientes "balayage".
les 2 bits de poids faibles des x (0,1) et des y (12,13) ne sont pas significatifs.

pour un debut d'image (theoriquement)
 31             24 23             16 15              8 7               0
| 0l0l-l-l-l-l-l- | -l-l-l-l-l-l-l- | -l-l-l-l-l-l-l- | -l-l-l-l-l-l-l- |
|	3	 |	F	  | .l.l.l.l.l.l.l. | .l.l.l.l.l.l.l. | .l.l.l.l.l.l.l. |
*/

 
/* ---- traduction des photons en memoire ------ */

	lcount = obsA_ptr->photons;
	CP40_phot_ptr = obsA_ptr->adr;		/* pointe premiere adresse dans source */
	dest_ptr = obsA_ptr->adr;		/* pointe premiere adresse dans but */
	nb_images = 0;
	*(obsA_ptr->debuts_image) = obsA_ptr->adr;
	im_nouv = FALSE;
	premiere_im = TRUE;
	
	while (--lcount >= 0L) {		/* parcourrir le buf */
		CP40_phot = *CP40_phot_ptr++;

/* ---- detection des debuts d'image ----- */
		
		if ((CP40_phot & masc_debut_im) == debut_im) {	/* debut d'im par marque */
			im_nouv = TRUE;
		}
		
		if (im_nouv) {
		   im_nouv = FALSE;
		   if (premiere_im) {
			premiere_im = FALSE;
			dest_ptr = obsA_ptr->adr;/* on jette la premiere im, souvent partielle */
	                }   
                        else 
/* JLP: New image: */
                        {
                          nb_images++;
                        }
		    *(obsA_ptr->debuts_image +nb_images) = dest_ptr;
			
		   if (nb_images >= MAX_IMAGES -1) {;
			printf(" traduction cp40 -> YX9M : trop d'images\n");
			break;
			}
			
/* sauter les eventuelles marques de debut d'image consecutives */
	           while ((*CP40_phot_ptr & masc_debut_im) == debut_im) {
			CP40_phot_ptr++;
			lcount--;	/* avancer tant que c'est un "non photon" */
			if (lcount <= 0L) goto fin; /* on est arrive au bout sans voir d'autre ph. */
			}
		}
		else {
/* --- on a un photon, on le met au format YX9 --- */
/* separer X et Y */
			y =  (CP40_phot & 0x7FC000) >> DECALY_CP40;
			x =  (CP40_phot & 0x7FC)  >> DECALX_CP40;
			
/* champ individuel d'un canal CP40 : 288 x 384 */
		   if ( y<288 && x<384) {
			if ((CP40_phot & masc_c1_3) == 0) {	/* canal 1 ou 3 */
				if ((CP40_phot & masc_c2_4) == 0) {
/* ***** CANAL 1 ***** bit 11 a 0 et bit 23 a 0 */
					y +=  recal_vert_c1;
					x +=  recal_horiz_c1;
					x += (288 - y)/50;	/* de-rotation du pave ccd */

				} else {
/* ***** CANAL 3 ***** bit 11 a 0 et bit 23 a 1 */
					y = -y + recal_vert_c3;
					x = -x + recal_horiz_c3;
				}
			} else {					/* canal 2 ou 4 */
				if ((CP40_phot & masc_c2_4) == 0) {	
				/* ***** CANAL 2 ***** bit 11 a 1 et bit 23 a 0 */
					x += recal_horiz_c2;
					y += recal_vert_c2;
					x -= (288 - y)/70;	/* de-rotation du pave ccd */
				}
				else {	
/* ***** CANAL 4 ***** bit 11 a 1 et bit 23 a 1 */
					x = -x + recal_horiz_c4;
					y = -y + recal_vert_c4;
				}
			}	
		
/* --- remballer X et Y au format YX9M --- */
			if ( y<VERT && x<HORIZ && y>=0 && x>=0)
/* JLP: Store photon coordinates: */
                                {
                                image1[x + y * nx1]++;
/* Increase number of photons: */
                                (*iphot)++;
/*
				*dest_ptr++ = (y << 9) + x;
*/
                                }
		}
	   }
	}
fin:
/* JLP95: previously:  obsA_ptr->format='YX9M'; */
	strcpy(obsA_ptr->format,"YX9M");
	obsA_ptr->echelle_zoomy = 1.0;
	
/* nb de photons traduits (images entieres) */
	obsA_ptr->photons = *(obsA_ptr->debuts_image +nb_images)
					  - *(obsA_ptr->debuts_image);
	obsA_ptr->nb_images = nb_images;
	obsA_ptr->duree = nb_images * 20;		/* 20 ms par image */
	if (mode_test) {
		printf ("fin traduction ph.CP40: Phots= %ld ; ims= %ld\r\n", obsA_ptr->photons, nb_images);
		if (nb_images < 1) {
			printf(" traduction ph.CP40: pas assez d'images\n");
		}
	}
}


/* ----------- Traduction des CP40 vers YX sur 9 bits avec correction de distorsion --- */

void CP40_distor_vers_YX9M (
	phot_buf_rec	*obsA_ptr,	/* pointeur de structure phot_buf */
					/* on y traduira les coors. de photons */
					/* on y actualisera nb d'images et la duree */
	camera			*quelle_CP40,
	int mode_test)
{
	long 			lcount;		/* nb de photons a traduire */
	photon			*CP40_phot_ptr;	/* pointe le photon a traduire */
	unsigned long	CP40_phot;		/* le photon pointe au format CP40 */
	unsigned long	x, y;			/* coordonnees du photon en pixels */
	long			xs;
	photon			*dest_ptr;	/* destination du photon traduit */
	long 			nb_images;	/* trouvees en detectant les debuts d'ims. */
	int			im_nouv, premiere_im;
	
	long			*x_distor;
	long			*y_distor;

	
	short recal_horiz_c1 = quelle_CP40->h_c1;
	short recal_vert_c1  = quelle_CP40->v_c1;
	
	short recal_horiz_c2 = quelle_CP40->h_c2;
	short recal_vert_c2  = quelle_CP40->v_c2;
	
	short recal_horiz_c3 = quelle_CP40->h_c3;
	short recal_vert_c3  = quelle_CP40->v_c3;

	short recal_horiz_c4 = quelle_CP40->h_c4;
	short recal_vert_c4  = quelle_CP40->v_c4;

	unsigned long debut_im		= quelle_CP40->debut_im;
	unsigned long masc_debut_im = quelle_CP40->masc_debut_im;
	
	unsigned long masc_c2_4 = quelle_CP40->masc_c2_4;
	unsigned long masc_c1_3 = quelle_CP40->masc_c1_3;
	
	y_distor = quelle_CP40->y_distor;
	x_distor = quelle_CP40->x_distor;

/*
Format coor CP40 :
 31             24 23             16 15              8 7               0
| -l-l-l-l-l-l-l- | -l-l-l-l-l-l-l- | -l-l-l-l-l-l-l- | -l-l-l-l-l-l-l- |
|	F	 |	F	  |**lylylylylylyly | ylyl.l.l*lxlxlx | xlxlxlxlxlxl.l. |
bit 23 : ** : ˆ 1 pour les 2 canaux du bas		(canal 3 ou 4)
bit 11 : *  : ˆ 1 pour les 2 canaux de droite	(canal 2 ou 4).
Les axes y et x sont orientŽs "balayage".
les 2 bits de poids faibles des x (0,1) et des y (12,13) ne sont pas significatifs.


pour un debut d'image (theoriquement)
 31             24 23             16 15              8 7               0
| 0l0l-l-l-l-l-l- | -l-l-l-l-l-l-l- | -l-l-l-l-l-l-l- | -l-l-l-l-l-l-l- |
|	3	 |	F	  | .l.l.l.l.l.l.l. | .l.l.l.l.l.l.l. | .l.l.l.l.l.l.l. |
*/

 
/* ---- traduction des photons en memoire ------ */

	lcount = obsA_ptr->photons;
	CP40_phot_ptr = obsA_ptr->adr;		/* pointe premiere adresse dans source */
	dest_ptr = obsA_ptr->adr;		/* pointe premiere adresse dans but */
	nb_images = 0;
	*(obsA_ptr->debuts_image) = obsA_ptr->adr;
	im_nouv = FALSE;
	premiere_im = TRUE;
	
	while (--lcount >= 0L) {		/* parcourrir le buf */
		CP40_phot = *CP40_phot_ptr++;

		/* ---- detection des debuts d'image ----- */
		
		if ((CP40_phot & masc_debut_im) == debut_im) {	/* debut d'im par marque */
			im_nouv = TRUE;
		}
		
		if (im_nouv) {
			im_nouv = FALSE;
			if (premiere_im) {
				premiere_im = FALSE;
				dest_ptr = obsA_ptr->adr; /* on jette la premiere im, souvent partielle */
			} else nb_images++;
			*(obsA_ptr->debuts_image +nb_images) = dest_ptr;
			
			if (nb_images >= MAX_IMAGES -1) {;
				printf(" traduction cp40 -> YX9M : trop d'images\r\n");
				break;
			}
			
			/* sauter les eventuelles marques de debut d'image consecutives */
			while ((*CP40_phot_ptr & masc_debut_im) == debut_im) {
				CP40_phot_ptr++;
				lcount--;		/* avancer tant que c'est un "non photon" */
				if (lcount <= 0L) goto fin; /* on est arrive au bout sans voir d'autre ph. */
			}
		}
		else {
		/* --- on a un photon, on le met au format YX9 --- */

			/* separer X et Y */
			y =  (CP40_phot & 0x7FC000) >> DECALY_CP40;
			x =  (CP40_phot & 0x7FC)  >> DECALX_CP40;
			
			/* champ individuel d'un canal CP40 : 288 x 384 */
			if ( y<288 && x<384) {
			
			  if ((CP40_phot & masc_c1_3) == 0) {		/* canal 1 ou 3 */
				if ((CP40_phot & masc_c2_4) == 0) {
				/* ***** CANAL 1 ***** bit 11 a 0 et bit 23 a 0 */
					y +=  recal_vert_c1;
					x +=  recal_horiz_c1;
					x += (288 - y)/50;	/* de-rotation du pave ccd */
					y -= (342 - x)/50;
					xs = (long)x - 289;	/* correction distor x */
					if (xs <=0) xs -=1;
					if (y < LGY_CP40)   xs *= x_distor [y];
					xs >>= 15;
					x = xs + 290;
					y = 288 - y;		/* correction distor y */
					if (x < LGX_CP40)   y *= y_distor [x];
					y >>= 15;
					y = 288 - y;
				} else {
				/* ***** CANAL 3 ***** bit 11 ˆ 0 et bit 23 a 1 */
					y = -y + recal_vert_c3;
					x = -x + recal_horiz_c3;
				}
			} else {				/* canal 2 ou 4 */
				if ((CP40_phot & masc_c2_4) == 0) {	
				/* ***** CANAL 2 ***** bit 11 ˆ 1 et bit 23 a 0 */
					x += recal_horiz_c2;
					y += recal_vert_c2;
					x -= (288 - y)/70;	/* de-rotation du pave ccd */
					xs = (long)x - 289;	/* de-distorsion x */
					if (xs <=0) xs -=1;
					xs *= x_distor [y];
					xs >>= 15;
					x = xs + 290;
					y = 288 - y;		/* correction distor y */
					y *= y_distor [x];
					y >>= 15;
					y = 288 - y;
				}
				else {	
				/* ***** CANAL 4 ***** bit 11 ˆ 1 et bit 23 a 1 */
					x = -x + recal_horiz_c4;
					y = -y + recal_vert_c4;
				}
			}	
			/* --- remballer X et Y au format YX9M --- */
			if ( y < VERT && x < HORIZ)
				*dest_ptr++ = (y << 9) + x;
		}
	   }
	}
fin:
/* JLP95: previously:  obsA_ptr->format='YX9M'; */
	strcpy(obsA_ptr->format,"YX9M");
	obsA_ptr->echelle_zoomy = 1.0;
	
/* nb de photons traduits (images entieres) */
	obsA_ptr->photons = *(obsA_ptr->debuts_image +nb_images)
					  - *(obsA_ptr->debuts_image);
	obsA_ptr->nb_images = nb_images;
	obsA_ptr->duree = nb_images * 20;		/* 20 ms par image */
	if (mode_test) {
		printf ("fin traduction ph.CP40: Phots= %ld ; ims= %ld\r\n", obsA_ptr->photons, nb_images);
		if (nb_images <= 1) {
			printf(" traduction ph.CP40: pas assez d'images\n");
		}
	}
}


/* ---- traduction photons PAPA sur 16 bits -> photons YX9 sur 32 bits ----- */

void	PAPA_vers_YX9 (phot_buf_rec *obsA_ptr, int mode_test)
/*
	les photons PAPA sont sur 2 octets, on va les traduire en YX9 sur 4 octets.
	Au depart, la moitie seulement du buffer doit etre remplie.
	On lit le buf du milieu vers le debut, par photons PAPA de 2 octets.
	On reecrit le buf de la fin vers le debut, par photons YX9 de 4 octets.
	La rŽsolution initiale de la PAPA Žtant de 256 par 256, on aura le bit
	de poids faible ˆ 0 dans les coors traduites en YX9 car on passe ˆ 512*512.
*/

{
	register long		lcount;			/* nb de photons a traduire */
	register unsigned long	y, x, yx;
	register short		*source_phot_ptr;	/* pointe une valeur sur 2 octets */
	register photon 	*dest_phot_ptr;		/* pointe une valeur sur 4 octets */
	
	lcount = obsA_ptr->photons;
	if(lcount > PHOT_BUF_SIZE) lcount = PHOT_BUF_SIZE; /* securite ... */
	obsA_ptr->duree = (float)(obsA_ptr->duree) * (float)lcount / obsA_ptr->photons;
	obsA_ptr->photons = lcount;
/* JLP95: previously:  obsA_ptr->format='\0YX9'; */
	sprintf(obsA_ptr->format,"%c%s",'\0',"YX9");
	obsA_ptr->echelle_zoomy = 1.0;

	obsA_ptr->nb_images = 0;		/* les photons ne sont pas groupes en images */
	*(obsA_ptr->debuts_image) = obsA_ptr->adr;
	*(obsA_ptr->debuts_image +1) = obsA_ptr->adr + obsA_ptr->photons;
	
	
	source_phot_ptr = (short *)(obsA_ptr->adr) + lcount;	/* milieu du buf +1 */
	dest_phot_ptr = 	obsA_ptr->adr + lcount;		/* bout du buf +1 */

	while (--lcount >= 0L) {
		yx = *--source_phot_ptr;
		
	/* separer x et y et mettre en 512*512 */
		y = (yx & 0xFF) <<10;
		x = (yx & 0xFF00) >>7;
		
	/* rempaqueter au format YX9 */
		*--dest_phot_ptr = y | x;
	}
	if (mode_test) printf(" traduction ph.PAPA: un buf lu\n");
}


/* format des photons CAR :
adresse	octet		 n			  		n+1				n+2				n+3
n¡ de bit	 31             24 23             16 15              8 7               0
			| -l-l-l-l-l-l-l- | -l-l-l-l-l-l-l- | -l-l-l-l-l-l-l- | -l-l-l-l-l-l-l- |
contenu		| tltltltltltlt|t | tltltltlylylyly | ylylylylylylxlx | xlxlxlxlxlxlxlx |
*/


/* ---- traduction photons YCAR sur 32 bits (YX10) -> YX9 sur 32 bits ----- */

void	YCAR_vers_YX9 ( phot_buf_rec	*obsA_ptr, int	mode_test)
{
	static long tprev;		/* pour mode test */
	long		nph = 0; 	/* pour mode test */
	
	long			lcount;
	unsigned long	y, x, t, yx10;
	photon			*source_phot_ptr;
	photon			*dest_phot_ptr;
	

	if (mode_test) {
		printf(" %ld photons a traduire\r\n", obsA_ptr->photons); /* on dit tout */
		source_phot_ptr = obsA_ptr->adr;
		lcount = obsA_ptr->photons;
		if( lcount > 20L) lcount = 20L;
		tprev = 0;

		while (--lcount >= 0L) {
			yx10 =  *source_phot_ptr++;
			y = yx10 >> 2 & 0x3FE00;
			y >>= 9;
			x = yx10 >> 1 & 0x1FF;
			t = (yx10 & 0x0FF00000) >> 20;
			printf ("%4ld x =%4ld y =%4ld t =%4ld Dt =%4ld\r\n",
							nph++, x, y, t, t-tprev);
			tprev = t;
		}
	}
	
	source_phot_ptr =   obsA_ptr->adr;
	dest_phot_ptr =     obsA_ptr->adr;
	lcount = 	    obsA_ptr->photons;
	tprev = 0;
	while (--lcount >= 0L) {
		yx10 =  *source_phot_ptr++;
		/* separer x et y */
		y = yx10 >> 2 & 0x3FE00;
		x = yx10 >> 1 & 0x1FF;
		t = (yx10 & 0x0FF00000) >> 20;
		
		if (t-tprev != 1) {
			/* rempaqueter au format YX9, on Žlimine les photons trop pressŽs */
			/* si le temps ne marche pas on a t-tprev = 0 et les photons sont pris */
			*dest_phot_ptr++ = y | x;
		}
		tprev = t;
	}
	
/* JLP95: previously:  obsA_ptr->format='\0YX9'; */
	sprintf(obsA_ptr->format,"%c%s",'\0',"YX9");
	obsA_ptr->photons = dest_phot_ptr - obsA_ptr->adr;	/* nb de photons traduits */
	obsA_ptr->echelle_zoomy = 1.0;
	
	obsA_ptr->nb_images = 0;		/* les photons ne sont pas groupes en images */
	*(obsA_ptr->debuts_image) = obsA_ptr->adr;
	*(obsA_ptr->debuts_image +1) = obsA_ptr->adr + obsA_ptr->photons;
}



/* ---- routines changeant l'echelle ou la resolution des coordonnees de photons ---- */
/*
	Certaines de ces routines peuvent changer le nb de photons par image, dans le cas des 
	donnees decoupees en images.
*/

void	prepa9_ac	(phot_buf_rec *obsA_ptr)

/* Masque le lsb des coordonnees en preparation de l'AC,
pour des coordonnees YX sur 9 bits chacun rangees dans des mots de 32 bits.
	Ceci a pour effet de diviser par 2 la resolution.
On pourrait ne pas perdre en resolution en gardant les 9 bits et
en rajoutant un bit a 0 a la fin de chaque coor. On aurait alors besoin de 4 fois plus
de memoire, ou (exclusif) de X fois plus de temps car il faudrait des tests de depassement
dans la boucle de correlation.
*/

{
	register photon			*phot_ptr;	/* adr source */
	register long			lcount;
	register unsigned long	mask = 0x3FDFE;
	
	lcount = obsA_ptr->photons;
	phot_ptr = obsA_ptr->adr;

	if (obsA_ptr->reduced == TRUE) {
		printf(" prepa9_ac: lsb deja masque\n");
		return;
	}

	while (--lcount >= 0L)	{		/* traite un photon a la fois */
		*phot_ptr++ &= mask;		/* mettre le lsb a 0 sur X et sur Y */
	}
	obsA_ptr->reduced = TRUE;
}


/* -------- zooms -------- */

void	zoom_Y9	( phot_buf_rec *obsA_ptr, Rect cadre)
/*
multiplie par 2 l'echelle en Y dans le  buffer, recentre,
puis coupe ce qui depasse VERT.
consiste a faire un zoom de 2 fois en Y sur le centre de l'image.
pour des coordonnŽes sur 9 bits.
*/

{
	register long 		lcount;
	register photon		*phot_ptr;	/* source adr */
	register photon		*dest_ptr;	/* destination */
	register long		yx;
	register long		y;
	register short		centre;
	photon			*debu_im_suiv;	/* adr. debut d'image suivant le ph. en cours */ 
	long			i;

	i = 1;
	debu_im_suiv = *(obsA_ptr->debuts_image +1);
	centre = cadre.bottom + cadre.top - VERT/2;
	
	phot_ptr = obsA_ptr->adr;
	dest_ptr = obsA_ptr->adr;
	*(obsA_ptr->debuts_image) = dest_ptr;

	lcount	 = obsA_ptr->photons;
	while (--lcount >= 0L){
		if ((obsA_ptr->nb_images >0) && (phot_ptr >= debu_im_suiv)) {
			/* Les ph. sont groupes par images et c'est un debut d'im dans la source.*/
			if (i >= MAX_IMAGES-1) {
				printf(" zoom Y : trop d'images \n");
				break;
			}
			/* on entre dans une nouvelle image */
			debu_im_suiv = *(obsA_ptr->debuts_image +(++i));
			*(obsA_ptr->debuts_image +i-1) = dest_ptr;
		}
		yx = *phot_ptr;
		y = (yx  & 0x3FE00) >> 8;	/* extraire Y (sur 9 bits) multipliŽ par 2 */
		y -= centre;			/* recenter l'origine */
		
		if ((y >= 0) && (y < VERT)) {
			yx &= 0xFFFC01FF;	/* masquer a 0 la precedente valeur de y, laisser le reste */
			yx |= y<<9;		/* remettre y (multiplie par 2 et recentre) a sa place */
			*dest_ptr = yx;
			dest_ptr++;
		}
		phot_ptr++;
	}
	obsA_ptr->nb_images = i;
	*(obsA_ptr->debuts_image +i) = dest_ptr;	/* un de plus que le dernier ph. */
	obsA_ptr->photons =  dest_ptr - obsA_ptr->adr;
	obsA_ptr->echelle_zoomy *= 2.0;
}


void	zoom_X9 ( phot_buf_rec *obsA_ptr, Rect cadre)
/*
multiplie par 2 l'echelle en X dans le  buffer, recentre,
puis coupe ce qui depasse HORIZ.
consiste a faire un zoom de 2 fois en X sur le centre de l'image.
pour des coordonnees sur 9 bits.
*/

{
	register long 		lcount;
	register photon		*phot_ptr;	/* source adr */
	register photon		*dest_ptr;	/* destination */
	register long		yx;
	register long		x;
	register short		centre;
	photon			*debu_im_suiv;	/* adresse du debut d'image suivant le ph. en cours */ 
	long			i;

	i = 1;
	debu_im_suiv	 = *(obsA_ptr->debuts_image +1);
	centre = cadre.left + cadre.right - HORIZ/2;


	phot_ptr = obsA_ptr->adr;
	dest_ptr = obsA_ptr->adr;
	*(obsA_ptr->debuts_image) = dest_ptr;

	lcount	 = obsA_ptr->photons;
	while (--lcount >= 0L){
		if ((obsA_ptr->nb_images >0) && (phot_ptr >= debu_im_suiv)) {
			/* Les ph. sont groupes par images et c'est un debut d'im dans la source.*/
			if (i >= MAX_IMAGES-1) {
				printf(" zoom X : trop d'images\n");
				break;
			}
			/* on entre dans une nouvelle image */
			debu_im_suiv = *(obsA_ptr->debuts_image +(++i));
			*(obsA_ptr->debuts_image +i-1) = dest_ptr;
		}
		yx = *phot_ptr;
		x = (yx & 0x1FF);		/* extraire X */
		x <<= 1;			/* multiplier par 2 */
		x -= centre;			/* recenter */
		
		if ((x >= 0) && (x < HORIZ)) {
			yx &= 0xFFFFFE00;	/* mettre a 0 la precedente valeur de x, laisser le reste */
			yx |= x;		/* remettre x modifie a sa place */
			*dest_ptr = yx;
			dest_ptr++;
		}
		phot_ptr++;
	}
	obsA_ptr->photons =  dest_ptr - obsA_ptr->adr;
	*(obsA_ptr->debuts_image +i) = dest_ptr;	/* un de plus que le dernier ph. */
	obsA_ptr->nb_images = i;
}


void	zoom_YX9 ( phot_buf_rec *obsA_ptr, Rect cadre)
/*
recentre et multiplie par 2 les echelles en X et Y dans le  buffer,
puis coupe ce qui depasse VERT en y et HORIZ en x.
consiste a faire un zoom de 2 fois sur le centre de l'image.
Pour recuperer la resolution de l'AC dans le cas des images de speckle.
*/
{
	register long 	lcount;
	register photon	*phot_ptr;	/* source addr */
	register photon	*dest_ptr;	/* destination */
	register long		x, y;
	register long		centre_x;
	register long		centre_y;
	register long 		yx; 		/* contient les 2 coors */

	photon			*debu_im_suiv;	/* adresse du debut d'image suivant le ph. en cours */ 
	long			i;

	i = 1;
	debu_im_suiv	 = *(obsA_ptr->debuts_image +1);
	centre_x = cadre.left + cadre.right - HORIZ/2;
	centre_y = cadre.top + cadre.bottom - VERT/2;

	phot_ptr = obsA_ptr->adr;
	dest_ptr = obsA_ptr->adr;
	*(obsA_ptr->debuts_image) = dest_ptr;
	
	lcount	 = obsA_ptr->photons;
	while (--lcount >= 0L) {
		if ((obsA_ptr->nb_images >0) && (phot_ptr >= debu_im_suiv)) {
			/* Les ph. sont groupes par images et c'est un debut d'im dans la source.*/
			if (i >= MAX_IMAGES-1) {
				printf(" zoom YX : trop d'images\n");
				break;
			}
			/* on entre dans une nouvelle image */
			debu_im_suiv = *(obsA_ptr->debuts_image +(++i));
			*(obsA_ptr->debuts_image +i-1) = dest_ptr;
		}
		yx = *phot_ptr;
		x = (yx & 0x1FF);			/* extraire x */
		x <<= 1;				/* mult par 2 */
		x -= centre_x;				/* recenter */
		if ((x >= 0) && (x < HORIZ)) {
			y = (yx  & 0x3FE00) >> 8;	/* extraire Y (sur 9 bits) multiplie par 2 */
			y -= centre_y;			/* recenter l'origine */
			if ((y >= 0) && (y < VERT)) {
				yx &= 0xFFFC0000;	/* masquer a 0 les precedente coors, laisser le reste */
				yx |= y<<9 | x;		/* remettre x et y (multiplies par 2 et recentres) */
				*dest_ptr++ = yx;
			}
		}
		phot_ptr++;
	}
	obsA_ptr->photons =  dest_ptr - obsA_ptr->adr;
	obsA_ptr->nb_images = i;
	*(obsA_ptr->debuts_image +i) = dest_ptr;	/* un de plus que le dernier ph. */
	obsA_ptr->echelle_zoomy *= 2.0;
}


void	reduc_YX9	(phot_buf_rec *obsA_ptr)
/*
divise par 2 les echelles en X et Y dans le  buffer
*/
{
	register long 	lcount;
	register photon	*phot_ptr;	/* source addr */
	register long		x, y;
	register long 		yx; 	/* contient les 2 coords */


	phot_ptr = obsA_ptr->adr;
	
	lcount	 = obsA_ptr->photons;
	while (--lcount >= 0L) {
		yx = *phot_ptr;
		x = (yx & 0x1FF);		/* extraire x */
		x >>= 1;			/* div par 2 */
		y = (yx & 0x3FE00) >> 10;	/* extraire y div par 2 */
		
		*phot_ptr = x | (y<<9);	        /* replacer les coordonnees */
		phot_ptr++;
	}
	obsA_ptr->echelle_zoomy *= 0.5;
}




/* -------- filtrage a travers une fenetre ------- */
void	filtre	( phot_buf_rec	*obs_src_ptr, phot_buf_rec *obs_dst_ptr, Rect fenetre)
/*
elimine de la liste les photons tombant en dehors du rectangle defini
dans la variable "fenetre".
*/
{
	register long 	lcount;
	register photon	*phot_ptr;		/* source addr */
	register photon	*dest_ptr;		/* destination */
	register long		x, y9;
	register long 		yx; 			/* contient les 2 coors */
	unsigned long		msbs = 0XFFFC0000;	/* bits de poids forts, au dela des coors YX9 */

	photon			*debu_im_suiv;	/* adr. du debut d'im suivant le ph. en cours */ 
	long			i;

	register long	x_inf = (long)fenetre.left;
	register long	x_sup = (long)fenetre.right;
	long			y_inf9 = ((long)fenetre.top) <<9;
	long			y_sup9 = ((long)fenetre.bottom) <<9;

	i = 1;
	debu_im_suiv	 = *(obs_src_ptr->debuts_image +1);

	phot_ptr = obs_src_ptr->adr;
	dest_ptr = obs_dst_ptr->adr;
	*(obs_dst_ptr->debuts_image) = dest_ptr;
	
	lcount	 = obs_src_ptr->photons;
	while (--lcount >= 0L) {
		if (phot_ptr >= debu_im_suiv) {	/* c'est un debut d'im dans la source */
			if (i >= MAX_IMAGES-1) {
				printf(" filtre : trop d'images \n");
				break;
			}
			/* on entre dans une nouvelle image */
			debu_im_suiv = *(obs_src_ptr->debuts_image +(++i));
			*(obs_dst_ptr->debuts_image +i-1) = dest_ptr;
		}
		yx = *phot_ptr;
		x = (yx & 0x1FF);				/* extraire x */
		if ((x >= x_inf) && (x < x_sup)) {		/* on est dans fenetre en x */
			y9 = (yx & 0x3FE00);			/* extraire y decalŽ de 9 bits */
			if ((y9 >= y_inf9) && (y9 < y_sup9)) {	/* on est dans fenetre en y */
				*dest_ptr = (yx & msbs) | y9 | x;	/* replacer les coordonnŽes */
				dest_ptr++;
			}
		}
		phot_ptr++;
	}
	obs_dst_ptr->photons =  dest_ptr - obs_dst_ptr->adr;
	obs_dst_ptr->nb_images = i;
	*(obs_dst_ptr->debuts_image +i) = dest_ptr;	/* un de plus que le dernier ph. */
	obs_dst_ptr->tries = TRUE;
	obs_src_ptr->tries = TRUE;
}




/* ------- preparation a l'ic Æx en flux ------- */
/*
Marquer le et/ou les msb des coors qui sont dans les fenetre i et/ou j,
recentrer les coordonnees. Si les fenetres ont une partie commune, recentrer l'image complete
en bloc sur le centre de l'union des deux fenetres.
*/

void marque_et_centre (
phot_buf_rec	*obs_src_ptr,
phot_buf_rec	*obs_dst_ptr,
Rect		cadre_i,		/* fenetre "i" dans l'image a intercorreler. */
Rect		cadre_j)		/* fenetre "j" dans l'image a intercorreler. */
{
/* JLP guess: */
typedef struct{int h; int v;} Point;

	register long	ph_count;		/* decompte du nb de photons restant a traiter */
	register photon	*src_ptr;		/* pointeur du ph. en cours dans le buffer source */
	register photon	*dst_ptr;		/* pointeur du ph. en cours dans le buffer destination */
	short		it = cadre_i.top;	/* pour accelerer */
	short		ib = cadre_i.bottom;
	short		il = cadre_i.left;
	short		ir = cadre_i.right;
	Point		centre_i;		/* offset entre milieu fenetre "i" et centre image */
	short		jt = cadre_j.top;
	short		jb = cadre_j.bottom;
	short		jl = cadre_j.left;
	short		jr = cadre_j.right;
	Point		centre_j;		/* offset entre milieu fenetre "j" et centre image */
	register long	x, y;				/* coordonnees separees */
	register long	mark;				/* pour marquage d'appartenance : */
	long		msb  = 0x80000000;	/* pour marquage d'appartenance a la fenetre j */
	long		m2sb  = 0x40000000;	/* pour marquage d'appartenance a la fenetre i */
	
	if (obs_src_ptr->photons < 1L) return;
		
	/* test si les fenetres ont une partie commune */
	if ((it > jb) || (jt > ib) || (il > jr) || (jl > ir)) {
		centre_i.h = (ir + il)/2 - HORIZ/2;		/* pas de parties communes :  */
		centre_i.v = (it + ib)/2 - VERT/2;		/* donc on recentrera separement */
		centre_j.h = (jr + jl)/2 - HORIZ/2;
		centre_j.v = (jt + jb)/2 - VERT/2;
	} else {
		centre_i.h = (ir + il + jr + jl)/4 - HORIZ/2;   /* il y a une partie commune aux */
		centre_i.v = (it + ib + jt + jb)/4 - VERT/2;    /* 2 fenetres : on recentrera d'un bloc */
		centre_j = centre_i;
	}

	src_ptr = obs_src_ptr->adr;		/* pointe au debut du buffer */
	dst_ptr = obs_dst_ptr->adr;
	ph_count = obs_src_ptr->photons;
	
	while (--ph_count >= 0L) {
		x = *src_ptr & 0x1FF;		/* fait l'hypothese que les coors sont sur 9 bits */
		y = (*src_ptr >>9) & 0x1FF;	/* et jointives : YX9 */
		src_ptr++;
		
/*
on fait ici le test d'appartenance a la fenetre "j" d'intercorrel Æx, pour
ne pas avoir a le faire dans la boucle interne de l'intercorrel, ou il serait
fait n * m fois au lieu de n fois ici. on marque le msb des coors, qui lui
devra etre teste n * m fois, mais ce test sera plus rapide.
Les photons n'appartenant a aucune des fenetres ne sont pas gardes
*/
		
		if ((x < jr) && (x > jl) && (y > jt) && (y < jb)) {	
			if ((x < ir) && (x > il) && (y > it) && (y < ib))
				mark = msb | m2sb;		/* dans fenetre "j" et dans fenetre "i" */
			else
				mark = msb;			/* dans fenetre "j" seulement */
			x -= centre_j.h;
			y -= centre_j.v;
			*dst_ptr++ = mark | (y & 0x1FF) <<9 | (x & 0x1FF);
		}
		else if ((x < ir) && (x > il) && (y > it) && (y < ib)) {
			x -= centre_i.h;			/* dans fenetre "i" seulement */
			y -= centre_i.v;
			*dst_ptr++ =  m2sb | (y & 0x1FF) <<9 | (x & 0x1FF);
		}
	}
	obs_dst_ptr->photons =  dst_ptr - obs_dst_ptr->adr;
	obs_dst_ptr->nb_images = 1;				/* en flux continu le buf represente 1 image */
	*(obs_dst_ptr->debuts_image) 	= obs_dst_ptr->adr;
	*(obs_dst_ptr->debuts_image +1)	= dst_ptr;	/* un de plus que le dernier ph. */
	obs_dst_ptr->tries = TRUE;
}


/* ------ dump des coordonnees de photons d'un buffer ------ */

void dump_coor (phot_buf_rec phot_buf)
{
	long i, imax;
	long x, y, t, t_prec, yx;
	photon *ph_ptr, **im_ptr;
	
	/* donner les etats */
	printf ("adr buf %5lX  adr tab debuts_image %5lX\r\n",
		phot_buf.adr, phot_buf.debuts_image);
		
/* JLP previously:
	printf ("duree %5ld  ph. %5ld   format %5ld   nb ims %5ld\r\n",
*/
	printf ("duree %5ld  ph. %5ld   format %s   nb ims %5ld\r\n",
		phot_buf.duree, phot_buf.photons, phot_buf.format, phot_buf.nb_images);
	printf ("plein %5d  processed. %5d   reduced %5d   tries %5d\r\n",
		phot_buf.plein, phot_buf.processed, phot_buf.reduced, phot_buf.tries);	
	
	/* lister quelques coordonnees */
	ph_ptr = phot_buf.adr;
	t_prec = 0;
        imax = phot_buf.photons; 
        if(imax > 30L) imax = 30L;
	for (i=0 ; i<imax ; i++) {
		yx = *ph_ptr;
		x = yx & 0x1FF;
		y = (yx & 0x3FE00) >>9;
		t = (yx & 0x0FF00000) >>20;
		printf ("ph %3ld dt %3ld y %3ld x %3ld  ", i, t-t_prec, y, x);
		t_prec = t;
/* Display in binary
		bindis(yx);
*/
		printf("%x",yx);
		printf ("\r\n");
		ph_ptr++;
	}
	/* lister les adresses de debuts d'image */
	im_ptr = phot_buf.debuts_image;
        imax = phot_buf.nb_images -1;
        if(imax > 10L) imax = 10L;
	for (i=0 ; i<imax; i++) {
		printf ("im %3ld   %9lX  ph/im %4ld      ",
				i+1, im_ptr [i], (im_ptr [i+1] - im_ptr [i]));
		i++;
		printf ("im %3ld   %9lX  ph/im %4ld\r\n",
				i+1, im_ptr [i], (im_ptr [i+1] - im_ptr [i]));
	}
}
