/** iP6: PC-6000/6600 series emualtor ************************/
/**                                                         **/
/**                           Unix.c                        **/
/**                                                         **/
/** Modified by Windy 2002-2003                             **/
/** This code is written by ISHIOKA Hiroshi 1998-2000       **/
/** This code is based on fMSX written by Marat Fayzullin   **/
/*************************************************************/
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef X11

#ifdef UNIX
/** Standard Unix/X #includes ********************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <signal.h>
#include <sys/time.h>
#include <unistd.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/keysym.h>
/** MIT Shared Memory Extension for X ************************/
#ifdef MITSHM
#include <sys/ipc.h>
#include <sys/shm.h>
#include <X11/extensions/XShm.h>
#endif

#include <locale.h>


#include "../P6.h"
#include "../Video.h"

#include "../Sound.h"
#include "../Refresh.h"
#include "Unix.h"
#include "Xconf.h"
#include "../Build.h"
#include "../keys.h"
#include "../mem.h"
#include "../Timer.h"

/** MIT Shared Memory Extension for X ************************/
#ifdef MITSHM

XShmSegmentInfo SHMInfo;
int UseSHM=1;
/** The following is used for MITSHM-auto-detection: **/
int ShmOpcode;       /* major opcode for MIT-SHM */
int (*OldHandler)();
#endif


/****************************************************************/
/*** These dummy functions are called when writes to sound    ***/
/*** registers occur. Replace them with the actual functions  ***/
/*** generating sound.                                        ***/
/****************************************************************/
#ifndef SOUND
void PSGOut(register byte R,register byte V)  { }
void SCCOut(register byte R,register byte V)  { }
#endif

//char *Title=PROGRAM_NAME" Unix/X  "BUILD_NO; // add 2002/4/3 by Windy
extern char * Title;
static char TitleBuf[256];

/** Variables to pass options to the X-drivers ***************/
int SaveCPU=1;
int do_install, do_sync;
int scale = 2;

/** X-related definitions ************************************/
#define KeyMask KeyPressMask|KeyReleaseMask
#define OtherMask FocusChangeMask|ExposureMask|StructureNotifyMask | ButtonPressMask
#define MyEventMask KeyMask|OtherMask

/** Various X-related variables ******************************/
char *Dispname=NULL;
Display *Dsp;
Window Wnd;
Atom DELETE_WINDOW_atom, WM_PROTOCOLS_atom;
Colormap CMap;
int CMapIsMine=0;
GC DefaultGC;

XImage *ximage;

Pixmap pixmap;

static unsigned long foreground_color;		// foreground color

/* variables related to colors */
XVisualInfo VisInfo;
long VisInfo_mask=VisualNoMask;
byte Xtab[4];
XID white,black;


int Mapped; /* flags */

void InitColor(int);
/*void TrashColor(void);*/

/* size of window */
Bool wide,high;
// int Width,Height; /* this is ximage size. window size is +20 */

/** Various variables and short functions ********************/


static long psec=0, pusec=0;

void OnBreak(int Arg) { CPURunning=0; }

void unixIdle(void)
{
	long csec, cusec;
	long l;
	struct timeval tv;

	gettimeofday(&tv, NULL);
	csec=tv.tv_sec; cusec=tv.tv_usec;
	l=(1000000.f/60.f)-((csec-psec)*1000000+(cusec-pusec));
	if(l>0) usleep(l);
	psec=csec; pusec=cusec;
	return;
}



int OSD_GetModulePath(char *path , int max)
{
    getcwd( path , max);
}


/** TrashMachine *********************************************/
/** Deallocate all resources taken by InitMachine().        **/
/*************************************************************/
void TrashMachine(void)
{

	/*TrashColor();*/
	if(Dsp&&Wnd)
		{
		 OSD_ReleaseSurface( surface1);
		 OSD_ReleaseSurface( surface2);
		 OSD_ReleaseSurface( screen);
		}
	if(CMapIsMine) XFreeColormap(Dsp, CMap);
	if(Dsp) XCloseDisplay(Dsp);

#ifdef SOUND
	StopSound();
	TrashSound();
#endif //SOUND
	if(Verbose) printf("Shutting down...\n");
}


static struct vc {
    int clasnum;
    char *name;
} visualClasses[6]=
  {
      { StaticGray, "StaticGray"},
      { GrayScale, "GrayScale"},
      { StaticColor, "StaticColor"},
      { PseudoColor, "PseudoColor"},
      { TrueColor, "TrueColor"},
      { DirectColor, "DirectColor"}
  };

void SetDepth(int x)
{
    VisInfo.depth=x;
    VisInfo_mask |= VisualDepthMask;
}

void SetScreen(int x)
{
    VisInfo.screen=x;
    VisInfo_mask |= VisualScreenMask;
}

void SetVisual(char *s)
{
    int i;
    char *vc, *sp;

    for (i=0; i<6; i++) {
        for (  vc=visualClasses[i].name, sp=s ; (*vc!=0) ; vc++, sp++ )
	    if (tolower(*vc)!=tolower(*sp))
		goto next;
	VisInfo.class=visualClasses[i].clasnum;
	VisInfo_mask |= VisualClassMask;
	return;
      next: ;
    }
    printf("%s is not a known visual class.\n",s);
}

void SetVisualId(long x)
{
    VisInfo.visualid=(VisualID) x;
    VisInfo_mask |= VisualIDMask;
}

/* Function to choose a PixMapFormat and set some variables associated
   with them.  Note that the depth comes from a visual and is
   therefore supported by the screen */
