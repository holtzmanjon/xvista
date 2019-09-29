#include "Vista.h"
/*
   X11 Windows image routines - John Tonry 5/12/88
   Implemented in xvista with various mods - J. Holtzman 5/90-5/91, 96
 */

#include <stdio.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#include <X11/Xresource.h>
#ifdef __HAIRS
#include <X11/cursorfont.h>
static Bool  hairs_on = False;
static  XSegment    crshr[4] = {
	{-1, -1, -1, -1},
	{-1, -1, -1, -1},
	{-1, -1, -1, -1},
	{-1, -1, -1, -1},
};
int usehairs = 0;
int private = 0;
#endif
#include <signal.h>

#define LONG int

int     fd_for_X = -1;
int waiting_for_key = 0;

#include "zimage.h"
int imagevalid;

int blackPixel(Display *, int);
int whitePixel(Display *, int);
  XColor testcolor[200];


/* Routine to exercise refresh without signals and interrupts (for debugging) */
imagerefresh_()
{
  if(wbase != 0) {
    xtv_refresh(0);
    XFlush(dpy);
  }
return(0);
}

int tvinit = 0;
/* Initialize X server connection and windows */

int id = 0;

int imageinit(winit,hinit,nclrs,zfinit,yupinit,resourcename,windowname,xoff,yoff)
int *winit, *hinit;     /* Size of window (request on input, actual returned */
int *nclrs;             /* Number of image color cells requested and actual */
int zfinit;             /* Initial zoom factor (0 for no zoom window) */
int yupinit;            /* flag = 0/1 for y increasing down/up */
char *resourcename;
char *windowname;
int xoff, yoff;
{
  int pfd[2];                     /* pipe descriptors */
  int i, ierr;
  char *option;
  XColor  curswcolor, cursbcolor;
  Pixmap csource, cmask;

  int nvisuals, status, nbytes;
  XVisualInfo *vinfo[16];
  XVisualInfo vtemplate;

  /* Initialize  necessary variable from include file: zimage.h  */
  /* These initializations were removed from the include file so that */
  /*   it can be included in separate files,  JAH 5/90  */
  dpy = NULL;
  wbase = (Window)0;
  wzoom = (Window)0;
  display = NULL;
  geometry = NULL;
  npalette = MAXCOLORS;
  vlastcolor = (-1);
  resize = 0;
  autozoomout = 0;
  zoomsample = 1;
  for (i=0; i<MAXSTORE; i++) image[i] = NULL;
  imbuf = NULL;
  outbuf = NULL;
  zim = 0;
  zoomf = DEFZOOMFAC;
  buttondown = 0;
  store = 0;
  sexages = 1;

  zoomf = zfinit;
  yup = yupinit;
  imagevalid = 0;

  /* Open the connection to the server, quit on failure */
  if( ! (dpy = XOpenDisplay(display)) ) {
    fprintf(stderr,"Can't open display %s\n",XDisplayName(display));
    return(ERR_CANT_OPEN_DISPLAY);
  }
  screen = XDefaultScreen(dpy);

/* What kind of visual are we running? */
  visual = DefaultVisual(dpy,screen);
  depth = DisplayPlanes(dpy,screen);
  directcolor = truecolor = 0;
  if (visual->class == PseudoColor) {
    fprintf(stderr,"Running PseudoColor display");
  } else if (visual->class == TrueColor) {
    fprintf(stderr,"Running TrueColor display");
    truecolor = 1;
  } else if (visual->class == DirectColor) {
    fprintf(stderr,"Running DirectColor display");
    directcolor = 1;
  }
  vtemplate.class = visual->class;
  *vinfo = XGetVisualInfo(dpy,VisualClassMask,&vtemplate,&nvisuals);
/*
  if (nvisuals == 0) {
    printf("Your X server does not have a PseudoColor mode\n");

    vtemplate.class = DirectColor;
    *vinfo = XGetVisualInfo(dpy,VisualClassMask,&vtemplate,&nvisuals);
    if (nvisuals == 0) {
      printf("dont have TrueColor either!\n");

      vtemplate.class = TrueColor;
      *vinfo = XGetVisualInfo(dpy,VisualClassMask,&vtemplate,&nvisuals);
      if (nvisuals == 0) {
        printf("dont have DirectColor either!\n Not ready to deal with this!\n");
        exit(0); 
      } else {
       truecolor = 1;
       printf("have a DirectColor mode %d %x %x %x\n",nvisuals,rmask,gmask,bmask);
      }
   }
   directcolor=1;
  }
*/
  visual = vinfo[0]->visual;
  depth = vinfo[0]->depth;
  rmask = vinfo[0]->red_mask;
  gmask = vinfo[0]->green_mask;
  bmask = vinfo[0]->blue_mask;

/* Can we allocate enough colors? If not, use a private colormap */
  defcmap = XDefaultColormap(dpy,screen);
  private = 0;
  if (truecolor) {
    ncolors=256;
    *nclrs = 256;
  } else
    ncolors = *nclrs;

  fprintf(stderr,"  ncolors: %d\n",ncolors);
  if (!truecolor) {
    /* Try to allocate *nclrs to see if they are available */
    if (XAllocColorCells(dpy, defcmap, 0, planes, 0, pixels, *nclrs) == 0) {
      fprintf(stderr,"Insufficient colors in default colormap: switching to private colormap\n");
      defcmap = XCreateColormap(dpy,RootWindow(dpy,screen),visual,AllocNone );
      private= 1;
      if (XAllocColorCells(dpy,defcmap,False,planes,0,pixels,*nclrs) ==0 )
        fprintf(stderr,"Error in XAllocColorCells: %d\n",*nclrs);
    } 
    for (i=0 ; i<ncolors; i++) {
      testcolor[i].pixel = pixels[i];
      cmap[i].pixel=pixels[i];
      cmap[i].flags = DoRed | DoGreen | DoBlue;
    }
  } 
 
  /* Get color addresses for standard colors for status lights, standard
      overlay colors (red, white, green, black, blue) */
  stcolor[0].red = 0xffff;
  stcolor[0].green = 0;
  stcolor[0].blue = 0;
  stcolor[1].red = 0xffff;
  stcolor[1].green = 0xffff;
  stcolor[1].blue = 0xffff;
  stcolor[2].red = 0;
  stcolor[2].green = 0xffff;
  stcolor[2].blue = 0;
  stcolor[3].red = 0;
  stcolor[3].green = 0;
  stcolor[3].blue = 0;
  stcolor[4].red = 0;
  stcolor[4].green = 0;
  stcolor[4].blue = 0xffff;
  for (i=0;i<5;i++) {
    XAllocColor(dpy,defcmap,stcolor+i);
    stcolor[i].flags = DoRed | DoGreen | DoBlue;
  }
  /* Set RGB as standard overlay colors. Default overlay color will be set
     with image color specification */
  nvcolors=4;
  vcolor[1].pixel = stcolor[0].pixel;
  vcolor[2].pixel = stcolor[2].pixel;
  vcolor[3].pixel = stcolor[4].pixel;
  for (i=0;i<nvcolors;i++) {
    vcolor[i].flags = DoRed | DoGreen | DoBlue;
  }

  /* Get memory for the lookup table */
  if ((lookup = (short *)malloc(1<<17) ) == NULL) {
    fprintf(stderr,"Can't allocate lookup table\n");
    return(ERR_BAD_ALLOC_LOOKUP);
  }
 
  /* Get memory for the palette array */
  if ((palette = (unsigned char *)malloc(4*MAXPALWIDTH))==NULL) {
    fprintf(stderr,"Can't allocate palette array\n");
    return(ERR_BAD_ALLOC_PALETTE);
  }
  
  /* Get memory for the display buffer; ask for X's maximum of 128k */
  if((imbuf = (unsigned char *)malloc(4*MAXSCREENPIX))==NULL) {
    fprintf(stderr,"Can't allocate display buffer\n");
    return(ERR_BAD_ALLOC_IMBUF);
  }
  
  /* Create the palette XImage structure */
  palimage = XCreateImage(dpy,visual,depth,ZPixmap,0,palette,
  	       MAXPALWIDTH,1,8,0);

  /* Create the data XImage structure */
  dataimage = XCreateImage(dpy,visual,depth,ZPixmap,0,imbuf,128,1024,8,0);
  
  /* If requested, set parameters for the zoom window */
  if (zoomf > 0) {
    zwidth = zheight = DEFZOOMSIZE;
    lastzoomx = lastzoomy = -100000;
    zoomimage = XCreateImage(dpy,visual,depth,ZPixmap,0,NULL,1,1,8,0);
  }

  /* Get the font to be used, and get the information about it */
  fontname1 = XGetDefault(dpy,resourcename,"FontName");
  fontname2 = DEFAULT_FONT2;
  if (fontname1 == NULL) fontname1 = DEFAULT_FONT1;
  if (  ((fontinfo = XLoadQueryFont(dpy,fontname1)) != NULL)
      ||((fontinfo = XLoadQueryFont(dpy,fontname2)) != NULL) ) {
  } else {
    fprintf(stderr,"Can't open font %s\n",fontname1);
    fprintf(stderr,"Put an appropriate font in your .Xdefaults file with index \
  xvista.FontName\n");
    fprintf(stderr,"You will need to log out before this takes effect\n");
    return(ERR_CANT_GET_FONT);
  }
  fontwidth = fontinfo->max_bounds.rbearing - fontinfo->min_bounds.lbearing;
  fontheight = fontinfo->max_bounds.ascent + fontinfo->max_bounds.descent;

  /* Initialize keyaction array */
  for(i=0;i<128;i++) {
    keyaction[i].action = NULL;
    keyaction[i].echo = 0;
  }
  keyaction[1].action = keyzoomin;
  keyaction[2].action = keyzoomout;
  keyaction[3].action = keypan;
  keyaction['@'].action = keyrecenter;
  keyaction['r'].action = keyrecenter;
  keyaction['R'].action = keyrecenter;
  keyaction['$'].action = keyzoomprint;
  keyaction['?'].action = keyhelp;
  imageinstallkey('f',0,zfreeze);
  imageinstallkey('F',0,zfreeze);
  imageinstallkey('p',0,zpeak);
  imageinstallkey('P',0,zpeak);
  imageinstallkey('v',0,zpeak);
  imageinstallkey('V',0,zpeak);
  imageinstallkey('s',0,zsex);
  imageinstallkey('S',0,zsex);
  imageinstallkey('d',0,zsex);
  imageinstallkey('D',0,zsex);
#ifdef __HAIRS
  imageinstallkey('h',0,zhairs);
  imageinstallkey('H',0,zhairs);
#endif
  imageinstallkey('-',0,lastim);
  imageinstallkey('+',0,nextim);
  imageinstallkey('=',0,nextim);
  imageinstallkey(']',0,vecclear);

  
  /* Open a pipe from parent to itself to use for blocking reads from handler */
  if (pipe(pfd) == -1)
    fprintf(stderr,"ximage: Can't open a pipe for handler\n");
  
  to_program = pfd[1];
  from_display = pfd[0]; 
  
  /* Get options for autoresize of windows and/or autozoomout of large images */

  option = XGetDefault(dpy,resourcename,"resize");
  if (option != NULL) resize = atoi(option);
  option = XGetDefault(dpy,resourcename,"autozoomout");
  if (option != NULL) autozoomout = atoi(option);
  option = XGetDefault(dpy,resourcename,"zoomsample");
  if (option != NULL) zoomsample = atoi(option);

  /* Get options for default window size and maximum window size */
  option = XGetDefault(dpy,resourcename,"width");
  if (option != NULL) *winit = atoi(option);
  option = XGetDefault(dpy,resourcename,"height");
  if (option != NULL) *hinit = atoi(option);

  maxwidth=DEFMAXWIDTH;
  maxheight=DEFMAXHEIGHT;
  option = XGetDefault(dpy,resourcename,"maxwidth");
  if (option != NULL) maxwidth = atoi(option);
  option = XGetDefault(dpy,resourcename,"maxheight");
  if (option != NULL) maxheight = atoi(option);
 
  *winit=MIN(maxwidth,*winit);
  *hinit=MIN(maxheight,*hinit);

  /* Create the windows */
  newsizesubwin(*winit,*hinit);
  ierr = createwindows(resourcename,windowname,xoff,yoff);
  if (ierr != 0) return(ierr);
  *winit = width;
  *hinit = height; 
  updateimage(0,0,width,height,width,NULL,0,0);

  tvinit = 1;
  return(0);
}

/* Create the base window and all its little friends */

