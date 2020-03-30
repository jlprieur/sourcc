##################################################################
# risley.make
# JLP
# Version 25-05-92
#
##################################################################
include ../jlp_make.mk
INC=-I../jlp_incl

.SUFFIXES:
.SUFFIXES: .o .c .exe $(SUFFIXES) 

.c.o:
	cc -c $(CFLAGS) $(INC) $*.c

.o.exe:
	cc -g -o $(EXEC)/$*.exe $*.o \
	$(JLIB) $(MIDLIB) $(MATHLIB) $(F77LIB) $(XLIB) -lm

.c.exe:
	cc -c $(CFLAGS) $(INC) $*.c
	cc -g -o $(EXEC)/$*.exe $*.o \
	$(JLIB) $(MIDLIB) $(MATHLIB) $(F77LIB) $(XLIB) -lm

all: risley.exe

risley.exe : risley.c risley.o 
	cc -c $(CFLAGS) -DMAIN_PROG $(INC) risley.c
	cc -g -o $(EXEC)/risley.exe risley.o \
	$(JLIB) $(MIDLIB) $(MATHLIB) $(F77LIB) $(XLIB) -lm

risley.o : risley.c 
	cc -c $(CFLAGS) $(INC) risley.c

clean:
	rm -f risley.o