int ChooseFormat(void)
{
	int i;
	int nForm; /* # pixmapformats, depth */
	XPixmapFormatValues *flist, choice; /* list of format describers
					  and choice */
	ColTyp T;
	char *order;

	flist=XListPixmapFormats(Dsp,&nForm);
	if (!flist) return (0); 

	choice.bits_per_pixel=choice.depth=0;
	for (i=0;i<nForm;i++)
	    {
	    if (flist[i].depth==VisInfo.depth)
		choice=flist[i];
    	}

	bitpix=choice.bits_per_pixel; 

	XFree((void *)flist);
	if (!bitpix) {
    	if (Verbose &1) printf("FAILED\n"); 
    	return (0);
  		}

	order="";
	if (bitpix>8)
		{
    	order=(ImageByteOrder(Dsp)==LSBFirst) ? "LSB first" : "MSB first";
    	/* how to swap the bytes ? */
    	T.ct_xid = (ImageByteOrder(Dsp)==LSBFirst)? 0x03020100L : 0x00010203L ;
    	if (ImageByteOrder(Dsp)==MSBFirst)
		for (i=bitpix ; i<32 ; i+=8)
		    T.ct_xid=(T.ct_xid>>8)+((T.ct_xid&0xFF)<<24);
    	/* if T were represented by the bytes 0 1 2 3 
    	   then no swapping would be needed.. */
		for (i=0;i<4;i++) 
    		Xtab[T.ct_byte[i]]=i;
		/*  Xtab[j]==i means that byte i in the representation of ColTyp used
	    by the machine on which fmsx is running, corresponds to byte j in
    	the representation of ColTyp in the XImage 
		*/
  		}
	if (bitpix<8)
	    order=(lsbfirst=(BitmapBitOrder(Dsp)==LSBFirst)) 
		? "lsb first" : "msb first"; 
	  if (Verbose & 1) 
	      printf("depth=%d, %d bit%s per pixel (%s)\n", VisInfo.depth,bitpix,
		     (bitpix==1)?"":"s",order);
	  choosefuncs(lsbfirst,bitpix);
	  /*setwidth(wide);*/
      setwidth(scale-1);
      

	return 1;
}

int ChooseVisual(void)
{
    int n_items,i;
    XVisualInfo *VisInfo_l;

    if (Verbose&1) printf("OK\n  Choosing Visual...");

    if (VisInfo_mask!=VisualNoMask) {
	VisInfo_l=XGetVisualInfo(Dsp, VisInfo_mask, &VisInfo, &n_items);
	if (n_items!=0) {
	    if ( (Verbose&1) && (n_items>1)) 
		printf("%d matching visuals...",n_items);
	    VisInfo=VisInfo_l[0]; /* this may not be the best match */
	    XFree(VisInfo_l);
	    goto found;
	} else 
	    if (Verbose&1)
		printf("no matching visual...");
    }

    if (Verbose&1)
	printf("using default...");

    VisInfo.screen= DefaultScreen(Dsp);
    VisInfo.visualid=DefaultVisual(Dsp, VisInfo.screen)->visualid;
    VisInfo_l=XGetVisualInfo(Dsp, VisualIDMask|VisualScreenMask, 
			     &VisInfo, &n_items);
    if (n_items!=0) {
	VisInfo=VisInfo_l[0]; /* this is probably the only match */
	XFree(VisInfo_l);
	goto found;
    }

    if (Verbose&1)
	printf("FAILED\n");
    return 0;

    found:
    for (i=6 ; i ; )
	if (visualClasses[--i].clasnum==VisInfo.class)
	    break;

    if (Verbose&1) {
	printf("\n    visualid=0x%lx: %s, ", VisInfo.visualid, 
	       (i>=0) ? visualClasses[i].name : "unknown");
    }

    return 1;
}

#ifdef MITSHM
/* Error handler, used only to trap BadAccess errors on XShmAttach() */
int Handler(Display * d, XErrorEvent * Ev)
{ 
	if ( (Ev->error_code==10) /* BadAccess */ && 
    	 (Ev->request_code==(unsigned char) ShmOpcode) &&
    	 (Ev->minor_code==1)  /* X_ShmAttach */ )
    	UseSHM=0; /* X-terminal apparently remote */
  	else return OldHandler(d,Ev);
  	return 0;
}

#endif

/*  Inform window manager about the window size, 
    attributes and other hints
**/
void SetWMHints(void) 
{
    XTextProperty win_name,icon_name;
    char * value[1];
    XSizeHints *size;
    XWMHints *hint;
    XClassHint *klas;
    char *argv[]= {NULL};
    int argc=0;
    
    size=XAllocSizeHints();
    hint=XAllocWMHints();
    klas=XAllocClassHint();
    
    size->flags=PSize|PMinSize|PMaxSize;
    size->min_width=size->max_width=size->base_width=Width+ +PADDINGW+BORDERW;
    size->min_height=size->max_height=size->base_height=Height+ +PADDINGH+BORDERH;
    
    hint->input=True;
    hint->flags=InputHint;
    
    /*value[0]=Title;*/
    value[0]=TitleBuf;
    XStringListToTextProperty(value, 1, &win_name);
    value[0]="iP6";
    XStringListToTextProperty(value, 1, &icon_name);
    
    klas->res_name="iP6";
    klas->res_class="IP6";
    
    XSetWMProperties(Dsp, Wnd, &win_name, &icon_name, argv, argc,
		     size, hint, klas);
    XFree(size); 
    XFree(hint);
    XFree(klas);

    WM_PROTOCOLS_atom =XInternAtom(Dsp, "WM_PROTOCOLS", False);
    DELETE_WINDOW_atom=XInternAtom(Dsp, "WM_DELETE_WINDOW", False);

    int stat = XChangeProperty(Dsp, Wnd, WM_PROTOCOLS_atom , XA_ATOM, 32,
		    PropModeReplace, (unsigned char*) &DELETE_WINDOW_atom,1);

	//printf("Protocols stat=%d \n",stat);
	//stat = XSetWMProtocols(Dsp, Wnd, &DELETE_WINDOW_atom,1);
	//printf("Protocols stat=%d \n",stat);
}

