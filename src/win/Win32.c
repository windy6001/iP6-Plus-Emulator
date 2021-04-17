/** iP6: PC-6000/6600 series emualtor ************************/
/**                                                         **/
/**                           Win32.c                       **/
/** by Windy 2002-2004                                      **/
/** by ISHIOKA Hiroshi 1998-2000                            **/
/** This code is based on fMSX written by Marat Fayzullin   **/
/*************************************************************/


#ifdef WIN32
#include <windows.h>
#include <stdio.h>
#include <time.h>
#include <assert.h>

#include <stdlib.h>
#include <string.h>
#include <ctype.h>


#include <direct.h>

#include "../types.h"
#include "../P6.h"
#include "../Refresh.h"
#include "../Sound.h"

#include "../Option.h"
#include "../Build.h"
#include "../Debug.h"
#include "../message.h"
#include "../buffer.h"



#include "WinMenu.h"
#include "Win32.h"



#include "../Video.h"
#include "../Timer.h"



extern char *Title;
extern int main(int argc ,char *argv[]);

// ****************************************************************************
//           internal function
// ****************************************************************************
//void PutImage(void);
static int makeWindow(char *name , HINSTANCE hThisInst, HINSTANCE hPrevInst, LPSTR lpszArgs, int nWinMode);

//int savesnapshot(char *path);
//void init_delay(int fps);
void destroy_lplogpal(void);



// ****************************************************************************
//           Variable
// ****************************************************************************
static DWORD foreground_color=15;		// foreground color

//word  ExitedAddr;						// exit code from Z80()
//int   UseJoystick;						// use joystick 1: use  add 2003/8/31
//unsigned int htime;

int   SaveCPU;


char szWinName[]= "MyWin";


HWND hwndMain =NULL;
HICON     hIcon;

HINSTANCE _hThisInst;
HINSTANCE _hPrevInst;
LPSTR     _lpszArgs;
int       _nWinMode;



extern HWND  hDebugDlg;		// debug dialog window handle


int dpiX,dpiY;		// screen DPI
float dpiBairitu;	// screen bairitu  96dpi を１にした

byte  Xtab[4];




int OSD_GetModulePath(char *path , int max)
{
 getcwd( path , max);
 return 1;
}



/** TrashMachine *********************************************/
/** Deallocate all resources taken by InitMachine().        **/
/*************************************************************/
void TrashMachine(void)
{
	if(Verbose) printf("Shutting down...\n");


	storeKeyrepeat();			// store key repeat

	storeDisplayMode(); 		// store display mode

#ifdef SOUND
	/* StopSound(); */
	TrashSound();
#endif SOUND
   destroy_lplogpal();
	OSD_TrashTimer();

	OSD_ReleaseSurface( surface1 );
	OSD_ReleaseSurface( surface2 );


}

/* ↑StopSound() を外してみる。音源Thread をSuspendしないほうが良いのでは？ 2003/8/14 */



// ****************************************************************************
//          InitMachine: MS-Windows のウインドウ初期化など
// ****************************************************************************
int InitMachine( void)
{
	OSD_InitTimer();

	setMenuTitle( WINDOW12);


	bitpix= DEPTH;
	Width =M6WIDTH * scale;
	Height=M6HEIGHT* scale;


	 makeWindow(Title , _hThisInst, _hPrevInst, _lpszArgs, _nWinMode);	/* make a window */

#ifndef WORDS_BIGENDIAN
	 lsbfirst= 1;			// Little Endian  (Intel)
#else
	 lsbfirst= 0;			// Big    Endian  (Motorola)
#endif
	 choosefuncs(lsbfirst,bitpix);

	// screen = OSD_setwindowsurface(Width ,Height ,bitpix ,WINDOW_SOFTWARE);
    // InitColor(0); 
	// surface1 = OSD_CreateSurface(Width ,Height ,bitpix ,SURFACE_BITMAP);
	// surface2 = OSD_CreateSurface(Width ,Height ,bitpix ,SURFACE_OFFSCREEN);
	// XBuf = surface1->pixels;
	// setRefreshSurface( surface1);

     InitColor(0); 
	 resizewindow( (float)scale,(float)scale,0);

	  /* ********** init p6 *************** */
	//SetValidLine_sr( drawwait);
	//SetClock(CPUclock*1000000);
	//SetTimerIntClock(3);



	
#ifdef SOUND
	  if(UseSound)
		{
		//if( !InitSound() )
		//	MessageBox(hwndMain, "Sound device open failed","",MB_OK);
		}
#endif SOUND

	 setwidth( scale-1);
	
	 setKeyrepeat(DEFAULT_KEY_REPEAT);		// set key repeat  2003/10/18
	 
	 saveDisplayMode();		// save current display mode  for fullscreen  2003/9/28
	
	 if( UseDiskLamp) PutDiskAccessLamp(255);	// ディスクランプの枠を表示

	OSD_Setcolor( 15 );


	{
	HDC hdc = GetDC(NULL);
	if( hdc) 
		{
		 dpiX = GetDeviceCaps( hdc, LOGPIXELSX);
		 dpiY = GetDeviceCaps( hdc, LOGPIXELSY);
	     dpiBairitu = (float) dpiX /96.0f;		 // 96dpi を１にした倍率

		 ReleaseDC(NULL,hdc);
		}
	}

	 return(1);
}
#define TIMER_1SEC 1

