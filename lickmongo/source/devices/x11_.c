#define _POSIX_SOURCE
/*
 *      ** COPYRIGHT ****  COPYRIGHT ****  COPYRIGHT ****  COPYRIGHT **
 *      This C file is Copyrighted software.
 *      The file COPYRIGHT must accompany this file.  See it for details.
 *      ** COPYRIGHT ****  COPYRIGHT ****  COPYRIGHT ****  COPYRIGHT **
 */
/*      These are the X11 drivers for Lick Mongo                        */
/*      Do not take any of this code seriously, it is not finished.     */
/*      Just in case you want to know...                                */
/*      The skeleton of this code was based upon the XV11R3 xim program,*/
/*      but by now there is very little evidence left of that.          */
/*      The bulk of the code was written by Steve Allen at Lick Obs.    */
/*      The addition of color and pixmap support was done by Phil Pinto.*/
/*      Hacks from Calvin Cliff @ UCLA may make this work with VMS      */

#include "Config.h"

#ifdef  VMS
#include <descrip.h>
#endif  /* VMS */
#include <errno.h>

#define MOVIE

#undef  VERBOSE
#define X11color

/* these are three different font possibilities, tried in order */
#define DEF_FONT1 "9x15"
#define DEF_FONT2 "-adobe-helvetica-bold-r-normal--14-100-100-100-p-82-iso8859-1"
#define DEF_FONT3 "helvetica_bold14"
#define DEF_GEOM "640x495+350+10"
#define DEF_WID (640)
#define DEF_HGT (480)

#define MIN_WID (100)
#define MIN_HGT (100)
#define MAX_WID (4096)
#define MAX_HGT (4096)
#define BDR_WID (3)

#define MAX_COLORS (27)

#define HAIRS           /* use full screen cross hairs and no cursor */

#include <X11/Xos.h>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <X11/Xutil.h>
#include <X11/cursorfont.h>
#include <X11/Xatom.h>
//#include <X11/Intrinsic.h>
#ifndef XtSpecificationRelease
    /* assume we are linking to an XV11R3 or earlier Xlib */
#   define XtSpecificationRelease 3
#endif
#include <stdio.h>
typedef unsigned char  byte;

/* Here is a really dumb idea!  Let the C code see MONGOPAR.    */
/* This has to be changed someday.                              */
extern struct {
    float       X1,X2,Y1,Y2, GX1,GX2,GY1,GY2;
    int         LX1,LX2,LY1,LY2;
    float       XP,YP, EXPAND,ANGLE;
    int         LTYPE;
    float       LWEIGHT, CHEIGHT,CWIDTH,CDEF,PDEF,COFF;
    int         TERMOUT,XYSWAPPED,NUMDEV;
    int         TERMIDLE,LVIS;
    float       TOTDIST;
    int         AUTOLWEIGHT,MCOLOR;
    float       COSANG,SINANG;
} UMGO(mongopar);

#include "xmongo.xbm"

#ifdef  HAIRS
static  Bool    hairs_on = False;
#define CROSSOFF()      if (hairs_on) (void) PTUV(mx11nohair)()
#else   /* HAIRS */
#define CROSSOFF()
#endif  /* HAIRS */

static  Display *dpy;
static  int     screen;
static  Window  root_win;
static  Visual  *visual = NULL;
static  unsigned long   blackpixel, whitepixel;

static int      des_x_open = 0;         /* desired x,y position of window */
static int      des_y_open = 0;         /* on opening */
static int      des_wid_open = DEF_WID; /* desired window width and height */
static int      des_hgt_open = DEF_HGT; /* on opening */
static unsigned long   user_pos = 0;    /* and flags for the above */

/* 
 * Redefined the first 8 to be black/white/red/green/blue/yellow/cyan/magenta
 * (i.e., the default blk/wht, then the 3 primary and 3 secondary colors
 * [rwp/osu 96jan16]
 */

#define PREDEFINED_COLORS (27)
static char    *color_name[] ={"#000000",  /* black*/
			       "#ffffff",  /* white */
			       "#ff0000",  /* red */
			       "#00ff00",  /* green */
			       "#0000ff",  /* blue */
			       "#ffff00",  /* yellow */
			       "#00ffff",  /* cyan */
			       "#ff00ff",  /* magenta */
			       "#7f7f7f",  /* grey */
			       "#7f0000",  /* dark red */
			       "#ff007f",  /* orange red */
			       "#7f007f",  /* dark purple */
			       "#ff7fff",  /* light magenta */
			       "#ff7f7f",  /* peach */
			       "#ff7f00",  /* coral */
			       "#ffff7f",  /* light yellow */
			       "#7f7f00",  /* olive yellow */
			       "#7fff00",  /* medium spring green */
			       "#007f00",  /* dark green */
			       "#7fff7f",  /* grey green */
			       "#00ff7f",  /* spring green */
			       "#007f7f",  /* dark cyan */
			       "#7fffff",  /* bright blue violet */
			       "#007fff",  /* slate blue*/
			       "#00007f",  /* dark blue */
			       "#7f7fff",  /* blue violet */
			       "#7f00ff"   /* medium slate blue */
			   };

static  unsigned long          current_color;
static  unsigned long          colors[MAX_COLORS];
Status                  status;
static  XColor          exact_def, xcblack, xcwhite;
static  Colormap        colormap;
static  Window          image_win;
static  XEvent          event;
static  XExposeEvent    *expose;
static  GC              image_gc, xor_gc, text_gc;
static  XFontStruct     *font_info;
static  Pixmap          icon_pixmap;
static  Cursor          curvis, curinvis, curprompt;
static  char butdwn = 0;        /* bitmask of which mouse buttons are down */

#ifdef  MOVIE
static  Pixmap          pixel_drw = (Pixmap) NULL;   /* this is where we are drawing at the moment */
static  Pixmap          pixel_keep = (Pixmap) NULL;   /* this is where we store the image */
static int              movieflag = -1;      /* =0 -> normal mode; >0 -> movie mode; <0 -> no image */

void UTUV(mx11direct)();
#else   /* MOVIE */
#define pixel_drw image_win
static  XImage          *image = NULL;
#endif  /* MOVIE */

static int     fd_for_X = -1;

	/* this is defined in xgets.c */
extern int ux11gets;
	/* pointers to space in which values will be returned */
static unsigned int     *ikey = (unsigned int *)NULL;
static int              *ix = (int *)NULL;
static int              *iy = (int *)NULL;
static unsigned int     *ista = (unsigned int *)NULL;
	/* set nonzero if event is a key or button down event */
static int              iup = 0;
	/* string input manipulation variables */
static int              iass = 0;
static char             *istr = NULL;   /* pointer to a string */
static int              listr = 0;      /* length of pointed string */

static char     display_name[64];
static byte     mono;
static char     *wind_name = "Lick Mongo";
static char     *icon_name = "LickMongo";
static int      wwidth, wheight;        /* actual present win size      */
static int      owidth, oheight;        /* size MONGO thinks it is      */
static int      lheight;
static Bool     need_Config = False;
static Bool     need_Expose = False;
static Bool     x_is_open = False;
static unsigned long cur_ev_mask;

static int      zero = 0, one = 1, two = 2;

void PTUV(mx11clearpixmap)();
void PTUV(mx11getbuffer)();
void PTUV(mx11pointerpos)();