createwindows(resourcename,windowname,xoff,yoff)
char *resourcename;
char *windowname;
int xoff,yoff;
{
XColor  curswcolor, cursbcolor, backcolor;
XWMHints  wmhints;
XSizeHints sizehints;

int border_width=2, i;
Pixmap csource, cmask;
char *option;

/* Set background color*/
option = XGetDefault(dpy,resourcename,"backred");
if (option != NULL) {
  i = atoi(option);
  backcolor.red = i<<8;
} else
  backcolor.red = 0;
option = XGetDefault(dpy,resourcename,"backgreen");
if (option != NULL) {
  i = atoi(option);
  backcolor.green = i<<8;
} else
  backcolor.green = 0;
option = XGetDefault(dpy,resourcename,"backblue");
if (option != NULL) {
  i = atoi(option);
  backcolor.blue = i<<8;
} else
  backcolor.blue = 0;
XAllocColor(dpy,defcmap,&backcolor);
xswa.colormap = defcmap;
xswa.background_pixel = backcolor.pixel;
xswa.border_pixel = whitePixel(dpy,screen);
xswa.backing_store = WhenMapped;
/*xswa.override_redirect = True; */

sizehints.flags = PPosition | PSize;
sizehints.width = width;
sizehints.height = height+XYZHEIGHT; 

option = XGetDefault(dpy,resourcename,"x");
if (option != NULL) {
  xoff = atoi(option);
  if (xoff < 0) xoff = DisplayWidth(dpy,screen) + xoff - width - 2*border_width;
  sizehints.flags |= (USSize | USPosition);
  sizehints.x = xoff;
}
option = XGetDefault(dpy,resourcename,"y");
if (option != NULL) {
  yoff = atoi(option);
  if (yoff < 0) yoff = DisplayHeight(dpy,screen) + yoff - height - XYZHEIGHT - 2*border_width;
  sizehints.flags |= (USSize | USPosition);
  sizehints.y = yoff;
}

wbase = XCreateWindow(dpy,RootWindow(dpy,screen),
		xoff,yoff,width,height+XYZHEIGHT,border_width,
		depth,InputOutput,visual,
		CWBackPixel | CWBorderPixel | CWColormap, &xswa);
if (!wbase) {
  fprintf(stderr, "XCreateWindow failed\n");
  return(ERR_CANT_CREATE_IMAGE_WINDOW);
}
XSetWindowColormap(dpy,wbase,defcmap);
/* See XSetStandardProperties for defining a fancy icon */
XSetIconName(dpy, wbase, "Ximage");
/* Map the window and see what size it is (if the user has resized it) */
XSetStandardProperties(dpy, wbase, windowname, windowname,
   None, NULL, 0, &sizehints);
wmhints.input = True;
wmhints.flags = InputHint;
XSetWMHints(dpy, wbase, &wmhints);
XMapWindow(dpy,wbase);

/*
XGetGeometry(dpy,wbase,&inforoot,&infox,&infoy,&infowidth,&infoheight,
       &infoborder,&infodepth);
*/
/* Set width and height to the size of the window */
/*
width = infowidth;
height = infoheight - XYZHEIGHT;
*/

/* Create the subwindows */

/* Create the image subwindow */
wimage = XCreateSimpleWindow(dpy,wbase,       /* Parent window */
       0,0,                             /* UL location (nominal) */
       width,height,                    /* size (nominal) */
       0,                               /* border width */
       whitePixel(dpy,screen),          /* border pixmap */
       backcolor.pixel);         /* background pixmap */
XChangeWindowAttributes(dpy,wimage,CWBackingStore|CWColormap,&xswa);

/* Create the palette subwindow */
wpal = XCreateSimpleWindow(dpy,wbase,         /* Parent window */
	       palx,paly,               /* UL location */
	       palwidth,palheight,      /* size */
	       XYZBORDER,               /* border width */
       whitePixel(dpy,screen),          /* border pixmap */
       blackPixel(dpy,screen));         /* background pixmap */
XChangeWindowAttributes(dpy,wpal,CWBackingStore|CWColormap,&xswa);

/* Define the new image cursor */
curswcolor.pixel = whitePixel(dpy,screen);
cursbcolor.pixel = blackPixel(dpy,screen);
XQueryColor(dpy,defcmap,&curswcolor);
XQueryColor(dpy,defcmap,&cursbcolor);

/* If we want full screen crosshairs, define a blank cursor here */
#ifdef __HAIRS
/*XDefineCursor(dpy,wimage,XCreateGlyphCursor(dpy, fontinfo->fid, fontinfo->fid,
  (unsigned int)' ', (unsigned int)' ',&curswcolor, &cursbcolor));
  #else */
csource = XCreateBitmapFromData(dpy,wimage,curs_bits,curs_width,curs_height);
cmask = XCreateBitmapFromData(dpy,wimage,curs_mask_bits,curs_width,curs_height);
curs = XCreatePixmapCursor(dpy,csource,cmask,
       &curswcolor,&cursbcolor,curs_x_hot,curs_y_hot);
XFreePixmap(dpy,csource);
XFreePixmap(dpy,cmask);
XDefineCursor(dpy,wimage,curs);
#endif

/* Define the palette cursor */
csource = XCreateBitmapFromData(dpy,wimage,palcurs_bits,
			  palcurs_width,palcurs_height);
cmask = XCreateBitmapFromData(dpy,wimage,palcurs_mask_bits,
			palcurs_width,palcurs_height);
palcurs = XCreatePixmapCursor(dpy,csource,cmask,&curswcolor,&cursbcolor,
/*               whitePixel(dpy,screen),blackPixel(dpy,screen), */
       palcurs_x_hot,palcurs_y_hot);
XDefineCursor(dpy,wpal,palcurs);

XFreePixmap(dpy,csource);
XFreePixmap(dpy,cmask);

/* xyz subwindow */
wxyz = XCreateSimpleWindow(dpy,wbase,xyzx,xyzy,xyzwidth,xyzheight,XYZBORDER,
       whitePixel(dpy,screen),blackPixel(dpy,screen));
XChangeWindowAttributes(dpy,wxyz,CWBackingStore|CWColormap,&xswa);
sprintf(numstr,"BUF");

/* light subwindows */
wlgt1 = XCreateSimpleWindow(dpy,wbase,lgtx,lgty[0],lgtwidth,lgtheight,XYZBORDER,
       whitePixel(dpy,screen),blackPixel(dpy,screen));
wlgt2 = XCreateSimpleWindow(dpy,wbase,lgtx,lgty[2],lgtwidth,lgtheight,XYZBORDER,
       whitePixel(dpy,screen),blackPixel(dpy,screen));
wlgt3 = XCreateSimpleWindow(dpy,wbase,lgtx,lgty[1],lgtwidth,lgtheight,XYZBORDER,
       whitePixel(dpy,screen),blackPixel(dpy,screen));
wlgt4 = XCreateSimpleWindow(dpy,wbase,lgtx,lgty[3],lgtwidth,lgtheight,XYZBORDER,
       whitePixel(dpy,screen),blackPixel(dpy,screen));
XChangeWindowAttributes(dpy,wlgt1,CWBackingStore|CWColormap,&xswa);
XChangeWindowAttributes(dpy,wlgt2,CWBackingStore|CWColormap,&xswa);
XChangeWindowAttributes(dpy,wlgt3,CWBackingStore|CWColormap,&xswa);
XChangeWindowAttributes(dpy,wlgt4,CWBackingStore|CWColormap,&xswa);
initmessage();

/* Create a zoom window if required              */
if (zoomf > 0) {
  sizehints.flags = PPosition | PSize;
  sizehints.width = zwidth*zoomf;
  sizehints.height = zheight*zoomf; 

  xoff = yoff = 3;
  option = XGetDefault(dpy,resourcename,"zoomx");
  if (option != NULL) {
    xoff = atoi(option);
    if (xoff < 0) xoff = DisplayWidth(dpy,screen) + xoff - zwidth - 2*border_width;
    sizehints.flags |= (USSize | USPosition);
    sizehints.x = xoff;
  }
  option = XGetDefault(dpy,resourcename,"zoomy");
  if (option != NULL) {
    yoff = atoi(option);
    if (yoff < 0) yoff = DisplayHeight(dpy,screen) + yoff - zheight - XYZHEIGHT - 2*border_width;
    sizehints.flags |= (USSize | USPosition);
    sizehints.y = yoff;
  }

  wzoom = XCreateWindow(dpy,RootWindow(dpy,screen),
		xoff,yoff,zwidth*zoomf,zheight*zoomf,1,
		depth,InputOutput,visual,
		CWBackPixel | CWBorderPixel | CWColormap, &xswa);
  XSetIconName(dpy, wzoom, "Xzoom");
  if (!wzoom) {
    fprintf(stderr, "XCreate failed to make zoom window\n");
    return(ERR_CANT_CREATE_ZOOM_WINDOW);
  }
  XSelectInput(dpy, wzoom, ButtonPressMask|ExposureMask|StructureNotifyMask);
  XSetStandardProperties(dpy, wzoom, "xvistaZoom", "xvista",
    None, NULL, 0, &sizehints);
  XMapWindow(dpy, wzoom);
}

/* Select inputs from the window */
/* Create the graphics context for images */
imagegc = XCreateGC(dpy, wbase, 0, NULL);
XSetState(dpy,imagegc,
    whitePixel(dpy,screen),blackPixel(dpy,screen),GXcopy,AllPlanes);

/* Create the graphics context for line drawing */
vectorgc = XCreateGC(dpy, wbase, 0, NULL);
XSetState(dpy,vectorgc,
    whitePixel(dpy,screen),blackPixel(dpy,screen),GXcopy,AllPlanes);
XSetForeground(dpy, vectorgc, whitePixel(dpy,screen));

textgc = XCreateGC(dpy, wbase, 0, NULL);
XSetState(dpy,textgc,
    whitePixel(dpy,screen),blackPixel(dpy,screen),GXcopy,AllPlanes);
XSetFont(dpy,textgc,fontinfo->fid);
XSetFillStyle(dpy,textgc,FillSolid);


XSelectInput(dpy, wbase, ExposureMask | LeaveWindowMask |
       ButtonPressMask | ButtonReleaseMask | StructureNotifyMask |
       PointerMotionMask | PointerMotionHintMask | KeyPressMask | 
       ColormapChangeMask);
XSelectInput(dpy,wimage, ExposureMask);

/* Map the subwindows */
XMapSubwindows(dpy,wbase);

/* Flush this pile of X output */
XFlush(dpy);

/* Ask for non-blocking IO on the input from X */
fd_for_X = ConnectionNumber(dpy);
/*UTUV(mx11register)(&fd_for_X,xtv_refresh);*/

return(0);
}

imageclose()
{
  return(0);
}

imageerase()
{

#ifdef __HAIRS
  if (usehairs && hairs_on) vnohair(); 
#endif
  XClearWindow(dpy,wimage);
  XFlush(dpy);
}

int zfreezeon = 0;
extern int imtvnum_;

imageupdate(x0,y0,nx,ny,map)
int x0, y0;     /* origin of area whose image to update (data coords) */
int nx, ny;     /* size of area whose image to update */
int map;
{
  int yim;
  if (data == NULL) return(ERR_NO_DATA);

  yim = y0 - daty;
  if (yup == 1) yim = imh-1 - (y0-daty);

  if (map==1) {
/*
  mapimage(x0,y0,nx,ny,datw,data,x0-datx,yim,imwidth,yup,
           image[store],lookup,branch); 
*/
    mapimage2(x0,y0,nx,ny,datw,data,x0-datx,yim,imwidth,yup,
           image[store],breakpts,ncolors,pixels,dataimage->bits_per_pixel,0L);
  } 
  if (zim > 0) {
    yim = winy + zim*(y0-daty-imy);
    if (yup == 1) yim = winy + zim*(imh-1 - (y0+ny-1) - imy + daty);

    writepix(zim*(x0-datx-imx)+winx, yim, zim*nx, zim*ny);
  } else {
    yim = winy + (y0-daty-imy)/(-zim);
    if (yup == 1) yim = winy + (imh-1 - (y0+ny-1) - imy + daty)/(-zim);

    writepix((x0-datx-imx)/(-zim)+winx, yim, nx/(-zim), ny/(-zim));
  }
  if (zoomf > 0 && !zfreezeon) updatezoom(lastzoomx,lastzoomy);
/*  xtv_refresh(0); */

  return(0);
}

tvload(a,nrow,ncol,nc,sr,sc,asr,asc,span,zero,flip,erase,color)
float   *a,             /* Input floating image array                   */
        span,          /* Intensity mapping span level.  This level    */
                        /* is used to scale the pixel data between 0    */
                        /* and 254 for display on the Grinnell.         */
        zero;          /* Intensity scale zero offest                  */
int     nrow,          /* Number of rows to be displayed               */
        ncol,          /* Number of columns to be displayed            */
        nc,            /* Row length in pixels of the array            */
        sr,            /* Row offset to first displayed row            */
        sc,            /* Column offset to first displayed row         */
        asr,           /* Row number of image array origin             */
        asc,           /* Column number of image array origin          */
        flip,          /* Flag for left-right image reflection         */
        erase,         /* Flag to erase screen                         */
        color;
{
  float c;
  int i;

  if (flip == 1)
    yup = 1;
  else
    yup = 0;

  imagemap(zero,span,ncolors);
  imagevnull();
  if (erase == 1) imageerase();
  imagedisplay(sc,sr,ncol,nrow,nc,asc,asr,a,color);

  return (0);
}