VOID CALLBACK timer_1sec_func(
	HWND hwnd,         // ウィンドウのハンドル
	UINT uMsg,         // WM_TIMER メッセージ
	UINT_PTR idEvent,  // タイマの識別子
	DWORD dwTime       // 現在のシステム時刻
)
{
	updateDateTime();
}


// ****************************************************************************
//          makeWindow: MS-Windows の Window作成処理
// ****************************************************************************
static int makeWindow(char *Title ,HINSTANCE hThisInst, HINSTANCE hPrevInst, LPSTR lpszArgs, int nWinMode)
{
//	HMENU hmenu;

	WNDCLASSEX wcl;
	wcl.cbSize= sizeof(WNDCLASSEX);

	wcl.hInstance  = hThisInst;      /* このインスタンスのハンドル*/
	wcl.lpszClassName = szWinName;   /* このウインドウクラスの名前 */
	wcl.lpfnWndProc   = WindowFunc;
	wcl.style= 0;
	wcl.hIcon = hIcon =  LoadIcon(hThisInst, "IC_PC66SR" /*IDI_APPLICATION*/);
	wcl.hIconSm= NULL;  /*LoadIcon(NULL, IDI_WINLOGO); */
	wcl.hCursor= LoadCursor(NULL, IDC_ARROW);

	wcl.lpszMenuName= "PCMENU";		/* menu */
	wcl.cbClsExtra =0;
	wcl.cbWndExtra =0;

	wcl.hbrBackground = (HBRUSH) GetStockObject( BLACK_BRUSH); /* back ground */

	if( !RegisterClassEx( &wcl)) 	/* register window class */
		return 0;

	hwndMain = CreateWindow(
	    szWinName,
	    Title,
	    WS_OVERLAPPED |WS_MAXIMIZEBOX | WS_MINIMIZEBOX |WS_SYSMENU |WS_CAPTION |WS_THICKFRAME,
	    CW_USEDEFAULT,				/* X */
	    CW_USEDEFAULT,				/* Y */
	    Width +BORDERW+PADDINGW*2,	/* WIDTH */
	    Height+BORDERH+PADDINGH*2,	/* HEIGHT */
	    HWND_DESKTOP,				/* non parent window*/
	    NULL,	
	    hThisInst, 		/* program window instance */
	    NULL
   	);


	ShowWindow(hwndMain, nWinMode);
	UpdateWindow( hwndMain);
   
//   hmenu= GetMenu(hwndMain);		// menu handle (global variable)

	SetTimer(hwndMain, TIMER_1SEC, 1000, timer_1sec_func);		// install 1sec timer


	init_directinput( hwndMain, hThisInst);
	return(1);
}


int win_keymap[256];