#define SWAP(a,b) { a^=b; b^=a; a^=b;}  /* from Wyvill in Graphics Gems */
#define LSTR 100
char    strng[2*LSTR];
char    ustrng[LSTR];
typedef void (*PFRV)();         /* pointer to a function returning a void */
typedef char *PC;               /* pointer char*/
PFRV    actbup      = NULL;     /* function called upon button release */
PFRV    actbdn      = NULL;     /* function called upon button press   */
PFRV    actkdn      = NULL;     /* function called upon button press   */
PFRV    actbmv      = NULL;     /* function called upon button drag    */
PFRV    actmot      = NULL;     /* function called upon pointer motion */
PFRV    actstr      = NULL;     /* function called upon pointer motion */
PFRV    oldact;
PC      PTUV(mx11_event_loop)();/* declaration of event_loop            */
/************************************************************************/
/************************************************************************/
/* open an input-output window, allocate colormap, set up cursor, etc. */
UTUV(mx11init)(dpy_name,ugeom,lendpyn,lenugeom)
char    *dpy_name;
char    *ugeom;
int     lendpyn, lenugeom;
{
    register unsigned           i, j, k;
    int                         iconfact;
    register byte               *icon_buf;

    Colormap                    GetColormap();
    XGCValues                   gc_val;
    XSetWindowAttributes        xswa;
    XSizeHints                  sizehints;      /* fallback for R3 */
    XWMHints                    wmhints;        /* fallback for R3 */
    XSizeHints                  *szhntp = NULL;
    XWMHints                    *wmhntp = NULL;
    char                        *font1 = DEF_FONT1;
    char                        *font2 = DEF_FONT2;
    char                        *font3 = DEF_FONT3;
    int                         geombits;

/*    char                        *malloc();    */
    extern void exit();

    if (x_is_open) {
	return(-1);
    }

    /*  Open the display & set defaults *********************************/
    strcpy(display_name,dpy_name);
#   ifdef VERBOSE
    (void)fprintf(stderr,"mx11init got `%s'\n\r",display_name);
    (void)fprintf(stderr,"XDispName(dpn) was `%s'\n\r",
		XDisplayName(display_name));
#   endif /* VERBOSE */
    if (*XDisplayName(display_name) == '\0') {
	(void)fprintf(stderr,"got display name `%s', but you\n\r",display_name);
	(void)fprintf(stderr,"are not running an X server, are you?  Death!\n\r");
	(void)fprintf(stderr,"  -- try setting your DISPLAY environment variable --\n\r");
	return(-1);
    }

    /*  There is a flaw in having MONGO open the X11 Server in this manner. */
    /*  In an application like Vista, Vista will also wish to open another  */
    /*  X11 Window for image display.  It is quite likely that the Vista    */
    /*  user will want both windows on the same display.  The scheme used   */
    /*  here is oblivious to that.  The likely result is that Vista will    */
    /*  end up with at least 2 separate connections to the same X Server.   */
    /*  This is not intolerable, and it is likely to be the state of things */
    /*  at least for a while.  Perhaps a solution lies in some X toolkit.   */
    /*  I rather hope to find a reasonable way around this which is simple. */
    /*  It should be simple for the sake of Vista and of other applications */
    /*  and of the people who have to write those applications.             */
    if ((dpy = XOpenDisplay(display_name)) == NULL)
	PTUV(mx11error)("Can't open display '%s'", XDisplayName(display_name));
    /* XSetCloseDownMode(dpy,RetainTemporary);  */
    screen = XDefaultScreen(dpy);
#   ifdef VERBOSE
    (void)fprintf(stderr,"mx11init opened `%s' %x\n\r",
	XDisplayName(display_name),dpy);
    (void)fprintf(stderr,"backing store:  ");
    if (DoesBackingStore(ScreenOfDisplay(dpy,screen)) == NotUseful) {
	(void)fprintf(stderr,"NotUseful\n\r");
    } else if (DoesBackingStore(ScreenOfDisplay(dpy,screen)) == WhenMapped) {
	(void)fprintf(stderr,"WhenMapped\n\r");
    } else if (DoesBackingStore(ScreenOfDisplay(dpy,screen)) == Always) {
	(void)fprintf(stderr,"Always\n\r");
    }
#   endif /* VERBOSE */
    root_win = RootWindow(dpy, screen);

    /* find out what kind of colormap we get by default */
    visual = DefaultVisual(dpy, screen);
    if (XDisplayPlanes(dpy, screen) == 1) {
	mono = True;
    }

#  ifdef FOOBAR
   if(visual->class==PseudoColor) fprintf(stderr,"PseudoColor with ");
   if(visual->class==StaticColor) fprintf(stderr,"StaticColor with ");
   if(visual->class==DirectColor) fprintf(stderr,"DirectColor with ");
   if(visual->class==TrueColor) fprintf(stderr,"TrueColor with ");
   fprintf(stderr,"%d colorcells possible\n",visual->map_entries);
   fprintf(stderr,"and %d planes\n",XDisplayPlanes(dpy, screen));
#  endif /* FOOBAR */

    /* set up for communication of events */
    fd_for_X = ConnectionNumber(dpy);
    UTUV(mx11register)(&fd_for_X,PTUV(mx11_event_loop));

    /* set up color support */
    blackpixel = BlackPixel(dpy, screen);
    whitepixel = WhitePixel(dpy, screen);
    colormap = XDefaultColormap(dpy,screen);
    xcblack.pixel = blackpixel;
    xcwhite.pixel = whitepixel;
    XQueryColor(dpy,colormap,&xcblack);
    XQueryColor(dpy,colormap,&xcwhite);

#   ifdef  X11color
    /* Install the colormap. ********************************************/
    if (mono) {  /* Monochrome display; use current colors */
	colors[0] = blackpixel;
	colors[1] = whitepixel;
    } else {
	for (i=0; i< PREDEFINED_COLORS; i++) {
	    if (!XParseColor(dpy,colormap,color_name[i],&exact_def)) {
		(void)fprintf(stderr,"color name %s not in database\n",
		color_name[i]);
	    }
	    if(!XAllocColor(dpy,colormap,&exact_def)) {
		(void)fprintf(stderr,"all colorcells allocated\n");
	    }
	    colors[i] = exact_def.pixel;
	}
	/* zero out rest of pixels */
	for(i=PREDEFINED_COLORS; i<MAX_COLORS;i++) colors[i] = 0;
    }
    current_color = 1;
#   endif  /* X11color */

    /* Get the desired font ready. **************************************/
    /* since C short-circuits, we should only load one font */
    if (     ((font_info = XLoadQueryFont(dpy,font1)) != NULL)
	  || ((font_info = XLoadQueryFont(dpy,font2)) != NULL)
	  || ((font_info = XLoadQueryFont(dpy,font3)) != NULL) ){
	lheight = font_info->ascent + font_info->descent;
    } else {
	(void) fprintf(stderr,"Can't get any X11 fonts\n\r");
	lheight = 0;
    }

    /* Figure out the geometry ******************************************/
    geombits = XGeometry(dpy,screen,ugeom,DEF_GEOM,BDR_WID,1,1,0,0,
    &des_x_open, &des_y_open, &des_wid_open, &des_hgt_open);

    /* Create 3 cursors *************************************************/
    curvis   = XCreateFontCursor(dpy, XC_crosshair);
    curinvis = XCreateGlyphCursor(dpy, font_info->fid, font_info->fid,
    (unsigned int)' ', (unsigned int)' ', &xcwhite,&xcblack);
    curprompt = XCreateFontCursor(dpy, XC_question_arrow);
    XRecolorCursor(dpy, curprompt, &xcwhite, &xcblack);

    /* Set window attributes ********************************************/
    cur_ev_mask = ExposureMask | ButtonPressMask | ColormapChangeMask |
	LeaveWindowMask | EnterWindowMask | ButtonReleaseMask ;
    xswa.event_mask = cur_ev_mask;
    xswa.background_pixel = colors[0];
    xswa.border_pixel = colors[1];
    xswa.colormap = colormap;
    xswa.cursor = curvis;
    xswa.backing_store = Always;
    xswa.save_under = 1;
    xswa.bit_gravity = NorthWestGravity;
    image_win = XCreateWindow(dpy, root_win, des_x_open, des_y_open,
	des_wid_open, des_hgt_open + lheight, BDR_WID,
	XDefaultDepth(dpy,screen),
	InputOutput, visual, CWBackPixel |CWEventMask |CWCursor |CWSaveUnder |
	CWBorderPixel |CWColormap |CWBackingStore |CWBitGravity, &xswa);
    icon_pixmap = XCreateBitmapFromData(dpy,image_win,mongoicon_bits,
    mongoicon_width, mongoicon_height);

    /* Set up the GC for the window *************************************/
    gc_val.function = GXcopy;
    gc_val.plane_mask = AllPlanes;
    gc_val.background = colors[0];
    gc_val.foreground = colors[1];
    gc_val.line_width = 1;
    /*gc_val.cap_style = CapProjecting;*/
    image_gc = XCreateGC(dpy,image_win, GCFunction | GCPlaneMask |
	GCForeground | GCBackground | GCLineWidth , &gc_val);
    text_gc = XCreateGC(dpy,image_win, GCFunction | GCPlaneMask |
	GCForeground | GCBackground | GCLineWidth , &gc_val);
    gc_val.line_width = 0;
    gc_val.foreground = colors[0] ^ colors[1];
    gc_val.function = GXxor;
    gc_val.cap_style = CapButt;
    xor_gc = XCreateGC(dpy,image_win, GCFunction | GCPlaneMask |
	GCForeground | GCBackground | GCLineWidth , &gc_val);
    if (font_info != NULL) XSetFont(dpy,image_gc,font_info->fid);
    if (font_info != NULL) XSetFont(dpy,text_gc,font_info->fid);

    /* set window manager hints *****************************************/
#   if ( XtSpecificationRelease < 4 )
    szhntp = &sizehints;
    wmhntp = &wmhints;
#   else /* ( XtSpecificationRelease < 4 ) */
    if (szhntp == NULL && ((szhntp = XAllocSizeHints()) == NULL)) {
	fprintf(stderr,"Could not allocate sizehints!\n\r");
	szhntp = &sizehints;
    }
    if (wmhntp == NULL && ((wmhntp = XAllocWMHints()) == NULL)) {
	fprintf(stderr,"Could not allocate WMhints!\n\r");
	wmhntp = &wmhints;
    }
#   endif /* ( XtSpecificationRelease < 4 ) */
    szhntp->flags = PPosition | PSize | PMinSize | PMaxSize;
    szhntp->width = des_wid_open;
    szhntp->min_width = MIN_WID;
    szhntp->max_width = MAX_WID;
    szhntp->height = des_hgt_open + lheight;
    szhntp->min_height = MIN_HGT + lheight;
    szhntp->max_height = MAX_HGT + lheight;
    owidth = wwidth = des_wid_open;
    oheight = wheight = des_hgt_open;
    wheight += lheight;
    szhntp->x = 0;
    szhntp->y = 0;
    if (geombits & (XValue | YValue)) {
	szhntp->x = des_x_open;
	szhntp->y = des_y_open;
	szhntp->flags |= (USSize | USPosition);
    }
    wmhntp->input = True;
    wmhntp->initial_state = NormalState;
    wmhntp->flags = IconPixmapHint | InputHint | StateHint;
    wmhntp->icon_pixmap = icon_pixmap;

    /* note that we do not set the argv and argc        */
    /* this is because Lick Mongo is not an X-only application (yet)  */
#   if ( XtSpecificationRelease < 4 )
    XSetStandardProperties(dpy, image_win, wind_name, icon_name,
    icon_pixmap, NULL, 0, szhntp);
    XSetWMHints(dpy, image_win, wmhntp);
#   else /* ( XtSpecificationRelease < 4 ) */
    {
	XClassHint class_hints;
	XTextProperty windTP, iconTP;

	class_hints.res_name = "LickMongo";
	class_hints.res_class = "Mongo";

	if (XStringListToTextProperty(&wind_name, 1, &windTP) == 0) {
	    fprintf(stderr,"Could not stuff windowTP\n\r");
	}
	if (XStringListToTextProperty(&icon_name, 1, &iconTP) == 0) {
	    fprintf(stderr,"Could not stuff windowTP\n\r");
	}
	XSetWMProperties(dpy, image_win, &windTP, &iconTP, NULL, 0,
	szhntp, wmhntp, &class_hints);
    }
#   endif /* ( XtSpecificationRelease < 4 ) */

    /* Select events to listen for **************************************/
    XSelectInput(dpy, image_win,
    cur_ev_mask=(ButtonPressMask | ButtonReleaseMask | ColormapChangeMask |
    ExposureMask | LeaveWindowMask | EnterWindowMask | StructureNotifyMask |
    KeyPressMask ));

    /* Map the image window. ********************************************/
    XMapWindow(dpy, image_win);

    /* Flush all the requests out to the X server ***********************/
    XFlush(dpy);

    x_is_open = True;
    /* insure that the first expose event comes along before any drawing*/
    need_Expose = True;
    (void)PTUV(mx11_event_loop)();

#   ifdef MOVIE
    /* get a buffer for drawing into */
    PTUV(mx11getbuffer)();
    UTUV(mx11direct)();
#   endif /* MOVIE */
}

