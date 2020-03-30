/* routines d'integration d'image et d'ac
a partir de coordonées de photons en mémoire.

coors de photons  sur 9 + 9 bits.
resolution reduite à 8 + 8 bits avant de calculer l' AC,
l'ac est dans un tableau de 512 par VERT d'entiers sur 2 octets.

mars 1989 L.K. */

#include "params.h"
#include "phottypes.h"
#include "utilk.h"

#include "lk_fmt1.h"


/* ---- integration d'images ---- */

/* 			I (x) = ƒ f(x,t) dt		*/

void integ_from_buffer9 (
register photon			*phot_ptr,		/* pointeur au photon courrant */
register long			ph_per_integ,	/* nombre de photons a integrer */
register unsigned short *integ)			/* pointe a l'origine du tab but (sum) */
{
	register short			*im_ptr;

	if (ph_per_integ < 1L) return;
	
	while ( --ph_per_integ >= 0L) {
		im_ptr = (short *)( (long)integ + ((*phot_ptr & 0x3FFFF) << 1) );
		(*im_ptr) ++;
		phot_ptr++;
	}
}


/* --- integ de peu de photons (1 image) directement dans le buf de display --- */

/* 			I (x) = ƒ f(x,t) dt		en 512*512 */

void integ_1_im9 (
	photon			*phot_ptr,		/* pointe le photon */
	long			ph_per_integ,	/* nb de photons a integrer */
	char			*buf_image)		/* pointe le buffer image */
{
	char			*im_ptr;
	unsigned long	y, x, yx;

	/* mise a "noir" initiale du buf image (noir = 255, blanc = 0, sombre = 1) */
	fillongs (0x1, (long *)buf_image, (long)VERT * (long)HORIZ / sizeof(long));

	if (ph_per_integ < 1L) return;
	while ( --ph_per_integ >= 0L)
	{
		yx = *phot_ptr;
		y = yx >>9;
		x = yx & 0x1FF;
		if ((y < VERT) && (x<HORIZ)) {
			im_ptr = buf_image + (*phot_ptr & 0x3FFFF);
			(*im_ptr)+= 200;
		}
		phot_ptr++;
	}
}



/*
	I (x) = ƒ f(x,t) dt		en 256*256

void moniteur256 (
	photon			*phot_ptr,		
	long			ph_per_integ,
	im_buf			*buf_image)	
{
	char			*im_ptr;
	unsigned long	y, x, yx;
	unsigned long	vert, horiz;


	vert = buf_image.vert;
	horiz = buf_image.horiz;
	
	fillongs (0x1, vert * horiz / sizeof(long));

	if (ph_per_integ < 1L) return;
	while ( --ph_per_integ >= 0L)
	{
		yx = *phot_ptr;
		y = yx >>10;
		x = yx & 0xFF;
		if ((y < vert) && (x<horiz)) {
			im_ptr = buf_image + *phot_ptr & 0x3FFFF;
			(*im_ptr)+= 200;
		}
		phot_ptr++;
	}
}

*/


/* -------- integration d'autocorrelations en flux continu -------- */

/* 			AC_f (a) = ƒƒ f(x,t) f(x-a,t) dx dt		*/


void correlate9 (
photon		*phot_ptr,			/* pointer to current photon */
long		ph_per_correl,		/* photons per correlation time */
long		ph_per_integ,		/* photons per integration time */
unsigned short	*ac_org)		/* pointer to origin of target array (storing AC) */

