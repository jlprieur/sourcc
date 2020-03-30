##################################################################
# cordisto.make
#
# JLP
# Version 10-04-93
#
##################################################################
smgolib=$(JLPLIB)/smgo
mylib=$(JLPLIB)/jlp
midir=$(JLPLIB)/midas
LIB=$(mylib)/jlpacc.a $(mylib)/jlputil.a $(mylib)/jlpacc.a 
#XLIB= -lXaw -lXmu -lXt -lX11
XLIB=
#F77LIB=$(F77DIR)/libF77.a $(F77DIR)/libI77.a $(F77DIR)/libU77.a 
F77LIB= -lc $(F77DIR)/libxlf.a
#MIDLIB=$(midir)/ftoclib.a $(midir)/stlib.a $(midir)/oslib.a
MIDLIB=$(midir)/libst.a $(midir)/libos.a
#NAGLIB=/usr1/midas/91MAY/lib/mathlib.a
NAGLIB=
ESOEXT=/midas/frozen/exec/esoext.exe
#FOURN=fourn.o
FOURN=fft_jlp.o poidev.o
COVER=jlp_cover.o
INC=-I../jlp_incl
CCLAGS=-c -g -C -D$(JLPSYSTEM) 

.SUFFIXES:
.SUFFIXES: .o .c .exe $(SUFFIXES) 

.c.o:
	cc $(CCLAGS) $(INC) $*.c

.o.exe:
	cc -g -o $(EXEC)/$*.exe $*.o \
	$(LIB) $(MIDLIB) $(NAGLIB) $(XLIB) -lm

.c.exe:
	cc $(CCLAGS) $(INC) $*.c
	cc -g -o $(EXEC)/$*.exe $*.o \
	$(LIB) $(MIDLIB) $(NAGLIB) $(XLIB) -lm

all: conju_grad.o cordisto.exe

#cc $(CCLAGS) $(INC) -o $(JLPLIB)/math/conju_grad.o conju_grad.c
conju_grad.o : conju_grad.c
	cc $(CCLAGS) $(INC) conju_grad.c
	mv conju_grad.o $(JLPLIB)/math/conju_grad.o

cordisto.exe : cordisto.c conju_grad.c  
	cc $(CCLAGS) $(INC) cordisto.c
	cc -g -o $(EXEC)/cordisto.exe cordisto.o \
	$(JLPLIB)/math/conju_grad.o $(LIB) $(MIDLIB) $(NAGLIB) $(XLIB) -lm