/************************************************************************/
#ifdef MOVIE
/************************************************************************/
void PTUV(mx11getbuffer)()
/*
 *      ** COPYRIGHT ****  COPYRIGHT ****  COPYRIGHT ****  COPYRIGHT **
 *      This C module is Copyright (c) 1991 Philip A. Pinto
 *      The file COPYRIGHT must accompany this file.  See it for details.
 *      ** COPYRIGHT ****  COPYRIGHT ****  COPYRIGHT ****  COPYRIGHT **
 */
{
    int         nbytes;

/* if we already have a buffer, return */
    if(pixel_keep != (Pixmap) NULL ) return;

/* allocate a pixmap */
    if ( (pixel_keep=XCreatePixmap(dpy, image_win, wwidth, wheight,
    XDefaultDepth(dpy,screen)))== (Pixmap) NULL)
	PTUV(mx11error)("Can't create Pixmap image buffer.","\0");
    PTUV(mx11clearpixmap)(0);

}
/************************************************************************/
/************************************************************************/
/* enter buffered movie mode */
void UTUV(mx11movie)()
/*
 *      ** COPYRIGHT ****  COPYRIGHT ****  COPYRIGHT ****  COPYRIGHT **
 *      This C module is Copyright (c) 1991 Philip A. Pinto
 *      The file COPYRIGHT must accompany this file.  See it for details.
 *      ** COPYRIGHT ****  COPYRIGHT ****  COPYRIGHT ****  COPYRIGHT **
 */
{
	movieflag = 1;
	pixel_drw = pixel_keep;
}
/* enter unbuffered update-on-idle mode */
void UTUV(mx11direct)()
{
	if(movieflag>0) movieflag = 0;
	pixel_drw = image_win;
}
/************************************************************************/
/************************************************************************/
void UTUV(mx11mapimage)()
/*
 *      ** COPYRIGHT ****  COPYRIGHT ****  COPYRIGHT ****  COPYRIGHT **
 *      This C module is Copyright (c) 1991 Philip A. Pinto
 *      The file COPYRIGHT must accompany this file.  See it for details.
 *      ** COPYRIGHT ****  COPYRIGHT ****  COPYRIGHT ****  COPYRIGHT **
 */
{
	XCopyArea(dpy,pixel_keep,image_win,image_gc,0,0,
		  wwidth, wheight, 0,0);
	XFlush(dpy);
}
/************************************************************************/
#endif /* MOVIE */
/************************************************************************/