int screen_num;

/** InitMachine **********************************************/
/** Allocate resources needed by Unix/X-dependent code.     **/
/*************************************************************/
int InitMachine(void)
{
	int K,L;
	int depth ;  /*, screen_num; */
	Window root;

	signal(SIGINT,OnBreak);
	signal(SIGHUP,OnBreak);
	signal(SIGQUIT,OnBreak);
	signal(SIGTERM,OnBreak);

	OSD_InitTimer();

#ifdef SOUND
//	if(UseSound) InitSound();
#endif //SOUND
	Width=M6WIDTH*scale;
	Height=M6HEIGHT*scale;
	if(Verbose)
	    printf("Initializing Unix/X drivers:\n  Opening display...");
	Dsp=XOpenDisplay(Dispname);

	if(!Dsp) { if(Verbose) printf("FAILED\n");return(0); }
	if (do_sync) XSynchronize(Dsp, True);

	if (!ChooseVisual()) 
      	return 0;
	screen_num=VisInfo.screen;  depth=VisInfo.depth;
	root=RootWindow(Dsp, screen_num);

  	if (!ChooseFormat()) 
      return 0;

	white=XWhitePixel(Dsp,screen_num);
	black=XBlackPixel(Dsp,screen_num);
	DefaultGC=DefaultGC(Dsp,screen_num);
	if ( do_install 
         ||(VisInfo.visualid != DefaultVisual(Dsp,screen_num)->visualid)) {
      		CMap=XCreateColormap(Dsp, root, VisInfo.visual, AllocNone);
      		CMapIsMine=1;
		 } 
	else CMap=DefaultColormap(Dsp,screen_num);

	if (Verbose & 1) 
	      printf("  Using %s colormap (0x%lx)\n", 
		  CMapIsMine?"private":"default", CMap);

		{
    	XSetWindowAttributes wat;
    
    	if(Verbose & 1) printf("  Opening window...");
    
    	wat.colormap=CMap;
    	wat.event_mask=MyEventMask;
    	wat.background_pixel=0;
/*      wat.background_pixmap=None; */
    	wat.border_pixel=0;
/*      wat.border_pixmap=None; */

    	Wnd=XCreateWindow(Dsp,root,0,0,Width+PADDINGW+BORDERW  ,Height+PADDINGH+BORDERH ,0,depth,InputOutput,
		      VisInfo.visual, 
		      CWColormap|CWEventMask|CWBackPixel|CWBorderPixel, &wat);
  		}
	strcpy(TitleBuf, Title); /* added */
/*	SetWMHints(); */       /* moved to OSD_setwindowsurface */
	if(!Wnd) { if(Verbose & 1) printf("FAILED\n");return(0); }
	if(Verbose & 1) printf("OK\n");

	InitColor(screen_num); 

	XMapRaised(Dsp,Wnd);
	XClearWindow(Dsp,Wnd);
#if 0
	screen = OSD_setwindowsurface(Width ,Height ,depth ,WINDOW_SOFTWARE);
	surface1 = OSD_CreateSurface(Width ,Height ,depth ,SURFACE_BITMAP);
	surface2 = OSD_CreateSurface(Width ,Height ,depth ,SURFACE_OFFSCREEN);
	status_surface = OSD_CreateSurface( STATUSBAR_WIDTH , STATUSBAR_HEIGHT , depth , SURFACE_BITMAP);

	if( !screen || !surface1 || !surface2 ) {printf("\nCreateSurface  failed \n"); return 0;}
	//XBuf = surface1->pixels;
#endif
    resizewindow((float)scale,(float)scale,0);
    
    setRefreshSurface( surface1);


	if(Verbose & 1) printf("OK\n");


  	build_conf();

  		{
		struct timeval tv;
    	gettimeofday(&tv,NULL);
    	psec=tv.tv_sec;pusec=tv.tv_usec;
  		}
  //Xt_init_menu();   // setup menu

	return(1);
}



int x11_keymap[65536];