imagedisplay(x0,y0,nx,ny,awidth,xoff,yoff,a,color)
float *a;               /* Input floating image array                   */
int x0, y0, nx, ny;     /* Origin and size of area to be displayed from a */
int awidth;             /* Number of pixels per row in a */
int xoff, yoff;         /* x and y coordinates to be used for pixel (0,0) */
int color;
{
 int i, yim, maxdim, mindim, maxw, maxh;
 unsigned long cmask;

/* First test to see whether any windows have been created */
 if(wbase == 0) {
   fprintf(stderr,"Windows not yet created\n");
   return(ERR_NOT_INITIALIZED);
 }

/* Check to see that the image array is large enough for the data */
 if (color==0 || !truecolor) {
   store = store+1>MAXSTORE-1 ? 0 : store+1;
   if (nimage[store] < ROUNDUP(nx)*ny || image[store] == NULL) {
     if(image[store] != NULL) free(image[store]);
     if((image[store] = 
      (unsigned char *)malloc((dataimage->bits_per_pixel/8)*ROUNDUP(nx)*ny)) == NULL) {
       fprintf(stderr,"Can't allocate image array\n");
       return(ERR_BAD_ALLOC_IMBUF);
     }
     nimage[store] = ROUNDUP(nx)*ny;
   }
   cmask = 0;
 } else {
   if (imw != nx || dath != ny) {
     fprintf(stderr,"image size does not match for single color change! %d %d %d %d",nimage[store],nx,ny,store);
     return(-1);
   }
   if (color==1)
     cmask=rmask;
   else if (color==2)
     cmask=gmask;
   else if (color==3)
     cmask=bmask;
 }

/* Update the display variables */
 updateimage(x0,y0,nx,ny,awidth,a,xoff,yoff);
 yim = 0;
 if(yup == 1) yim = imh - 1;
/*
  mapimage(datx,daty,imw,imh,datw,data,0,yim,imwidth,yup,image[store],lookup,branch); 
*/
 mapimage2(datx,daty,imw,imh,datw,data,0,yim,imwidth,yup,image[store],breakpts,ncolors,pixels,dataimage->bits_per_pixel,cmask);

 imagevalid = 1;
 imagevnull();

/* If the image can be zoomed and fit it the display window, do it */
 maxdim = MAX(nx,ny);
 while (maxdim*2 <= MIN(width,height)) {
   updatepan(imw/2,imh/2,1);
   maxdim = maxdim*2;
 }

/* If image is too big for display, zoom out if option is set */
 if (autozoomout && (nx>width || ny>height)  ) {
    if (resize) {
      maxw = maxwidth;
      maxh = maxheight;
    } else {
      maxw = width;
      maxh = height;
    }
    while ( (nx/ABS(zim))>maxw || (ny/ABS(zim))>maxh )  {
      updatepan(imw/2,imh/2,-1);
    }
  }

/* Update the size of the display if it is too small */
 if (resize && (width < nx/ABS(zim) || height < ny)) {
   width = MIN(maxwidth,MAX(width,nx));
   height = MIN(maxheight,MAX(height,ny));
   updatepan(imw/2,imh/2,0);
   updatesize(width,height);
   if (zim > 0) {
     writepix(winx, winy, zim*imw, zim*imh);
   } else {
     writepix(winx, winy, imw/(-zim), imh/(-zim));
   }
 }

/* Otherwise damage the image window so as to get it displayed */

 else {
  if (zim > 0) {
    writepix(winx, winy, zim*imw, zim*imh);
  } else {
    writepix(winx, winy, imw/(-zim), imh/(-zim));
  }
  /* xtv_refresh(0); */
 }
 XFlush(dpy);
 return (0);
}

/* Routine to set up the data -> image mapping */
imagemap(zero,span,ncolors)
float zero,span;        /* values of the break points */
int ncolors;
{
  float c;
  int i;

  palbrkpt = breakpts;
/*  brancher(ncolors,pixels,breakpts,lookup,branch); */
  c = ABS(span) / (ncolors-2);
  for(i=0;i<ncolors-1;i++) {
    breakpts[i] = zero + c * i;
  }
  return(0);
}

/* Routine to load the X Windows color lookup table */

imagepalette(n,r,g,b,flag)
int n;                  /* Number of table entries      */
short *r, *g, *b;       /* Pointer to red, green and blue arrays */
int flag;               /* 0/1 for image/vector color load */
{
  int i, k;
  Colormap *list;
  int num;

  if (flag == 0) {
    for (i=0;i<npalette;i++) {
      k = (i*n) / npalette;
      palcolors[i].red = 256*( r[k] & 0x00ff);
      palcolors[i].green = 256*( g[k] & 0x00ff);
      palcolors[i].blue = 256*( b[k] & 0x00ff);
    }
    newcolors(0,npalette-1);
  } else {
    vcolor[0].red = 256*( r[0] & 0x00ff);
    vcolor[0].green = 256*( g[0] & 0x00ff);
    vcolor[0].blue = 256*( b[0] & 0x00ff);
    XAllocColor(dpy, defcmap, vcolor);
  }
  return (0);
}

imageinstallkey(key,echo,function)
int key;                        /* key to cause the action */
int echo;                       /* 0/1 to echo key in x,y,z display */
int (*function)();              /* action to be taken */
			/* function(x,y,xuser,yuser,key) */
{
  keyaction[key].action = function;
  keyaction[key].echo = echo;
}

imageuninstallkey(key)
int key;                        /* key to cause the action */
{ 
  keyaction[key].action = NULL;
  keyaction[key].echo = 0;
}   

ATUS(imagetext)(xuser,yuser,text,textlen,fill,nt)
int *xuser, *yuser, *textlen, nt, *fill;
char *text;
{
  int zad, x, y;

  if(zim > 0) {
    zad = (zim - 1) / 2;
    x = zim * (*xuser - imx - datx - offx) + winx + zad;
    if(yup == 0)
      y = winy + zim * (*yuser - imy - daty - offy) + zad;
    else
      y = winy + zim * (imh-1 - (*yuser - offy - daty) - imy) + zad;
  } else {
    x = (*xuser - imx - datx - offx)/(-zim) + winx;
    if(yup == 0)
      y = winy + (*yuser - imy - daty - offy)/(-zim);
    else
      y = winy + (imh-1 - (*yuser - offy - daty) - imy)/(-zim);
  }
  xtlast = x;
  ytlast = y;
  textlist[tcount].xt = *xuser;
  textlist[tcount].yt = *yuser;
  text[*textlen] = 0;
  sprintf(textlist[tcount].text,"%s",text);
  textlist[tcount].color = -1;
  tcount = MIN(tcount+1,MAXTEXT-1);
  if (*fill==0) {
    XSetBackground(dpy,textgc,whitePixel(dpy,screen));
    XSetForeground(dpy,textgc,blackPixel(dpy,screen));
  } else {
    XSetBackground(dpy,textgc,blackPixel(dpy,screen));
    XSetForeground(dpy,textgc,whitePixel(dpy,screen));
  }
  XDrawImageString(dpy,wimage,textgc,x,y,text,*textlen);
  XSetBackground(dpy,textgc,blackPixel(dpy,screen));
  XSetForeground(dpy,textgc,whitePixel(dpy,screen));
  XFlush(dpy);
}

imagebox(x,y,nx,ny,color)
int x,y,nx,ny,color;
{
  imagerelocate(x,y);
  imagedraw(x,y+ny,color);
  imagedraw(x+nx,y+ny,color);
  imagedraw(x+nx,y,color);
  imagedraw(x,y,color);
}

imagecross(x,y)
int x, y;
{
  imagerelocate(x-3,y);
  imagedraw(x+3,y,0);
  imagerelocate(x,y-3);
  imagedraw(x,y+3,0);
  return(0);
}

imagerelocate(xuser,yuser)
int xuser, yuser;               /* x,y coordinates in user's system */
{
  int x, y, zad;

  if(zim > 0) {
    zad = (zim - 1) / 2;
    x = zim * (xuser - imx - datx - offx) + winx + zad;
    if(yup == 0)
      y = winy + zim * (yuser - imy - daty - offy) + zad;
    else
      y = winy + zim * (imh-1 - (yuser - offy - daty) - imy) + zad;
  } else {
    x = (xuser - imx - datx - offx)/(-zim) + winx;
    if(yup == 0)
      y = winy + (yuser - imy - daty - offy)/(-zim);
    else
      y = winy + (imh-1 - (yuser - offy - daty) - imy)/(-zim);
  }

  xvlast = x;
  yvlast = y;
  veclist[vcount].xv = xuser;
  veclist[vcount].yv = yuser;
  veclist[vcount].color = -1;
  vcount = MIN(vcount+1,MAXVEC-1);
  return (0);
}

imagedraw(xuser,yuser,color)
int xuser, yuser;               /* x,y coordinates in user's system */
int color;
{
  int x, y, zad;
  vlastcolor=-1;
  if(zim > 0) {
    zad = (zim - 1) / 2;
    x = zim * (xuser - imx - datx - offx) + winx + zad;
    if(yup == 0)
      y = winy + zim * (yuser - imy - daty - offy) + zad;
    else
      y = winy + zim * (imh-1 - (yuser - offy - daty) - imy) + zad;
  } else {
    x = (xuser - imx - datx - offx)/(-zim) + winx;
    if(yup == 0)
      y = winy + (yuser - imy - daty - offy)/(-zim);
    else
      y = winy + (imh-1 - (yuser - offy - daty) - imy)/(-zim);
  }
  if (color>nvcolors-1) color=0;
  if (color != vlastcolor) {
    XSetForeground(dpy, vectorgc, vcolor[color].pixel);
  if (_XErrorEvent.serial!=0) 
   printf("loc 1: %d %s", _XErrorEvent.serial,_XErrorEvent.error_code);
    vlastcolor = color;
  }
  XDrawLine(dpy,wimage,vectorgc,xvlast,yvlast,x,y);

  xvlast = x;
  yvlast = y;
  veclist[vcount].xv = xuser;
  veclist[vcount].yv = yuser;
  veclist[vcount].color = color;
  vcount = MIN(vcount+1,MAXVEC-1);
  XFlush(dpy);
  return (0);
}

imagedrawflush(xuser,yuser,color)
int xuser, yuser;               /* x,y coordinates in user's system */
int color;
{
  imagedraw(xuser,yuser,color);
  XFlush(dpy);
}

vecclear()
{
  int yim;

  if (image[store]!=NULL) {
    imagevnull();

    if (data == NULL)
      lights(-2);
    else
      lights(2);

    if (zim>0) {
      yim = winy + zim*(daty-daty-imy);
      if (yup == 1) yim = winy + zim*(imh-1 - (daty+imh-1) - imy + daty);
      writepix(zim*(datx-datx-imx)+winx, yim, zim*imw, zim*imh);
    } else {
      yim = winy + (daty-daty-imy)/-zim;
      if (yup == 1) yim = winy + (imh-1 - (daty+imh-1) - imy + daty)/-zim;
      writepix((datx-datx-imx)/-zim+winx, yim, imw/-zim, imh/-zim);
    }
    if(zoomf > 0 && !zfreezeon) updatezoom(lastzoomx,lastzoomy);
    xtv_refresh(0);
   }
}

storeclear()
{
  int i;
  for(i=0;i<MAXSTORE;i++)
   if (i!=store) image[i]=NULL;
}

imagevnull()
{
  vcount = 0;
  tcount = 0;
  return(0);
}

imagetreplay()
{
  struct imagetext *textp;
  int i,zad, x, y, textlen;

  textp = textlist;
  for (i = 0; i < tcount; i++) {
   if(zim > 0) {
    zad = (zim - 1) / 2;
    x = zim * (textp->xt - imx - datx - offx) + winx + zad;
    if(yup == 0)
      y = winy + zim * (textp->yt - imy - daty - offy) + zad;
    else
      y = winy + zim * (imh-1 - (textp->yt - offy - daty) - imy) + zad;
   } else {
    x = (textp->xt - imx - datx - offx)/(-zim) + winx;
    if(yup == 0)
      y = winy + (textp->yt - imy - daty - offy)/(-zim);
    else
      y = winy + (imh-1 - (textp->yt - offy - daty) - imy)/(-zim);
   }
   textlen = strlen(textp->text);
   XSetForeground(dpy,textgc,whitePixel(dpy,screen));
   XDrawImageString(dpy,wimage,textgc,x,y,textp->text,textlen);
   XFlush(dpy);
   xtlast = x;
   ytlast = y;
   textp++;
  }
}

imagevreplay()
{
  struct imagevector *vecp;
  int i, x, y, zad;

  vecp = veclist;
  zad = (zim - 1) / 2;
  vlastcolor=-1;
  for (i = 0; i < vcount; i++) {
    if(zim > 0) {
      x = zim * (vecp->xv - imx - datx - offx) + winx + zad;
      if(yup == 0)
        y = winy + zim * (vecp->yv - imy - daty - offy) + zad;
      else
        y = winy + zim * (imh-1 - (vecp->yv - offy - daty) - imy) + zad;
    } else {
      x = (vecp->xv - imx - datx - offx)/(-zim) + winx;
      if(yup == 0)
        y = winy + (vecp->yv - imy - daty - offy)/(-zim);
      else
        y = winy + (imh-1 - (vecp->yv - offy - daty) - imy)/(-zim);
    }
    if (vecp->color >= 0) {
      if(vecp->color != vlastcolor) {
        XSetForeground(dpy, vectorgc, vcolor[vecp->color].pixel);
  if (_XErrorEvent.serial!=0) 
   printf("loc 2: %d %s", _XErrorEvent.serial,_XErrorEvent.error_code);
      vlastcolor = vecp->color;
      }
      XDrawLine(dpy,wimage,vectorgc,xvlast,yvlast,x,y);
    }

    xvlast = x;
    yvlast = y;
    vecp++;
  }
  return (0);
}

int lgtstatus[4];
int lgtenable=0;
char *uppermess1=NULL, *lowermess1=NULL;
char *uppermess2=NULL, *lowermess2=NULL;

char lgtmessage[4][2][8];

setmessage(ilight,istate,mess)
int ilight, istate;
char *mess;
{
  sprintf(lgtmessage[ilight-1][istate],"%s",mess);
}