#define mm_PER_inch     (25.4)
void UTUV(mx11param)(lx1,lx2,ly1,ly2,xpin,ypin,cdef,cheight,cwidth,coff)
int *lx1, *lx2, *ly1, *ly2;
float *xpin, *ypin;
float *cdef, *cheight, *cwidth, *coff;
{
    *lx1 = *ly1 = 0;
    *lx2 = owidth - 1;
    *ly2 = oheight - 1;
    *xpin =
    DisplayWidth(dpy,screen) / DisplayWidthMM(dpy,screen) * mm_PER_inch;
    *ypin =
    DisplayHeight(dpy,screen) / DisplayHeightMM(dpy,screen) * mm_PER_inch;
    if (font_info) {
	int cap_height;         /* height in pixels of Capital letters  */
	int c_width;            /* width in pixels of some wide letter  */

	/* assuming that this is an ASCII font, choose cap M to inspect */
	if (font_info->per_char) {
	    cap_height =
	    font_info->per_char['M'-font_info->min_char_or_byte2].ascent;
	    c_width =
	    font_info->per_char['M'-font_info->min_char_or_byte2].width;
	} else {
	    cap_height = font_info->max_bounds.ascent;
	    c_width    = font_info->max_bounds.width;
	}

	/* see the MONGOPAR.F file if you wonder about these values     */
	*cheight = (font_info->ascent + font_info->descent);
	*cwidth = c_width;
	*coff = (cap_height + 1) / (-2.);
    }
}
/************************************************************************/
/************************************************************************/
/*  A most tragic and fatal error */
#define BELL    7
PTUV(mx11error)(s1, s2)
char *s1, *s2;   /* Error description string. */
{
//#ifndef HAVE_SYS_NERR
//    extern int errno, sys_nerr;
//    extern char *sys_errlist[];
//#endif

    (void)fprintf(stderr,"%c%s: Error =>\n%c", BELL, "MONGO", BELL);
    (void)fprintf(stderr, s1, s2);
//#ifdef __LINUX
    if (errno >0) fprintf(stderr," (%s)", strerror(errno));
//#else
//    if ((errno > 0) && (errno < sys_nerr))
//	(void)fprintf(stderr, " (%s)", sys_errlist[errno]);
//#endif
    (void)fprintf(stderr, "\n");
    exit(1);
}
/************************************************************************/
/************************************************************************/
/*      current location of pen */
int xc, yc;
/************************************************************************/
/************************************************************************/
UTUV(mx11line)(x1, y1, x2, y2)
int *x1, *y1, *x2, *y2;
{
#   ifdef  HAIRS
    if (hairs_on) PTUV(mx11nohair)();
#   endif  /* HAIRS */
    if (*x1 == *x2 && *y1 == *y2) {
	XDrawPoint(dpy,pixel_drw,image_gc,*x1,oheight - *y1);
    } else {
	XDrawLine(dpy,pixel_drw,image_gc,*x1,oheight - *y1,*x2,oheight - *y2);
    }
}
/************************************************************************/
/************************************************************************/
UTUV(mx11reloc)(xd,yd)
int *xd, *yd;
{
    xc = *xd;
    yc = *yd;
}
/************************************************************************/
/************************************************************************/
UTUV(mx11warp)(xd, yd)
int *xd, *yd;
{
    XWarpPointer(dpy,None,image_win,None,None,None,None,*xd,oheight - *yd);
}
/************************************************************************/
/************************************************************************/
UTUV(mx11draw)(xd, yd)
int *xd, *yd;
{
    int newx, newy;

#   ifdef  HAIRS
    if (hairs_on) PTUV(mx11nohair)();
#   endif  /* HAIRS */
    newx = *xd;
    newy = *yd;
    UTUV(mx11line)(&xc, &yc, &newx, &newy);
    xc = *xd;
    yc = *yd;
}
/************************************************************************/
/************************************************************************/
UTUV(mx11char)(n, str)
int *n;
#ifdef  VMS
struct dsc$descriptor_s *str;
#else   /* VMS */
char *str;
#endif  /* VMS */
{
#   ifdef  HAIRS
    if (hairs_on) PTUV(mx11nohair)();
#   endif  /* HAIRS */
#   ifdef  VMS
    XDrawString(dpy,pixel_drw,image_gc,xc,oheight-yc,str->dsc$a_pointer,*n);
#   else   /* VMS */
    (void) XDrawString(dpy,pixel_drw,image_gc,xc,oheight-yc,str,*n);
#   endif  /* VMS */
}
/************************************************************************/
/************************************************************************/
void UTUV(mx11txtwid)(n,string,slength)
int *n;
#ifdef  VMS
struct dsc$descriptor_s *string;
#else   /* VMS */
char *string;
#endif  /* VMS */
float *slength;
{
    if (font_info) {
#       ifdef  VMS
	*slength = XTextWidth( font_info, string->dsc$a_pointer, *n );
#       else   /* VMS */
	*slength = XTextWidth(font_info,string,*n);
#       endif  /* VMS */
    }
}
/************************************************************************/
/************************************************************************/
UTUV(mx11poly)(xy,nxy,fcolor)
XPoint  *xy;
int     *nxy;
int     *fcolor;
{
    XRectangle gc_rect;
    int i;

    for (i = 0; i < *nxy; i++) {
	xy[i].y = oheight - xy[i].y;
    }
    gc_rect.x      = UMGO(mongopar).GX1;
    gc_rect.y      = oheight - UMGO(mongopar).GY2;
    gc_rect.width  = UMGO(mongopar).GX2 - UMGO(mongopar).GX1;
    gc_rect.height = UMGO(mongopar).GY2 - UMGO(mongopar).GY1;
#   ifdef  HAIRS
    if (hairs_on) PTUV(mx11nohair)();
#   endif  /* HAIRS */
    XSetClipRectangles(dpy,image_gc,0,0,&gc_rect,1,YXSorted);
    XFillPolygon(dpy,pixel_drw,image_gc,xy,*nxy,Complex,CoordModeOrigin);
    XSetClipMask(dpy,image_gc,None);
}
/************************************************************************/
/************************************************************************/
UTUV(mx11rect)(xy,fcolor)
/* draws a filled rectangle */
XRectangle *xy;
int     *fcolor;
{
    XRectangle gc_rect;
    int i, f;

    xy->y = oheight - xy->y;
    gc_rect.x      = UMGO(mongopar).GX1;
    gc_rect.y      = oheight - UMGO(mongopar).GY2;
    gc_rect.width  = UMGO(mongopar).GX2 - UMGO(mongopar).GX1;
    gc_rect.height = UMGO(mongopar).GY2 - UMGO(mongopar).GY1;
#   ifdef  HAIRS
    if (hairs_on) PTUV(mx11nohair)();
#   endif  /* HAIRS */
    XSetClipRectangles(dpy,image_gc,0,0,&gc_rect,1,YXSorted);
    XDrawRectangle(dpy,pixel_drw,image_gc,xy->x,xy->y,xy->width,xy->height);
    /* it is permissible to modify the call values because this routine
       should only ever be called from the mrectang subroutine */
    if (xy->width > 1 || xy->height > 1) {
	if (xy->width  > 0) xy->width--;
	if (xy->height > 0) xy->height--;
	XFillRectangle(dpy,pixel_drw,image_gc,(xy->x)+1,(xy->y)+1,
	xy->width,xy->height);
    }
    XSetClipMask(dpy,image_gc,None);
}
/************************************************************************/
/************************************************************************/
UTUV(mx11lvis)(lv)
int *lv;
{
    if (*lv == 2) {
	/* XOR mode */
	XSetFunction(dpy,image_gc,GXxor);
	XSetForeground(dpy,image_gc,colors[0]^colors[current_color]);
    } else if (*lv == 1) {
	/* draw in background color */
	XSetFunction(dpy,image_gc,GXcopy);
	XSetForeground(dpy,image_gc,colors[0]);
    } else {
	/* draw in foreground color */
	XSetFunction(dpy,image_gc,GXcopy);
	XSetForeground(dpy,image_gc,colors[current_color]);
    }
}
/************************************************************************/
/************************************************************************/
UTUV(mx11color)(lc)
int *lc;
/*
 *      ** COPYRIGHT ****  COPYRIGHT ****  COPYRIGHT ****  COPYRIGHT **
 *      This C module is Copyright (c) 1991 Philip A. Pinto
 *      The file COPYRIGHT must accompany this file.  See it for details.
 *      ** COPYRIGHT ****  COPYRIGHT ****  COPYRIGHT ****  COPYRIGHT **
 */
{
    int ilc;

    ilc = (mono && (*lc > 1)) ? 1 : *lc; /* temporary--really want greymaps*/
#   ifdef X11color
    if (ilc<0 | ilc>MAX_COLORS-1) {
	(void)fprintf(stderr,"arguments to color must be between");
	(void)fprintf(stderr," 0 and %d\n",MAX_COLORS-1);
    } else {
	current_color = ilc;
	if (UMGO(mongopar).LVIS == 2) {
	    XSetForeground(dpy,image_gc,colors[0]^colors[ilc]);
	} else {
	    XSetForeground(dpy,image_gc,colors[ilc]);
	}
    }
#   endif /* X11color */
}
/************************************************************************/
/************************************************************************/
/* swaps colors 0 and lt, and sets background to the new color 0 */
void UTUV(mx11reverse)(lt)
int *lt;
{
    unsigned long       tmp, ilt;
    ilt = (mono && (*lt > 1)) ? 1 : *lt; /* temporary--really want greymaps*/
    /* swap the pixel values */
    tmp = colors[0];
    colors[0] = colors[ilt];
    colors[ilt] = tmp;
    /* reset the background color */
    XSetWindowBackground(dpy,image_win,colors[0]);
    XClearWindow(dpy, image_win);
    XSetBackground(dpy,image_gc,colors[0]);
    XSetBackground(dpy,xor_gc,colors[0]);
    /* turn off inverted mode */
    UMGO(mongopar).LVIS = 0;
    /* reset the foreground color */
    XSetForeground(dpy,image_gc,colors[ilt]);
    XSetForeground(dpy,xor_gc,colors[ilt]^colors[0]);
    current_color = ilt;
    /*
    XSetForeground(dpy,image_gc,colors[current_color]);
    XSetWindowBackground(dpy,image_win,colors[ilt]);
    XClearWindow(dpy, image_win);
    */
    XFlush(dpy);
}
/************************************************************************/
/************************************************************************/
/* set background color to lt w/o altering colormap */
void UTUV(mx11background)(lt)
int *lt;
/*
 *      ** COPYRIGHT ****  COPYRIGHT ****  COPYRIGHT ****  COPYRIGHT **
 *      This C module is Copyright (c) 1992 Philip A. Pinto
 *      The file COPYRIGHT must accompany this file.  See it for details.
 *      ** COPYRIGHT ****  COPYRIGHT ****  COPYRIGHT ****  COPYRIGHT **
 */
{
    unsigned long       tmp, ilt;
    ilt = (mono && (*lt > 1)) ? 1 : *lt; /* temporary--really want greymaps*/
    /* reset the background color */
    if(movieflag==0) {
	XSetWindowBackground(dpy,image_win,colors[ilt]);
	XClearWindow(dpy, image_win);
    } else {
	PTUV(mx11clearpixmap)(*lt);
    }
    XSetBackground(dpy,image_gc,colors[ilt]);
    XSetBackground(dpy,xor_gc,colors[ilt]);
    XFlush(dpy);
}
/************************************************************************/
/************************************************************************/
#define MAXINTEN        65535
#define COLFACT         65536
UTUV(mx11makecolor)(lt, red, green, blue)
int *lt;
/*
 *      ** COPYRIGHT ****  COPYRIGHT ****  COPYRIGHT ****  COPYRIGHT **
 *      This C module is Copyright (c) 1991 Philip A. Pinto
 *      The file COPYRIGHT must accompany this file.  See it for details.
 *      ** COPYRIGHT ****  COPYRIGHT ****  COPYRIGHT ****  COPYRIGHT **
 */