// ****************************************************************************
//           init OS keymap 
// ****************************************************************************
void OSD_initKeymap(void)
{
	int i;
	for(i=0; i<65535; i++)
		x11_keymap[i] = OSDK_UNKNOWN;

	x11_keymap[ XK_BackSpace] =       OSDK_BACKSPACE;	
	x11_keymap[ XK_Tab] =        OSDK_TAB; 		
	x11_keymap[ XK_Clear] =      OSDK_CLEAR;		
	x11_keymap[ XK_Return] =     OSDK_RETURN;	
	x11_keymap[ XK_Pause] =      OSDK_PAUSE;		
	x11_keymap[ XK_Escape] =     OSDK_ESCAPE;	
	x11_keymap[ XK_space] =      OSDK_SPACE;		
	x11_keymap[ XK_comma] =      OSDK_COMMA;		
	x11_keymap[ XK_minus] =      OSDK_MINUS;		
	x11_keymap[ XK_period] =     OSDK_PERIOD;	
	x11_keymap[ XK_slash] =      OSDK_SLASH;	
	x11_keymap[ XK_0] =          OSDK_0;                   
	x11_keymap[ XK_1] =          OSDK_1;
	x11_keymap[ XK_2] =          OSDK_2;
	x11_keymap[ XK_3] =          OSDK_3;
	x11_keymap[ XK_4] =          OSDK_4;
	x11_keymap[ XK_5] =          OSDK_5;
	x11_keymap[ XK_6] =          OSDK_6;
	x11_keymap[ XK_7] =          OSDK_7;
	x11_keymap[ XK_8] =          OSDK_8;
	x11_keymap[ XK_9] =          OSDK_9;
	x11_keymap[ XK_colon] =      OSDK_COLON ;			// :
	x11_keymap[ XK_semicolon] =  OSDK_SEMICOLON ;		// ;
	x11_keymap[ XK_backslash] =  OSDK_BACKSLASH ;		// ･

	x11_keymap[ XK_a] =   OSDK_A;
	x11_keymap[ XK_b] =   OSDK_B;
	x11_keymap[ XK_c] =   OSDK_C;
	x11_keymap[ XK_d] =   OSDK_D;
	x11_keymap[ XK_e] =   OSDK_E;
	x11_keymap[ XK_f] =   OSDK_F;
	x11_keymap[ XK_g] =   OSDK_G;
	x11_keymap[ XK_h] =   OSDK_H;
	x11_keymap[ XK_i] =   OSDK_I;
	x11_keymap[ XK_j] =   OSDK_J;
	x11_keymap[ XK_k] =   OSDK_K;
	x11_keymap[ XK_l] =   OSDK_L;
	x11_keymap[ XK_m] =   OSDK_M;
	x11_keymap[ XK_n] =   OSDK_N;
	x11_keymap[ XK_o] =   OSDK_O;
	x11_keymap[ XK_p] =   OSDK_P;
	x11_keymap[ XK_q] =   OSDK_Q;
	x11_keymap[ XK_r] =   OSDK_R;
	x11_keymap[ XK_s] =   OSDK_S;
	x11_keymap[ XK_t] =   OSDK_T;
	x11_keymap[ XK_u] =   OSDK_U;
	x11_keymap[ XK_v] =   OSDK_V;
	x11_keymap[ XK_w] =   OSDK_W;
	x11_keymap[ XK_x] =   OSDK_X;
	x11_keymap[ XK_y] =   OSDK_Y;
	x11_keymap[ XK_z] =   OSDK_Z;

	x11_keymap[ XK_Delete] =   OSDK_DELETE;

	x11_keymap[ XK_KP_Enter	] =   OSDK_RETURN;		
	x11_keymap[ XK_KP_Insert] =   OSDK_KP0;		
	x11_keymap[ XK_KP_End	] =   OSDK_KP1;		
	x11_keymap[ XK_KP_Down	] =   OSDK_KP2;		
	x11_keymap[ XK_KP_Page_Down	] =   OSDK_KP3;		
	x11_keymap[ XK_KP_Left	] =   OSDK_KP4;		
	x11_keymap[ XK_KP_Begin	] =   OSDK_KP5;		
	x11_keymap[ XK_KP_Right	] =   OSDK_KP6;		
	x11_keymap[ XK_KP_Home	] =   OSDK_KP7;		
	x11_keymap[ XK_KP_Up	] =   OSDK_KP8;		
	x11_keymap[ XK_KP_Page_Up] =   OSDK_KP9;		
	x11_keymap[ XK_KP_Decimal] =   OSDK_KP_PERIOD	;
	x11_keymap[ XK_KP_Divide] =    OSDK_KP_DIVIDE	;
	x11_keymap[ XK_KP_Multiply] =   OSDK_KP_MULTIPLY;
	x11_keymap[ XK_KP_Subtract] =    OSDK_KP_MINUS	;
	x11_keymap[ XK_KP_Add] =    OSDK_KP_PLUS	;

	x11_keymap[ XK_Up] =    OSDK_UP		;
	x11_keymap[ XK_Down] =  OSDK_DOWN	;	
	x11_keymap[ XK_Right] = OSDK_RIGHT;		
	x11_keymap[ XK_Left] =  OSDK_LEFT	;	
	x11_keymap[ XK_Insert] =OSDK_INSERT	;
	x11_keymap[ XK_Home] =  OSDK_HOME		;
	x11_keymap[ XK_End] =   OSDK_END		;
	x11_keymap[ XK_Page_Up] = OSDK_PAGEUP	;
	x11_keymap[ XK_Page_Down] =  OSDK_PAGEDOWN	;

	x11_keymap[ XK_F1] =   OSDK_F1;
	x11_keymap[ XK_F2] =   OSDK_F2;		
	x11_keymap[ XK_F3] =   OSDK_F3;		
	x11_keymap[ XK_F4] =   OSDK_F4;		
	x11_keymap[ XK_F5] =   OSDK_F5;		
	x11_keymap[ XK_F6] =   OSDK_F6;		
	x11_keymap[ XK_F7] =   OSDK_F7;		
	x11_keymap[ XK_F8] =   OSDK_F8;		
	x11_keymap[ XK_F9] =   OSDK_F9;		
	x11_keymap[ XK_F10] =   OSDK_F10;		
	x11_keymap[ XK_F11] =   OSDK_F11;		
	x11_keymap[ XK_F12] =   OSDK_F12;		
	x11_keymap[ XK_F13] =   OSDK_F13;		
	x11_keymap[ XK_F14] =   OSDK_F14;		
	x11_keymap[ XK_F15] =   OSDK_F15;		

	x11_keymap[ XK_Num_Lock] =  OSDK_NUMLOCK 	;
	x11_keymap[ XK_Caps_Lock] =  OSDK_CAPSLOCK	;
	x11_keymap[ XK_Scroll_Lock] =   OSDK_SCROLLOCK	;
	//x11_keymap[ XK_Shift_L] =    OSDK_SHIFT		;
	x11_keymap[ XK_Shift_R] =   OSDK_RSHIFT	;
	x11_keymap[ XK_Shift_L] =   OSDK_LSHIFT	;
	x11_keymap[ XK_Control_R] = OSDK_LCTRL	;	
	x11_keymap[ XK_Control_L] = OSDK_LCTRL	;	
	x11_keymap[ XK_Alt_R] =    OSDK_RALT		;
	x11_keymap[ XK_Alt_L] =    OSDK_LALT		;
	//x11_keymap[ VK_MENU ] =    OSDK_ALT	;
	x11_keymap[ XK_Meta_R] =     OSDK_RSUPER	;
	x11_keymap[ XK_Meta_L] =     OSDK_LSUPER	;

	x11_keymap[ XK_Help] =     OSDK_HELP		;
	x11_keymap[ XK_Print] = OSDK_PRINT	;	
	//x11_keymap[ XK_Cancel] =   OSDK_BREAK	;	
	//x11_keymap[ VK_APPS] =     OSDK_MENU;


	x11_keymap[ XK_at ] =      OSDK_AT;				// @
	x11_keymap[ XK_bracketleft]=  OSDK_LEFTBRACKET;  /* OSDK_LBRACKET;*/	// [
    x11_keymap[ XK_bracketright]= OSDK_RIGHTBRACKET; /* OSDK_RBRACKET;*/	// ]
    //x11_keymap[ VK_UPPER   ]=   OSDK_UPPER;			// ^
    x11_keymap[ XK_yen ] = OSDK_BACKSLASH;           // \\

	x11_keymap[ XK_underscore] =OSDK_UNDERSCORE;	// _ 
	//x11_keymap[ VK_KANA]    =   OSDK_KANA;			// kana

}