initmessage()
{
  int i;
  for (i=0;i<4;i++) lgtstatus[i] = i+1;
  setmessage(1,0,"ASYNC");
  setmessage(1,1,"INPUT");
  setmessage(2,0,"NO DATA");
  setmessage(2,1,"DATA");
  setmessage(3,0,"FREEZE");
  setmessage(3,1,"UPDATE");
  setmessage(4,0,"ZOOM");
  setmessage(4,1,"NORM");
}

lights(state)
int state;
{
  char string[10];
  int j, i;
  if (state!=0) {
    j = (state>0 ? state : -state);
    i = state>0 ? 1 : 0;
    imagelight(j,lgtmessage[j-1][i],i);
  }
/*
  if (state == -1 ) imagelight(1,"ASYNC",0);
  if (state == 1 ) imagelight(1,"INPUT",1);
  if (state == -2 ) imagelight(2,"NO DATA",0);
  if (state == 2 ) imagelight(2,"READY",1);
  if (state == -3 ) imagelight(3,"FREEZE",0);
  if (state == 3 ) imagelight(3,"UPDATE",1);
  if (state == -4 ) {
    if (zim==1)
      imagelight(4,"NORM",1);
    else if (zim>0) {
      sprintf(string,"ZOOM*%d",zim);
      imagelight(4,string,0);
    } else {
      sprintf(string,"ZOOM/%d",-zim);
      imagelight(4,string,0);
    }
  }
  if (state == 4 ) imagelight(4,"NORM",1);
*/

  if (state == 0) {
    for (j=0;j<4;j++) lights(lgtstatus[j]);
  } else {
    i = state > 0 ? state : -1*state;
    lgtstatus[i-1] = state;
  }
}

imagelight(upper,mess,color)
int upper;              
/* flag = -1 to redraw messages */
/* flag = 1 for upper right status light */
/* flag = 2 for lower right status light */
/* flag = 3 for upper left status light */
/* flag = 4 for lower left status light */
char *mess;                     /* String to be written (null terminated) */
int color;                      /* flag = 0/1 for red/green */
{
  Window *w;
  XColor testcolor;
  int x, y, up, in,i;
  static int oldcolor[3] = {0,0,0};

  if (lgtwidth<=10) return(0);

  y = fontheight + (lgtheight-fontheight)/2 - 1;

  if(upper == (-1)) {
    if(uppermess1 != NULL) {
      x = (lgtwidth-strlen(uppermess1)*fontwidth)/2;
      XSetForeground(dpy,textgc,stcolor[2*oldcolor[0]].pixel);
  if (_XErrorEvent.serial!=0) 
   printf("loc 3: %d %s", _XErrorEvent.serial,_XErrorEvent.error_code);
      XFillRectangle(dpy,wlgt1,textgc,0,0,lgtwidth,lgtheight);
      XSetForeground(dpy,textgc,stcolor[2*oldcolor[0]+1].pixel);
  if (_XErrorEvent.serial!=0) 
   printf("loc 4: %d %s", _XErrorEvent.serial,_XErrorEvent.error_code);
      XDrawString(dpy,wlgt1,textgc,x,y,uppermess1,strlen(uppermess1));
    }
    if(lowermess1 != NULL) {
      x = (lgtwidth-strlen(lowermess1)*fontwidth)/2;
      XSetForeground(dpy,textgc,stcolor[2*oldcolor[1]].pixel);
  if (_XErrorEvent.serial!=0) 
   printf("loc 5: %d %s", _XErrorEvent.serial,_XErrorEvent.error_code);
      XFillRectangle(dpy,wlgt2,textgc,0,0,lgtwidth,lgtheight);
      XSetForeground(dpy,textgc,stcolor[2*oldcolor[1]+1].pixel);
  if (_XErrorEvent.serial!=0) 
   printf("loc 6: %d %s", _XErrorEvent.serial,_XErrorEvent.error_code);
      XDrawString(dpy,wlgt2,textgc,x,y,lowermess1,strlen(lowermess1));
    }
    if(uppermess2 != NULL) {
      x = (lgtwidth-strlen(uppermess2)*fontwidth)/2;
      XSetForeground(dpy,textgc,stcolor[2*oldcolor[2]].pixel);
  if (_XErrorEvent.serial!=0) 
   printf("loc 7: %d %s", _XErrorEvent.serial,_XErrorEvent.error_code);
      XFillRectangle(dpy,wlgt3,textgc,0,0,lgtwidth,lgtheight);
      XSetForeground(dpy,textgc,stcolor[2*oldcolor[2]+1].pixel);
  if (_XErrorEvent.serial!=0) 
   printf("loc 8: %d %s", _XErrorEvent.serial,_XErrorEvent.error_code);
      XDrawString(dpy,wlgt3,textgc,x,y,uppermess2,strlen(uppermess2));
    }
    if(lowermess2 != NULL) {
      x = (lgtwidth-strlen(lowermess2)*fontwidth)/2;
      XSetForeground(dpy,textgc,stcolor[2*oldcolor[3]].pixel);
  if (_XErrorEvent.serial!=0) 
   printf("loc 9: %d %s", _XErrorEvent.serial,_XErrorEvent.error_code);
      XFillRectangle(dpy,wlgt4,textgc,0,0,lgtwidth,lgtheight);
      XSetForeground(dpy,textgc,stcolor[2*oldcolor[3]+1].pixel);
  if (_XErrorEvent.serial!=0) 
   printf("loc 10: %d %s", _XErrorEvent.serial,_XErrorEvent.error_code);
      XDrawString(dpy,wlgt4,textgc,x,y,lowermess2,strlen(lowermess2));
    }
  XFlush(dpy);
  return(0);
  }

  oldcolor[upper-1] = color;
  x = (lgtwidth-strlen(mess)*fontwidth)/2;

  if (upper == 1) {
    w = &wlgt1;
    uppermess1 = mess;
  } else if (upper==2) {
    w = &wlgt2;
    lowermess1 = mess;
  } else if (upper==3) {
    w = &wlgt3;
    uppermess2 = mess;
  } else if (upper==4) {
    w = &wlgt4;
    lowermess2 = mess;
  }

  XSetForeground(dpy,textgc,stcolor[2*color].pixel);
  if (_XErrorEvent.serial!=0) 
   printf("loc 11: %d %s", _XErrorEvent.serial,_XErrorEvent.error_code);
  XFillRectangle(dpy,*w,textgc,0,0,lgtwidth,lgtheight);
  
  XSetForeground(dpy,textgc,stcolor[2*color+1].pixel);
  if (_XErrorEvent.serial!=0) 
   printf("loc 12: %d %s", _XErrorEvent.serial,_XErrorEvent.error_code);
  XDrawString(dpy,*w,textgc,x,y,mess,strlen(mess));

  XFlush(dpy);
}

/*
* Blocking read from the display of one character
*/
imageread(x,y,key)
int *x, *y;             /* User coordinates where the key was struck */
char *key;              /* ASCII key entered */
{
#if defined(__alpha) || defined(__solaris)
  size_t nbytes = 1;
  ssize_t tmp;
#else
  int nbytes = 1;
  int tmp;
#endif

  lights(1);
  waiting_for_key = 1;            /* Set flag for interupt routine */

  xtv_refresh(0);
  tmp = read(from_display,key,nbytes);

  if (tmp > 0) {
    *x = lastx;
    *y = lasty;
    waiting_for_key = 0;
    lights(-1);
    return(0);
  } else {
    fprintf (stderr,"Program-display pipe failure...\n");
    waiting_for_key = 0;
    lights(-1);
    return(ERR_BAD_DISPLAY_PIPE);
  }
  lights(-1);
}

/*
* Update the image and window dimensions for a new image
*/
updateimage(x0,y0,nx,ny,awidth,a,x1,y1)
int x0, y0;                     /* location of image in data */
int nx, ny;                     /* size of image to be displayed */
int awidth;                     /* pixels/line in data */
float *a;                       /* new data array */
int x1, y1;                     /* offset of data coordinate system */
{
  data = sdata[store] = a;
  datw = sdatw[store] = awidth;
  dath = sdath[store] = ny;
  imw = simw[store] = nx;
  imwidth = simwidth[store] = ROUNDUP(imw);
  /*  fprintf(stderr,"imw = %d imwidth = %d \n",imw,imwidth); */
  imh = simh[store] = ny;
  zim = szim[store] = 1;
  offx = soffx[store] = x1;
  offy = soffy[store] = y1;
  datx = sdatx[store] = x0;
  daty = sdaty[store] = y0;
  winw = swinw[store] = MIN(imw,width);
  winh = swinh[store] = MIN(imh,height);
  imx = simx[store] = MAX(0,(imw-winw)/2);
  imy = simy[store] = MAX(0,(imh-winh)/2);
  winx = swinx[store] = MAX(0,(width-imw)/2);
  winy = swiny[store] = MAX(0,(height-imh)/2);
  imtv = simtv[store] = imtvnum_;
  sprintf(imname[store]," ");

}

releasetv_(loc)
float **loc;
{
  int i;

  if (tvinit == 0) return(0);
  if (data == *loc) {
    data = NULL;
    lights(-2);
  }

  for (i=0; i<MAXSTORE; i++) 
    if (sdata[i] == (float *)*loc) sdata[i] = NULL;
}

updatestore()
{
  data = sdata[store];
  datw = sdatw[store];
  dath = sdath[store];
  imw = simw[store];
  imwidth = simwidth[store];
  imh = simh[store];
  offx = soffx[store];
  offy = soffy[store];
  datx = sdatx[store];
  daty = sdaty[store];
  /* zim = szim[store];
  imx = simx[store];
  imy = simy[store];
  winx = swinx[store];
  winy = swiny[store]; */
  if (zim>0) {
    winw = MIN(width-winx,zim*(imw-imx));
    winh = MIN(height-winy,zim*(imh-imy));
  } else {
    winw = MIN(width-winx,(imw-imx)/(-zim));
    winh = MIN(height-winy,(imh-imy)/(-zim));
  }
  imtv = imtvnum_ = simtv[store];
}

/*
* Update the centering of the image, center it on image (x,y) and zoom it in
* for in = 1, out for in = -1
*/
updatepan(xc,yc,in)
int xc, yc, in;
{
//#define MAXZOOMFACTOR 32
#define MAXZOOMFACTOR 128
  if (in == 1) {
    if(zim >= 1) zim = MIN(MAXZOOMFACTOR,zim*2);
    else if(zim == -2) zim = 1;
    else zim = zim/2;
  } else if(in == -1) {
    if(zim > 1) zim = zim/2;
    else if(zim == 1) zim = -2;
    else zim = MAX(-MAXZOOMFACTOR,zim*2);
  } else if(in == 2) {
    zim = MAXZOOMFACTOR;
  }
  if (zim > 0) {
    imx = MAX(0,xc-width/(2*zim));
    imy = MAX(0,yc-height/(2*zim));
    winx = MAX(0,zim*(width/(2*zim)-xc));
    winy = MAX(0,zim*(height/(2*zim)-yc));
    winw = MIN(width-winx,zim*(imw-imx));
    winh = MIN(height-winy,zim*(imh-imy));
  } else {
    imx = MAX(0,xc-(-zim*width)/2);
    imy = MAX(0,yc-(-zim*height)/2);
    winx = MAX(0,((-zim*width)/2-xc)/(-zim));
    winy = MAX(0,((-zim*height)/2-yc)/(-zim));
    winw = MIN(width-winx,(imw-imx)/(-zim));
    winh = MIN(height-winy,(imh-imy)/(-zim));
  }
  if (!lgtenable) {
    if (abs(zim) ==1) lights(4);
    else lights(-4);
  }
}

/*
* Update all the windows to a new overall size
*/
updatesize(wid,hgt)
int wid, hgt;
{
/* Set width and height to the size of the window */

  wid = MAX(wid,100);
  hgt = MAX(hgt,100);

  XResizeWindow(dpy,wbase,wid,hgt+XYZHEIGHT);

  newsizesubwin(wid,hgt);
  resizesubwin();
}

newsizesubwin(wid,hgt)
int wid, hgt;
{
  int i;
  unsigned char *pc;
  unsigned short *ps;
  unsigned LONG *pl;

  /* size of image window */
  width = wid;
  height = hgt;

  /* xyz window width */
  xyzwidth = MIN(XYZWIDTH*fontwidth,width-30-3*XYZBORDER);
  xyzheight = MIN(height,XYZHEIGHT-2*XYZBORDER);

  /* light window width */
  lgtwidth = MIN((MAXMESS)*fontwidth,width);

  /* palette window width */
  palwidth = MAX(30,width-xyzwidth-3*XYZBORDER-lgtwidth);

  /* readjust xyzwidth to make things fit (if we had to use min palwidth=30)*/
  xyzwidth = width-lgtwidth-palwidth-3*XYZBORDER;

  if (palimage->bits_per_pixel == 8) {
      pc=(unsigned char *)palette;
      for(i=0;i<MIN(MAXPALWIDTH,palwidth);i++)
        *pc++ = cmap[(i*ncolors)/palwidth].pixel & 0xff;
  } else if (palimage->bits_per_pixel == 16) {
      ps=(unsigned short *)palette;
      for(i=0;i<MIN(MAXPALWIDTH,palwidth);i++)
        *ps++ = cmap[(i*ncolors)/palwidth].pixel & 0xffff;
  } else if (palimage->bits_per_pixel == 32) {
      pl=(unsigned LONG *)palette;
      for(i=0;i<MIN(MAXPALWIDTH,palwidth);i++)
        *pl++ = cmap[(i*ncolors)/palwidth].pixel;
  }

  palheight = xyzheight;
  lgtheight = xyzheight/4;
  xyzx = -1;
  xyzy = height+1;

  palx = xyzx + xyzwidth + XYZBORDER;
  paly = xyzy;

  lgtx = palx + palwidth + XYZBORDER;
  for (i=0; i<4; i++)
    lgty[i] = xyzy + i*XYZHEIGHT/4;

  textx = fontwidth;
  for (i=0; i<3; i++)
    texty[i] = (i+1)*fontheight + (i+1)*(xyzheight-3*fontheight)/4-1;

}