// ****************************************************************************
//          OS の keymap 初期設定
// ****************************************************************************
void OSD_initKeymap(void)
{
	int i;
	for(i=0; i<256; i++)
		win_keymap[i] = OSDK_UNKNOWN;

	win_keymap[ VK_BACK] =       OSDK_BACKSPACE;	
	win_keymap[ VK_TAB] =        OSDK_TAB; 		
	win_keymap[ VK_CLEAR] =      OSDK_CLEAR;		
	win_keymap[ VK_RETURN] =     OSDK_RETURN;	
	win_keymap[ VK_PAUSE] =      OSDK_PAUSE;		
	win_keymap[ VK_ESCAPE] =     OSDK_ESCAPE;	
	win_keymap[ VK_SPACE] =      OSDK_SPACE;		
	win_keymap[ VK_COMMA] =      OSDK_COMMA;		
	win_keymap[ VK_MINUS] =      OSDK_MINUS;		
	win_keymap[ VK_PERIOD] =     OSDK_PERIOD;	
	win_keymap[ VK_SLASH] =      OSDK_SLASH;	
	win_keymap[ VK_0] =          OSDK_0;                   
	win_keymap[ VK_1] =          OSDK_1;
	win_keymap[ VK_2] =          OSDK_2;
	win_keymap[ VK_3] =          OSDK_3;
	win_keymap[ VK_4] =          OSDK_4;
	win_keymap[ VK_5] =          OSDK_5;
	win_keymap[ VK_6] =          OSDK_6;
	win_keymap[ VK_7] =          OSDK_7;
	win_keymap[ VK_8] =          OSDK_8;
	win_keymap[ VK_9] =          OSDK_9;
	win_keymap[ VK_COLON] =      OSDK_COLON ;			// :
	win_keymap[ VK_SEMICOLON] =  OSDK_SEMICOLON ;		// ;
	win_keymap[ VK_BACKSLASH] =  OSDK_BACKSLASH ;		// \

	win_keymap[ VK_A] =   OSDK_A;
	win_keymap[ VK_B] =   OSDK_B;
	win_keymap[ VK_C] =   OSDK_C;
	win_keymap[ VK_D] =   OSDK_D;
	win_keymap[ VK_E] =   OSDK_E;
	win_keymap[ VK_F] =   OSDK_F;
	win_keymap[ VK_G] =   OSDK_G;
	win_keymap[ VK_H] =   OSDK_H;
	win_keymap[ VK_I] =   OSDK_I;
	win_keymap[ VK_J] =   OSDK_J;
	win_keymap[ VK_K] =   OSDK_K;
	win_keymap[ VK_L] =   OSDK_L;
	win_keymap[ VK_M] =   OSDK_M;
	win_keymap[ VK_N] =   OSDK_N;
	win_keymap[ VK_O] =   OSDK_O;
	win_keymap[ VK_P] =   OSDK_P;
	win_keymap[ VK_Q] =   OSDK_Q;
	win_keymap[ VK_R] =   OSDK_R;
	win_keymap[ VK_S] =   OSDK_S;
	win_keymap[ VK_T] =   OSDK_T;
	win_keymap[ VK_U] =   OSDK_U;
	win_keymap[ VK_V] =   OSDK_V;
	win_keymap[ VK_W] =   OSDK_W;
	win_keymap[ VK_X] =   OSDK_X;
	win_keymap[ VK_Y] =   OSDK_Y;
	win_keymap[ VK_Z] =   OSDK_Z;

	win_keymap[ VK_DELETE] =   OSDK_DELETE;

	win_keymap[ VK_NUMPAD0] =   OSDK_KP0;		
	win_keymap[ VK_NUMPAD1] =   OSDK_KP1;		
	win_keymap[ VK_NUMPAD2] =   OSDK_KP2;		
	win_keymap[ VK_NUMPAD3] =   OSDK_KP3;		
	win_keymap[ VK_NUMPAD4] =   OSDK_KP4;		
	win_keymap[ VK_NUMPAD5] =   OSDK_KP5;		
	win_keymap[ VK_NUMPAD6] =   OSDK_KP6;		
	win_keymap[ VK_NUMPAD7] =   OSDK_KP7;		
	win_keymap[ VK_NUMPAD8] =   OSDK_KP8;		
	win_keymap[ VK_NUMPAD9] =   OSDK_KP9;		
	win_keymap[ VK_DECIMAL] =   OSDK_KP_PERIOD	;
	win_keymap[ VK_DIVIDE] =    OSDK_KP_DIVIDE	;
	win_keymap[ VK_MULTIPLY] =   OSDK_KP_MULTIPLY;
	win_keymap[ VK_SUBTRACT] =    OSDK_KP_MINUS	;
	win_keymap[ VK_ADD] =    OSDK_KP_PLUS	;

	win_keymap[ VK_UP] =    OSDK_UP		;
	win_keymap[ VK_DOWN] =  OSDK_DOWN	;	
	win_keymap[ VK_RIGHT] = OSDK_RIGHT;		
	win_keymap[ VK_LEFT] =  OSDK_LEFT	;	
	win_keymap[ VK_INSERT] =OSDK_INSERT	;
	win_keymap[ VK_HOME] =  OSDK_HOME		;
	win_keymap[ VK_END] =   OSDK_END		;
	win_keymap[ VK_PRIOR] = OSDK_PAGEUP	;
	win_keymap[ VK_NEXT] =  OSDK_PAGEDOWN	;

	win_keymap[ VK_F1] =   OSDK_F1;
	win_keymap[ VK_F2] =   OSDK_F2;		
	win_keymap[ VK_F3] =   OSDK_F3;		
	win_keymap[ VK_F4] =   OSDK_F4;		
	win_keymap[ VK_F5] =   OSDK_F5;		
	win_keymap[ VK_F6] =   OSDK_F6;		
	win_keymap[ VK_F7] =   OSDK_F7;		
	win_keymap[ VK_F8] =   OSDK_F8;		
	win_keymap[ VK_F9] =   OSDK_F9;		
	win_keymap[ VK_F10] =   OSDK_F10;		
	win_keymap[ VK_F11] =   OSDK_F11;		
	win_keymap[ VK_F12] =   OSDK_F12;		
	win_keymap[ VK_F13] =   OSDK_F13;		
	win_keymap[ VK_F14] =   OSDK_F14;		
	win_keymap[ VK_F15] =   OSDK_F15;		

	win_keymap[ VK_NUMLOCK] =  OSDK_NUMLOCK 	;
	win_keymap[ VK_CAPITAL] =  OSDK_CAPSLOCK	;
	win_keymap[ VK_SCROLL] =   OSDK_SCROLLOCK	;
	win_keymap[ VK_SHIFT] =    OSDK_SHIFT		;
	win_keymap[ VK_RSHIFT] =   OSDK_RSHIFT	;
	win_keymap[ VK_LSHIFT] =   OSDK_LSHIFT	;
	win_keymap[ VK_CONTROL] =  OSDK_CTRL	;	
	win_keymap[ VK_RCONTROL] = OSDK_RCTRL	;	
	win_keymap[ VK_LCONTROL] = OSDK_LCTRL	;	
	win_keymap[ VK_RMENU] =    OSDK_RALT		;
	win_keymap[ VK_LMENU] =    OSDK_LALT		;
	win_keymap[ VK_MENU ] =    OSDK_ALT	;
	win_keymap[ VK_RWIN] =     OSDK_RSUPER	;
	win_keymap[ VK_LWIN] =     OSDK_LSUPER	;

	win_keymap[ VK_HELP] =     OSDK_HELP		;
	win_keymap[ VK_SNAPSHOT] = OSDK_PRINT	;	
	win_keymap[ VK_CANCEL] =   OSDK_BREAK	;	
	win_keymap[ VK_APPS] =     OSDK_MENU;


	win_keymap[ VK_AT ] =      OSDK_AT;				// @
	win_keymap[ VK_LBRACKET]=  OSDK_LEFTBRACKET;		// [
    win_keymap[ VK_RBRACKET]=   OSDK_RIGHTBRACKET;		// ]
    win_keymap[ VK_UPPER   ]=   OSDK_UPPER;			// ^

	win_keymap[ VK_UNDERSCORE] =OSDK_UNDERSCORE;	// _ 
//	win_keymap[ VK_KANA]    =   OSDK_KANA;			// kana

}