float *red, *green, *blue;
{
    int txc;

    if(mono) return(1);

    exact_def.red   = ((txc = *red   * COLFACT) > MAXINTEN ) ? MAXINTEN : txc;
    exact_def.green = ((txc = *green * COLFACT) > MAXINTEN ) ? MAXINTEN : txc;
    exact_def.blue  = ((txc = *blue  * COLFACT) > MAXINTEN ) ? MAXINTEN : txc;

    if(colors[*lt]!=0)
    XFreeColors(dpy, colormap,
		(unsigned long *)&colors[*lt], 1, 0);

    if(!XAllocColor(dpy,colormap,&exact_def)) {
	(void)fprintf(stderr,"cannot allocate that color\n");
    } else {
	colors[*lt] = exact_def.pixel;
	if (*lt == 0) {
	    XSetWindowBackground(dpy,image_win,colors[0]);
	    XSetBackground(dpy,image_gc,colors[0]);
	    XSetBackground(dpy,xor_gc,colors[0]);
	}
    }
}
/************************************************************************/
/************************************************************************/
UTUV(mx11eraset)(fwidth, fheight)
int *fwidth, *fheight;
{
    /* In case someone is using Lick Mongo from fortran w/o using mx11gets   */
    /* try to catch up on all the unprocessed events here.      */
    (void)PTUV(mx11_event_loop)();
#   ifdef  HAIRS
    if (hairs_on) PTUV(mx11nohair)();
#   endif  /* HAIRS */
    /* now clear the screen     */

#   ifdef MOVIE
    PTUV(mx11clearpixmap)(0);
    if (movieflag <= 0)
	  XClearWindow(dpy,pixel_drw);
#   else /*MOVIE*/
    XClearWindow(dpy,pixel_drw);
#   endif /* MOVIE */

    /* The Lick Mongo policy is to acknowledge a resize only during an erase.*/
    /* This is not required, it is just what we chose to do to maintain */
    /* consistency with the previous ISI, Sun, and Dec window drivers. */
    *fwidth = owidth = wwidth;
    *fheight = oheight = wheight - lheight;
}
/************************************************************************/
/************************************************************************/
UTUV(mx11idle)()
{
    XFlush(dpy);
}
/************************************************************************/
/************************************************************************/
UTUV(mx11sync)()
{
    XSync(dpy,False);
}
/************************************************************************/
/************************************************************************/
UTUV(mx11lwid)(w)
float *w;
{
    XGCValues                   gc_val;

    if (*w >= 0.) {
	*w = (int)(*w + 0.5);
    } else {
	*w = 0;
    }
    /* both of the following options should lead to identical results */
#   ifdef  Steve_is_stupid
    XSetLineAttributes(dpy,image_gc,(unsigned int)*w,
    LineSolid,CapButt,JoinMiter);
#   else   /* Steve_is_stupid */
    gc_val.line_width = (int)(*w);
    XChangeGC(dpy,image_gc,GCLineWidth,&gc_val);
#   endif  /* Steve_is_stupid */
}
/************************************************************************/
/************************************************************************/
UTUV(mx11close)()
{
    XFreeGC(dpy,image_gc);
    /* WARNING:  This will prove problematic when Lick Mongo is */
    /* given the capability to create more than one window.     */
    while (XCheckMaskEvent(dpy,~0,&event)) {};

#   ifdef MOVIE
    if(pixel_keep != (Pixmap) NULL) XFreePixmap(dpy,pixel_keep);
    pixel_keep = (Pixmap) NULL;
    movieflag = -1;
#   endif /* MOVIE */

    XCloseDisplay(dpy);
    UTUV(mx11unregister)(&fd_for_X);
    fd_for_X = -1;
    x_is_open = False;
}
/***********************************************************************/
/***********************************************************************/
UTUV(mx11resize)(lx1,lx2,ly1,ly2)
int *lx1, *lx2, *ly1, *ly2;
{
    XSizeHints  sizehints;
    int         nwidth, nheight;

#   ifdef  HAIRS
    if (hairs_on) PTUV(mx11nohair)();
#   endif  /* HAIRS */
    nwidth  = *lx2 - *lx1 + 1;
    nheight = *ly2 - *ly1 + 1 + lheight;

    /* constrain the window not to be bigger than the screen    */
    /* this is not required by X11, it is a Lick Mongo policy */
    if (nwidth > DisplayWidth(dpy,screen)) {
	nwidth = DisplayWidth(dpy,screen) - 3 * BDR_WID;
	sizehints.x = 0;
    } else {
	sizehints.x = des_x_open;
    }
    if (nheight > DisplayHeight(dpy,screen)) {
	nheight = DisplayHeight(dpy,screen) - 3 * BDR_WID;
	sizehints.y = 0;
    } else {
	sizehints.y = des_y_open;
    }
    /* if the window is already the right size, just return     */
    if (nwidth == wwidth && nheight == wheight) return(0);

    /* Tell the window manager about the new size       */
    sizehints.flags = PSize | PPosition | PMinSize | PMaxSize |
    USSize | USPosition;
    sizehints.min_width = MIN_WID;
    sizehints.max_width = MAX_WID;
    sizehints.min_height = MIN_HGT + lheight;
    sizehints.max_height = MAX_HGT + lheight;
    sizehints.width  = nwidth;
    sizehints.height = nheight;
    /* note that we do not set the argv and argc        */
    /* this is because probably are not useful for image display, (yet)  */
    XSetStandardProperties(dpy, image_win, "Lick Mongo", icon_name,
	    icon_pixmap, NULL, 0, &sizehints);

    /* Actually do the re-size  */
    XResizeWindow(dpy, image_win, nwidth, nheight);
    need_Config = True;
    (void)PTUV(mx11_event_loop)();
    XMapWindow(dpy, image_win);

#   ifdef MOVIE
    /* free up the space & get new space of correct size */
    if(pixel_keep != (Pixmap) NULL) XFreePixmap(dpy,pixel_keep);
    pixel_keep = (Pixmap) NULL;
    PTUV(mx11getbuffer)();  /* get new image buffer */
#   endif /* MOVIE */

}
/***********************************************************************/
/***********************************************************************/
void UTUV(mx11curs)(fikey,fix,fiy,fista,wantdn)
unsigned int *fikey;            /* pointer to place to return key */
unsigned int *fista;            /* pointer to place to return kbd state */
int *fix, *fiy;                 /* pointers to place to return event coords */
int *wantdn;                    /* nonzero if we want key down events */
				/* returned nonzero if this was a key down */
{

    /* reset local pointers to indicate return locations */
    ix = fix;
    iy = fiy;
    ikey = fikey;
    ista = fista;
    /* input and prompt strings should be cleared */
    iass = 0;
    listr = 0;
    XDefineCursor(dpy, image_win, curvis);
    /* this is almost certainly important */
    XMapWindow(dpy, image_win);
    /* this is cosmetic, but seems desireable */
    XRaiseWindow(dpy,image_win);
    /* go wait for the indicated event until appropriate key event */
    do
	(void)PTUV(mx11_event_loop)();
    while (*wantdn && iup);
    *wantdn = ! iup;
    /* we got the event, and return values have been stuffed into above locs */
    /* reset local pointer to indicate lack of return location */
    ikey = (unsigned int *)NULL;
}
/***********************************************************************/
/***********************************************************************/
void UTUV(mx11prompt)(pstring,lpstring)
char *pstring;  /* a string to use as prompt in bottom line */
int  lpstring;  /* length of the bottom line prompt */
{
    /* it is NOT assumed that pstring is null terminated */
    istr = pstring;
    listr = lpstring;

    /* this is almost certainly important */
    XMapWindow(dpy, image_win);

    /* when we are prompting, no crosshairs are drawn and cursor is question */
    if (hairs_on) PTUV(mx11nohair)();
    XDefineCursor(dpy, image_win, curprompt);

    /* clear the bottom of the window */
    iass = LSTR;
    ustrng[--iass] = '\0';
    while(iass > 0) ustrng[--iass]=' ';
    XDrawImageString(dpy,image_win,text_gc,0,
    oheight+font_info->ascent,ustrng,sizeof(ustrng));

    /* write this at the bottom of the window */
    XDrawImageString(dpy,image_win,text_gc,0,
    oheight+font_info->ascent,istr,listr);
    XFlush(dpy);

    /* if user is using this function then we know we can erase cursor */
    ux11gets = True;
}
/***********************************************************************/
/************************************************************************/
/* a little macro used for strings in the event loop */
#define bottomline()                                                    \
    if (listr || iass) {                                                \
	/* this is asynchronus string mode, concatenate  */             \
	(void)strncpy(strng,istr,listr);                                \
	(void)strncpy(strng+listr,ustrng,iass);                         \
	strng[listr+iass] = '\0';                                       \
    } else if (actstr) {                                                \
	/* the user gets to choose what string goes at bottom */        \
	(void)(*actstr)(&mwx,&mrty, &xpos,&ypos, strng,sizeof(strng));  \
    } else {                                                            \
	/* simply write out x and y coordinates */                      \
	sprintf(strng," %+-#012.5g %+-#012.5g\0",xpos, ypos);           \
    }                                                                   \
    /* write these world coordinates at the bottom of the window */     \
    XDrawImageString(dpy,image_win,text_gc,0,                           \
    oheight+font_info->ascent,strng,strlen(strng));
