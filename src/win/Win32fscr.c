/** iP6: PC-6000/6600 series emualtor ************************/
/**                                                         **/
/**                           Win32fscr.c                   **/
/**                                                         **/
/** by Windy 2003                                           **/
/*************************************************************/
/*
フルスクリーン　<-->  窓の切り替えです。
Win32 API版

Date:  2003/9/29
*/
#ifdef WIN32
#include <windows.h>
#include <mmsystem.h>
#include <stdio.h>
#include <time.h>

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <signal.h>
#include <direct.h>
#include "../Refresh.h"
#include "../P6.h"
#include "WinMenu.h"
#include "Win32.h"
#include "Win32gui.h"
#include "Win32fscr.h"

//extern int Verbose;
extern int paddingw,paddingh;
extern void setMenuOnOff(int type ,int sw);
extern HWND hwndMain;


static DEVMODE pre_devmode;
static int  isFullScr=0;


void savewindowstate( HWND hwnd);
void restorewindowstate(HWND hwnd);


// ****************************************************************************
//          isFullScreen: ディスプレイモードの取得
// ****************************************************************************
int isFullScreen(void)
{
	return( isFullScr);
}


// ****************************************************************************
//          saveDisplayMode: ディスプレイモードの保存
// ****************************************************************************
void saveDisplayMode(void)
{
	HDC	hdc;
	
    hdc = GetDC(0);

	pre_devmode.dmSize = sizeof(DEVMODE);
	pre_devmode.dmPelsWidth  = GetDeviceCaps(hdc, HORZRES);
	pre_devmode.dmPelsHeight = GetDeviceCaps(hdc, VERTRES);
	pre_devmode.dmFields = DM_PELSWIDTH | DM_PELSHEIGHT;

	// Windows NT/2000用
	if ((GetVersion() & 0x80000000) == 0) 
    	{				
		pre_devmode.dmFields |= DM_DISPLAYFREQUENCY;
		pre_devmode.dmDisplayFrequency = GetDeviceCaps(hdc, VREFRESH);
		}

	ReleaseDC(0,hdc);
}

// ****************************************************************************
//          storeDisplayMode: ディスプレィモードを元に戻す。
// ****************************************************************************
void storeDisplayMode(void)
{

	 if( isFullScr) toggleFullScr();
}


// ****************************************************************************
//          togglefullScr: フルスクリーン <---> ウインドウ
// ****************************************************************************
int toggleFullScr(void)
{
	int ret=0;
	DEVMODE	devmode;


	if( !isFullScr )
    	{			// change to full screen 
		int xx ,yy;
		setMenuOnOff( IDM_SCREENSIZE , 0);
		if(scale ==1 && win_scale !=3 ) resizewindow(2.0 ,2.0 ,0);		/* scale 1なら２にする */

		if( win_scale ==2) { xx = 320* win_scale; yy = 240* win_scale; }
		if( win_scale ==3) { xx = 1024; yy = 768; }


		devmode = pre_devmode;
		devmode.dmPelsWidth  = xx;
		devmode.dmPelsHeight = yy;
		devmode.dmFields = DM_PELSWIDTH | DM_PELSHEIGHT;

		if(Verbose) printf("Changing to FULL SCREEN ...");

        savewindowstate( hwndMain);

		if (SetWindowPos(hwndMain,NULL,0,0, 0 ,0 ,SWP_NOMOVE | SWP_NOSIZE |SWP_SHOWWINDOW))
		  if (SetWindowLong(hwndMain,GWL_STYLE,WS_VISIBLE))
			if (ChangeDisplaySettings(&devmode, CDS_FULLSCREEN) == DISP_CHANGE_SUCCESSFUL)
				if (SetWindowPos(hwndMain,HWND_TOPMOST,0,0, devmode.dmPelsWidth ,devmode.dmPelsHeight ,SWP_SHOWWINDOW))
                	{
               		if(Verbose) printf("OK\n");

                     savemenu();	// save menu
					 hidemenu();	// hide menu
                     paddingw = 0;	// SET padding zero
                     paddingh = PADDINGH/2; // SET padding half
                     isFullScr=1;
					 ret=1;
                    }
        }
	else
    	{			// change to window
		if(Verbose) printf("Changing to WINDOW ...");
		devmode = pre_devmode;
		if (SetWindowPos(hwndMain,NULL,0,0, 0 ,0 ,SWP_NOMOVE | SWP_NOSIZE |SWP_SHOWWINDOW))
	      if( ChangeDisplaySettings( &devmode , CDS_FULLSCREEN ) == DISP_CHANGE_SUCCESSFUL )
            {
       		if(Verbose) printf("OK\n");
	        restorewindowstate( hwndMain);
			restoremenu();
			setMenuOnOff( IDM_SCREENSIZE , 1);
	         paddingw = PADDINGW;		// RESTORE padding 
    	     paddingh = PADDINGH;
        	 isFullScr=0;
         	 ret=1;
            }
        }

	if( Verbose && !ret) printf("FAILED \n");
	OSD_ClearWindow();						// clear window

	return(ret);
}



static RECT  prev_rect;
static DWORD prev_wndstyle;
static DWORD prev_wndstyleEx;


// ****************************************************************************
//          savewindowstate: ウインドウの状態を保存
// ****************************************************************************
void savewindowstate( HWND hwnd)
{
	GetWindowRect( hwnd , &prev_rect);
	prev_wndstyle   = GetWindowLong( hwnd , GWL_STYLE);
	prev_wndstyleEx = GetWindowLong( hwnd , GWL_EXSTYLE);
}

// ****************************************************************************
//          restorewindowstate: ウインドウの状態を復帰
// ****************************************************************************
void restorewindowstate(HWND hwnd)
{
	SetWindowLong( hwnd ,GWL_STYLE   , prev_wndstyle);
	SetWindowLong( hwnd ,GWL_EXSTYLE , prev_wndstyleEx);
	SetWindowPos(  hwnd ,HWND_NOTOPMOST,prev_rect.left ,
    									prev_rect.top  ,
                                        prev_rect.right - prev_rect.left,
                                        prev_rect.bottom- prev_rect.top,
                                        SWP_SHOWWINDOW);
}




#endif