resizesubwin()
{
  XMoveResizeWindow(dpy,wimage,0,0,width,height);
  XMoveResizeWindow(dpy,wxyz,xyzx,xyzy,xyzwidth,xyzheight);
  XMoveResizeWindow(dpy,wlgt1,lgtx,lgty[0],lgtwidth,lgtheight);
  XMoveResizeWindow(dpy,wlgt2,lgtx,lgty[2],lgtwidth,lgtheight);
  XMoveResizeWindow(dpy,wlgt3,lgtx,lgty[1],lgtwidth,lgtheight);
  XMoveResizeWindow(dpy,wlgt4,lgtx,lgty[3],lgtwidth,lgtheight);
  XMoveResizeWindow(dpy,wpal,palx,paly,palwidth,palheight);
}

updatename(text,n)
char *text;
int n;
{
  strncpy(imname[store],text,19);
  imname[store][19]=0;
  updatecoords(-1,-1,-1);
}


tvimnum(str,n)
char *str;
int n;
{
  imtv = simtv[store] = n;
  strncpy(numstr,str,19);
  numstr[19]=0;
  updatecoords(-1,-1,-1);
}

updatecoords(wx,wy,key)
int wx, wy, key;
{
  float mousez;
  int x,y,imousez,i;
  float ra, dec;
  char rastring[14],decstring[14];
  int rah, ram, decd, decm;
  float ras, decs;

  /*  If cursor is out of window, just display buffer number and name */
  /*  First draw lower string with x, y, value, object name           */
  if (wx<winx || wx>=(winx+winw) || wy<winy || wy>= (winy+winh)) {
    xyzstring[0] = 0;
  } else {

    x = USERXCOORD(wx);
    y = USERYCOORD(wy);
    getwcs_(&imtv,&x,&y,&ra,&dec);
    if (ra!=0) {
     if (sexages) {
      ra /=15;
      rah = (int)ra;
      ram = (int)((ra-rah)*60);
      ras = (ra-rah-ram/60.)*3600;
      if (dec<0)
        decstring[0]='-';
      else
        decstring[0]='+';
      dec = (dec>0?dec:-dec);
      decd = (int)dec;
      decm = (int)((dec-decd)*60);
      decs = (dec-decd-decm/60.)*3600;
      sprintf(rastring,"%2.2d:%2.2d:%05.2f",rah,ram,ras);
      sprintf(decstring+1,"%2.2d:%2.2d:%04.1f",decd,decm,decs);
     } else {
      sprintf(rastring,"%12.6f",ra);
      sprintf(decstring,"%12.6f",dec);
     }
    } else {
      rastring[0] = 0;
      decstring[0] = 0;
    }

    if (data == NULL) {
      sprintf(xyzstring,"%4d %4d         NO DATA",x,y);
    } else { 
      mousez = *(data+(y-offy)*datw+(x-offx));
      if (ABS(mousez) < 1e6 && ABS(mousez) > 1e-2) {
        imousez = ABS(mousez) + 0.5;
        if (imousez >= 1000) {
          if(mousez < 0) imousez = -imousez;
          sprintf (xyzstring, "%4d %4d %8d %s %s",x,y,imousez,rastring,decstring);
        }
        else if (imousez >= 100)
          sprintf(xyzstring, "%4d %4d %8.1f %s %s",x,y,mousez,rastring,decstring);
        else if (imousez >= 10)
          sprintf(xyzstring, "%4d %4d %8.2f %s %s",x,y,mousez,rastring,decstring);
        else
          sprintf(xyzstring, "%4d %4d %8.3f %s %s",x,y,mousez,rastring,decstring);
      } else {
        sprintf(xyzstring, "%4d %4d %8.2e %s %s",x,y,mousez,rastring,decstring);
      }
    }

  }

  XSetForeground(dpy,textgc,whitePixel(dpy,screen));
  if (_XErrorEvent.serial!=0) 
   printf("loc 13: %d %s", _XErrorEvent.serial,_XErrorEvent.error_code);
  for (i=strlen(xyzstring);i<NXYZ;i++) xyzstring[i] = ' ';
  xyzstring[NXYZ] = 0;
  if (key >= 0 && key <= 127 && keyaction[key].echo == 1) xyzstring[20] = key;
  XDrawImageString(dpy,wxyz,textgc,textx,texty[2],xyzstring,NXYZ);

  /*  Now draw upper label with headings and buffer number     */
  if(wx<winx || wx>=(winx+winw) || wy<winy || wy>= (winy+winh)) {
    xyzstring[0] = 0;
  }
  else 
    if (data != NULL) {
      sprintf(xyzstring,"  X    Y     VALUE     RA         DEC");
    } else
      xyzstring[0] = 0;
  
  for (i=strlen(xyzstring);i<NXYZ;i++) xyzstring[i] = ' ';
  xyzstring[NXYZ] = 0;
  XDrawImageString(dpy,wxyz,textgc,textx,texty[1],xyzstring,strlen(xyzstring));

  /*  Now draw upper label with headings and buffer number     */
  if (data != NULL)  
      sprintf(xyzstring,"%s %d : %s",numstr,imtv,imname[store]);
  else
      xyzstring[0] = 0;
  
  for (i=strlen(xyzstring);i<NXYZ;i++) xyzstring[i] = ' ';
  xyzstring[NXYZ] = 0;
  XDrawImageString(dpy,wxyz,textgc,textx,texty[0],xyzstring,strlen(xyzstring));

  /* Show image scaling parameters */
  if (palwidth > 14*fontwidth) {
    brkwrite(xyzstring,breakpts[0],1);
    XDrawImageString(dpy,wpal,textgc,0,palheight-1,xyzstring,7);
  
    brkwrite(xyzstring,breakpts[ncolors-2],0);
    XDrawImageString(dpy,wpal,textgc,palwidth-7*fontwidth,palheight-1,xyzstring,7);
  }

  XFlush(dpy);
}

updatebrkpt(x,ibut)
int x, ibut;
{
  if (x<0 || x>=ncolors) return(0);
  if (x == 0) {
    strncpy(xyzstring," z<                  ",40);
    brkwrite(xyzstring+4,breakpts[0],1);
  } else if(x == ncolors-1) {
    strncpy(xyzstring," z>=                  ",40);
    brkwrite(xyzstring+5,breakpts[ncolors-2],1);
  } else {
    strncpy(xyzstring,"       <=z<         ",40);
    brkwrite(xyzstring,breakpts[x-1],0);
    brkwrite(xyzstring+11,breakpts[x],1);
  }
  
  XSetForeground(dpy,textgc,whitePixel(dpy,screen));
  if (_XErrorEvent.serial!=0) 
   printf("loc 15: %d %s", _XErrorEvent.serial,_XErrorEvent.error_code);
  XDrawImageString(dpy,wxyz,textgc,textx,texty[2],xyzstring,strlen(xyzstring));
  
  XFlush(dpy);
  /*
  fprintf(stderr,"Mouse at %d %d, image = %d\n",x,y,*(image+y*datw+x));
  */
}

brkwrite(s,b,left)
char *s;
float b;
{
  int ib, i, j;
  char buf[8];
  if(ABS(b) < 1e6 && ABS(b) > 1e-2) {
    ib = ABS(b) + 0.5;
    if(ib >= 1000) {
      if(b < 0) ib = -ib;
      sprintf(buf,"%7d",ib);
    }
    else if(ib >= 100)
      sprintf(buf,"%7.1f",b);
    else if(ib >= 10)
      sprintf(buf,"%7.2f",b);
    else
      sprintf(buf,"%7.3f",b);
  } else {
    //sprintf(buf,"%7.2e",b);
    sprintf(buf,"%7.1e",b);
  }
  if (left) {
    j=0;
    for (i=0;i<7;i++) {
      s[i] = ' ';
      if (buf[i] != ' ') s[j++] = buf[i];
    } 
  } else
    strncpy(s,buf,7);
}

resetcolors()
{
  newcolors(0,npalette-1);
}

newcolors(n1,n2)
int n1, n2;
{
  int i, k, i1, i2;
  unsigned char *pc;
  unsigned short *ps;
  unsigned LONG *pl;
  float zero, span, c;

/* attempt at rescaling image instead of changing colors
  if (truecolor) {
    zero = breakpts[n1];
    span = breakpts[n2]-breakpts[n1];
    imagemap(zero,span,ncolors);
    n1 = 0;
    n2 = npalette-1;
  }
*/

  n1 = (n1 + npalette) % npalette;
  n2 = (n2 + npalette) % npalette;
  i1 = (n1*ncolors)/npalette;
  i2 = (n2*ncolors)/npalette;
  if (i2 == i1) i2 = (i2+ncolors-1) % ncolors;
  if (i2 < i1) i2 += ncolors;

  for (i=MAX(0,i2+1-ncolors);i<MAX(ncolors,i2+1);i++) {
    if (i < i1) k = 0;
    else if (i > i2) k = npalette-1;
    else k = MAX(0,MIN(npalette-1,(npalette*(i-i1))/(i2-i1)));
    cmap[i%ncolors].red = palcolors[k].red;
    cmap[i%ncolors].green = palcolors[k].green;
    cmap[i%ncolors].blue = palcolors[k].blue;
  }

  pal0 = n1;
  pal1 = n2;

  if (truecolor) {
    for (i=0; i<ncolors; i++) {
      XAllocColor(dpy,defcmap,cmap+i);
      pixels[i] = cmap[i].pixel;
    }

    if (palimage->bits_per_pixel == 8) {
      pc=(unsigned char *)palette;
      for(i=0;i<MIN(MAXPALWIDTH,palwidth);i++)
        *pc++ = cmap[(i*ncolors)/palwidth].pixel & 0xff;
    } else if (palimage->bits_per_pixel == 16) {
      ps=(unsigned short *)palette;
      for(i=0;i<MIN(MAXPALWIDTH,palwidth);i++)
        *ps++ = cmap[(i*ncolors)/palwidth].pixel & 0xffff;
    } else if (palimage->bits_per_pixel == 32) {
      pl=(unsigned LONG *)palette;
      for(i=0;i<MIN(MAXPALWIDTH,palwidth);i++)
        *pl++ = cmap[(i*ncolors)/palwidth].pixel;
    }

  /* Redraw image */
    imageupdate(datx,daty,imw,imh,1);

  } else {
    XStoreColors(dpy,defcmap,cmap,ncolors);
  }

  /* Redraw color bar */
  for (i=0;i<palheight;i++)
    XPutImage(dpy,wpal,imagegc,palimage,0,0,0,i,palwidth,1);
  updatecoords(-1,-1,-1);
  XFlush(dpy);

}

static int whichbreak;

updatenewpal(x0,ibut)
int x0, ibut;
{

if (ibut == 2) 
  whichbreak = x0 - pal0;
else {
  if (ABS(x0-pal0) < ABS(x0-pal1)) whichbreak = 0;
else 
  whichbreak = 1;
}

/*  fprintf(stderr,"updatepal: x = %d ibut = %d whichbreak = %d pal0 = %d pal1 = %d\n",x0,ibut,whichbreak,pal0,pal1); */

}

updatepal(x,ibut)
int x, ibut;
{
  if (ibut == 2)
    newcolors(x-whichbreak,x-whichbreak+pal1-pal0);
  else {
    if (whichbreak==0) {
      newcolors(x,pal1);
    } else {
      newcolors(pal0,x);
    }
  }
/*  fprintf(stderr,"updatepal: x = %d ibut = %d whichbreak = %d pal0 = %d pal1 = %d\n",x,ibut,whichbreak,pal0,pal1); */
}

static int zoombytes=0;

updatezoom(xw,yw)
int xw, yw;
{
  int x, y, i, j, f, n, lc, m;
  int x0, y0, x1, y1, w, h;
  if (xw<winx || xw>=(winx+winw) || yw<winy || yw>= (winy+winh)) return(0);
  if (zim > 0) {
    x = (xw-winx)/zim + imx;
    y = (yw-winy)/zim + imy;
  } else {
    x = (-zim)*(xw-winx) + imx;
    y = (-zim)*(yw-winy) + imy;
  }

  f = zoomf;
  n = zoomf*zwidth;
  m = zoomf*zheight;

  if(zoombytes != n*m) {
    if (zoombytes > 0) free(zimage);
    zoombytes = n*m;
    if ((zimage = 
     (unsigned char *)malloc((dataimage->bits_per_pixel/8)*zoombytes))==NULL) {
	 fprintf(stderr,"Can't allocate zoom array %d %d\n",dataimage->bits_per_pixel/8,zoombytes);
	 return(ERR_BAD_ALLOC_ZOOM);
       }
    lastzoomx = lastzoomy = -100000;
    zoomimage->data = (char *)zimage;
  }

  replicate(x-zwidth/2,y-zheight/2,imwidth,imh,image[store],
	    f, 0,0,n,m,n,zimage,
	    1, dataimage->bits_per_pixel);
/* j is the location of the top left of the central pixel */
  j = n*f*(zheight/2) + f*(zwidth/2);
  zcursor(j);

  zoomimage->width = n;
  zoomimage->height = f*zheight;
  zoomimage->bytes_per_line = n*(dataimage->bits_per_pixel/8);

  XPutImage(dpy,wzoom,imagegc,zoomimage,0,0,0,0,n,f*zheight);

  lastzoomx = xw;
  lastzoomy = yw;
}