/************************************************************************/
/***********************************************************************/
#define ALL_X_EVENTS    (~0)
#define TRNLN 8
static int CursorUp = True;

#ifdef  HAIRS
static  XSegment    crshr[4] = {
	{-1, -1, -1, -1},
	{-1, -1, -1, -1},
	{-1, -1, -1, -1},
	{-1, -1, -1, -1},
};
static  XSegment    drag[2] = {
	{-1, -1, -1, -1},
	{-1, -1, -1, -1},
};
#endif  /* HAIRS */

PC PTUV(mx11_event_loop)()
{
    char        xcbuf[TRNLN];   /* to receive translation of key events */
    KeySym      xks;            /* to receive translation of key events */
    int         hmc;            /* how many characters long was translation?    */
    /* next clump of variables is for pointer position querying */
    Window              mroot, mchild;
    static int          mrtx, mrty;
    static int          mwx, mwy;
    static float        xpos, ypos;
    static unsigned int mkb;
    /********************************************************************/
    /* if any of the following conditions is true       */
    /*fprintf(stderr,"ikey %x\n\r",ikey);*/
    while (
    /* Should we stay in this while loop indefinitely, or return to caller? */
    (ikey != NULL || need_Config || need_Expose || butdwn) ?
    /* Stay in this while loop until key/button press or Config or Expose */
    (XWindowEvent(dpy,image_win,ALL_X_EVENTS,&event), 1) :
    /* Return to caller as soon as X event queue is emptied */
    XCheckWindowEvent(dpy,image_win,ALL_X_EVENTS,&event) ) {
	/*fprintf(stderr,"2ikey %x\n\r",ikey);*/
	iup = 0;
	switch((int)event.type) {
	case Expose:
#           ifdef VERBOSE
	    expose = (XExposeEvent *)&event;
	    (void)fprintf(stderr, "Actual expose: x%d  y%d  w%d  h%d\n",
	    expose->x, expose->y, expose->width, expose->height);
#           endif /* VERBOSE */

#           ifdef MOVIE
	    if(movieflag==0) UTUV(mx11mapimage)();
#           endif /* MOVIE */

	    need_Expose = False;
	    break;
	case KeyRelease:
	    iup = 1;
	case KeyPress:
	    hmc = XLookupString((XKeyEvent *)&event,xcbuf,TRNLN,&xks,(XComposeStatus *)NULL);
	    mwx = hmc ?         /* does this key have an ASCII translation? */
	    *xcbuf :            /* yes, stuff the first character */
	    xks;                /* no, stuff the X keysym */
	    PTUV(mx11pointerpos)(event.xbutton.x,event.xbutton.y,&xpos,&ypos);
	    mwy = oheight - event.xbutton.y;
	    /* if there is any button press function registered, call it */
	    if (event.type == KeyPress) {
	      if (actkdn) {
		    (void)(*actkdn)(&mwx, &event.xbutton.state,
		    &event.xbutton.x, &mwy, &xpos, &ypos, xcbuf);
	      } 
		if (!IsModifierKey(xks)) {
		    if (ikey != NULL) {
			/* we are supposed to return with this char now */
			*ix = event.xkey.x;
			*iy = mwy;
			*ikey = mwx;
			*ista = event.xbutton.state;
			/* fprintf(stderr,"#c %d, cbuf |%s|, ksym %d\n\r",
			hmc,xcbuf,xks);  */
			return NULL;
		    } else if (xks == XK_BackSpace || xks == XK_Delete) {
			/* remove a character from the string */
			if (iass) {
			    ustrng[iass-1] = ' ';
			    bottomline();
			    iass--;
			}
		    } else if (mwx == '\r' || mwx == '\n') {
			/* we return with the string */
			iass = 0;
			listr = 0;
			XDefineCursor(dpy, image_win, curvis);
			return ustrng;
		    } else if (iass && (mwx != '\025') && (xks != XK_Escape)) {
			/* save this char onto the string */
			ustrng[iass++] = mwx;
		    } else {
			/* the string now has or should be made zero length */
			/* explicitly clear string and begin fresh */
			iass = LSTR;
			ustrng[--iass] = '\0';
			while(iass > 0) ustrng[--iass]=' ';
			if (mwx == '\025') {
			    /* the user hit control-U = \025 */
			    /* force display of blank-filled string */
			    iass = LSTR;
			    bottomline();
			    iass = 0;
			} else if (xks == XK_Escape) {
			    /* return a nullified string */
			    listr = 0;
			    XDefineCursor(dpy, image_win, curvis);
			    return ustrng;
			} else {
			    /* normal addition of the first character */
			    ustrng[iass++] = mwx;
			}
		    }
		} /* if(!IsModifierKey) */
	    } else /* event.type == KeyRelease */ {
	      /* for the present, there are no keyup actions */
	      /* there are buttonup actions, though */
	      /*if (actbup) {
		    (void)(*actbup)(&mwx, &event.xbutton.state,
		    &event.xbutton.x, &mwy, &xpos, &ypos);
		}  */
	    }
	    bottomline();
	    break;
	case ButtonRelease:
	    iup = 1;
	case ButtonPress:
	    iass = 0;
	    if (event.type == ButtonRelease) {
		butdwn &= ~(1 << event.xbutton.button);
		/* if there is a button release function registered, call it */
		if (actbup) {
		    PTUV(mx11pointerpos)(event.xbutton.x,event.xbutton.y,
		    &xpos,&ypos);
		    mwx = -event.xbutton.button;
		    mwy = oheight - event.xbutton.y;
		    (void)(*actbup)(&mwx, &event.xbutton.state,
		    &event.xbutton.x, &mwy, &xpos, &ypos);
		}
	    } else /* event.type == ButtonPress */ {
		/* if there is a button press function registered, call it */
		if (actbdn) {
		    PTUV(mx11pointerpos)(event.xbutton.x,event.xbutton.y,
		    &xpos,&ypos);
		    mwx = -event.xbutton.button;
		    mwy = oheight - event.xbutton.y;
		    (void)(*actbdn)(&mwx, &event.xbutton.state,
		    &event.xbutton.x, &mwy, &xpos, &ypos);
		}
		butdwn |= (1 << event.xbutton.button);
	    }
	    if (ikey != NULL) {
		/* we have been waiting for this before we could return */
		*ikey = -event.xbutton.button;
		*ista =  event.xbutton.state;
		*ix   =  event.xbutton.x;
		*iy   =  oheight - event.xbutton.y;
		return NULL;
	    }
	    break;
	case LeaveNotify:
#           define TRACKIT
#           ifdef TRACKIT
	    /* fprintf(stderr,"leavenotify event\n\r"); */
	    cur_ev_mask ^= (PointerMotionMask | PointerMotionHintMask);
	    XSelectInput(dpy,image_win,cur_ev_mask);
#           endif /* TRACKIT */
#           ifdef HAIRS
	    if (hairs_on) PTUV(mx11nohair)();
	    if (! CursorUp) {
		/* When mx11gets is being used, we null out the cursor. */
		/* Cursor is needed when we leave the window during drag. */
		XDefineCursor(dpy, image_win, curvis);
		CursorUp = True;
	    }
#           endif /* HAIRS */
	    break;
	case EnterNotify:
#           ifdef TRACKIT
	    /*  fprintf(stderr,"enternotify event\n\r");    */
	    cur_ev_mask |= (PointerMotionMask | PointerMotionHintMask);
	    XSelectInput(dpy,image_win,cur_ev_mask);
#           endif /* TRACKIT */
#           ifdef HAIRS
	    if (ux11gets && CursorUp) {
		if (listr) {
		    /* If we are prompting, we want cursor to show it */
		    XDefineCursor(dpy, image_win, curprompt);
		} else {
		    /* When mx11gets is being used, we null out the cursor. */
		    /* Cursor is not needed when we are drawing crosshairs. */
		    XDefineCursor(dpy, image_win, curinvis);
		    CursorUp = False;
		}
	    }
#           endif /* HAIRS */
	    break;
#       ifdef TRACKIT
	case MotionNotify:
	    /* fprintf(stderr,"motionnotify event\n\r"); */
#           define JUNK /* I did not expect this to work or be useful. */
#           ifdef JUNK  /* I was wrong.  It is extremely useful. */
	    if (butdwn) {
		/* consume all other button down events */
		while (XCheckMaskEvent(dpy,ButtonMotionMask,&event)) {
#                   ifdef VERBOSE
		    fprintf(stderr,"|");
#                   endif /* VERBOSE */
		}
	    } else {
		/* consume all other pending pointermotion events */
		while (XCheckTypedWindowEvent(dpy,image_win,MotionNotify,
		&event)) {
#                   ifdef VERBOSE
		    fprintf(stderr,"|");
#                   endif /* VERBOSE */
		}
	    }
	    /* find out where the pointer is now        */
	    if (!XQueryPointer(dpy,image_win,&mroot,&mchild,
	    &mrtx,&mrty,&mwx,&mwy,&mkb)) {
		/* pointer is inside window but not on screen! */
		/* this may happen during the passive grab which the server */
		/* does when a button is dragged */
		fprintf(stderr,"Pointer in window but off screen\n\r");
		/* if it does happen, there is no reasonable action, so */
		break;
	    }
	    /* determine the world coordinates of the cursor */
	    PTUV(mx11pointerpos)(mwx, mwy, &xpos, &ypos);
	    mrty = oheight - mwy;
	    /* call any actions that need to be performed */
	    if (actmot) (void)(*actmot)(&mwx, &mrty, &xpos, &ypos);
	    if (actbmv && butdwn) {
		/* perform the drag procedure defined by the user */
		(void)(*actbmv)(&mwx, &mrty, &xpos, &ypos);
	    } else {
#               ifdef HAIRS
		/* default action is to draw crosshairs across entire window */
		crshr[0] = crshr[2];
		crshr[1] = crshr[3];
		crshr[2].x1 = 0;
		crshr[2].x2 = wwidth;
		crshr[2].y1 = crshr[2].y2 = mwy;
		crshr[3].y1 = 0;
		crshr[3].y2 = wheight;
		crshr[3].x1 = crshr[3].x2 = mwx;
		if (hairs_on) {
		    /* erase the old hairs, and draw new hairs */
		    XDrawSegments(dpy,image_win,xor_gc,crshr,4);
		} else if (!listr) {
		    /* only draw the new hairs */
		    XDrawSegments(dpy,image_win,xor_gc,crshr+2,2);
		    hairs_on = True;
		}
#               endif /* HAIRS */
	    }
	    bottomline();
#           endif /* JUNK */
	    break;
#       endif /* TRACKIT */
	case ConfigureNotify:
	    /* window has been resized by the window manager    */
	    wwidth  = event.xconfigure.width;
	    wheight = event.xconfigure.height;
#           ifdef VERBOSE
	    fprintf(stderr,"configurenotify event: wwidth %d wheight %d\n\r",
	    wwidth,wheight);
#           endif /* VERBOSE */
	    need_Config = False;
	    break;
	default:
#           ifdef VERBOSE
	    (void)fprintf(stderr,"Unprocessed X event.\n");
#           endif /* VERBOSE */
	    break;
	} /* switch((int)event.type) */
    } /* end of the large while loop of event processing */
    return NULL;
}
/************************************************************************/
/************************************************************************/
#ifdef  HAIRS
PTUV(mx11nohair)()
{
    crshr[0] = crshr[2];
    crshr[1] = crshr[3];
    crshr[2].x1 = crshr[2].x2 = crshr[2].y1 = crshr[2].y2 = -1;
    crshr[3].y1 = crshr[3].y2 = crshr[3].x1 = crshr[3].x2 = -1;
    XDrawSegments(dpy,image_win,xor_gc,crshr,2);
    hairs_on = False;
}
#endif  /* HAIRS */
/************************************************************************/
/************************************************************************/
void PTUV(mx11pointerpos)(mwx,mwy, xpos, ypos)
int mwx, mwy;
float  *xpos, *ypos;
{
	float tmp;
		    /* determine, via MONGOPAR, the user coordinates */
/*
		    *xpos =( (mwx - UMGO(mongopar).GX1)*
                    (UMGO(mongopar).X2 - UMGO(mongopar).X1) / 
                    (UMGO(mongopar).GX2 - UMGO(mongopar).GX1) ) + 
                    UMGO(mongopar).X1;
*/
		    tmp =( (mwx - UMGO(mongopar).GX1)*
                    (UMGO(mongopar).X2 - UMGO(mongopar).X1) / 
                    (UMGO(mongopar).GX2 - UMGO(mongopar).GX1) ) + 
                    UMGO(mongopar).X1;
	            *xpos = tmp;
/*
	printf("mwx: %d gx1: %f gx2: %f x1: %f x2: %f xpos: %f %f %d\n",mwx, 
UMGO(mongopar).GX1, UMGO(mongopar).GX2,
UMGO(mongopar).X1, UMGO(mongopar).X2, *xpos, tmp, xpos);
 	printf("%f %f %f %f\n",(mwx - UMGO(mongopar).GX1),
                    (UMGO(mongopar).X2 - UMGO(mongopar).X1),
                    (UMGO(mongopar).GX2 - UMGO(mongopar).GX1),
                    UMGO(mongopar).X1);
	printf("mwx: %d gx1: %f gx2: %f x1: %f x2: %f xpos: %f %f %d\n",mwx, 
UMGO(mongopar).GX1, UMGO(mongopar).GX2,
UMGO(mongopar).X1, UMGO(mongopar).X2, *xpos, tmp, xpos);
*/
		    *ypos = ((oheight-mwy) - UMGO(mongopar).GY1)*
		    (UMGO(mongopar).Y2 - UMGO(mongopar).Y1)/
		    (UMGO(mongopar).GY2 -
		    UMGO(mongopar).GY1) + UMGO(mongopar).Y1;
}
/************************************************************************/
/************************************************************************/
#ifdef MOVIE
void PTUV(mx11clearpixmap)(lt)
int lt;
/*
 *      ** COPYRIGHT ****  COPYRIGHT ****  COPYRIGHT ****  COPYRIGHT **
 *      This C module is Copyright (c) 1991 Philip A. Pinto
 *      The file COPYRIGHT must accompany this file.  See it for details.
 *      ** COPYRIGHT ****  COPYRIGHT ****  COPYRIGHT ****  COPYRIGHT **
 */
{
    XSetForeground(dpy,image_gc,colors[lt]);
    XFillRectangle(dpy, pixel_keep,image_gc,0,0,wwidth,wheight);
    XSetForeground(dpy,image_gc,colors[1]);
}
#endif /* MOVIE */
/************************************************************************/
/************************************************************************/
/* define a macro which replicates X11 event action functions */
#define ACTREG(funcname,vactptr)                                        \
PFRV funcname(activate, function)                                       \
int     *activate;      /* see case statements for details */           \
int     *function;      /* possible new X11 event action */             \
{                                                                       \
    oldact = vactptr;                                                   \
    switch (*activate) {                                                \
    case 1:                                                             \
	/* this is used when calling from C or Fortran */               \
	/* From C, pass a pointer to a function */                      \
	/* From Fortran, pass a function declared as "external" */      \
	vactptr = (PFRV)function;                                       \
	break;                                                          \
    case -1:                                                            \
	/* If from Fortran you have saved the "original" func */        \
	/* and you want to restore it, you must use "-1" */             \
	/* This is because FORTRAN 77 cannot conceive of a */           \
	/* "pointer to function". */                                    \
	vactptr = (PFRV)(*function);                                    \
	break;                                                          \
    case 0:                                                             \
    default:                                                            \
	/* if activate is 0 or none of above, we disable the */         \
	/* function by setting its pointer to NULL */                   \
	vactptr = NULL;                                                 \
    }                                                                   \
    return oldact;                                                      \
}
/* now use the above macro to create funcs (you can hate me if you like--SLA) */