/* integre l'ac  2-dim a partir de coordonnées de photons en mémoire.
		renvoie le pointeur de photons mis à jour.
		L'ac est calculée par soustraction de coordonnée d'un photon à n autres photons.
		On tire avantage du fait que :
		1)  x & y sont côte à côte en mémoire : on les traite en une seule opération.
			Pour que la retenue éventuelle d'une soustraction des X n'affecte pas
			les Y, il faut qu'il y ait un bit "vide" séparant X de Y.
			Ceci est fait au prix d'une baisse de résolution, en mettant à 0
			le lsb de X et de Y, par la routine half_res9.
			Maintenir la résolution initiale serait possible en doublant la
			taille en X et en Y de la fenêtre d'ac calculée.
			L'intérêt de cette façon de faire est qu'il n'y a qu'une soustraction
			et aucun test dans la boucle de calcul.
		2) Le résultat d'une soustraction de coordonnées correspond directement 
		à l'adresse d'un pixel dans le tableau contenant l'autocorrelation, à condition
		que le nombre de pixels par ligne soit un puissance de 2.
		L. Koechlin, Center for Astrophycs, Cambridge Mass, 1988
*/
{
	register short			*ac_ptr;		/* points current pixel in ac */
	long					integ_count;	/* photons left to integrate */
	register long			correl_count;	/* photons left to correlate with current photon */
	register unsigned long	xiyi;			/* coordonnée couplée d'un photon : xi & yi */
	unsigned long			xi;				/* coordonnée seule */
	unsigned long			yi;
	register photon	*phot_ptrj;				/* will point ph.to correlate with current photon */
	register long	ac = (long)ac_org;

	integ_count = ph_per_integ;
	if (integ_count < 1L)
		return;

	while (--integ_count >= 0L)
	{
		xiyi = *phot_ptr++ & 0x3FFFF;
/* ----- attention, ici on fait l'hypothese que xi est codé sur 9 bits ----- */
		xi = xiyi & 0x1FF;		/* xi va de 0 à 510 par 2  (0 à HORIZ-2) */
		xi |= 0x200;			/* maintenant xi va de 512 à 1022 par 2 (HORIZ à 2*HORIZ-2) */
								/* HORIZ doit impèrativement être une puissance de 2 */
								
		yi = xiyi >>9;			/* yi va de 0 à VERT par 2 */
		yi += VERT;				/* maintenant yi va de VERT a 2*VERT-2 par 2 */
		
		xiyi = xi | (yi<<9);	/* on remet ensemble xi et yi modifiés */
/* ----- fin de la partie dépendante du nb de bits de codage des x ----- */
		
		phot_ptrj = phot_ptr;		/* pointe sur xj & yj */
		correl_count = ph_per_correl;
		while (--correl_count >= 0L) {
			ac_ptr = (short *)(ac + xiyi - *phot_ptrj);
									/*	xj va de 0 à HORIZ-2 par pas de 2
										yj va de 0 à VERT-2 par pas de 2.
										
										xi-xj va de 0 à 2*HORIZ-2 par pas de 2.
										yi-yj va de 0 à 2*VERT-2 par pas de 2.
										
										Si HORIZ est une puissance de 2,
										yi-yj correspond aux bits de poids forts
										de l'adresse du pixel à incrémenter dans l'ac,
										et xi-xj aux bits de poids faibles.
										
										Si l'ac est in tableau d'entiers sur 2 octets,
										deux pixels consecutifs  ont une diff. d'adresse de 2,
										comme les yi-yj et xi-xj sont par pas de 2, 
										il n'y a pas de recalage à faire.

										Ajouter à l'adresse de début de tableau ac pour
										trouver l'adresse du pixel à incrementer dans l'ac. */
			(*ac_ptr)++;			/* incrementer pixel but dans l'ac */
			phot_ptrj++;			/* xj & yj suivants */
		}
	}
}	
		


/* -------- Triple corr en flux continu : calcul du plan à m constant -------- */

/* 			Tc_f (m, a) = ƒƒ f(x,t) f(x-m,t) f(x-a,t) dx dt		*/

void plantricorr9 (
photon		*phot_ptr,			/* pointe photon en cours */
long		ph_per_correl,		/* photons par correlation t */
long		ph_per_integ,		/* photons par integration t */
Rect		m,					/* pour le decentrage du plan de la 3C à calculer */
unsigned short	*ac_org,		/* pointe origine du tab but (stockage AC) */
unsigned short	*integ_temp)	/* pointe origine tab annexe (stockage integ courte) */