oldzcursor(j) 
/* j is the location of the top left of the central pixel */
int j;
{
  int i, f, n, a1, a2, a3, a4;
  f = zoomf;
  n = zoomf*zwidth;
  a1 = j - n - 1;
  for(i=0;i<4*(f+1);i++) {
    zimage[a1] = ((i%2) == 0) ?
      blackPixel(dpy,screen) : whitePixel(dpy,screen);
    if(i<f+1) a1++;
    else if(i<2*f+2) a1 += n;
    else if(i<3*f+3) a1--;
    else a1 -= n;
  }
  a1 = j - n + f;
  a2 = a1 - f - 1;
  a3 = a2 + (f+1)*n;
  a4 = a3 + f + 1;
  for(i=0;i<8;i++) {
    zimage[a1] = zimage[a2] = zimage[a3] = zimage[a4] = ((i%2) == 0) ?
      blackPixel(dpy,screen) : whitePixel(dpy,screen);
    a1 -= (n-1);
    a2 -= (n+1);
    a3 += (n-1);
    a4 += (n+1);
  }
}

zcursor(j)
/* j is the location of the top left of the central pixel */
int j;
{
  int i, f, n, a1, a2, a3, a4, nbytes;
  unsigned char *zc;
  unsigned short *zs;
  unsigned LONG *zl;
  unsigned LONG zpixel;

  zc = (unsigned char *)zimage;
  zs = (unsigned short *)zimage; 
  zl = (unsigned LONG *)zimage;
  nbytes = zoomimage->bits_per_pixel / 8;

  f = zoomf;
  n = zoomf*zwidth;
  a1 = j - n - 1;
  for(i=0;i<4*(f+1);i++) {
    zpixel = ((i%2) == 0) ? blackPixel(dpy,screen) : whitePixel(dpy,screen);
    if (nbytes==1) 
      *(zc+a1) = zpixel;
    else if (nbytes==2) 
      *(zs+a1) = zpixel;
    else if (nbytes==4) 
      *(zl+a1) = zpixel;
    
    if (i<f+1) a1++;
    else if(i<2*f+2) a1 += n;
    else if(i<3*f+3) a1--;
    else a1 -= n;
  }
  a1 = j - n + f;
  a2 = a1 - f - 1;
  a3 = a2 + (f+1)*n;
  a4 = a3 + f + 1;
  for (i=0;i<8;i++) {
    zpixel = ((i%2) == 0) ? blackPixel(dpy,screen) : whitePixel(dpy,screen);
    if (nbytes==1) 
      *(zc+a1) = *(zc+a2) = *(zc+a3) = *(zc+a4) = zpixel;
    else if (nbytes==2) 
      *(zs+a1) = *(zs+a2) = *(zs+a3) = *(zs+a4) = zpixel;
    else if (nbytes==4) 
      *(zl+a1) = *(zl+a2) = *(zl+a3) = *(zl+a4) = zpixel;
    a1 -= (n-1);
    a2 -= (n+1);
    a3 += (n-1);
    a4 += (n+1);
  }
}

/*
 * writepix(x,y,wid,hgt) writes a portion of the image to the image window
 * The region displayed should start at (x,y) in the window and extend for
 * (wid,hgt)
 */

writepix(x,y,wid,hgt)
int x, y, wid, hgt;
{
  int i0, j0, x0, y0, x1, y1, w0, h0;
  int i, imageoffset;

/* First, calculate where the display should actually be */
/* We will not bother clearing the window first (for the time being) */
/* (i0,j0) are the UL pixel of image, (x0,y0) are its location in the window */
/* (x1,y1) are the LR pixel of image + 1 in each dimension */

/*
  fprintf(stderr,"writepix: x, y, w, h = %5d %5d %5d %5d\n",x,y,wid,hgt);
  fprintf(stderr,"winx, winw, winy, winh, height: = %5d %5d %5d %5d %5d\n",
winx,winw,winy,winh,height);
*/

/* Return if no data available yet */
  if (zim == 0) return(0);

  if (zim > 0) {
    x0 = MAX(zim*(x/zim),winx);
    y0 = MAX(zim*(y/zim),winy);
    x1 = MIN(zim*((x+wid+zim-1)/zim),winx+winw);
    y1 = MIN(height,MIN(zim*((y+hgt+zim-1)/zim),winy+winh));

    i0 = (x0-winx)/zim + imx;
    j0 = (y0-winy)/zim + imy;
  } else {
    x0 = MAX(x,winx);
    y0 = MAX(y,winy);
    x1 = MIN(x+wid,winx+winw);
    y1 = MIN(height,MIN(y+hgt,winy+winh));

    i0 = (-zim)*(x0-winx) + imx;
    j0 = (-zim)*(y0-winy) + imy;
  }
/*
  fprintf(stderr,"  x0 = %6d    y0 = %6d\n",x0,y0);
  fprintf(stderr,"  x1 = %6d    y1 = %6d\n",x1,y1);
*/
  if( x1 <= x0 || y1 <= y0) return(0);

/* (w0,h0) are the size of the displayed piece of image */
  w0 = (x1-x0)/zim;
  h0 = (y1-y0)/zim;
/*
  fprintf(stderr,"   x = %6d     y = %6d -----\n",x,y);
  fprintf(stderr," wid = %6d   hgt = %6d\n",wid,hgt);
  fprintf(stderr," imw = %6d   imh = %6d\n",imw,imh);
  fprintf(stderr,"winw = %6d  winh = %6d\n",winw,winh);
  fprintf(stderr," imx = %6d   imy = %6d\n",imx,imy);
  fprintf(stderr,"winx = %6d  winy = %6d\n",winx,winy);
  fprintf(stderr," zim = %6d ",zim);
  fprintf(stderr,"  x0 = %6d    y0 = %6d\n",x0,y0);
  fprintf(stderr,"  x1 = %6d    y1 = %6d\n",x1,y1);
  fprintf(stderr,"  i0 = %6d    j0 = %6d\n",i0,j0);
  fprintf(stderr,"  w0 = %6d    h0 = %6d\n",w0,h0); 
*/
  dataimage->width = imw;
  dataimage->bytes_per_line = imwidth;
/*
  if (zim == 17) {
    dataimage->data = (char *)image[store];
    dataimage->height = y1-y0;

    XPutImage(dpy,wimage,imagegc,               
	      dataimage,
	      i0,j0,     x0,y0,                 
	      ROUNDUP(x1-x0),y1-y0);            

  } else {
*/
    dataimage->width = ROUNDUP(x1-x0);
    dataimage->height = y1 - y0;
    dataimage->bytes_per_line = ROUNDUP(x1-x0)*(dataimage->bits_per_pixel/8);
    dataimage->data = (char *)imbuf;

/* Write the appropriately zoomed data into imbuf from image */
    if (zim == 1) {
      duplicate(i0, j0, imwidth, imh,  image[store], 0, 0,
                ROUNDUP(x1-x0), y1-y0,
                ROUNDUP(x1-x0), imbuf, 1, dataimage->bits_per_pixel);
    } else if (zim>1) {
      replicate(i0, j0, imwidth, imh,  image[store], zim, 0, 0,
                ROUNDUP(x1-x0), y1-y0,
                ROUNDUP(x1-x0),  imbuf, 1, dataimage->bits_per_pixel);
    } else  {
      samplicate(i0, j0, imwidth, imh,  image[store], -zim, 0, 0,
                 ROUNDUP(x1-x0), y1-y0,
                 ROUNDUP(x1-x0),  imbuf, 1, dataimage->bits_per_pixel);
    }

    XPutImage(dpy,wimage,imagegc,  
	      dataimage,0,0,x0,y0,ROUNDUP(x1-x0),y1-y0); 

/*
  }
*/
  imagevreplay();
  imagetreplay();
  XFlush(dpy);

}

/*
 * Interrupt handler for X Events
 */

