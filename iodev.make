##################################################################
# jlp_iodev.make
# JLP
# Version 25-05-92
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
MIDLIB=$(midir)/stlib.a $(midir)/oslib.a
#NAGLIB=/usr1/midas/91MAY/lib/mathlib.a
NAGLIB=
ESOEXT=/midas/frozen/exec/esoext.exe
#FOURN=fourn.o
FOURN=fft_jlp.o poidev.o
COVER=jlp_cover.o
INC=-I../midincl
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

all: jlp_iodev.o ctape.exe

jlp_iodev.o : jlp_iodev.c 
	cc $(CCLAGS) $(INC) jlp_iodev.c

ctape.exe : ctape.c 
	cc $(CCLAGS) $(INC) ctape.c
	cc -g -o $(EXEC)/ctape.exe ctape.o jlp_iodev.o \
	$(LIB) $(MIDLIB) $(NAGLIB) $(XLIB) -lm