/*
Revient à faire une integ courte puis ne calculer les histos des distances entre photons
que si l'un des deux photons a les memes coors qu'un troisième décalé de m présent dans
le meme intervalle de temps de corrélation.
*/

{
	short			*ac_ptr;		/* pte le pixel courrant dans ac */
	short			*im_ptr;		/* pte le pixel courrant dans integ courte */
	short			im_val;			/* valeur du pixel courrant dans integ courte */
	long			correl_count;	/* photons restant à corréler avec photon courrant */
	long			i;
	register unsigned long	xiyi;	/* coordonnée groupée d'un photon : xi & yi */
	unsigned long	xi, yi;			/* coordonnée déballée */
	long 			xm, ym;			/* décalage */
	long 			xdec, ydec;		/* décalés */
	photon			*phot_ptrj;		/* pointera ph.à correler avec ph. en cours */
	long			ac = (long)ac_org;

	if (ph_per_integ < 2L) return;
	
	xm = (m.right - m.left) & 0XFFFFFFFE;		/* tronqué au pair inférieur */
	ym = (m.bottom - m.top) & 0XFFFFFFFE;		/* pour s'aligner sur les pts de l'integ */
	if ((xm > HORIZ/2) || (ym > VERT/2)) xm = ym = 0;


	/* --- mettre à zéro l'integ de f(x) puis l'initialiser aux premiers photons --- */
	
	fillongs (0L, (long *)integ_temp, (long)VERT*(long)HORIZ * sizeof(short)/sizeof(long));

	/* faire une integ courte en prenant du photon "0" au photon "ph_per_correl-1" */
	for (i = 0 ; i< ph_per_correl ; i++) {
		im_ptr = (short *)( (long)integ_temp + (phot_ptr[i] << 1) );
		(*im_ptr) ++;
	}
	

	for (i = 0 ; i < ph_per_integ ; i++) {
		/* ajouter le phot "i+ph_per_correl" à l'integ courte et retirer le phot "i" */
		im_ptr = (short *)( (long)integ_temp + (phot_ptr [ph_per_correl] << 1) );
		(*im_ptr) ++;
		im_ptr = (short *)( (long)integ_temp + (*phot_ptr << 1) );
		(*im_ptr) --;
		
		xiyi = *phot_ptr++;		/* coors du phot.i ; phot_ptr pointera le phot i+1 */

		if ((xm | ym) == 0) im_val = *im_ptr;
		else {						/* décalage m non nul */
			xdec = xiyi & 0x1FF;		/* xi va de 0 à 510 par 2  (0 à HORIZ-2) */
			ydec = xiyi >>9;			/* yi va de 0 à VERT par 2 */
			xdec -= xm;				/* on décale de m */
			ydec -= ym;
			if ((xdec < 0) || (ydec < 0) || (xdec > HORIZ) || (ydec > VERT))
				im_val = 0;
			else {
										/* on remet ensemble xi et yi décalés */
				im_ptr = (short *)((long)integ_temp + (xdec<<1 | (ydec<<10) ) );
				im_val = *im_ptr;		/* on pêche la valeur au point décalé de l'integ */
			}
		}
		

		/* ne faire que s'il y a un autre phot à la position dans ƒƒf(x-m,t)dxdt */
		
		if (im_val > 0) {			/* f(xi) >1 */
			xi = xiyi & 0x1FF;		/* xi va de 0 à 510 par 2  (0 à HORIZ-2) */
			xi |= 0x200;			/* maintenant xi va de 512 à 1022 par 2 (HORIZ à 2*HORIZ-2) */
									/* HORIZ doit impèrativement être une puissance de 2 */
									
			yi = xiyi >>9;			/* yi va de 0 à VERT par 2 */
			yi += VERT;				/* maintenant yi va de VERT a 2*VERT-2 par 2 */
			
			xiyi = xi | (yi<<9);	/* on remet ensemble xi et yi modifiés */
			
			phot_ptrj = phot_ptr;	/* pointe sur xj & yj : les phots i+1 à i+n */
			correl_count = ph_per_correl;
			while (--correl_count >= 0L) {
				ac_ptr = (short *)(ac + xiyi - *phot_ptrj);
				(*ac_ptr) += im_val;	/* incrementer pixel but dans l'ac */
				phot_ptrj++;			/* xj & yj suivants */
			}
		}
	}
}