ACTREG(UTUV(regkdn),actkdn)   /* keypress action */

ACTREG(UTUV(regbdn),actbdn)   /* buttondown action */

ACTREG(UTUV(regbup),actbup)   /* buttonup   action */

ACTREG(UTUV(regbmv),actbmv)   /* buttondrag action */

ACTREG(UTUV(regmot),actmot)   /* motion     action */

ACTREG(UTUV(regstr),actstr)   /* motion     string */

/************************************************************************/
/************************************************************************/
#define SILLY -1.e30
void UTUV(philbut)(b, s, dx, dy, wx, wy)
unsigned int    *b;             /* -(event.xbutton.button) */
unsigned int    *s;             /*   event.xbutton.state  */
int             *dx, *dy;       /* Lick Mongo device coordinates */
float           *wx, *wy;       /* Lick Mongo world coordinates */
{
    static int  Lv_Erase = 1;
    int         slv;
    static int  kosher = 0;     /* is it ok to undraw? */
    static float dxl = SILLY, dyl = SILLY;

    switch(*b) {
    case -Button1:
#       ifdef VERBOSE
	(void)fprintf(stderr,"(x,y): %d %d\n", *wx, *wy);
#       endif /* VERBOSE */
	break;
    case -Button2:
	/* Rick Pogge added this in like fashion to Phil Pinto's code below */
	kosher = kosher && (*s & ControlMask);
	if (kosher) {
	    /* undraw the last segment, roughly, but so what */
	    /* Any further action along this line requires a complete */
	    /* rethink of the parser and command history mechanism. */
	    UMGO(getlvis)(&slv);        /* save graphic state */
	    UMGO(setlvis)(&Lv_Erase);
	    UMGO(gdraw)(&dxl,&dyl);     /* undraw previous segment */
	    UMGO(setlvis)(&slv);        /* back to old graphic state */
	    UTUV(delcom)(" ",&slv,1);   /* remove com from combuffer */
	} else {
	    /* save last position */
	    dxl = UMGO(mongopar).XP;
	    dyl = UMGO(mongopar).YP;
	    /* draw to the current pointer position */
	    UMGO(draw)(wx,wy);
	    /* save in the command buffer - caveats apply */
	    sprintf(strng,"draw %f %f \0", *wx,*wy);
	    UTUV(saveline)(strng,&slv,strlen(strng));
	    /* print to stderr in command window as if typed - caveats apply */
	    (void)fprintf(stderr, "%s\n * ", strng);
	}
	kosher = !kosher;
	break;
    case -Button3:
	/* Phil Pinto invented this functionality */
	/* relocate the current position to the pointer */
	UMGO(relocate)(wx,wy);
	/* save the command to the buffer */
	/* note the consequences of this, all of the Lick Mongo */
	/* command history must now be linked in with any */
	/* program.  This may be a very bad thing to do. */
	sprintf(strng,"relocate %f %f \0",*wx,*wy);
	UTUV(saveline)(strng,&slv,strlen(strng));

	/* write a relocate command to the screen as if typed */
	/* note the consequences of this, some programs calling */
	/* the Lick Mongo graphics subroutines may not even have */
	/* a Standard Error to write at, this may be very bad */
	(void)fprintf(stderr, "%s\n * ", strng);
	break;
    } /* switch(b) */
}
/************************************************************************/
/************************************************************************/
#ifndef __MATH__
#    include <math.h>
#else
#    include <values.h>
#endif
#define Pi 3.14159265358979