// ****************************************************************************
//          translate from scancode to keycode
// ****************************************************************************
int OSD_transkey( int scancode)
{
	int J;
	if( scancode >=0 && scancode <65536)
		{
		J= x11_keymap[ scancode];
		}
	else
		{
		J= 0;
		}
	return J;
}


// ****************************************************************************
//           get color		(X11)
// ****************************************************************************
ColTyp OSD_Getcolor(int R ,int G , int B ,int H)
{
	ColTyp col;
	XColor Color;
	int i;

	Color.flags=DoRed|DoGreen|DoBlue;
	Color.red=  R;
	Color.green=G;
	Color.blue= B;
	
	col.ct_xid =XAllocColor(Dsp,CMap,&Color)? Color.pixel:black;
	return col;
}

// ****************************************************************************
//           get Xtab		(Unix)
// ****************************************************************************
byte * OSD_GetXtab(void)
{
	return Xtab;
}


#ifdef MITSHM
/* Function that tries to XShmAttach an XImage, returns 0 in case of
faillure */
int TrySHM2( OSD_Surface *surface , int depth, int screen_num ,int width ,int height)
{
	long size;
	XImage *_ximage;
	static XShmSegmentInfo _SHMInfo;
	byte *_XBuf;

	if(Verbose & 1)
	    printf("  Using shared memory:\n    Creating image...");
	_ximage=XShmCreateImage(Dsp,VisInfo.visual,depth,
                           ZPixmap,NULL,&_SHMInfo,Width,Height);
	if(!_ximage) goto shm_error;  /* return (0);  */

	size=(long)_ximage->bytes_per_line*_ximage->height;
	if(Verbose & 1) printf("OK\n    Getting SHM info for %ld %s...",
			 (size&0x03ff) ? size : size>>10,
			 (size&0x03ff) ? "bytes" : "kB" );

	_SHMInfo.shmid=shmget(IPC_PRIVATE, size, IPC_CREAT|0777);
	if(_SHMInfo.shmid<0) goto shm_error;  /* return (0); */

	if(Verbose & 1) printf("OK\n    Allocating SHM...");
	_XBuf=(byte *)(_ximage->data=_SHMInfo.shmaddr=shmat(_SHMInfo.shmid,0,0));
	if(!_XBuf) goto shm_error; /* return (0); */

	_SHMInfo.readOnly=False;
	if(Verbose & 1) printf("OK\n    Attaching SHM...");
	OldHandler=XSetErrorHandler(Handler);
	if (!XShmAttach(Dsp,&_SHMInfo)) goto shm_error; /* return(0); */
	XSync(Dsp,False);
	XSetErrorHandler(OldHandler);
	
	surface->SHMInfo = &_SHMInfo;
	surface->ximage  = _ximage;
	surface->pixels  = _XBuf;
	return(UseSHM);

shm_error:
	if(_SHMInfo.shmaddr) shmdt(_SHMInfo.shmaddr);
	if(_SHMInfo.shmid>=0) shmctl(_SHMInfo.shmid,IPC_RMID,0);
    if(_ximage) XDestroyImage(_ximage);
    if(Verbose & 1) printf("FAILED\n");

	surface->SHMInfo = NULL;
	return 0;
}
#endif



