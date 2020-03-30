 program filtre

! on calcule la fonction de psf, gaussienne 2-D
! exp(-0.5*(i^2+j^2)/sig2) normalisee, avec sig2=1.5, soit sig=1.22

 implicit none

 integer	:: i, j, ii
 real		:: sig2, arg, DPi, somch

 real, dimension(16)   :: chapeau	

 open (10, file = 'filtre.dat') 
    
! calcul de la fonction de lissage
    sig2 = 1.5
    DPi = 8.0*atan(1.0)		! c'est 2*pi
    somch = 0.
    do j= -7,8
    	do i= -7,8
	   ii = i+8
	   arg = 0.5*(i*i+j*j)/sig2
	   chapeau(ii) = 1.0*exp(-arg)/DPi/sig2
	   somch = somch + chapeau(ii)
	enddo

	write(10,'(16(f9.6))') chapeau
	
    enddo
    
    print*,'somch = ', somch

end program filtre
