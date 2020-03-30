########################################################################
# jlp_make.mk  (linux version : SUSE 6.4)
# JLP
# Version 24/07/00 
##############################################################################
# For Dec3100: 
F77DIR=/usr/lib/cmplrs/f77
F77LIB=$(F77DIR)/libF77.a $(F77DIR)/libI77.a $(F77DIR)/libU77.a \
       $(F77DIR)/libF77.a
# For Dec5100 (Pic du Midi): F77LIB= -lfor -lutil -lots -li (remove -lc)
F77LIB= -lfor -lutil -lots -li
# For SUN (Manchester University): (mas1 or mas2)
F77DIR=/opt/SUNWspro/SC3.0.1/lib
F77LIB=$(F77DIR)/libF77.a  $(F77DIR)/libsunmath_mt.a $(F77DIR)/libM77.a
#
# For Jodrell Bank (agrona, Sun/Sparc5):
F77DIR=/opt/SUNWspro/lib
F77LIB= $(F77DIR)/libF77.so $(F77DIR)/libC.so $(F77DIR)/libsunmath.so.1 \
        $(F77DIR)/libM77.so -lsocket -lnsl
# For IBM/ts:
F77LIB=-lc -lxlf
# For Linux: 
# Linux: SUSE 5.3
# F77LIB=-lf2c
# Linux: SUSE 6.4 
F77LIB=-lg2c
# Linux: Scientific Linux 5.0, Centos 6 
F77DIR=-L/usr/lib/gcc/x86_64-redhat-linux/3.4.6/
F77LIB= $(F77DIR) -lg2c
###############################################
# For Sun and Dec:
LIBC=
# For Jodrell Bank (agrona, Sun/Sparc5):
LIBC= -lsocket -lc /usr/lib/ld.so.1 -lnsl
LIBC= -lsocket -lc -lnsl
# For IBM/ts:
#Linux:
# SUSE 5.3:
#LIBC= /usr/lib/libc.a
#Scientific Linux:
%LIBC=/usr/lib/gcc/x86_64-redhat-linux/4.1.1/libstdc++.a
# Centos6
LIBC=/usr/lib64/libstdc++.so.6
# Debian10 
LIBC=/usr/lib/x86_64-linux-gnu/libstdc++.so.6
##############################################
mylib=$(JLPLIB)/jlp
JLIB_NOGRAPHIC=$(mylib)/jlpacc.a 
JLIB=$(mylib)/newplot0.a $(mylib)/jlp_x11plot.a \
	$(mylib)/jlp_wxplot.a $(mylib)/jlp_splot.a \
	$(mylib)/jlp_splot_idv.a $(mylib)/jlp_splot.a \
	$(mylib)/jlp_x11plot.a $(mylib)/jlpacc.a 
JLIB=$(mylib)/newplot0.a \
	$(mylib)/jlp_wxplot.a $(mylib)/jlp_splot.a \
	$(mylib)/jlp_splot_idv.a $(mylib)/jlp_splot.a \
	$(mylib)/jlpacc.a
##############################################
# midir=$(JLPLIB)/midas
# MIDLIB=$(midir)/stlib.old $(midir)/udiolib.old $(midir)/oslib.old
# MIDLIB=$(midir)/libst.a $(midir)/libdio.a $(midir)/libos.a
# JLP2007: Scientific linux 5.0
#FITSLIB=/usr/lib64/libcfitsio.a
#CFITSIO_INCL_DIR=/usr/include/cfitsio
# JLP2013: CENTOS 
FITSLIB=/usr/lib64/libcfitsio.so.0
FITSLIB=$(JLPLIB)/jlp/jlp_fits.a $(mylib)/libcfitsio.a 
CFITSIO_INCL_DIR=$(JLPSRC)/jlplib/cfitsio/incl
##############################################
# Manchester Univ., (mas1 or mas2):
MATHLIB= /export/local/star/nag/nagfl15df/libnag.a
# For Jodrell Bank (agrona, Sun/Sparc5):
MATHLIB= /star/lib/libnag.a
# For IBM/carl:
MATHLIB =/usr/local/lib/libnag.a
# Linux (Toulouse, dec 2006):
MATHLIB = $(JLPLIB)/math/jlp_numeric.a $(JLPLIB)/math/mymath.a \
	$(JLPLIB)/math/libfftw.a $(JLPLIB)/math/lbfgs.a
# CENTOS/Linux (Toulouse, dec 2013):
MATHLIB = $(JLPLIB)/jlp/jlp_numeric.a $(JLPLIB)/math/libfftw.a
# CENTOS/Linux (fev 2016):
MATHLIB = $(JLPLIB)/jlp/jlp_numeric.a $(JLPLIB)/math/libfftw.a \
	$(JLPLIB)/math/mynag.a 
# Debian10
MATHLIB = $(JLPLIB)/jlp/jlp_numeric.a $(JLPLIB)/math/libfftw3.a 
##############################################
##XLIB= -lXaw -lXmu -lXt -lX11
#XLIB= -lX11
# Linux: SUSE 5.3
# XLIB= /usr/X11R6/lib/libX11.a
# Linux: SUSE 6.4 
XLIB=/usr/X11/lib/libX11.so
# Linux: Scientific Linux 5.0 
XLIB=-lX11
##############################################
# For Sun
F77=f77
# For IBM:
F77=xlf
# For Linux: 
F77=f77
# For Linux:
CC=gcc
CPP=c++
##############################################
INC=-I. 
##############################################
# For Manchester Univ. Sun (mas1, mas2)
FFLAGS=-a 
# For Jodrell Bank (agrona, Sun/Sparc5):
FFLAGS= 
# For IBM, DEC, etc:
FFLAGS=-g
# For SUSE 6.4: -fno_globals to avoid pb with MADRID() ...
#FFLAGS=-g -fno-globals 
#Centos6 :
#FFLAGS=-g -fno-globals -Wall
# Debian10
FFLAGS=-g -Wall
##############################################
# For Manchester Univ. Sun (mas1, mas2)
jlp_include=-I$(JLPSRC)/jlp_incl -I$(CFITSIO_INCL_DIR)
CFLAGS=-g -D$(JLPSYSTEM) -I. $(jlp_include) -I/export/local/X11R5/include
# For Jodrell Bank (agrona, Sun/Sparc5):
##CFLAGS=-g -D$(JLPSYSTEM) -I. $(jlp_include) -I/usr/openwin/share/include
#or: /opt/X11R5
CFLAGS= -D$(JLPSYSTEM) -I. $(jlp_include) -I/opt/X11R5/include
# For Dec, IBM
CFLAGS=-g -D$(JLPSYSTEM) -I. $(jlp_include)
# For linux:
# Check if /* is in comments:
# Check correct format in "printf" "scanf", ... 
CFLAGS=-g -Wcomment -Wformat -Wall -D$(JLPSYSTEM) -I. $(jlp_include)
