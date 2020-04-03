/** iP6: PC-6000/6600 series emualtor ************************/
/**                                                         **/
/**                      Refresh.h                          **/
/**                                                         **/
/** modified by Windy                                       **/
/** by ISHIOKA Hiroshi 1998,1999                            **/
/** This code is based on fMSX written by Marat Fayzullin   **/
/*************************************************************/


	// --------------- êÈåæÇ∑ÇÈèÍçáÇÕÅA#define GLOBAL Ç∑ÇÈ -------------
#ifndef _REFRESH_H
#define _REFRESH_H



#include "types.h"
#include "os.h"

#ifdef X11
#include <X11/Xlib.h>
#ifdef MITSHM
#include <sys/ipc.h>
#include <sys/shm.h>
#include <X11/extensions/XShm.h>
#endif


#else
typedef unsigned long XID;
#endif


#define SURFACE_BITMAP    0x10
#define SURFACE_WINDOW    0x20
#define SURFACE_OFFSCREEN 0x40 

#define WINDOW_SOFTWARE  0x100
#define WINDOW_HARDWARE  0x110

#define FULLSCREEN       0x4


#define BPP32_ARGB    1
#define BPP32_RGBA    2

#define BPP24_RGB     4 
#define BPP24_BGR     8

enum {
	C_BLACK,
	C_ORANGE,
	C_BLUEGREEN,
	C_YELLOWGREEN,
	C_BLUEPURPLE,
	C_REDPURPLE,
	C_BLUESKY,
	C_GRAY,
	C_RED = 9,
	C_GREEN,
	C_YELLOW,
	C_BLUE,
	C_MAGENDA,
	C_WHITE
};


/** Some typedef's **/
//typedef union _ColTyp {
//    XID ct_xid;
//    byte ct_byte[4];
//} ColTyp; /* sizeof ColTyp should be 4 */


typedef union ColTyp {
	unsigned int  ct_xid;
	unsigned char ct_byte[4];
} ColTyp;


typedef struct OSD_Rect {
	int x, y;
	int w, h;
} OSD_Rect;

typedef struct OSD_Color2 {
	byte r;
	byte g;
	byte b;
	byte unused;
} OSD_Color;


typedef struct OSD_PixelFormat {
	int       ncolors;			// number of color
	OSD_Color colors[256];		// palet color
	byte  BitsPerPixel;		// 32bpp / 24bpp / 8bpp
	byte  BytesPerPixel;
	int   palette;
	int   byteOrder;           // byte order   RGBA / ARGB / RGB 
} OSD_PixelFormat;



typedef struct _OSD_Surface {
	int flags;				// flag
	OSD_PixelFormat *format;	// pixel format
	int w, h;					// width ,height
	int pitch;				// pitch
	void *pixels;				// pixels

#ifdef WIN32
	HBITMAP				handle;			// bitmap handle
	HDC			    	hdcBmp;				// bitmap hdc
#endif

#ifdef USE_QUARTZ
	void*           context;
#endif
#ifdef CARBON
#ifdef USE_QUICKDRAW
	WindowRef window;
	GWorldPtr gworld;
#endif
#endif


#ifdef X11
	XImage *ximage;
	Pixmap pixmap;
#ifdef MITSHM
	XShmSegmentInfo *SHMInfo;
#endif
#endif



} OSD_Surface;

OSD_Surface * OSD_CreateSurface(int width, int height, int bitpix, int flags);
void OSD_ReleaseSurface(OSD_Surface *surface);
void OSD_BlitSurface(OSD_Surface *dst_surface, int ex, int ey, int w, int h, OSD_Surface *src_surface, int sx, int sy);
OSD_Surface * OSD_setwindowsurface(int width, int height, int bitpix, int flags);


int OSD_textout(OSD_Surface *surface, int x, int y, char *str, int size);
void OSD_Setcolor(int color);
void OSD_Setcolor_rgb(int r, int g, int b);
ColTyp OSD_Getcolor(int R, int G, int B, int H);
void OSD_Rectangle(OSD_Surface *surface, int sx, int sy, int w, int h, int boxFill);

