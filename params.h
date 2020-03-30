/* LK 15 6 94  */

/* reglages initiaux pour specklator */
#define T_CYCLE (8000)
#define T_POSE (1000)
#define T_CORR (20)
#define T_DECORR (100)

/* ------ taille des buffers de photons en octets ------ */

#define PHOT_BUF_SIZE	100000L /* 3Mo->30000 2Mo->20000 taille buf en nb de ph.*/
#define MAX_IMAGES		(PHOT_BUF_SIZE / 8)  /* minimum 8 photons par image en moyenne */


/* ----- interfranges et taille des speckles (pixels calcul:1024*1024) ------ */

#define INTERF_PAR_DEFAUT (18.5)	/* interf: Deneb I2T-> 24.7; beta UMa GI2T->19.5 */
#define D_INTERF_2400	  (-1.12e-3)/* (-1.12e-3) d(interf)/dx pour reseau 2400 tr/mm */
#define SPECKLE_PAR_DEFAUT (90)		/* taille speckle: Deneb I2T-> 74; beta UMa GI2T->74 */
									/* Gamma Lyr 21/7/94 -> 90 */
#define NB_SPKL_PAR_DEFAUT (10)
#define NB_SPKL_MAX (10)

#define ECHELLE_DDM		(10.)		/* µm/pixel : rapport ddm / pos.pic */


/* --------- Resolution lors du calcul (pixels echelle calcul TF ligne) ------- */

#define LGX_TFM  (128)	/* resolution en x de la TF ligne (doit etre puiss de 2) */
#define LGY_TFM (1024)	/* resolution en y de la TF ligne (doit etre puiss de 2) */
#define LGT_TFM (4096)	/* resolution en t de la TF ligne */

#define LGY_HYS (1024)	/* resolution en y de l'histo (doit etre puiss de 2) */


/* --------- Resolution lors l'affichage (pixels affichage) --------------- */

#define LGX_VISU (512)	/* (doit etre puiss de 2) */
#define LGY_VISU (512)	/* (doit etre puiss de 2) */




/* --------- Taille des fenetres et images (pixels affichage:512*512) --------- */

/* ------ fenetre visu rapide ------ */

#define HORIZ_MONI	(256)		
#define VERT_MONI	(256)

/* ------ fenetre AC et somme ------ */

#define HORIZ		(512)	/* nb de colones dans l'image (doit être une puissance de 2) */
#define VERT		(512)	/* 3Mo -> 420 ; 2Mo -> 250 nb de l. dans l'ima.*/
#define H_ORG		  (2)	/* gauche de la fenetre display, EN COORDONNEES ECRAN */
#define V_ORG		 (32)	/* haut  de la fenetre display, EN COORDONNEES ECRAN  */
#define MARGE		(120)	/* place pour l'affichage des paramètres sur la fenetre image. */
#define V_MARGE		 (10)	/* haut des listes de paramètres sur la fenetre d'affichage. */
#define H_MARGE		  (5)	/* marge des listes de paramètres */
#define INTERLINE	 (15)	/* interligne pour affichage des parametres */

/* ------ fenetres TF ------ */

#define HORIZ_TF	(128)	/* taille de la fenetre de source TF dans l'ac - */
#define VERT_TF		 (64)	/* - a transfourrier et du plan UV calcule */
#define ECH_V_DSPL_TF (4)	/* pour l'affichage : agrandir d'un facteur 2 verticalement */
#define TF_MARGE	 (60)	/* un peu de place pour afficher les valeurs du pic */
#define INTERLINE_TF (12)


/* ----------- Resolution lors de l'aquisition (pixels caméra) -------------- */

/* ----- Camera a Anode Resistive Ranicon ------- */
			/* resolutions des donnees brutes */
#define LGX_CAR (1024)	/* resolution brute en x de la camera (doit etre puiss de 2) */
#define LGY_CAR (1024)	/* resolution brute en y de la camera (doit etre puiss de 2) */
#define LGT_CAR (4048)	/* resolution brute en t de la camera (doit etre puiss de 2) */
			/* ordonnancement des donnees */
#define DECALX_CAR (0)	/* les x de la CAR sont a partir du bit 0 */
#define DECALY_CAR (10)	/* les y sont a partir du bit 10 */
#define DECALT_CAR (20)	/* les t sont a partir du bit 20 */

/* -------- Camera PAPA ----------- */
#define LGX_PAPA (256)				/* resolutions brutes */
#define LGY_PAPA (256)

#define DECALX_PAPA (0)				/* ordonnancement des donnees */
#define DECALY_PAPA (8)

/* ---------- Cameras CP40 ---------- */
/* champ individuel d'un canal CP40 : 288 x 384 */
#define LGX_CP40 (512)				/* resolutions brutes (codage) */
#define LGY_CP40 (512)

#define DECALX_CP40 (2)				/* ordonnancement des donnees */
#define DECALY_CP40 (14)

#define	masc_c_pair		(0x800000)	/* position des bits indicateurs de canal */
#define	masc_c_impair	(0x800)