// ****************************************************************************
//          OSD_CreateSurface: 新しいサーフェスを作成して、返却する
// In: HWND           handle of window
//     width, height  new bitmap size
//	   bitpix         color depth   1bpp 8bpp 24bpp 32bpp
//Out: OSD_surface    
// ****************************************************************************
OSD_Surface * OSD_CreateSurface(int width , int height, int bitpix ,int flags)
	{
	int K,L;

	OSD_Surface *surface;
    bitpix = VisInfo.depth;
   
	//bitpix= depth; // use 'depth' as the color depth

	surface = malloc( sizeof(OSD_Surface) );
	surface->format = malloc( sizeof(OSD_PixelFormat) );
#ifdef MITSHM
	surface->SHMInfo = NULL;
#endif // MITSHM
	surface->pixmap  = 0;

	if( flags == SURFACE_BITMAP)		// bitmap  
		{

#ifdef MITSHM
		if(UseSHM) /* check whether Dsp supports MITSHM */
	    	UseSHM=XQueryExtension(Dsp,"MIT-SHM",&ShmOpcode,&K,&L);
		if(UseSHM)
			{ 
        	if(!(UseSHM=TrySHM2(surface , bitpix /*depth */, screen_num ,width ,height))) /* Dsp might still be remote */ 
				{
      			/*if(SHMInfo.shmaddr) shmdt(SHMInfo.shmaddr);
      			if(SHMInfo.shmid>=0) shmctl(SHMInfo.shmid,IPC_RMID,0);
      			if(ximage) XDestroyImage(ximage);
      			if(Verbose & 1) printf("FAILED\n");*/
    			}
  			} 
		if (!UseSHM)
#endif // MITSHM
			{
    		long size;
			byte *_XBuf;
			XImage *_ximage;

    		//size=(long)sizeof(byte)*height*width*bitpix/8;
		size=(long)sizeof(byte)*height*width*32/8;
    		if(Verbose & 1) printf("  Allocating %ld %s RAM for image...",
				   (size&0x03ff) ? size : size>>10,
				   (size&0x03ff) ? "bytes" : "kB");
    		_XBuf=(byte *)malloc(size);
    		if(!_XBuf) { if(Verbose & 1) printf("FAILED\n");goto createsurface_error; }


    		if(Verbose & 1) printf("OK\n  Creating image...");
    		_ximage=XCreateImage(Dsp,VisInfo.visual,bitpix,
                        ZPixmap,0,_XBuf,width,height,8,0);
    		if(!_ximage) { if(Verbose & 1) printf("FAILED\n");goto createsurface_error; }

			surface->pixels = _XBuf;
			surface->ximage = _ximage;
  			}
		if(Verbose & 1) printf("OK\n");
		}

	else if( flags == SURFACE_OFFSCREEN)	// offscreen
		{
		Pixmap _pixmap;

				// Create off screen     2006/11/4
    	 if(Verbose & 1) printf("\n  Creating offscreen image...");
		 _pixmap = XCreatePixmap( Dsp, Wnd ,Width ,Height ,bitpix);
		 if( _pixmap == BadAlloc || _pixmap == BadDrawable || _pixmap==BadPixmap || _pixmap == BadValue)
			{
			_pixmap = 0; // Cannot use pixmap
			if(Verbose & 1) {printf("FAILED\n"); goto createsurface_error;}
			}
		if(Verbose & 1) {printf("OK\n");}
		surface->pixels = NULL;
		surface->pixmap = _pixmap;
		}
	else 
		{
		 if(Verbose & 1) printf("[Unix][CreateSurface] bad parameter : flags = %X \n ",flags);
		 goto createsurface_error;
		}
//#endif

	surface->format->BitsPerPixel = bitpix;
	surface->format->BytesPerPixel = bitpix/8;
	surface->w      = width;
	surface->h      = height;
	surface->pitch  = width *(bitpix/8);
	surface->flags  = flags;

	return surface;			// successfull

createsurface_error:
	if( surface)
		{
		free( surface->format);
		free( surface);
		surface=NULL;
		}
	return NULL;			// failed
	}


// ****************************************************************************
//          OSD_setwindowsurface: ウインドウのサーフェスを作成して、返却する
// In: width, height  window size
//	   bitpix         color depth   1bpp 8bpp 24bpp 32bpp
//Out: OSD_surface    
// ****************************************************************************
OSD_Surface * OSD_setwindowsurface(int width , int height, int bitpix, int flags)
{
//	OSD_PixelFormat *pixelformat;
	OSD_Surface *surface;
    bitpix = VisInfo.depth;

	surface = malloc( sizeof(OSD_Surface) );
	surface->format = malloc( sizeof(OSD_PixelFormat) );

	surface->format->BitsPerPixel = bitpix;
	surface->format->BytesPerPixel = bitpix/8;
	surface->w      = width;
	surface->h      = height;
	surface->pixels = NULL;
#ifdef WIN32
	surface->handle = 0;
	surface->hdcBmp = 0;
#endif

	surface->ximage =0;
	surface->pixmap =0;
#ifdef MITSHM
	surface->SHMInfo=0;
#endif

	surface->flags  = SURFACE_WINDOW;			// window
	
    SetWMHints();

	XResizeWindow( Dsp,Wnd ,width +PADDINGW+BORDERW ,height+PADDINGH+BORDERH);		// resize window

	
	return surface;
}

int OSD_LockSurface( OSD_Surface *surface)
{
}

int OSD_UnlockSurface( OSD_Surface *surface)
{
}


// ****************************************************************************
//          OSD_ReleaseSurface: サーフェスを破棄する
//Int: OSD_surface    
// ****************************************************************************
void OSD_ReleaseSurface( OSD_Surface *surface)
{
#ifdef MITSHM
	static XShmSegmentInfo *_SHMInfo;
#endif // MITSHM
    if(surface ==NULL) return;

	if(Dsp&&Wnd)
		{
#ifdef MITSHM
	    if(UseSHM)
		    {
			  _SHMInfo = surface->SHMInfo;

		      if( _SHMInfo) 
					{
					 XShmDetach(Dsp,_SHMInfo);
		      		 if(_SHMInfo->shmaddr) shmdt(_SHMInfo->shmaddr);
		      		 if(_SHMInfo->shmid>=0) shmctl(_SHMInfo->shmid,IPC_RMID,0);
					 if( Verbose )printf("   Detached shm memory...OK\n");
					}
			}
	    else
	       {
#endif //MITSHM

	    if( surface->ximage !=NULL) {
	           XDestroyImage( surface->ximage);
               if(Verbose) printf("  Destroyed Image...OK\n");
               }     

		if( surface->pixmap !=0) {
		       XFreePixmap( Dsp ,surface->pixmap);
               if(Verbose) printf("  Destroyed offscreen Image...OK\n");
               }
#ifdef MITSHM
          }
#endif
		}
	
	if( surface)
		{
		free( surface->format);
		free( surface);
		surface = NULL;
		}
}