// ****************************************************************************
//          translate from scancode to keycode
// ****************************************************************************
int OSD_transkey( int scancode)
{
	int J;
	if( scancode >=0 && scancode <256)
		J= win_keymap[ scancode];
	else
		J= OSDK_UNKNOWN;
	return J;
}


unsigned char OSD_get_stick(void)
{
	char KeyState[256];
	unsigned char stick =0;

	GetKeyboardState(KeyState);		// キーボードの状態を取得
	if (KeyState[VK_SPACE] & 0x80)
		stick |= STICK0_SPACE;
	if (KeyState[VK_LEFT] & 0x80)
		stick |= STICK0_LEFT;
	if (KeyState[VK_RIGHT] & 0x80)
		stick |= STICK0_RIGHT;
	if (KeyState[VK_DOWN] & 0x80)
		stick |= STICK0_DOWN;
	if (KeyState[VK_UP] & 0x80)
		stick |= STICK0_UP;
	if (KeyState[VK_PAUSE] & 0x80)
		stick |= STICK0_STOP;
	if (KeyState[VK_SHIFT] & 0x80)
		stick |= STICK0_SHIFT;

	return stick;
}


// ****************************************************************************
//          　　　　論理パレット
// ****************************************************************************

static int        maxpalette = 16;		// 使用する色数
static HPALETTE   hpalette;				// 論理パレットのハンドル
static LOGPALETTE *lplogpal = NULL;		// 論理パレットを作るための注文用パレット
static RGBQUAD    dibPal[256];			// DIB BITMAP 用のパレット


// ****************************************************************************
//          create_lplogpal: 論理パレットの作成
// ****************************************************************************
int create_lplogpal(void)
{
	printf("Allocating log palette...");
	lplogpal = malloc( sizeof(LOGPALETTE)+ sizeof(PALETTEENTRY)*(256-1));
    if( lplogpal ==NULL) {printf("FAILED!"); return(0);}

    lplogpal->palVersion    = 0x0300;
    lplogpal->palNumEntries = maxpalette;

    printf("OK\n");
    return(1);
}

// ****************************************************************************
//          set_lplogpal: 論理パレットの設定
// ****************************************************************************
void set_lplogpal(int i ,int r ,int g ,int b)
{
    dibPal[i].rgbRed   = lplogpal->palPalEntry[i].peRed   = r;
    dibPal[i].rgbGreen = lplogpal->palPalEntry[i].peGreen = g;
    dibPal[i].rgbBlue  = lplogpal->palPalEntry[i].peBlue  = b;
    lplogpal->palPalEntry[i].peFlags = 0;

}

// ****************************************************************************
//          destroy_lplogpal: 論理パレットの解放
// ****************************************************************************
void destroy_lplogpal(void)
{
	if( lplogpal ) 
    	{
    	free( lplogpal);
    	DeleteObject( hpalette);
        lplogpal=NULL;
        }
}


int make_lplogpal(unsigned char rgb[256][4])
{
	int i;
	if( bitpix ==8)
  		{
		HPALETTE hpalette;
        printf("Creating Win32 log palette...");
		if(!create_lplogpal()) return 0;		// 256 カラーだと、論理パレットを作成

		for(i=0; i< 16 ; i++)
	 		set_lplogpal(i,rgb[i][2],rgb[i][1], rgb[i][0]);	//  Win32の論理パレットを作成する。

	  	hpalette = CreatePalette( lplogpal);	// Win32の 論理パレットを作成
        if( hpalette !=0) 
        	printf("OK\n");
        else
        	printf("FAILED \n");
        }
	return 1;
}



OSD_SetPalete( unsigned char rgb[256][4])
{
	make_lplogpal( rgb);
}

// ****************************************************************************
//          realize_lplogpal: 論理パレットをシステムパレットに反映
//  IN: hdc
//      flag TRUE / FALSE
// ****************************************************************************
int realize_lplogpal(HDC hdc,int flag)
{
 	int ret;
	ret=0;
	if( bitpix ==8)
    	{
		int tmp;
	  	SelectPalette( hdc , hpalette, flag);	// Win32 の論理パレットを選択
	    tmp = RealizePalette( hdc);					// Win32 の論理パレットを反映
	    //printf("Realized Palette == %d \n",tmp);
		ret=1;
        }
	return(ret);
}

// ****************************************************************************
//          OSD_Getcolor: RGB値から、実際にサーフェスに書き込むデータを取得
//  IN:   R,G,B,H
//  OUT: 
// ****************************************************************************
ColTyp OSD_Getcolor(int R ,int G ,int B ,int H)
{
	ColTyp col;

	col.ct_byte[3] = (H>>8) & 0xff;	   // not use
    col.ct_byte[2] = (R>>8) & 0xff;     // red
    col.ct_byte[1] = (G>>8) & 0xff;     // green 
    col.ct_byte[0] = (B>>8) & 0xff;     // blue

	return col;
}