void OSD_ClearWindow(void);
int OSD_LockSurface(OSD_Surface *surface);
int OSD_UnlockSurface(OSD_Surface *surface);
OSD_Surface * OSD_GetVideoSurface(void);


extern OSD_Surface *surface1;
extern OSD_Surface *surface2;
extern OSD_Surface *screen;
extern OSD_Surface *status_surface;

ColTyp getpixel(OSD_Surface *surface, int x, int y);
int toggleFullScr(void);
int isFullScreen(void);



#include "types.h"
//#include "Video.h"

#ifdef  GLOBAL
#undef  EXTRN
#define EXTRN
#else
#undef  EXTRN
#define EXTRN extern
#endif

#ifndef Bool
#define Bool int
#endif

#if 0            
/** Some typedef's **/
//	typedef union {
//		unsigned int ct_xid;
//		unsigned char ct_byte[4];
//	} ColTyp; /* sizeof ColTyp should be 4 */

#endif

#define DEPTH	  8 // 24 // 8 //  24   		// color depth   8 / 24

#ifdef __MACOSX__
#undef  DEPTH
#define DEPTH     32
#endif

//#if __IPHONE_OS_VERSION_MIN_REQUIRED >= 30000
//#define DEPTH     8
//#endif



	extern int paddingw;
	extern int paddingh;

	extern int param[16][3];
	extern unsigned short trans[];
	extern int Pal11[4];
	extern int Pal12[8];
	extern int Pal13[8];
	extern int Pal14[4];
	extern int Pal15[8];
	extern int Pal53[32];



	extern void(*SCR[2 + 2][4])();	/* Screen Mode Handlers */
	EXTRN  int IntLac;
	EXTRN  int IntLac_bak;

	EXTRN  int scale;			/* emulator scale */
	EXTRN  int new_scale;		/* new emulator scale */
	EXTRN  int win_scale;		/* window scale */
	EXTRN  int backup_scale;    /* backup scale */

	/* functions and variables in Unix.c used by Refresh.c */
	EXTRN  int Mapped;
	EXTRN  byte *XBuf;
	EXTRN  int bitpix;


	EXTRN ColTyp BPal[16], BPal53[32], BPal11[4], BPal12[8], BPal13[8], BPal14[4], BPal15[8], BPal62[32], BPal61[16];



	/* functions and variables in Refresh.c used by Unix.c */
	EXTRN  Bool lsbfirst;
	EXTRN  int  Width, Height;

	extern XID black;

	void choosefuncs(int lsbfirst, int bitpix);
	void setwidth(int wide);


	int InitScrF(void);
	void ClearScr(void);
	void RefreshScr10(void);
	void RefreshScr11(void);
	void RefreshScr12(void);
	void RefreshScr13(void);
	void RefreshScr13a(void);
	void RefreshScr13b(void);
	void RefreshScr13c(void);
	void RefreshScr13d(void);
	void RefreshScr13e(void);
	void RefreshScr51(void);
	void RefreshScr52(void);
	void RefreshScr53(void);
	void RefreshScr54(void);

	void RefreshScr61(void);        /* add  2002/2 */
	void RefreshScr62(void);		/* add  2002/2 */
	void RefreshScr63(void);		/* add  2002/5/2 */


	void setRefreshSurface(OSD_Surface *in_surface);
	void do_palet(int dest, int src);

	//ColTyp getpixel(unsigned char *image, int x, int y,int bitpix);
	ColTyp OSD_Getcolor(int R, int G, int B, int H);
	void putOneChar(int sx, int sy, char chr, char attr);
	//OSD_Surface * getRefreshSurface( void);
	void RefreshDebugWindow(void);
	OSD_Surface * getRefreshSurface(void);


#endif // _REFRESH_H