// ****************************************************************************
//          OSD_BlitSurface: サーフェスからサーフェスに　blit する
// In:  dst_surface : 出力先サーフェス
//      ex,ey       : 出力先サーフェスの左上の座標
//      w,h         : 出力サーフェスのサイズ
//      src_surface : 転送元サーフェス
//      sx,sy       : 転送元サーフェスの左上の座標
// ****************************************************************************

void OSD_BlitSurface(OSD_Surface *dst_surface,int ex,int ey, int w,int h ,OSD_Surface *src_surface ,int sx ,int sy)
{
				// blit offscreen -> window
	if( src_surface->flags == SURFACE_OFFSCREEN && dst_surface->flags == SURFACE_WINDOW)
		{
		 Drawable src_drw, dst_drw;

		 src_drw = src_surface->pixmap;
		 dst_drw = Wnd;
		 XCopyArea( Dsp ,src_drw ,dst_drw ,DefaultGC ,sx , sy ,w ,h ,ex ,ey );
		XFlush(Dsp);
		}
	else		// blit bitmap -> window or   bitmap -> offscreen
		{
		Drawable drw;

		if( dst_surface->flags == SURFACE_WINDOW)
			drw = Wnd;
		else if( dst_surface->flags == SURFACE_OFFSCREEN)
			drw = dst_surface->pixmap;

		/* blit ximage -> window */
#ifdef MITSHM
		if(src_surface->SHMInfo) 
	    	XShmPutImage
	    	     (Dsp, drw,DefaultGC,src_surface->ximage, sx , sy , ex , ey , w, h ,False);
  		else
#endif // MITSHM
		XPutImage(Dsp, drw,DefaultGC,src_surface->ximage, sx , sy , ex , ey , w, h );
		XFlush(Dsp);
		}

}



// ****************************************************************************
//          ClearScr: エミュレータ画面を全クリア
// ****************************************************************************
//void ClearScr(void)
//{
//	memset(XBuf,(byte)black,Width*Height*bitpix/8);
//}


// ****************************************************************************
//          ClearWindow: ウインドウ全体をクリア
// ****************************************************************************
void OSD_ClearWindow( void)
{

}


void OSD_SetWindowTitle( char *name )
{
    strcpy( TitleBuf,name );
	SetWMHints();
}

// ****************************************************************************
//          Application イベントループ
// ****************************************************************************
int OSD_RunApplicationEventLoop(void)
{
	XEvent E;
	printf("RunApplicationEventLoop %d \n",CPURunning);

	while(CPURunning)
		{
		 if( OSD_ChkEvent() ==-1) break;
		}
    return 0;
}


// ****************************************************************************
//          chkEvent : イベントをチェックして、実行する
// ****************************************************************************
int chkEvent( XEvent E)
{
	int J;
	switch( E.type)
		{
		case KeyPress:
		case KeyRelease:						// keyboard event
			J=XLookupKeysym((XKeyEvent *)&E,0);
			int scancode = J;
			int osdkeycode = OSD_transkey( scancode);
			printf("scancode =%04X  osdkeycode =%04X \n",scancode , osdkeycode);
			write_keybuffer(keybuffer , 0, (E.type==KeyPress) ,scancode,osdkeycode);
			break;

		case FocusOut:							// focus out event
			if(SaveCPU)
			  {
#ifdef SOUND
				StopSound();          /* Silence the sound        */
#endif
				while(!XCheckWindowEvent(Dsp,Wnd,FocusChangeMask,&E)&&CPURunning)
		    		{
					if(XCheckWindowEvent(Dsp,Wnd,ExposureMask,&E)) PutImage();
		      		XPeekEvent(Dsp,&E);
		    		}
#ifdef SOUND
    			ResumeSound();        /* Switch the sound back on */
#endif
				}
			break;

		case ClientMessage:				// close window event. but not work! FIX ME!!
			if( E.xclient.message_type == WM_PROTOCOLS_atom && 
			    E.xclient.data.l[0]    == DELETE_WINDOW_atom)
					{
					printf("close \n");
					return 0;
					}
			else
					printf("close x  \n");
			break;

		default:  //printf("event type =%d \n",E.type); 
			break;
		}
	return 1;
	}




// ****************************************************************************
//           cpu run		(Unix)
// ****************************************************************************
int OSD_ChkEvent(void)
{
	XEvent E;

		if(XCheckWindowEvent(Dsp,Wnd,KeyPressMask|KeyReleaseMask|FocusChangeMask|ButtonPressMask,&E))
			{
			 if( !chkEvent( E)) return -1;		// if return =0 ,program exit
			 return 1;
			}
		else
			{
			 CPUThreadProc(NULL);
			 return 0;
			}
}

// ****************************************************************************
//          OSD_InstallTimerEvent
//    
//     Install   Timer Event Handler
// ****************************************************************************
void OSD_InstallTimerEvent(void)
{
    // Not Implimented dayo!
}





// ****************************************************************************
//          OSD_Setcolor: 色を設定する
// In:  color : P6 の色のインデックス
// ****************************************************************************
void OSD_Setcolor( int color)
{

	if( color!=0)
		foreground_color = white;
	else
		foreground_color = black;		// true black
}


// ****************************************************************************
//          OSD_Setcolor: 色を設定する
// In:  r,g,b : RGB の色の深さ
// ****************************************************************************
void OSD_Setcolor_rgb( int r, int g ,int b)
{
//	foreground_color = RGB( r,g,b);
}



// ****************************************************************************
//          OSD_Rectangle: 長方形を描画する
// In:  surface : 出力先サーフェス
//      sx ,sy  : 左上の座標
//      w  ,h   : サイズ
// ****************************************************************************
void OSD_Rectangle( OSD_Surface *surface ,int sx ,int sy ,int w ,int h,int boxfill)
{
	Drawable drw;
	if( surface->flags == SURFACE_WINDOW)
		drw= Wnd;
	else
		drw= surface->pixmap;

	XSetForeground( Dsp ,DefaultGC , foreground_color );
	
	XFillRectangle( Dsp ,drw ,DefaultGC ,sx ,sy , w ,h);
	XFlush( Dsp );

}