xtv_refresh(signo)
int signo;
{
  Window wmouse, wroot;
  XEvent event;
  XExposeEvent *expw  = (XExposeEvent *)&event;
  XButtonEvent *but = (XButtonEvent *)&event;
  XKeyEvent *key = (XKeyEvent *)&event;
  XPointerMovedEvent *pmove = (XPointerMovedEvent *)&event;
  XColormapEvent *cevent = (XColormapEvent *)&event;
  KeySym ks;
  int wmask;
#define KEYLEN 10
  char keystring[KEYLEN];
  XComposeStatus compose_status;
  int x=0, y=0, i, ix, iy, set, iii;
  static int configx=0, configy=0;
  static int ibut;
  unsigned char keycode;
  char keychar;
  char outbuf[32];
  float fcoord;

  if (tvinit == 0) return(0);
  whichkey = -1;

#define ALL_X_EVENTS   (~0)
while (
    /* Should we stay in this while loop indefinitely, or return to caller? */
    (waiting_for_key == 1) ?
    /* Stay in this while loop until key/button press or Config or Expose */
    (XWindowEvent(dpy,wbase,ALL_X_EVENTS,&event), 1) :
    /* Return to caller as soon as X event queue is emptied */
    XCheckWindowEvent(dpy,wbase,ALL_X_EVENTS,&event) || 
    XCheckWindowEvent(dpy,wzoom,ALL_X_EVENTS,&event)  
) {
/*
    fprintf(stderr," got event w = %d, x,y,w,h = %d %d %d %d type = %d\n", 
	    expw->window,expw->x,expw->y,expw->width,expw->height,event.type); 
    fprintf(stderr,"wbase: %d wimage: %d wzoom: %d\n",wbase,wimage,wzoom);
*/
/*
      switch((int)event.type) {
      case ColormapNotify:
        fprintf(stderr,"%d %d %d\n",cevent->send_event, cevent->new, cevent->state);
        XInstallColormap(dpy, defcmap);
      }
*/

/* If the event came from the zoom window, update it */
    if (zoomf != 0 && expw->window == wzoom && imagevalid) {
      switch((int)event.type) {
      case Expose:
	zoomf = ABS(zoomf);
	XGetGeometry(dpy,wzoom,&inforoot,&infox,&infoy,&infowidth,&infoheight,
		     &infoborder,&infodepth);
	zwidth = (infowidth+zoomf-1) / MAX(1,zoomf);
	zheight = (infoheight+zoomf-1) / MAX(1,zoomf);
	if (zoomf >= 0 && !zfreezeon) updatezoom(mousex,mousey);
	break;
      case UnmapNotify:
	zoomf = -ABS(zoomf);
	break;
      case ButtonPress:
	whichkey = but->button;
	if(whichkey == Button2) zoomf = MAX(zoomf-1,1);
	if(whichkey == Button1) zoomf++;
	XGetGeometry(dpy,wzoom,&inforoot,&infox,&infoy,&infowidth,&infoheight,
		     &infoborder,&infodepth);
	zwidth = (infowidth+zoomf-1) / MAX(1,zoomf);
	zheight = (infoheight+zoomf-1) / MAX(1,zoomf);
	if(zoomf > 0 && !zfreezeon) updatezoom(lastzoomx,lastzoomy);
	XFlush(dpy);
      }
/* Otherwise the event came from the base window */
    } else {

/* Switch on the type of event */
      switch((int)event.type) {

/*
 * WINDOW EXPOSURE EVENT
 */
      case Expose:
      case ConfigureNotify:
/*      case VisibilityNotify: */
/*      XQueryWindow(wbase,&winfo); */
/* If the event came from a subwindow, dump it for now */
/*
 fprintf(stderr,"Exposure event: T = %d (E = %d, V = %d); W = %d (I = %d, B = %d);  x = %d y = %d w = %d h = %d n = %d\n",
		(int)event.type,Expose,VisibilityNotify,expw->window,wimage,wbase,
		expw->x,expw->y,expw->width,expw->height,expw->count);
*/

        if (ABS(expw->x-configx)<10 && ABS(expw->y-configy)<10) break;
        configx=expw->x;
        configy=expw->y;

/* If the window was resized, update the subwindow sizes */
#ifdef __HAIRS
        if (usehairs && hairs_on) vnohair();
#endif
	XGetGeometry(dpy,wbase,&inforoot,&infox,&infoy,&infowidth,&infoheight,
		     &infoborder,&infodepth);
	if (infowidth != width || infoheight != height+XYZHEIGHT) {
	  newsizesubwin(infowidth,infoheight-XYZHEIGHT);
	  resizesubwin();
	  updatepan(imw/2,imh/2,0);
	  XClearWindow(dpy,wbase);
	}

/* Rewrite the image */
/*
        imageupdate(datx,daty,imw,imh,0);
	for (i=0;i<palheight;i++) {
	  XPutImage(dpy,wpal,imagegc,palimage,0,0,0,i,palwidth,1);
	}
*/
        lights(0);
	break;

/*
 * BUTTON PRESSED: A mouse button was pressed
 */
      case ButtonPress:
	whichkey = but->button;
/*      fprintf(stderr,"Button pressed  %d \n", whichkey); */
	ibut = 0;
	if(whichkey == Button1) ibut = 1;
	if(whichkey == Button2) ibut = 2;
	if(whichkey == Button3) ibut = 3;
	lastx = but->x;
	lasty = but->y;
	buttondown = 1;

/* The button was pressed in the image window, zoom and center */
	if(but->subwindow == wimage) {
	  if(keyaction[ibut].action != NULL) {
	    (*(keyaction[ibut].action))
	      (but->x,but->y,USERXCOORD(but->x),USERYCOORD(but->y),ibut);
	  }
	}

/* The button was pressed in the palette window */
	else if(but->subwindow == wpal) {
#ifdef __HAIRS
          if (usehairs && hairs_on) vnohair();
#endif
	  updatenewpal((npalette*(but->x-palx))/palwidth,ibut);
	}

/* Button in light window 3 toggles zfreezeon */
        else if (but->subwindow == wlgt3) {
          if (!lgtenable) {
            zfreezeon = !zfreezeon;
            if (zfreezeon==1)
              lights(-3);
            else
            lights(3);
          } else {
          lgtstatus[2] = -1*lgtstatus[2];
          lights(lgtstatus[2]);
          }
        }
        else if (lgtenable && but->subwindow == wlgt1) {
          lgtstatus[0] = -1*lgtstatus[0];
          lights(lgtstatus[0]);
        }
        else if (lgtenable && but->subwindow == wlgt2) {
          lgtstatus[1] = -1*lgtstatus[1];
          lights(lgtstatus[1]);
        }
        else if (lgtenable && but->subwindow == wlgt4) {
          lgtstatus[3] = -1*lgtstatus[3];
          lights(lgtstatus[3]);
        }

	break;

/*
 * BUTTON RELEASED: The button was released
 */
      case ButtonRelease:
	buttondown = 0;
	break;

/*
 * KEY PRESSED: A Keyboard key was pressed
 */
      case KeyPress:

/* Only accept /r' key from palette subwindow   */
	XLookupString(key,keystring,KEYLEN,&ks,&compose_status);
/*        ks = XLookupKeysym(key,0); */
	if (key->subwindow == wpal) {
	  if (keystring[0] == 'r' || keystring[0] == 'R') {
/*
            breakpts[0] = zero0;
            breakpts[npalette-1]=zero0+span0;
*/
	    newcolors(0,npalette-1);
          }
	  keystring[0] = '\0';
	}

/* Ignore keyboard events in subwindows */
	if (key->subwindow != wimage) break;

/* Ignore modifier keys */
	if (IsModifierKey(ks)) break ;

/* If an arrow key, move the mouse */
	if (IsCursorKey(ks)) {
	  XQueryPointer(dpy,wbase,&wroot,&wmouse,
			&infox,&infoy,&mousex,&mousey,&wmask);
/*	  if(ks == XK_Left) mousex--;
	  else if(ks == XK_Right) mousex++;
	  else if(ks == XK_Down) mousey++;
	  else if(ks == XK_Up) mousey--; */
          if(ks == XK_Left) mousex -= MAX(zim,1);
          else if(ks == XK_Right) mousex += MAX(zim,1);
          else if(ks == XK_Down) mousey += MAX(zim,1);
          else if(ks == XK_Up) mousey -= MAX(zim,1);
/*	  fprintf(stderr,"Move mouse to %d %d  %d %d %d %d %d\n",mousex,mousey,
		  key->keycode,XK_Left,XK_Right,XK_Down,XK_Up); */
	  XWarpPointer(dpy,None,wimage,0,0,0,0,mousex,mousey);
	  updatecoords(mousex,mousey,-1);
	  XFlush(dpy);
	  break;
	}

/* Get the character typed */
	keychar = keystring[0];
/*      if(i == 0) break; */
	whichkey = keychar;
	if(whichkey >= 0 && whichkey <= 127 &&
	   keyaction[whichkey].action != NULL) {
	  (*(keyaction[whichkey].action))
	    (key->x,key->y,USERXCOORD(key->x),USERYCOORD(key->y),whichkey);
	}
	updatecoords(key->x,key->y,whichkey);
	keystring[0] = '\0';

/* waiting_for_key means write into pipe to program */
	if(waiting_for_key) {
	  *outbuf = keychar;
	  lastx = USERXCOORD(key->x);
	  lasty = USERYCOORD(key->y);
	  write(to_program,outbuf,1);
          waiting_for_key = 0;
	} 
	XFlush(dpy);
	break;

/*
 * MOUSE MOVED: Mouse movement
 */
      case LeaveNotify:
#ifdef __HAIRS
	if (usehairs && hairs_on) vnohair(); 
#endif
	break;

      case MotionNotify:
/* If mouse moved in image window with button down, ignore it */
	if(buttondown && pmove->subwindow == wimage) break;
	XQueryPointer(dpy,wbase,&wroot,&wmouse,
			&infox,&infoy,&mousex,&mousey,&wmask);
#ifdef __HAIRS
	if (usehairs && wmouse == wimage) {
              crshr[0] = crshr[2];
	      crshr[1] = crshr[3];
	      crshr[2].x1 = 0; 
	      crshr[2].x2 = width;
	      crshr[2].y1 = crshr[2].y2 = mousey;
	      crshr[3].y1 = 0;
	      crshr[3].y2 = height;
	      crshr[3].x1 = crshr[3].x2 = mousex;
              if (hairs_on) {
                XSetForeground(dpy, vectorgc, blackPixel(dpy,screen));
if (_XErrorEvent.serial!=0) 
 printf("loc 17: %d %s", _XErrorEvent.serial,_XErrorEvent.error_code);
	        XDrawSegments(dpy,wimage,vectorgc,crshr,2);
                XSetForeground(dpy, vectorgc, vcolor[0].pixel);
if (_XErrorEvent.serial!=0) 
 printf("loc 18: %d %s", _XErrorEvent.serial,_XErrorEvent.error_code);
	        writepix(crshr[0].x1,crshr[0].y1,width,1);
	        writepix(crshr[1].x1,crshr[1].y1,1,height);
	        XDrawSegments(dpy,wimage,vectorgc,crshr+2,2);
	      } else {
                XSetForeground(dpy, vectorgc, vcolor[0].pixel);
if (_XErrorEvent.serial!=0) 
 printf("loc 19: %d %s", _XErrorEvent.serial,_XErrorEvent.error_code);
	        XDrawSegments(dpy,wimage,vectorgc,crshr+2,2);
	        hairs_on = True;
	      }
	}
#endif      /* HAIRS */

	if (wmouse == wpal) {
#ifdef __HAIRS
          if (usehairs && hairs_on) vnohair(); 
#endif
/* If mouse moved in palette window with button down, update palette */
	  if (buttondown) {
	    updatepal((npalette*(mousex-palx))/palwidth,ibut);
	    break;
	  } else {
/* If mouse moved in palette window tell what the breakpt is... */
	    updatebrkpt((ncolors*(mousex-palx))/palwidth,ibut);
	    break;
	  }
	}

/* Check out current position of mouse */

/* If mouse in image window, update coords and zoom */
	if(wmouse == wimage) {
	  updatecoords(mousex,mousey,-1);
	  if (zoomf > 0 && !zfreezeon) updatezoom(mousex,mousey);
	}
	break;
      } /* end switch on eventtype in base window */
    } /* end if (event came from base window) */
  } /* end of while(1) */
  return(0);
} /* end of function xtv_refresh() */

keyzoompan(x,y,inout)
int x, y, inout;
{
  int ix, iy;
  if(zim > 0) {
    ix = (x - winx)/zim + imx;
    iy = (y - winy)/zim + imy;
  } else {
    ix = (-zim)*(x - winx) + imx;
    iy = (-zim)*(y - winy) + imy;
  }
  XClearWindow(dpy,wimage);
  XWarpPointer(dpy,None,wimage,0,0,0,0,width/2,height/2);
  updatepan(ix,iy,inout);
  writepix(0,0,width,height);
  if(zoomf > 0 && !zfreezeon) updatezoom(width/2,height/2);
}

keyzoomin(x,y,xuser,yuser,key)
int x, y, xuser, yuser, key;
{
  keyzoompan(x,y,1);
}

keyzoomout(x,y,xuser,yuser,key)
int x, y, xuser, yuser, key;
{
  keyzoompan(x,y,-1);
}

keypan(x,y,xuser,yuser,key)
int x, y, xuser, yuser, key;
{
  keyzoompan(x,y,0);
}

keyrecenter(x,y,xuser,yuser,key)
int x, y, xuser, yuser, key;
{
  int maxdim, maxw, maxh;

  zim = 1;
  XClearWindow(dpy,wimage);
  XWarpPointer(dpy,None,wimage,0,0,0,0,width/2,height/2);
  updatepan(imw/2,imh/2,0);
/* If the image can be zoomed and fit it the display window, do it */
  maxdim = MAX(imw,imh);
  while (maxdim*2 <= MIN(width,height)) {
    updatepan(imw/2,imh/2,1);
    maxdim = maxdim*2;
  }
/* If image is too big for display, zoom out if option is set */
 if (autozoomout && (imw>width || imh>height)  ) {
    if (resize) {
      maxw = maxwidth;
      maxh = maxheight;
    } else {
      maxw = width;
      maxh = height;
    }
    while ( (imw/ABS(zim))>maxw || (imh/ABS(zim))>maxh )  {
      updatepan(imw/2,imh/2,-1);
    }
  }

  writepix(0,0,width,height);
  if (zoomf > 0 && !zfreezeon) updatezoom(width/2,height/2);
}

keyzoomprint(x,y,xuser,yuser,key)
int x, y, xuser, yuser, key;
{
  keyzoompan(x,y,2);
  imageprintval();
}

keyhelp()
{
  printf("Image window:\n");
  printf("  Left button zooms in at location of mouse\n");
  printf("  Center button zooms out at location of mouse\n");
  printf("  Right button pans (same magnifaction) to location of mouse\n");
  printf("  r key redisplays image as originally displayed (centered, etc.)\n");
  printf("\n");
  printf("Color bar: \n");
  printf("  Left or right button drags end of color map down or up\n");
  printf("      i.e., increases the contrast\n");
  printf("  Center mouse button rolls color map\n");
}


/*  Routine that freezes the zoom window on one location */
zfreeze(x,y,xuser,yuser,key)
int x, y, xuser, yuser, key;
{
  zfreezeon = !zfreezeon;
  if (!lgtenable) {
    if (zfreezeon == 1) {
      lights(-3);
    }
    else {
      lights(3);
    }
  }
}

zsex(x,y,xuser,yuser,key)
int x, y, xuser, yuser, key;
{
  if ( ((key == (int)'s' || key == (int)'S')) ) sexages = 1;
  if ( ((key == (int)'d' || key == (int)'D')) ) sexages = 0;
}

/*  Routine to loop to the next image stored in video memory */
nextim(x,y,xuser,yuser,key)
int x, y, xuser, yuser, key;
{
  int yim;

  store = (store+1>MAXSTORE-1 ? 0 : store+1);
  while (image[store]==NULL)
    store = (store+1>MAXSTORE-1 ? 0 : store+1);
  updatestore();
  if (data == NULL)
    lights(-2);
  else
    lights(2);

  if (zim>0) {
    yim = winy + zim*(daty-daty-imy);
    if (yup == 1) yim = winy + zim*(imh-1 - (daty+imh-1) - imy + daty);
    writepix(zim*(datx-datx-imx)+winx, yim, zim*imw, zim*imh);
  }
  else {
    yim = winy + (daty-daty-imy)/-zim;
    if (yup == 1) yim = winy + (imh-1 - (daty+imh-1) - imy + daty)/(-zim);
    writepix((datx-datx-imx)/(-zim)+winx, yim, imw/(-zim), imh/(-zim));
  }
  if(zoomf > 0 && !zfreezeon) updatezoom(lastzoomx,lastzoomy);
/*  xtv_refresh(0);  */
}

/*  Routine to loop to the previous image stored in video memory */
lastim(x,y,xuser,yuser,key)
int x, y, xuser, yuser, key;
{
  int yim;

  store = (store-1<0 ? MAXSTORE-1 : store-1);
  while (image[store]==NULL)
    store = (store-1<0 ? MAXSTORE-1 : store-1);
  updatestore();
  if (data == NULL)
    lights(-2);
  else
    lights(2);
  if (zim>0) {
    yim = winy + zim*(daty-daty-imy);
    if (yup == 1) yim = winy + zim*(imh-1 - (daty+imh-1) - imy + daty);
    writepix(zim*(datx-datx-imx)+winx, yim, zim*imw, zim*imh);
  }
  else {
    yim = winy + (daty-daty-imy)/-zim;
    if (yup == 1) yim = winy + (imh-1 - (daty+imh-1) - imy + daty)/(-zim);
    writepix((datx-datx-imx)/(-zim)+winx, yim, imw/(-zim), imh/(-zim));
  }
  if(zoomf > 0 && !zfreezeon) updatezoom(lastzoomx,lastzoomy);
/*  xtv_refresh(0); */
}