/* -------- integ d'autocorrelations image par image -------- */

/* 			AC_f (a) = ƒƒ f(x,t) f(x-a,t) dx dt		*/


void correlim9 (
photon		*phot_ptri,			/* pointeur 1er photon de l'im*/
long		ph_per_im,			/* photons par image */
unsigned short	*ac_org)		/* pointeur d'origine du tableau but (AC) */
{
	register short			*ac_ptr;		/* points current pixel in ac */
	long					integ_count;	/* photons left to integrate */
	register long			correl_count;	/* photons left to correlate with current photon */
	register unsigned long	xiyi;			/* coordonnée couplée d'un photon : xi & yi */
	unsigned long			xi;				/* coordonnée seule */
	unsigned long			yi;
	register photon			*phot_ptrj;		/* will point ph.to correlate with current photon */
	register long			ac = (long)ac_org;
	
	integ_count = ph_per_im;
	if (integ_count < 1L)
		return;

	while (--integ_count >= 0L)
	{
		xiyi = *phot_ptri++;
/* ----- attention, ici on fait l'hypothese que xi est codé sur 9 bits ----- */
		xi = xiyi & 0x1FF;		/* xi va de 0 à 510 par 2  (0 à HORIZ-2) */
		xi |= 0x200;			/* maintenant xi va de 512 à 1022 par 2 (HORIZ à 2*HORIZ-2) */
								/* HORIZ doit impèrativement être une puissance de 2 */
								
		yi = xiyi >>9;			/* yi va de 0 à VERT par 2 */
		yi += VERT;				/* maintenant yi va de VERT a 2*VERT-2 par 2 */
		
		xiyi = xi | (yi<<9);	/* on remet ensemble xi et yi modifiés */
/* ----- fin de la partie dépendante du nb de bits de codage des x ----- */
		
		phot_ptrj = phot_ptri;		/* pointe sur xj & yj */
		correl_count = integ_count;
		while (--correl_count >= 0L) {
			ac_ptr = (short *)(ac + xiyi - *phot_ptrj);
										
			(*ac_ptr)++;			/* increment target pixel in ac */
			phot_ptrj++;			/* next xj & yj */
		}
	}
}	
		





/* -------- inter-correlation ∆t en flux continu -------- */

/* 			IC_f (a, ∆t) = ƒƒ f(x,t) f(x-a,t+∆t) dx dt		*/

void de_correlate9 (
photon			*phot_ptr,		/* pointer to current photon */
long			ph_per_correl,	/* equivalent to a correlation time */
long			ph_per_integ,	/* equivalent to an integration time */
long			ph_per_decorr,	/* equivalent to a de_correlation time */
unsigned short	*de_ac_org)		/* pointer to origin af target array (storing de_ac) */
{
	register short			*de_ac_ptr;		/* points current pixel in de_ac */
	long					integ_count;	/* photons left to integrate */
	register long			correl_count;	/* photons left to correlate with current photon */
	register unsigned long	xiyi;			/* current photon coordinate : xi & yi */
	unsigned long			xi;				/* coordonnée seule */
	unsigned long			yi;
	register photon	*phot_ptrj;				/* will point ph.to correlate with current photon */

	register long	de_ac = (long)de_ac_org;

	integ_count = ph_per_integ;
	if (integ_count < 1L)
		return;
		
	while (--integ_count >= 0L)
	{
		xiyi = *phot_ptr++;
		/* ----- attention, ici on fait l'hypothese que xi est codé sur 9 bits ----- */
		xi = xiyi & 0x1FF;		/* xi va de 0 à 510 par 2  (0 à HORIZ-2) */
		xi |= 0x200;			/* maintenant xi va de 512 à 1022 par 2 (HORIZ à 2*HORIZ-2) */
								/* HORIZ doit impèrativement être une puissance de 2 */
								
		yi = xiyi >>9;			/* yi va de 0 à VERT par 2 */
		yi += VERT;				/* maintenant yi va de VERT a 2*VERT-2 par 2 */
		
		xiyi = xi | (yi<<9);	/* on remet ensemble xi et yi modifiés */
		/* ----- fin de la partie dépendante du nb de bits de codage des x ----- */

		phot_ptrj = phot_ptr + ph_per_decorr;	/* pointe xj & yj */
		correl_count = ph_per_correl;
		while (--correl_count >= 0L)
		{		
			de_ac_ptr = (short *)(de_ac + xiyi - *phot_ptrj);
			(*de_ac_ptr)++; 
			phot_ptrj++;
		}
	}
}




