#%/bin/csh
##################################################################
# Parameters: size reference binary f_atm,f_tel D/F,angle,sep nlines
##################################################################
# in /dt/speckle3_tbl/98aug31: 
runs fringes 256 hr933v1_m capellav_m 25,48 F,53.,35. 10
# Now:
#runs fringes nber_of_classes reference binary f_atm,f_tel D/F,angle,sep nlines
runs fringes 1 reference binary f_atm,f_tel F,angle,sep 5
clean:
rm -f dist.dat erreur2d.fits erreurrot.fits 
rm -f visib2d.fits visibfit.dat visibrot.fits
end:
