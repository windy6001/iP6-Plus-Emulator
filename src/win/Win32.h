/** iP6: PC-6000/6600 series emualtor ************************/
/**                                                         **/
/**                           Win32.h                       **/
/**                                                         **/
/** by Windy 2003                                           **/
/*************************************************************/
#ifndef _WIN32_H
#define _WIN32_H

#ifdef WIN32
#include <windows.h>
#endif

#include "../types.h"

#include "Win32gui.h"
#include "Win32stick.h"
#include "Win32fscr.h"

#include "../keys.h"	// for OSDK_xxxx  2006/11/26

#include "../Refresh.h"	// for Coltype

#include "../Video.h"


#define DEFAULT_KEY_REPEAT 10

void SLEEP(int s);

byte * OSD_GetXtab(void);

void PutImage(void);
void do_palet(int dest,int src);
int  messagebox(char *str, char *title);
void outputdebugstring(char *buff);
void putlasterror(void);

extern void choosefuncs(int lsbfirst, int bitpix);
extern void setwidth(int wide);
int setMenuTitle(int type);
//int resizewindow( int bitmap_scale,int win_scale);

// ****************************************************************************
//          dibsection
// ****************************************************************************
typedef struct dibstruct {
	int					Width ,Height;		// size
	BYTE				*lpBMP;				// pixel buffer
	BITMAPINFO			*lpBmpInfo;			// Dibsection info
	BITMAPINFOHEADER	*lpBmpInfoh;		// BMP file info header
	HBITMAP				hBitmap1;			// bitmap handle
	HDC					hdcBmp;				// bitmap hdc
	int					Enable;				// enable
	} HIBMP;


extern HWND hwndMain;
extern HINSTANCE _hThisInst /*hInst */;
extern HICON     hIcon;


extern int   UseJoystick;						// use joystick 1: use  add 2003/8/31
extern int   CPUThreadRun;         			// CPU Thread  1: run

#ifndef word
#define word unsigned short
#endif

extern word  ExitedAddr;						// exit code from Z80()
extern int paddingw;
extern int paddingh;

extern int   UseJoystick;						// use joystick 1: use  add 2003/8/31



int InitMachine( void );
void OSD_SetWindowTitle(char *name);

LRESULT CALLBACK WindowFunc( HWND ,UINT ,WPARAM, LPARAM);



/*
int textout(OSD_Surface *surface ,int x,int y,char *str ,int size); //          textout: エミュレータの画面に文字を出力
OSD_Surface * setwindowsurface(int width , int height, int bitpix);
*/

//void PutDiskAccessLamp(int sw);
//void PutImage(void);
//void putStatusBar(void);
//int savesnapshot(char *path);

int  debug_printf( char *fmt ,...);
extern int debug_level;

/* ----------- key repeat  --------------- */
void setKeyrepeat(int value); 	//  setKeyrepeat: キーリピートの速度を設定する
void storeKeyrepeat(void);		//  storeKeyrepeat: キーリピートの速度を元に戻す

int realize_lplogpal(HDC hdc,int flag);

byte * GetXtab(void);


//#undef EXTRN
#endif