/* ----- inter-correlation entre deux sequences de photons (images) ----- */

/* 			IC_f1f2 (a) = ƒ f1(x,t) f2(x-a,t) dx		*/


void inter_correlim9 (
photon			*phot_ptrA,		/* pointe vers le 1er ph. de la sequence A à intercorr */
long			phots_dans_A,	/* nb de photons dans la seqence A */
photon			*phot_ptrB,		/* pointe vers le 1er ph. de la sequence B à intercorr */
long			phots_dans_B,	/* nb de photons dans la seqence B */
unsigned short	*de_ac_org)		/* pointe vers origine tableau but (intercorrelation) */
{
	register short			*de_ac_ptr;		/* pixel but dans intercorr */
	long					ph_count_A;		/* photons restant a integ dans A */
	register long			ph_count_B;		/* photons restant a integ dans B */
	register unsigned long	xiyi;			/* couple de coordonnee : xi & yi */
	unsigned long			xi;				/* coordonnée seule */
	unsigned long			yi;
	photon					*phot_ptri;
	register photon			*phot_ptrj;

	register long	de_ac = (long)de_ac_org;

	phot_ptri = phot_ptrA;
	ph_count_A = phots_dans_A;
	
	if (ph_count_A < 1L)
		return;
	
	while (--ph_count_A >= 0L)
	{
		xiyi = *phot_ptri++;
		/* ----- attention, ici on fait l'hypothese que xi est codé sur 9 bits ----- */
		xi = xiyi & 0x1FF;		/* xi va de 0 à 510 par 2  (0 à HORIZ-2) */
		xi |= 0x200;			/* maintenant xi va de 512 à 1022 par 2 (HORIZ à 2*HORIZ-2) */
								/* HORIZ doit impèrativement être une puissance de 2 */
								
		yi = xiyi >>9;			/* yi va de 0 à VERT par 2 */
		yi += VERT;				/* maintenant yi va de VERT a 2*VERT-2 par 2 */
		
		xiyi = xi | (yi<<9);	/* on remet ensemble xi et yi modifiés */
		/* ----- fin de la partie dépendante du nb de bits de codage des x ----- */

		phot_ptrj = phot_ptrB;	/* points to xj & yj */
		ph_count_B = phots_dans_B;
		while (--ph_count_B >= 0L)
		{		
			de_ac_ptr = (short *)(de_ac + xiyi - *phot_ptrj);
			(*de_ac_ptr)++; 
			phot_ptrj++;
		}
	}
}



/* ---------------- intercorrelations ∆x en flux continu ---------------- */

/* 			IC_f1f2 (a) = ƒ f1(x,t) f2(x-a,t) dx		*/

void inter_correlflux9 (
photon		*phot_ptr,			/* pointe photon courrent*/
long		ph_per_correl,		/* photons par t_correlation */
long		ph_per_integ,		/* photons par t_integration */
unsigned short	*ac_org)		/* pointe l'origine du tab but (stockage AC) */