// ****************************************************************************
//          OSD_GetXtab: RGB値の並ぶ順番を取得
//  IN:   non
//  OUT:  Xtab
// ****************************************************************************
byte * OSD_GetXtab(void)
{
	static byte Xtab[4]={0,1,2,3};
	
	return Xtab;
}



// ****************************************************************************
//          BITMAP CONTROL
// ****************************************************************************

/*
typedef struct tagBITMAPFILEHEADER { 	// ------- bmpFileHeader -----------
    WORD    bfType; 
    DWORD   bfSize; 
    WORD    bfReserved1; 
    WORD    bfReserved2; 
    DWORD   bfOffBits; 
} BITMAPFILEHEADER;

typedef struct tagBITMAPINFOHEADER{ 	// ------- hb.lpBmpInfoh -----------
    DWORD  biSize; 			// size of structor
    LONG   biWidth; 		// Width
    LONG   biHeight; 		// Height
    WORD   biPlanes; 		// 0
    WORD   biBitCount;		// 1/ 4 / 8 / 16 / 24 /32 bpp
    DWORD  biCompression; 	// BI_RGB: non compression
    DWORD  biSizeImage; 	// size of image
    LONG   biXPelsPerMeter; // 0
    LONG   biYPelsPerMeter; // 0
    DWORD  biClrUsed; 		// number of color table
    DWORD  biClrImportant;  // number of important color
} BITMAPINFOHEADER; 

typedef struct tagBITMAPINFO { // hb.lpBmpInfo
    BITMAPINFOHEADER bmiHeader; 
    RGBQUAD          bmiColors[1]; 
} BITMAPINFO;

typedef struct tagRGBQUAD {
    BYTE    rgbBlue; 
    BYTE    rgbGreen; 
    BYTE    rgbRed; 
    BYTE    rgbReserved; 
} RGBQUAD;

論理パレット構造体
typedef struct tagLOGPALETTE { // lgpl 
    WORD         palVersion; 	// 0x0300: palet version
    WORD         palNumEntries; // number of color table
    PALETTEENTRY palPalEntry[1]; 
} LOGPALETTE; 

typedef struct tagPALETTEENTRY { // pe 
    BYTE peRed; 
    BYTE peGreen; 
    BYTE peBlue; 
    BYTE peFlags; 
} PALETTEENTRY;


256カラーを描画するには、

//　論理パレットを作成する:
//  hpalette = CreatePalette( &logpalette);
//　デバイスコンテキストに、論理パレットを設定:
//  HPALETTE = SelectPalette( hdc, hpalette, TRUE / FALSE);
//　論理パレットを　システムパレットに設定
//  num = RealizePalette(hdc);

*/

// ****************************************************************************
//          OSD_CreateSurface: 新しいサーフェスを作成して、返却する
// In: HWND           handle of window
//     width, height  new bitmap size
//	   bitpix         color depth   1bpp 8bpp 24bpp 32bpp
//Out: OSD_surface    
// ****************************************************************************
OSD_Surface * OSD_CreateSurface(int width , int height, int bitpix ,int flags)
{
	HDC   hdc;
	HIBMP hb;
	int   flag;
	int   colors;
	HWND  hwnd;

	OSD_Surface *surface;


	hwnd = hwndMain;


	// ------------------------------------------------------------
	colors = (bitpix<=8)? 1<<bitpix: 1;
	
		hb.lpBmpInfo = malloc( sizeof( BITMAPINFO)+ sizeof(RGBQUAD)*(colors-1));
		hb.lpBmpInfoh= malloc( sizeof( BITMAPINFOHEADER));
	
		hb.lpBmpInfoh->biSize   = sizeof( BITMAPINFOHEADER);
		hb.lpBmpInfoh->biWidth  = width;
		hb.lpBmpInfoh->biHeight = height*(-1);	// top down bitmap
		hb.lpBmpInfoh->biPlanes = 1;
		hb.lpBmpInfoh->biBitCount= bitpix; /*depth; */
		hb.lpBmpInfoh->biCompression= BI_RGB;
		hb.lpBmpInfoh->biSizeImage  = 0;
		hb.lpBmpInfoh->biClrUsed    = maxpalette;
		hb.lpBmpInfoh->biClrImportant= 0;
		hb.lpBmpInfo->bmiHeader = *hb.lpBmpInfoh;


		hb.Width = width;
		hb.Height= height;

		// ---- Create dib section bitmap ---
		// flag   DIB_PAL_COLORS:  PALET INDEX   DIB_RGB_COLORS:  RGB COLOR
		// XBuf   pixel buffer  ( auto allocation memory)
		flag = DIB_RGB_COLORS;
	
		hdc= GetDC( hwnd);					 // get window DC
		hb.hBitmap1 = CreateDIBSection( hdc,(BITMAPINFO*)hb.lpBmpInfo, flag,(void**)&hb.lpBMP,NULL,0);
		if( hb.hBitmap1 !=NULL)
			{
			// ---- mapping to compatible DC ---
			hb.hdcBmp = CreateCompatibleDC(hdc);    // create compatible DC
			SelectObject(hb.hdcBmp, hb.hBitmap1);	 // select bitmap to compatible DC

			// ---- get bitmap information from bitmap handle (hBitmap1 --> bitmap1)
			//GetObject(   hBitmap1, sizeof(BITMAP), &bitmap1);
			}
		else
    		{
			printf("CreateDIBSection failed \n"); 
			putlasterror();
			exit(1);
			}

		SetDIBColorTable (hb.hdcBmp, 0, 236, dibPal);   //ＤＩＢにパレットを割り当て

		//test_putcolortable(hb);

		ReleaseDC( hwnd,hdc);

	surface = malloc( sizeof(OSD_Surface) );
	surface->format = malloc( sizeof(OSD_PixelFormat) );

	surface->format->BitsPerPixel = bitpix;
	surface->format->BytesPerPixel = bitpix/8;
	surface->w      = width;
	surface->h      = height;
	surface->pixels = hb.lpBMP;
	surface->handle = hb.hBitmap1;
	surface->hdcBmp = hb.hdcBmp;
    surface->pitch  = width * (bitpix/8);       // row width
	surface->flags  = SURFACE_BITMAP;			// bitmap

	if(bitpix==8) memset( surface->pixels, 16, surface->pitch *surface->h);		// clear true black
	return surface;
}


