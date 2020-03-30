/*
 * xgif.c - displays GIF pictures on an X11 display
 *
 *  Author:    John Bradley, University of Pennsylvania
 *                (bradley@cis.upenn.edu)
 */
 
#define MAIN
#include "xgif.h"
 
/*******************************************/
main(argc, argv)
    int   argc;
    char *argv[];
/*******************************************/
{
    int        i;
    char      *display, *geom, *fname;
    XEvent     event;
 
    cmd = argv[0];
    display = geom = fname = NULL;
    expImage = NULL;
 
    expand = 1;  strip = 0;  nostrip = 0;
 
    /*********************Options*********************/
 
    for (i = 1; i < argc; i++) {
        char *strind;
 
        if (argv[i][0] == '=') {
            geom = argv[i];
            continue;
            }
 
        if (!strncmp(argv[i],"-g",2)) {        /* geometry */
            i++;
            geom = argv[i];
            continue;
            }
 
        strind = index(argv[i], ':');
        if(strind != NULL) {
            display = argv[i];
            continue;
            }
 
        if (!strcmp(argv[i],"-e")) {
            i++;
            expand=atoi(argv[i]);
            continue;
            }
 
        if (!strcmp(argv[i],"-s")) {
            i++;
            strip=atoi(argv[i]);
            continue;
            }
 
        if (!strcmp(argv[i],"-ns")) {
            nostrip++;
            continue;
            }
 
        if (argv[i][0] != '-') {        /* the file name */
            fname = argv[i];
            continue;
            }
 
        Syntax(cmd);
    }
 
    if (fname==NULL) fname="-";
    if (expand<1 || expand>MAXEXPAND) Syntax(cmd);
    if (strip<0 || strip>7) Syntax(cmd);
 
    /*****************************************************/
 
    /* Open up the display. */
 
    if ( (theDisp=XOpenDisplay(display)) == NULL) {
        fprintf(stderr, "%s: Can't open display\n",argv[0]);
        exit(1);
        }
 
    theScreen = DefaultScreen(theDisp);
    theCmap   = DefaultColormap(theDisp, theScreen);
    rootW     = RootWindow(theDisp,theScreen);
    theGC     = DefaultGC(theDisp,theScreen);
    fcol      = WhitePixel(theDisp,theScreen);
    bcol      = BlackPixel(theDisp,theScreen);
    theVisual = DefaultVisual(theDisp,theScreen);
 
    dispcells = DisplayCells(theDisp, theScreen);
    if (dispcells<255)
        FatalError("This program requires an 8-plane display, at least.");
 
    /****************** Open/Read the File  *****************/
    LoadGIF(fname);
    iWIDE = theImage->width;  iHIGH = theImage->height;
 
    eWIDE = iWIDE * expand;  eHIGH = iHIGH * expand;
    if (eWIDE > DisplayWidth(theDisp,theScreen))
        eWIDE = DisplayWidth(theDisp,theScreen);
    if (eHIGH > DisplayHeight(theDisp,theScreen))
        eHIGH = DisplayHeight(theDisp,theScreen);
 
    /**************** Create/Open X Resources ***************/
    if ((mfinfo = XLoadQueryFont(theDisp,"variable"))==NULL)
       FatalError("couldn't open 'variable' font\n");
    mfont=mfinfo->fid;
    XSetFont(theDisp,theGC,mfont);
    XSetForeground(theDisp,theGC,fcol);
    XSetBackground(theDisp,theGC,bcol);
 
    CreateMainWindow(cmd,geom,argc,argv);
    Resize(eWIDE,eHIGH);
 
    XSelectInput(theDisp, mainW, ExposureMask | KeyPressMask
                               | StructureNotifyMask);
    XMapWindow(theDisp,mainW);
 
    /**************** Main loop *****************/
    while (1) {
        XNextEvent(theDisp, &event);
        HandleEvent(&event);
        }
}
 
/****************/
HandleEvent(event)
    XEvent *event;
/****************/
{
    switch (event->type) {
        case Expose: {
            XExposeEvent *exp_event = (XExposeEvent *) event;
 
            if (exp_event->window==mainW)
                DrawWindow(exp_event->x,exp_event->y,
                           exp_event->width, exp_event->height);
            }
            break;
 
        case KeyPress: {
            XKeyEvent *key_event = (XKeyEvent *) event;
            char buf[128];
            KeySym ks;
            XComposeStatus status;
 
            XLookupString(key_event,buf,128,&ks,&status);
            if (buf[0]=='q' || buf[0]=='Q') Quit();
            }
            break;
 
        case ConfigureNotify: {
            XConfigureEvent *conf_event = (XConfigureEvent *) event;
 
            if (conf_event->window == mainW &&
                 (conf_event->width != eWIDE || conf_event->height != eHIGH))
                Resize(conf_event->width, conf_event->height);
            }
            break;
 
        case CirculateNotify:
        case MapNotify:
        case DestroyNotify:
        case GravityNotify:
        case ReparentNotify:
        case UnmapNotify:       break;
 
        default:
            printf("Unexpected event type [%ld]\n",event->type);
            /* FatalError("Unexpected X_Event"); */
 
        }  /* end of switch */
}
 