void UTUV(slastr)(dx, dy, wx, wy, str, lstr)
int     *dx, *dy;
float   *wx, *wy;
char    *str;
int     lstr;
{
    float r, thet;

    r = sqrt(*wx * *wx + *wy * *wy);
    thet = (*wx == 0. && *wy == 0.) ? 0. : atan2(*wy, *wx)*(180./Pi);
    sprintf(strng," %+-#012.5g %+-#012.5g %+-#012.5g %+-#012.5g\0",
    *wx, *wy, r, thet);

}
/************************************************************************/
/************************************************************************/

#define ICON_STRN_LEN 128
char win_name[ICON_STRN_LEN], icn_name[ICON_STRN_LEN];

void ATUS(tuvwinname)(window_name, icon_name, len1, len2)
    char *window_name, *icon_name;
    int len1, len2;
/*
 *      ** COPYRIGHT ****  COPYRIGHT ****  COPYRIGHT ****  COPYRIGHT **
 *      This C module is Copyright (c) 1992 Philip A. Pinto
 *      The file COPYRIGHT must accompany this file.  See it for details.
 *      ** COPYRIGHT ****  COPYRIGHT ****  COPYRIGHT ****  COPYRIGHT **
 */
{
    int i;

   if(len1-1>ICON_STRN_LEN || len2-1>ICON_STRN_LEN) {
	PTUV(mx11error)("argument string too long in call to winname","\0");
	exit(1);
	}

/* null-terminate f77 strings by copying them into local storage*/
   for(i=len1-1; i > 0 && window_name[i]==' '; i--);
   (void) strncpy(win_name,window_name,i+1);

/* if window_name is longer than i (always true since it is blank-terminated),
   strncpy will not null-terminate; we do so here */
   if(i>=ICON_STRN_LEN)
     win_name[ICON_STRN_LEN] = '\0';
   else {
     if(i==0)
       win_name[ICON_STRN_LEN] = '\0';
     else
       win_name[i+1] = '\0';
   }

   for(i=len2-1; i > 0 && icon_name[i]==' '; i--);
   (void) strncpy(icn_name,icon_name,i+1);
   if(i>=ICON_STRN_LEN)
     icn_name[ICON_STRN_LEN] = '\0';
   else {
     if(i==0)
       icn_name[ICON_STRN_LEN] = '\0';
     else
       icn_name[i+1] = '\0';
   }


/* set the window names */
    XSetStandardProperties(dpy, image_win, win_name, icn_name,
	icon_pixmap, NULL, 0, 0);

    XFlush(dpy);

    fprintf(stderr,"Window name set to: %s\n",win_name);
}
/************************************************************************/
/************************************************************************/
iconify_window()
{
  fprintf(stderr,"iconify returns: %s\n",XIconifyWindow(dpy,image_win,screen));
}

