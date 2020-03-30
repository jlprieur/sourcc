program modcor

! calcul de la fonction de correlation des scintillations
implicit none

integer	:: i, j, k, nb_int 	! nombre de pas pour le calcul de l'integrale
real 	:: Pi, epo, pasf, pasp, lambda, arhm, rhoi, arho
real	:: coef, xj, yj, zj, resint, restot, bessj0
real, dimension(5)	:: hm, Jt, rescint
real, dimension(5000)  	:: fk	

 Pi = 4.0*atan(1.0)
 epo = -8/3.
 nb_int = 5000
 pasf = 0.02
 pasp = 7.63e-3
 lambda = 0.53e-6 	! longueur d'onde, en metre	

 coef = 0.243*4*Pi*Pi/lambda/lambda
 coef = coef*3.0e-14	! soit un r_{0} de 0.7 m a 0.53 \mu m

open (11, file = 'modcor.dat')

hm(1)=5000; hm(2)=10000; hm(3)=15000; hm(4)=20000; hm(5)=25000
! hm(1)=17200; hm(2)=13400; hm(3)=9600; hm(4)=4250	! distance de la couche en m
! Jt(1)=0.25  ; Jt(2)=0.25 ; Jt(3)=0.5; Jt(4)=1.0	! contenu de turbulence 
 
do i=-50,50
	rhoi = pasp*i	! rho en metre
	arho = 2*Pi*rhoi
	restot = 0.
	
  do k=1,5
 	arhm = Pi*lambda*hm(k)
	
	do j=1,nb_int
		xj = pasf*j
		yj = arhm*xj*xj
		zj = arho*xj
		fk(j) = xj**epo*sin(yj)*sin(yj)*bessj0(zj)
	enddo
	
	resint = coef*pasf*( 0.5*(fk(1) + fk(nb_int)) + &
		 sum( (/(fk(j), j=2,nb_int-1)/)) )
	rescint(k) = resint	
	
	restot = restot + rescint(k)
  enddo
  
  write (11, '(i5, 7(f10.6))') i, rhoi, rescint, restot
enddo
	
end program modcor

	
	