/***********************************/
Syntax()
{
    printf("Usage: %s filename [=geometry | -geometry geom] [display]\n",cmd);
    printf("       [-e 1..%d] [-s 0-7] [-ns]\n",MAXEXPAND);
    exit(1);
}
 
/***********************************/
FatalError (identifier)
       char *identifier;
{
    fprintf(stderr, "%s: %s\n",cmd, identifier);
    exit(-1);
}
 
/***********************************/
Quit()
{
    exit(0);
}
 
/***********************************/
DrawWindow(x,y,w,h)
{
    XPutImage(theDisp,mainW,theGC,expImage,x,y,x,y,w,h);
}
 
/***********************************/
Resize(w,h)
int w,h;
{
    int  ix,iy,ex,ey;
    byte *ximag,*ilptr,*ipptr,*elptr,*epptr;
    static char *rstr = "Resizing Image.  Please wait...";
 
    /* warning:  this code'll only run machines where int=32-bits */
 
    if (w==iWIDE && h==iHIGH) {        /* very special case */
        if (expImage != theImage) {
            if (expImage) XDestroyImage(expImage);
            expImage = theImage;
            eWIDE = iWIDE;  eHIGH = iHIGH;
            }
        }
 
    else {                /* have to do some work */
        /* if it's a big image, this'll take a while.  mention it */
        if (w*h>(500*500)) {
            XDrawImageString(theDisp,mainW,theGC,CENTERX(mfinfo,w/2,rstr),
                  CENTERY(mfinfo,h/2),rstr, strlen(rstr));
            XFlush(theDisp);
            }
 
    /* first, kill the old expImage, if one exists */
    if (expImage && expImage != theImage) XDestroyImage(expImage);
 
        /* create expImage of the appropriate size */
 
        eWIDE = w;  eHIGH = h;
        ximag = (byte *) malloc(w*h);
        expImage = XCreateImage(theDisp,theVisual,8,ZPixmap,0,ximag,
                        eWIDE,eHIGH,8,eWIDE);
 
        if (!ximag || !expImage) {
            fprintf(stderr,"ERROR: unable to create a %dx%d image\n",w,h);
            exit(0);
            }
 
        elptr = epptr = (byte *) expImage->data;
 
        for (ey=0;  ey<eHIGH;  ey++, elptr+=eWIDE) {
            iy = (iHIGH * ey) / eHIGH;
            epptr = elptr;
            ilptr = (byte *) theImage->data + (iy * iWIDE);
            for (ex=0;  ex<eWIDE;  ex++,epptr++) {
                ix = (iWIDE * ex) / eWIDE;
                ipptr = ilptr + ix;
                *epptr = *ipptr;
                }
            }
        }
}
 
/***********************************/
CreateMainWindow(name,geom,argc,argv)
    char *name,*geom,**argv;
    int   argc;
{
    XSetWindowAttributes xswa;
    unsigned int xswamask;
    XSizeHints hints;
    int i,x,y,w,h;
 
    x=y=w=h=1;
    i=XParseGeometry(geom,&x,&y,&w,&h);
    if (i&WidthValue)  eWIDE = w;
    if (i&HeightValue) eHIGH = h;
 
    if (i&XValue || i&YValue) hints.flags = USPosition;
                         else hints.flags = PPosition;
 
    hints.flags |= USSize;
 
    if (i&XValue && i&XNegative)
        x = XDisplayWidth(theDisp,theScreen)-eWIDE-abs(x);
    if (i&YValue && i&YNegative)
        y = XDisplayHeight(theDisp,theScreen)-eHIGH-abs(y);
 
    hints.x=x;             hints.y=y;
    hints.width  = eWIDE;  hints.height = eHIGH;
 
    xswa.background_pixel = bcol;
    xswa.border_pixel     = fcol;
    xswamask = CWBackPixel | CWBorderPixel;
 
    mainW = XCreateWindow(theDisp,rootW,x,y,eWIDE,eHIGH,2,0,CopyFromParent,
                          CopyFromParent, xswamask, &xswa);
 
    XSetStandardProperties(theDisp,mainW,"xgif","xgif",None,
                            argv,argc,&hints);
 
    if (!mainW) FatalError("Can't open main window");
 
}
 