/*  Routine to move cursor to local peak of data  */
zpeak(x,y,xuser,yuser,key)
int x, y, xuser, yuser, key;
{
  int xdata, ydata, xmin, ymin, xmax, ymax, ix, iy, xp, yp, xm, ym;
  float pixel, pmax, *row;

  if (data == NULL) return(0);

  xdata = xuser - offx;
  ydata = yuser - offy;
  xmin = ( xdata-7>datx ? xdata-7 : datx);
  ymin = ( ydata-7>daty ? ydata-7 : daty);
  xmax = ( (xdata+7)<imw+datx ? xdata+7 : imw+datx);
  ymax = ( (ydata+7)<imh+daty ? ydata+7 : imh+daty);
  pmax = *(data+ydata*datw+xdata);
  xp = xdata;
  yp = ydata;

  for (iy=ymin; iy<=ymax; iy++){
    row = data + (iy*datw);
    for (ix=xmin; ix<=xmax; ix++) {
      pixel = *(row+ix);
      if ( ((key == (int)'v' || key == (int)'V') && pixel < pmax) ||
           ((key == (int)'p' || key == (int)'P') && pixel > pmax) ) {
        pmax = pixel;
        xp = ix;
        yp = iy;
      }
    }
  }
/*
  printf("offx: %d offy: %d datx: %d daty: %d\n",offx,offy,datx,daty); 
  printf("xp: %d yp: %d imx: %d imy: %d\n",xp,yp,imx,imy); 
  printf("datw: %d dath: %d xuser: %d yuser: %d winx: %d winy: %d\n\n",
       datw, dath, xuser, yuser, winx, winy); 
*/
  if (zim>0) {
    xm = (xp - datx - imx) * zim + winx;
    ym = (yp - daty - imy) * zim + winy;
    if (yup ==1) ym = (imh-1 - yp - imy + daty) * zim + winy;
    if (zim>1) {
      xm = xm + zim/2 - 1;
      ym = ym + zim/2 - 1;
    }
  } else {
    xm = (xp - datx - imx) / -zim + winx;
    ym = (yp - daty - imy) / -zim + winy;
    if (yup ==1) ym = (imh-1 - yp - imy + daty) / -zim + winy;
  }
/*
  printf("xm: %d ym: %d \n",xm,ym);
*/

  XWarpPointer(dpy,None,wimage,0,0,0,0,xm,ym);
  updatecoords(xm,ym,-1);
  if(zoomf > 0 && !zfreezeon) updatezoom(xm,ym);
  XFlush(dpy);

}

#ifdef __HAIRS
extern int usehairs;
zhairs(x,y,xuser,yuser,key)
int x, y, xuser, yuser, key;
{
  XColor curswcolor, cursbcolor;
  Pixmap csource, cmask;

  if (usehairs == 0) {
XDefineCursor(dpy,wimage,XCreateGlyphCursor(dpy, fontinfo->fid, fontinfo->fid,
    (unsigned int)' ', (unsigned int)' ',&curswcolor, &cursbcolor));
    usehairs = 1;
  }
  else {
vnohair();
curswcolor.pixel = WhitePixel(dpy,screen);
XQueryColor(dpy,defcmap,&curswcolor);
cursbcolor.pixel = BlackPixel(dpy,screen);
XQueryColor(dpy,defcmap,&cursbcolor);
csource = XCreateBitmapFromData(dpy,wimage,curs_bits,curs_width,curs_height);
cmask = XCreateBitmapFromData(dpy,wimage,curs_mask_bits,curs_width,curs_height);
curs = XCreatePixmapCursor(dpy,csource,cmask,
         &curswcolor,&cursbcolor,curs_x_hot,curs_y_hot);
XFreePixmap(dpy,csource);
XFreePixmap(dpy,cmask);
XDefineCursor(dpy,wimage,curs);
    usehairs = 0;
  }
}
#endif

imageprintval()
{
  float z;
  int x0, y0, x, y, i0, i1, i, j0, j1, j, iz, printlen;
  char printstring[16];

  if(zim <= 0 || data == NULL) return(0);

/* Find the lower left pixel (i0,j0) */
  i0 = (-winx)/zim+imx+datx+offx;
  if (yup==0) j1 = (winh-winy)/zim+imy+daty+offy;
  else j0 = (imh-1-(winh-winy)/zim-imy)+daty+offy + 1;

/* Find the upper right pixel (i1,j1) */
  i1 = (winw-winx)/zim+imx+datx+offx;
  if (yup==0) j0 = (-winy)/zim+imy+daty+offy;
  else j1 = (imh-1-(-winy)/zim-imy)+daty+offy + 1;

/* Find the (x,y) location of the lower left pixel */
  x0 = zim * (i0 - imx - datx - offx) + winx + (zim - 1) / 2;
  if (yup == 0) y0 = winy + zim * (j0 - imy - daty - offy) + (zim - 1) / 2;
  else y0 = winy + zim * (imh-1 - (j0 - offy - daty) - imy) + (zim - 1) / 2;

//  fprintf(stderr,"%4d %4d %4d %4d %4d %4d \n",i0,i1,j0,j1,x0,y0);

  y = y0;
  XSetForeground(dpy,textgc,whitePixel(dpy,screen));
  for (j=j0;j<j1;j++) {
    x = x0;
    for (i=i0;i<i1;i++) {

 //     fprintf(stderr,"%4d %4d %4d %4d %4d %4d %4d %4d %4d %4d \n",
 //     i,j,i0,i1,j0,j1,x,y,x0,y0);

      if(i==i0 && j==j0) {
      } else if(i==i0) {
        sprintf(printstring,"* %d *",j);
      } else if(j==j0) {
        sprintf(printstring,"* %d *",i);
      } else {
        z = *(data+(j-offy)*datw+(i-offx));
        if(ABS(z) < 1e6 && ABS(z) > 1e-2) {
          iz = ABS(z) + 0.5;
/* Make it an integer no matter what 
          if(iz >= 1000) { */
          if(iz >= 000) { 
            if(z < 0) iz = -iz;
            sprintf(printstring,"%d",iz);
          }
          else if(iz >= 100)
            sprintf(printstring,"%.1f",z);
          else if(iz >= 10)
            sprintf(printstring,"%.2f",z);
          else
            sprintf(printstring,"%.3f",z);
        } else {
          sprintf(printstring,"%7.1e",z);
        }
      }
      printlen = strlen(printstring);
      XSetForeground(dpy,textgc,whitePixel(dpy,screen));
      XDrawImageString(dpy,wimage,textgc,
                       x-(fontwidth*printlen)/2,y+fontheight/2,
                       printstring,printlen);
      x = x + zim;
    }
    if (yup == 0) y = y + zim;
    else y = y - zim;
  }
}


tvblink_(a,b,nrow,ncol,nc,sr,sc,asr,asc,span,zero,flip,ibl)

float   *a,             /* Input floating image array                   */
	*b,             /* second floating array                        */
	*span,          /* Intensity mapping span level.  This level    */
			/* is used to scale the pixel data between 0    */
			/* and 254 for display on the Grinnell.         */
	*zero;          /* Intensity scale zero offest                  */

int     *nrow,          /* Number of rows to be displayed               */
	*ncol,          /* Number of columns to be displayed            */
	*nc,            /* Row length in pixels of the array            */
	*sr,            /* Row offset to first displayed row            */
	*sc,            /* Column offset to first displayed row         */
	*asr,           /* Row number of image array origin             */
	*asc,           /* Column number of image array origin          */
	*flip,          /* Flag for left-right image reflection         */
	*ibl;           /* Number of grey levels per image              */

{
	short color[128];
	int i, j, ii, iblink, ntoget, row, col;
	char *table, ch;
	register unsigned char *g;
	register float *pix,*pix2,      /* Pointer to within array      */
			lev;
	int jump, npix, ipart, nr, iif;
	int z, x, y, tvzoom_(), tvzoomc_();
	float val,val2;
	double junk;
	int nx, ny, x0, y0, awidth, xoff, yoff;

/*      Allocate the memory for the destination image                   */
	nx = *ncol;
	ny = *nrow;
	x0 = *sc;
	y0 = *sr;
	awidth = *nc;
	xoff = *asc;
	yoff = *asr;
	if (wbase == 0) {
	  fprintf(stderr,"windows not yet created\n");
	  return(ERR_NOT_INITIALIZED);
	}
	store = store+1>MAXSTORE-1 ? 0 : store+1;
	if (nimage[store] < ROUNDUP(nx)*ny || image[store] == NULL) {
	   if (image[store] != NULL) free(image[store] );
	   if ((image[store]  = (unsigned char *)malloc(ROUNDUP(nx)*ny)) == NULL) {
	     fprintf(stderr,"Can't allocate image array\n");
	     return(ERR_BAD_ALLOC_IMBUF);
	   }
	   nimage[store] = ROUNDUP(nx)*ny;
	}

/*      Initialize variables                                            */

	lights(-2);
	pix     = a + *sc + *sr * *nc;  /* Pointer to first pixel       */
	pix2    = b + *sc + *sr * *nc;  /* Pointer to first pixel       */
	jump    = *nc - *ncol;          /* Jump between rows            */

/*      Load up destination image with data from 2 images               */
	if (*span > 0.0) {
	g       = image[store] ;
	lev     = (*ibl * *ibl - 1) / *span;
	for (nr = *nrow; nr > 0; nr--) {
		for (npix = *ncol; npix > 0; npix--) {
			val     = ( *pix++  - *zero );
			val2    = ( *pix2++ - *zero );
			val = lev * (val - ((int)(val / *span) * *span));
			val2 = lev * (val2 - ((int)(val2 / *span) * *span));
			if (val < 0.0)
				val     =0.0;
			if (val2 < 0.0)
				val2    = 0.0;
			ipart   = (int)((val / *ibl) +
					(int)(val2 / *ibl) * *ibl);
			*g++    = pixels[ipart];
		}
		pix     += jump;
		pix2    += jump;
	}
	}

/*      This block of code just clips pixels instead of letting them wrap */

	else {
	iif = *ibl * *ibl - 1;
	lev     = iif / -*span;
	g       = image[store] ;
	for (nr = *nrow; nr > 0; nr--) {
		for (npix = *ncol; npix > 0; npix--) {
			val     = lev * ( *pix++  - *zero );
			val2    = lev * ( *pix2++ - *zero );
			if (val < 0.0)
				val     =0.0;
			else if (val > iif)
				val     = iif;
			if (val2 < 0.0)
				val2    = 0.0;
			else if (val2 > iif)
				val2    = iif;
			ipart   = (int)((val / *ibl) +
					(int)(val2 / *ibl) * *ibl);
			*g++    = pixels[ipart];
		}
		pix     += jump;
		pix2    += jump;
	}
	}

	updateimage(x0,y0,nx,ny,awidth,a,xoff,yoff);

	imagevalid = 1;
	imagevnull();

	if (resize && (width < nx || height < ny)) {
	  width = MIN(1024,MAX(width,nx));
	  height = MIN(865,MAX(height,ny));
	  updatepan(imw/2,imh/2,0);
	  updatesize(width,height);
	}
	else {
          if (zim>0)
	    writepix(winx, winy, zim*imw, zim*imh);
          else
	    writepix(winx, winy, imw/-zim, imh/-zim);
	  xtv_refresh(0);
	}
	XFlush(dpy);
	lights(-1);
	lights(2);
	lights(3);
	lights(4);

/*      Load the blink color tables and blink                           */

	iblink = 0;
	ch = ' ';
	printf("In TV window, enter E or Q to quit, anything else \
to blink:\n");
	ntoget = 1;
	while (ch != 'E' && ch != 'e' && ch != 'Q' && ch != 'q') {
	  if (iblink == 0) {
	    ii = 0;
	    for (i=0; i<*ibl; i++) {
	      for (j=0; j<*ibl; j++) {
		color[ii++] = (short)(j * (255. / *ibl));
	      }
	    }
	    updateimage(x0,y0,nx,ny,awidth,a,xoff,yoff);
	    imagepalette(*ibl* *ibl, color, color, color, 0);
	    iblink = 1;
	    imageread(&col,&row,&ch);
	  }
	  else {
	    ii = 0;
	    for (i=0; i<*ibl; i++) {
	      for (j=0; j<*ibl; j++) {
		color[ii++] = (short)(i * (255. / *ibl));
	      }
	    }
	    updateimage(x0,y0,nx,ny,awidth,b,xoff,yoff);
	    imagepalette(*ibl* *ibl, color, color, color, 0);
	    iblink = 0;
	    imageread(&col,&row,&ch);
	  }
	}

	return(0);

}
#ifdef __HAIRS
vnohair()
{
    crshr[0] = crshr[2];
    crshr[1] = crshr[3];
    crshr[2].x1 = crshr[2].x2 = crshr[2].y1 = crshr[2].y2 = -1;
    crshr[3].y1 = crshr[3].y2 = crshr[3].x1 = crshr[3].x2 = -1;
    XSetForeground(dpy, vectorgc, blackPixel(dpy,screen));
if (_XErrorEvent.serial!=0) 
 printf("loc 20: %d %s", _XErrorEvent.serial,_XErrorEvent.error_code);
    XDrawSegments(dpy,wimage,vectorgc,crshr,2);
    XSetForeground(dpy, vectorgc, vcolor[0].pixel);
    writepix(crshr[0].x1,crshr[0].y1,width,1);
    writepix(crshr[1].x1,crshr[1].y1,1,height);
}
#endif  /* HAIRS */
int blackPixel(Display *dpy, int screen)
{
  if (private)
    return(stcolor[3].pixel);
  else
    return(BlackPixel(dpy,screen));
}
int whitePixel(Display *dpy, int screen)
{
  if (private)
    return(stcolor[1].pixel);
  else
    return(WhitePixel(dpy,screen));
}