// ****************************************************************************
//          textout: サーフェスに文字列を描画する
// In:  surface     : 出力先サーフェス
//      x,y         : 出力先サーフェスの左上の座標
//      str         : 文字列
//      size        : 文字列のサイズ
// ****************************************************************************
// draw string on window
int OSD_textout(OSD_Surface *surface ,int x,int y,char *str,int size)
{
	XFontSet fs;
	GC	gc;
	char **miss ,*def;
	int    n_miss;

	return 1;  // do nothing

	setlocale(LC_ALL,"");
	if( XSupportsLocale() == False) {printf("not support locale \n"); return 0;}

	gc =XCreateGC( Dsp,Wnd ,0,0);
	XSetForeground( Dsp,gc, white);
	XSetBackground( Dsp,gc, black);

	fs =XCreateFontSet( Dsp,"-*-*-medium-r-normal--14-*",&miss ,&n_miss, &def);
	XmbDrawString(Dsp,Wnd,fs,gc,x,y, str,strlen( str));
	XFlush(Dsp);
	//printf("textout: %s\n",str);
	return 1;
}



OSD_Surface * OSD_GetVideoSurface()
{
	return screen;
}

// ****************************************************************************
//          OSD_SelectMachine  互換ROM用　機種選択
// ****************************************************************************
int OSD_SelectMachine(void)
{
    return -1;
}


// ******************************** messagebox ******************************
int OSD_MessageBox( const char *mes, const char *cap, int type )
{
	// Not Implimented
}

int isFullScreen(void)
{
    return 0;
}

int toggleFullScr(void)
{
}

/* ************************** set menu title ****************************** */
/* set filename to open menu */
int setMenuTitle(int type)
{
	// Not Implimented
}

/* *********************** make extkanjirom ****************************** */
/* In: mem  EXTKANJIROM
  Out: ret  1:success  0: failed  */
int make_extkanjirom(char *mem)
{
	return(0);		// not implimented
}

/* *********************** JoystickGetState ****************************** */
int JoystickGetState(int joy_no)
{
	return(0);		// not implimented
}

// ****************************************************************************
//          ClearWindow: ウインドウ全体をクリア
// ****************************************************************************
void ClearWindow(void)
{
	XClearWindow(Dsp,Wnd);
}


// ****************************************************************************
//          sleepus: 眠る
// ****************************************************************************
void SLEEP(int s)
{
	usleep( s *1000);
}


// ****************************************************************************
//          outputdebugstring: デバッガにメッセージを送る
// ****************************************************************************
void outputdebugstring(char *buff)
{
}

// ****************************************************************************
//          ロケールを取得
// ****************************************************************************
int OSD_getlocale(void)
{
	int ret;
	 
	 char *usrLang;
	 usrLang = getenv("LANG");
	 if( usrLang ==NULL) {
		ret = OSDL_OT;
		return ret;
		}
	 if (strcmp( usrLang ,"ja_JP.UTF-8")==0) {
		 ret = OSDL_JP;
	 }else 
	 if (strcmp( usrLang ,"C")==0) {

		 ret = OSDL_EN;
	 }
	 else
	 {
		 ret = OSDL_OT;
	 }
	 return ret;
}


void OSD_SetModulePath(void)
{	
}

void OSD_OpenFiler(char *path)
{	
}


int stricmp(const char* a, const char* b)
{
	return strcasecmp(a, b);
}

/* ********************************************************************************************* */




/*menu_testcode()
{
	XFontSet fs;
	char **miss ,*def;
	int    n_miss;

	Window r,p,w[6];
	GC gc;
	XEvent e;
	int i;
	static char string[]="Item0";
	
	r = RootWindow(Dsp,0);
	gc = XCreateGC(Dsp,r,0,0);
	
	p= XCreateSimpleWindow( Dsp,r,600,50,100,240,2,white,black);
	for(i=0; i<6; i++)
		{
		 w[i]= XCreateSimpleWindow( Dsp,p,0,i*40,95,36,2,white,black);
		 XSelectInput( Dsp,w[i], ExposureMask|ButtonPressMask);
		 if( i==5) XSelectInput( Dsp ,w[i] , ExposureMask | ButtonPressMask | ButtonReleaseMask);
		}

	XMapWindow( Dsp,p);
	XMapSubwindows( Dsp,p);
	while(1)
		{
		 if( XCheckTypedEvent( Dsp, ButtonRelease, &e)) {XDestroyWindow( Dsp,p); return(0);}
		 for(i=0; i<6; i++)
			{
			 if( XCheckWindowEvent( Dsp, w[i], ExposureMask, &e))
				{
				 string[4]= '0'+ i;
				 //XDrawString( Dsp, w[i], DefaultGC,10,40,string,5);
					setlocale(LC_ALL,"");
				if( XSupportsLocale() == False) {printf("not support locale \n"); return 0;}

				gc =XCreateGC( Dsp,Wnd ,0,0);
				XSetForeground( Dsp,gc, white);
				XSetBackground( Dsp,gc, black);

			fs =XCreateFontSet( Dsp,"-*-*-medium-r-normal--14-*",&miss ,&n_miss, &def);
			XmbDrawString(Dsp,w[i],fs,gc,10,10, string,strlen( string));

				 XFlush( Dsp);
				}
			if( XCheckWindowEvent( Dsp, w[i], ButtonPressMask, &e)) 
				{
 				 printf(" %d selected \n", i);
				}
			}
		}
}

*/

#endif // UNIX

#endif // X11