/* integre l'ic  2-dim a partir de coordonnées de photons en mémoire.
		renvoie le pointeur de photons mis à jour.
		L'ic est calculée par soustraction de coordonnée d'un photon à n autres photons.
		L'appartenance des coordonnées des photons à l'une ou l'autre des fenetres
		d'intercorrelation a été pré-marquée par la routine "marque et centre"
		en mettant à 1 l'un ou/et l'autre des deux bits de poids fort (mot de 32 bits).
		Ceci permet d'accelerer le calcul, en reduisant le temps necessaire aux tests
		dans la boucle interne executée n*m fois : le marquage a necessité
		au maximum n + m opérations.
*/
{
	register photon			*phot_ptrj;		/* pointe vers le ph.a correler avec ph. en cours */
	register short			*ac_ptr;		/* pixel but dans l'ac */
	long					integ_count;	/* photons restant a integrer */
	register long			correl_count;	/* photons restant a correler avec photon en cours */
	
	register unsigned long	xiyi;			/* coordonnée couplée d'un photon : xi & yi */
	register long			xjyj;			/* pas "unsigned" pour que le test sur msb marche */
	unsigned long			xi, yi;
	unsigned long			bit_fenetre_i = 0X40000000; /* bit a 1 => appartenance a fen. i */
	register unsigned long	ac = (long)ac_org;	/* origine a ajouter */
	register unsigned long	mask = 0x3FFFFFFF;	/* pour couper les bits de poids forts (aoh!) */
	
	integ_count = ph_per_integ;
	if (integ_count < 1L)
		return;

	while (--integ_count >= 0L) {
		xiyi = *phot_ptr++;
		if ((xiyi & bit_fenetre_i) != 0) { 			/* on est dans la fenetre i */
/* ----- attention, ici on fait l'hypothese que xi est codé sur 9 bits ----- */
			xi = xiyi & 0x1FF;		/* xi va de 0 à 510 par 2  (0 à HORIZ-2) */						
			yi = (xiyi >>9) & 0x1FF;/* yi va de 0 à VERT par 2 */
			xi |= 0x200;		/* maintenant xi va de 512 à 1022 par 2 (HORIZ à 2*HORIZ-2) */
			yi += VERT;			/* maintenant yi va de VERT a 2*VERT-2 par 2 */
			xiyi = xi | (yi<<9);/* on remet ensemble xi et yi modifiés */
/* ----- fin de la partie dépendante du nb de bits de codage des x ----- */
			
			phot_ptrj = phot_ptr;		/* pointe sur xj & yj */
			correl_count = ph_per_correl;
			while (--correl_count >= 0L) {
				xjyj = *phot_ptrj;
				if (xjyj < 0 ) {		/* le msb est marqué : on est dans la fenetre j */
					xjyj &= mask;		/* ne garder que les coordonnées */
					ac_ptr = (short *)(ac + xiyi - xjyj);	/* faire l'intercorrelation */
					(*ac_ptr)++;		/* incrementer pixel dans ic */
				}
				phot_ptrj++;
			}
		}
	}
}


/* ------- inter_∆t triple correlation : calcul du plan à m et ∆t constant ------- */

/*			iTc_f (m, a, ∆t) = ƒƒ f(x,t) f(x-m,t) f(x-a,t+∆t) dx dt			*/