// ****************************************************************************
//          OSD_setwindowsurface: ウインドウのサーフェスを作成して、返却する
// In: width, height  window size
//	   bitpix         color depth   1bpp 8bpp 24bpp 32bpp
//Out: OSD_surface    
// ****************************************************************************
OSD_Surface * OSD_setwindowsurface(int width , int height, int bitpix,int flags)
{
	OSD_Surface *surface;

	surface = malloc( sizeof(OSD_Surface) );
	surface->format = malloc( sizeof(OSD_PixelFormat) );

	surface->format->BitsPerPixel = bitpix;
	surface->format->BytesPerPixel = bitpix/8;
	surface->w      = width;
	surface->h      = height;
	surface->pixels = NULL;
	surface->handle = 0;
	surface->hdcBmp = 0;
	surface->flags  = SURFACE_WINDOW;			// window

	if( flags & WINDOW_NOBOARDER == WINDOW_NOBOARDER) {		// resize window
		SetWindowPos( hwndMain, 0, 0, 0, width , height , SWP_NOMOVE);  
	} else {
		SetWindowPos( hwndMain, 0, 0, 0, width +BORDERW+PADDINGW*2 , height+BORDERH+PADDINGH*2 ,SWP_NOMOVE);
	}
	return surface;
}


int test_putcolortable(HIBMP hb)
{
    int i;
    for(i=0; i<256; i++)
    	{
	     printf("\n i= %3d ", i);
         printf("R == %2X ",hb.lpBmpInfo->bmiColors[i].rgbRed);
         printf("G == %2X ",hb.lpBmpInfo->bmiColors[i].rgbGreen);
         printf("B == %2X ",hb.lpBmpInfo->bmiColors[i].rgbBlue);
         printf("Re== %2X ",hb.lpBmpInfo->bmiColors[i].rgbReserved);
        }
	return(1);
}

// ****************************************************************************
//          OSD_ReleaseSurface: サーフェスを破棄する
//Int: OSD_surface    
// ****************************************************************************
void OSD_ReleaseSurface( OSD_Surface *surface)
{
//	DeleteDC(hb.hdcBmp);
//	DeleteObject(hb.hBitmap1);


	if( surface)
		{
		DeleteDC( surface->hdcBmp );
		DeleteObject( surface->handle );

		free( surface->format);
		free( surface);
		surface = NULL;
		}
}









// ****************************************************************************
//          ClearScr: エミュレータ画面を全クリア
// ****************************************************************************
/*void ClearScr()
{
 //memset(XBuf,0,Width*Height*bitpix/8);
}
*/

// ****************************************************************************
//          ClearWindow: ウインドウ全体をクリア
// ****************************************************************************
void OSD_ClearWindow( void)
{
#if 1
    HDC hdc;
    HBRUSH hbrush, hOldbrush;
	int w=0,h=0;
    int r=0,g=0,b=0;

	if( !screen ) return;

	w= screen->w *2 ;  h= screen->h *2;

    hdc = GetDC( hwndMain);
    hbrush = CreateSolidBrush( RGB(r,g,b));
    hOldbrush = (HBRUSH) SelectObject(hdc,hbrush);
	Rectangle( hdc, 0,0,w+paddingw ,h+paddingh); 

	ClearScr();

    SelectObject( hdc,hOldbrush);
    DeleteObject( hbrush);
    ReleaseDC(hwndMain,hdc);
    //InvalidateRect(hwndMain, NULL,1);
#endif

//		int r=0, g=0, b=0;
//	OSD_Setcolor_rgb( r ,g ,b);
//	OSD_Rectangle( surface ,0, 0, Width+paddingw , Height+paddingh);
//	ClearScr();


}



// ****************************************************************************
//          SetTitle: ウインドウのタイトルバーの変更
// ****************************************************************************
void OSD_SetWindowTitle(char *name)
{
  SetWindowText(hwndMain , name);		// add windy 2002/10/27
}




#ifndef SOUND
void PSGOut(register byte R,register byte V)
{
}
#endif


