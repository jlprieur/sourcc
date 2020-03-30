XLIB = -lX11
CFLAGS = -O
 
OBJS = xgif.o xgifload.o
 
all: xgif
 
xgif: $(OBJS)
	cc $(CFLAGS) -o xgif $(OBJS) $(XLIB) $(CLIBS)
 
clean:
	rm -f $(OBJS)
 