void de_plantricorr9 (
photon		*phot_ptr,			/* pointe photon en cours */
long		ph_per_correl,		/* photons par correlation t */
long		ph_per_integ,		/* photons par integration t */
long		ph_per_decorr,		/* photons par séparation ∆t */
Rect		m,					/* pour le decentrage du plan de la 3C à calculer */
unsigned short	*ac_org,		/* pointe origine du tab but (stockage AC) */
unsigned short	*integ_temp)	/* pointe origine tab annexe (stockage integ courte) */
{
	short			*ac_ptr;		/* pte le pixel courrant dans ac */
	short			*im_ptr;		/* pte le pixel courrant dans integ courte */
	short			im_val;			/* valeur du pixel courrant dans integ courte */
	long			correl_count;	/* photons restant à corréler avec photon courrant */
	long			i;
	unsigned long	xiyi;		/* coordonnée groupée d'un photon : xi & yi */
	unsigned long	xi, yi;		/* coordonnée déballée */
	long 			xm, ym;			/* décalage */
	long 			xdec, ydec;		/* décalés */
	photon			*phot_ptrj;		/* pointera ph.à correler avec ph. en cours */
	long			ac = (long) ac_org;

	if (ph_per_integ < 2L) return;

	xm = (m.right - m.left) & 0XFFFFFFFE;		/* tronqué au pair inférieur */
	ym = (m.bottom - m.top) & 0XFFFFFFFE;		/* pour s'aligner sur les pts de l'integ */
	if ((xm > HORIZ/2) || (ym > VERT/2)) xm = ym = 0;

	/* --- mettre à zéro l'integ f(x) puis l'initialiser aux premiers photons --- */
	fillongs (0L, (long *)integ_temp, (long)VERT*(long)HORIZ * sizeof(short)/sizeof(long));

	/* faire une integ courte en prenant du photon "0" au photon "ph_per_correl-1" */
	for (i = 0 ; i< ph_per_correl ; i++) {
		im_ptr = (short *)( (long)integ_temp + (phot_ptr[i] << 1) );
		(*im_ptr) ++;
	}

	for (i = 0 ; i < ph_per_integ ; i++) {
		/* ajouter le phot "i+ph_per_correl" à l'integ courte et retirer le phot "i" */
		im_ptr = (short *)( (long)integ_temp + (phot_ptr [ph_per_correl] << 1) );
		(*im_ptr) ++;
		im_ptr = (short *)( (long)integ_temp + (*phot_ptr << 1) );
		(*im_ptr) --;
		im_val = *im_ptr;
		
		xiyi = *phot_ptr++;		/* coors du phot.i ; phot_ptr pointera le phot i+1 */

		if ((xm | ym) == 0) {
			im_val = *im_ptr;
		}
		else {							/* décalage m non nul */
			xdec = xiyi & 0x1FF;		/* xi va de 0 à 510 par 2  (0 à HORIZ-2) */
			ydec = xiyi >>9;			/* yi va de 0 à VERT par 2 */
			xdec -= xm;				/* on décale de m */
			ydec -= ym;
			if ((xdec < 0) || (ydec < 0) || (xdec > HORIZ) || (ydec > VERT))
				im_val = 0;
			else {
										/* on remet ensemble xi et yi décalés */
				im_ptr = (short *)((long)integ_temp + (xdec<<1 | (ydec<<10) ) );
				im_val = *im_ptr;		/* on pêche la valeur au point décalé de l'integ*/
			}
		}
		

		/* ne faire que s'il y a un autre phot à la pos. dans ƒƒf(x-m,t+∆t)dxdt */
		if (im_val > 0) {			/* f(xi) >1 */
			xi = xiyi & 0x1FF;		/* xi va de 0 à 510 par 2  (0 à HORIZ-2) */
			xi |= 0x200;			/* maintenant xi va de 512 à 1022 par 2 (HORIZ à 2*HORIZ-2) */
									/* HORIZ doit impèrativement être une puissance de 2 */
									
			yi = xiyi >>9;			/* yi va de 0 à VERT par 2 */
			yi += VERT;				/* maintenant yi va de VERT a 2*VERT-2 par 2 */
			
			xiyi = xi | (yi<<9);	/* on remet ensemble xi et yi modifiés */
			
			phot_ptrj = phot_ptr + ph_per_decorr;	/* pte xj & yj : phots i+N+1 à i+N+n*/
			correl_count = ph_per_correl;
			while (--correl_count >= 0L) {
				ac_ptr = (short *)(ac + xiyi - *phot_ptrj);
				(*ac_ptr) += im_val;	/* incrementer pixel but dans l'ac */
				phot_ptrj++;			/* xj & yj suivants */
			}
		}
	}
}	