// ****************************************************************************
//          Application イベントループ
// ****************************************************************************
int OSD_RunApplicationEventLoop(void)
{
	MSG  message;
	while(GetMessage( &message,NULL,0,0))
		{
		if( !IsDialogMessage( hDebugDlg , &message))
			{
        	TranslateMessage(&message);	/* キーボードメッセージを変換*/
			DispatchMessage( &message);	/* Windows に制御を返す */
			}
        }
  return message.wParam;
}


// ****************************************************************************
//          Application イベントチェック
//    Out:  1: event 有り
//          0: event 無し
//         -1: エラー
// ****************************************************************************
int OSD_ChkEvent(void)
{
	int ret=0;
    MSG  message;

    if(PeekMessage( &message , NULL,0,0,PM_NOREMOVE	))
    	{
    	if(!GetMessage( &message,NULL,0,0)) return -1;
		if( !IsDialogMessage( hDebugDlg , &message))
			{
        	TranslateMessage(&message);	/* キーボードメッセージを変換*/
        	DispatchMessage( &message);	/* Windows に制御を返す */
			}
		 ret=1;
		}
	return ret;
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




update_debugdialog()
{
	SendMessage( hDebugDlg , WM_PAINT , 0,0);
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
	HDC dst_hdc;
	int sw,sh;	// source width , height
	int ew,eh;	// dest   width , height
	sw = src_surface->w;
	sh = src_surface->h;
//	ew = dst_surface->w;
//	eh = dst_surface->h;
	ew = w;
	eh = h;

	if( dst_surface->flags == SURFACE_WINDOW)
		dst_hdc= GetDC(hwndMain);
	else
		dst_hdc= dst_surface->hdcBmp;

	if( (sw == ew && sh == eh) || dst_surface == surface2)	// FIX ME!: スクロールの切り貼りのときは、BitBlt を使う　暫定
		    BitBlt( dst_hdc ,  ex, ey ,w   ,h ,src_surface->hdcBmp ,sx ,sy ,SRCCOPY);
	else
		StretchBlt( dst_hdc ,  ex, ey ,ew ,eh ,src_surface->hdcBmp ,sx ,sy , sw, sh , SRCCOPY);
		

	if( dst_surface->flags ==SURFACE_WINDOW)
		ReleaseDC(hwndMain,dst_hdc);
}


int OSD_LockSurface(OSD_Surface *surface)
{
	return 1;
}

int OSD_UnlockSurface(OSD_Surface *surface)
{
	return 1;
}

// ****************************************************************************
//          OSD_Setcolor: 色を設定する
// In:  color : P6 の色のインデックス
// ****************************************************************************
void OSD_Setcolor( int color)
{

	if( color>0 && color <=16)
		foreground_color = RGB( dibPal[color].rgbRed , dibPal[color].rgbGreen , dibPal[color].rgbBlue);
	else
		foreground_color = RGB( 0,0,0);		// true black
}

// ****************************************************************************
//          OSD_Setcolor: 色を設定する
// In:  r,g,b : RGB の色の深さ
// ****************************************************************************
void OSD_Setcolor_rgb( int r, int g ,int b)
{
	foreground_color = RGB( r,g,b);
}


// ****************************************************************************
//          OSD_Rectangle: 長方形を描画する
// In:  surface : 出力先サーフェス
//      sx ,sy  : 左上の座標
//      w  ,h   : サイズ
// ****************************************************************************
void OSD_Rectangle( OSD_Surface *surface ,int sx ,int sy ,int w ,int h ,int boxFill)
{
	HDC hdc;
	HBRUSH hbrush, hOldbrush;
	int ex= sx+ w;
	int ey= sy+ h;

	if( surface->flags == SURFACE_WINDOW)
		hdc= GetDC(hwndMain);
	else
		hdc= surface->hdcBmp;

	hbrush = CreateSolidBrush( foreground_color );
	hOldbrush = (HBRUSH) SelectObject(hdc ,hbrush);
	
	if( boxFill)
		Rectangle( hdc, sx   ,sy  , ex , ey); 
	else {
		MoveToEx( hdc,  sx   ,sy  , NULL);
		LineTo  ( hdc,  sx   ,ey  );
		LineTo  ( hdc,  ex   ,ey  );
		LineTo  ( hdc,  ex   ,sy  );
	}

	SelectObject( hdc,hOldbrush);
	DeleteObject( hbrush);

	if( surface->flags ==SURFACE_WINDOW)
		ReleaseDC(hwndMain,hdc);
}


// ****************************************************************************
//          OSD_textout: サーフェスに文字を出力
//   surface: サーフェス
//   surface->flags  0:対象はbitmap  1:対象はscreen
//   x,y      座標
//   str      文字列
//   size     文字の大きさ
// ****************************************************************************
int OSD_textout(OSD_Surface *surface ,int x,int y,char *str ,int size)
{
	HFONT hfont;
	HFONT hfontbak;
	HDC hdc;
	int ret;

	if( surface->flags == SURFACE_WINDOW)
		hdc   = GetDC( hwndMain);	// screen
	else
		hdc   = surface->hdcBmp;	// bitmap

	hfont = CreateFont(size,0, 
    				FW_DONTCARE, FW_DONTCARE, FW_REGULAR,
					FALSE, FALSE, FALSE, 
					SHIFTJIS_CHARSET,
					OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
					DEFAULT_QUALITY, 
					FF_MODERN | FIXED_PITCH,
					 "");

	if( hfont ==NULL )
		{
		printf("[Win32gui][textout] CreateFont failed \n"); 
		return 0;
		}

	hfontbak = SelectObject(hdc, hfont);

	//SetTextColor( hdc ,RGB(110,110,110));     // character color

	SetTextColor( hdc ,foreground_color);     // character color


	SetBkColor(   hdc, RGB(0,0,0));


	TextOut(hdc ,x,y,str,strlen(str));    // write to window or bitmap


	// フォントの解放
	SelectObject( hdc, hfontbak );
	DeleteObject( hfont );

	if( surface->flags == SURFACE_WINDOW)
		ReleaseDC( hwndMain, hdc);

	ret=1;
	return ret ;
}


OSD_Surface * OSD_GetVideoSurface()
{
	return screen;
}

// ****************************************************************************
//          ロケールを取得
// ****************************************************************************
int OSD_getlocale(void)
{
	int ret;
	 WORD ja_JP = MAKELANGID(LANG_JAPANESE, SUBLANG_JAPANESE_JAPAN);
	 WORD en_US = MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US);
	 
	 DWORD usrLang = GetUserDefaultLCID();

	 if (usrLang == ja_JP) {
		 ret = OSDL_JP;
	 }else 
	 if (usrLang == en_US) {
		 ret = OSDL_EN;
	 }
	 else
	 {
		 ret = OSDL_OT;
	 }
	 return ret;
}

// ****************************************************************************
//          ロケールをメッセージで取得
// ****************************************************************************
char * OSD_getlocale_msg(void)
{
	static char msg[3];
	int stat = OSD_getlocale();
	switch (stat)
	{
	case OSDL_EN: strcpy(msg, "LANG_EN"); break;
	case OSDL_JP: strcpy(msg, "LANG_JP"); break;
	}
	return msg;
}





// ****************************************************************************
///          WinMain: MS-Windows のメイン関数
// ****************************************************************************
int WINAPI WinMain( HINSTANCE hThisInst, HINSTANCE hPrevInst, LPSTR lpszArgs, int nWinMode)
{
	int ret;
    char *argv[100];
    int argc;
	static char tmp[1020];

    _hThisInst = hThisInst;	// save hThisInst 
	_hPrevInst = hPrevInst;
	_lpszArgs  = lpszArgs;
	_nWinMode   = nWinMode;

	strcpy( tmp , "ip6plus.exe ");
	strncat( tmp , lpszArgs , 1000);

	conv_argv( tmp, &argc , argv);     // convert lpszArgs --> argc, argv
	if(!chkOption( argc, argv)) return(0);	// <--- option check

	ret = main( argc,argv);

	return ret;
}






/* ************************************** not use ********************************************************* */

static int keyrepeat=-1;
static int new_keyrepeat;

// ****************************************************************************
//          setKeyrepeat: キーリピートの速度を設定する
// ****************************************************************************
void setKeyrepeat(int value)
{
#if 0
 	if( keyrepeat ==-1)		/* 起動時のキーリピートの値を取得する */
		SystemParametersInfo(SPI_GETKEYBOARDSPEED, 0    , &keyrepeat, 0);	// get key repeat
	SystemParametersInfo(SPI_SETKEYBOARDSPEED, value, 0, 0);	// set key repeat
    new_keyrepeat = value;
#endif
}

// ****************************************************************************
//          storeKeyrepeat: キーリピートの速度を元に戻す
// ****************************************************************************
void storeKeyrepeat(void)
{
#if 0
	if( keyrepeat !=-1)
    	{
/*		if( keyrepeat == new_keyrepeat)
			keyrepeat=30; */

     	SystemParametersInfo(SPI_SETKEYBOARDSPEED, keyrepeat, 0, 0);	// restore key repeat 
        keyrepeat = -1;
        }
#endif
}



// ****************************************************************************
//          usleep: UNIXの usleep 代替関数
// ****************************************************************************
// 参考URL: http://www.gidforums.com/t-14836.html
int usleep (unsigned int us) {
  int i=0;
  LARGE_INTEGER start, end, d;

  QueryPerformanceCounter(&start);								//Get the start interval
  //d.QuadPart = g_ptFreq.QuadPart * ((double)us / 1000000.0);	//Caculate the total number of tics are equivalent to (us) microseconds
  //d.QuadPart =  ((double)us / 1000000.0);	//Caculate the total number of tics are equivalent to (us) microseconds
  d.QuadPart =  us*2;

							// Tic number [color=white].................[/color]  (us)
							//  ---------- = second = ----------
							//  Frequence [color=white].................[/color]1 000 000
  do {
    QueryPerformanceCounter(&end);								//Get the end interval
	Sleep(0);
	i++;
  } while (end.QuadPart - start.QuadPart < d.QuadPart);			//Check if the end is older
  
  return 0;

}


#if 0		// moves to iP6.c
    InitVariable();			// <--- init variable
    init_keybuffer();			// <--- init key buffer
	OSD_initKeymap();		// init keymap

    ConfigInit();				// <--- config init
    ConfigRead();				// <--- config read 

	ret = OsdCpuRun();			// VM run
//         hCPUThread = CreateThread( NULL,0,(LPTHREAD_START_ROUTINE) CPUThreadProc ,NULL,0,&dwThreadId);
//         CloseHandle( hCPUThread);
#endif


#endif // WIN32

