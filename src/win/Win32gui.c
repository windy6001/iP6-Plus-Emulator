/** iP6: PC-6000/6600 series emualtor ************************/
/**                                                         **/
/**                           Win32gui.c                    **/
/** by Windy 2002-2004                                      **/
/*************************************************************/
// Win32 のGUI周りです。


#ifdef WIN32
#include <windows.h>
#include <commctrl.h>

#include <stdio.h>
#include <time.h>
#include <assert.h>

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <signal.h>
#include <direct.h>

#include <shlobj.h>

#include "../P6.h"
#include "../Refresh.h"
#include "../Sound.h"
#include "../fm.h"

#include "WinMenu.h"
#include "../Option.h"
#include "../Build.h"
#include "../Debug.h"
#include "../message.h"
#include "../buffer.h"
#include "../pngrw.h"	// for write_png()

#include "Win32.h"
#include "Win32fscr.h"
#include "Win32gui.h"

#include "../Timer.h"
#include "../mem.h"
#include "../dokodemo.h"
#include "../romaji.h"
#include "../resource.h"

HMENU getPopupMenu( void);
extern int isScroll(void);
extern int  setMenuCheck(int type ,int sw);
extern void setMenuOnOff(int type ,int sw);
extern int savescreenshot(char *path);


void showRegisters(HWND hdwnd);
void OnPushExecuteDebugCommand(HWND hdwnd , char *command);
void changeDialogToEnglish(HWND hdwnd);

int OSD_MessageBox(const char *mes, const char *cap, int type);


// ****************************************************************************
//          configure variable: コンフィグダイアログの 変数
// ****************************************************************************
static char Ext1Name_bak[PATH_MAX] = {""}; /* Extension ROM 1 backup */

static HWND hEditboxWnd1,hEditboxWnd2,hEditboxWnd3;
static HWND udWnd1,udWnd2 ,udWnd3;
static HWND hSoundRatebox;

static HWND hPSGVolWnd;
static HWND hFMVolWnd;

static int  CPUclock_bak;

int   UseJoystick;						// use joystick 1: use  add 2003/8/31


extern int dpiX,dpiY;		// screen DPI
float dpiBairitu;	// screen bairitu  96dpi を１にした


static is_maximized;	// 1:最大化した 0:してない

// ****************************************************************************
//          debug variable: デバッグダイアログの 変数
// ****************************************************************************
HWND  hDebugDlg =0;		// debug dialog window handle


// ****************************************************************************
//          _win32_getWindowHandle: HWND (ウインドウのハンドル）取得
// ****************************************************************************
HWND _win32_getWindowHandle(void)
{
	return( hwndMain);
}



// ****************************************************************************
//          _win32_gethinstance: HINSTANCE 取得
// ****************************************************************************
HINSTANCE _win32_gethinstance(void)
{
	return( (HINSTANCE) GetWindowLong( _win32_getWindowHandle() , GWL_HINSTANCE));
}

static void compatiblerom_onoff(HWND hdwnd)
{
	int stat;
	stat = !new_Use_CompatibleROM;
	EnableWindow(GetDlgItem(hdwnd, ID_RDPC62), stat);
	EnableWindow(GetDlgItem(hdwnd, ID_RDPC64), stat);
	EnableWindow(GetDlgItem(hdwnd, ID_RDPC68), stat);
}

// ****************************************************************************
//          configure_OnInitDialog: コンフィグダイアログの 初期化処理
// ****************************************************************************
// configureFunc から呼ばれる。
static int configure_OnInitDialog(HWND hdwnd)
{
 HINSTANCE hInst;
 hInst = _win32_gethinstance();
 
	{// --------- set Configure title ------
    char  tmp[100];
    char  str[ 1024];
	switch( P6Version)
		{
		case 0: strcpy( tmp , "PC-6001");     break;
		case 1: strcpy( tmp , "PC-6001mk2");  break;
		case 2: strcpy( tmp , "PC-6001mk2SR");break;
		case 3: strcpy( tmp , "PC-6601");     break;
		case 4: strcpy( tmp , "PC-6601SR");   break;
		}
	sprintf(str,"Configure - %s  %s   [ %s ]",PROGRAM_NAME ,BUILD_VER, tmp);
	SetWindowText( hdwnd, str );
    }

	compatiblerom_onoff( hdwnd);	// Use_compatibleROM がTRUE だと、互換ROMがない機種は無効にする

										// ---- use compatible ROM -----
	SendDlgItemMessage(hdwnd, ID_USE_COMPATIBLEROM, BM_SETCHECK, new_Use_CompatibleROM, 0);

	// ************* Init RADIO BUTTONS and CHECKBOXES *************************
    /* AUTORADIOBUTTON の場合、チェックしたい奴に BST_CHECKED 送るだけでいいみたい*/
										 // ---- machine type -----
    SendDlgItemMessage( hdwnd , ID_RDPC60+newP6Version*2,BM_SETCHECK,BST_CHECKED,0);
                                         // ---- fd numbers -------
    SendDlgItemMessage( hdwnd , ID_RDFD0+new_disk_num, BM_SETCHECK, BST_CHECKED,0);
                                         // ----- screen scale --------
    SendDlgItemMessage( hdwnd , ID_SCALE1+scale-1,BM_SETCHECK,BST_CHECKED,0);
                                         // ---- fast tape -------
	SendDlgItemMessage( hdwnd , ID_FASTTAPE, BM_SETCHECK , FastTape,0);
                                         // ------ screen update ----
	//printf("Initdialog: UPeriod %d \n",UPeriod);

	SendDlgItemMessage( hdwnd , ID_60FPS+UPeriod-1, BM_SETCHECK,BST_CHECKED,0);
                                         // ------ screen 4 color ----
    SendDlgItemMessage( hdwnd , ID_SCR4MONO+scr4col,BM_SETCHECK,BST_CHECKED,0);
                                         // -- scan line -- 2003/10/24
	SendDlgItemMessage( hdwnd , ID_SCANLINE, BM_SETCHECK , IntLac,0);
	IntLac_bak = IntLac;

	if(P6Version != PC60) {EnableWindow( GetDlgItem( hdwnd, ID_EXTRAM32), FALSE);}	/* not PC6001: disable extram */
	if(P6Version == PC60) {EnableWindow( GetDlgItem( hdwnd, ID_EXTRAM64), FALSE); new_use_extram64=0;}	/* PC6001: disable extram64 */
	SendDlgItemMessage( hdwnd , ID_EXTKANJION  , BM_SETCHECK , new_extkanjirom,0);
	SendDlgItemMessage( hdwnd , ID_ROMAJI_MODE , BM_SETCHECK , romaji_mode, 0);

    SendDlgItemMessage( hdwnd , ID_EXTRAM32    , BM_SETCHECK , new_use_extram32,0);
    SendDlgItemMessage( hdwnd , ID_EXTRAM64    , BM_SETCHECK , new_use_extram64,0);

    SendDlgItemMessage( hdwnd , ID_SOUNDON     , BM_SETCHECK , newUseSound,0);
	SendDlgItemMessage( hdwnd , ID_SAVETAPEMENU, BM_SETCHECK , UseSaveTapeMenu,0);
	SendDlgItemMessage( hdwnd , ID_STATUSBAR   , BM_SETCHECK , UseStatusBar,0);
	SendDlgItemMessage( hdwnd , ID_DISKLAMP    , BM_SETCHECK , UseDiskLamp ,0);
 	
	//SendDlgItemMessage( hdwnd , ID_KEYCLICK_SOUND, BM_SETCHECK , keyclick ,0);
	SendDlgItemMessage( hdwnd , ID_STOP_2MSTIMER, BM_SETCHECK , !TimerSWFlag ,0);

	

	// ************ volume ******************************
	{
	int tmp = psg_vol; 
	hPSGVolWnd = CreateWindow( TRACKBAR_CLASS, "track bar", WS_CHILD | WS_VISIBLE |WS_TABSTOP | TBS_AUTOTICKS | TBS_TOOLTIPS |WS_BORDER,
											(int)(375.0 *dpiBairitu),(int)(48.0 *dpiBairitu),(int)(104.0*dpiBairitu) ,(int)(20.0*dpiBairitu),hdwnd, NULL,hInst, NULL);
	SendMessage( hPSGVolWnd , TBM_SETRANGE, (WPARAM) 1 , (LPARAM) MAKELONG( 0,20));	// set trackbar range
	SendMessage( hPSGVolWnd , TBM_SETPOS  , (WPARAM) 1 , (LPARAM) tmp);				// set trackbar first position
	}
	{
	int tmp = fm_vol; 
	hFMVolWnd = CreateWindow( TRACKBAR_CLASS, "track bar", WS_CHILD | WS_VISIBLE |WS_TABSTOP | TBS_AUTOTICKS | TBS_TOOLTIPS |WS_BORDER,
											(int)(375.0 *dpiBairitu) ,(int)(76.0 *dpiBairitu),(int)(104.0*dpiBairitu) ,(int)(20.0*dpiBairitu) ,hdwnd, NULL,hInst, NULL);
	SendMessage( hFMVolWnd , TBM_SETRANGE, (WPARAM) 1 , (LPARAM) MAKELONG( 0,20));	// set trackbar range
	SendMessage( hFMVolWnd , TBM_SETPOS  , (WPARAM) 1 , (LPARAM) tmp);				// set trackbar first position
	}


	// ************ set spin edit **************************
 	hEditboxWnd1 =  GetDlgItem( hdwnd, ID_CPUCLOCK);	// ---- cpu clock ------
 	/*udWnd1 = */ CreateUpDownControl( WS_CHILD | WS_BORDER | WS_VISIBLE | 
 	    UDS_SETBUDDYINT | UDS_ALIGNRIGHT, 
 	    5,100,20,10, hdwnd, ID_CPUCLOCK_UPDOWN ,hInst, hEditboxWnd1, 20,1,CPUclock);

#if 0
 	hEditboxWnd2 =  GetDlgItem( hdwnd, ID_DRAWWAIT);	// ---- drawwait ------
 	  /*udWnd2 = */ CreateUpDownControl( WS_CHILD | WS_BORDER | WS_VISIBLE | 
 	    UDS_SETBUDDYINT | UDS_ALIGNRIGHT, 
 	    5,100,20,10, hdwnd, ID_CPUCLOCK_UPDOWN ,hInst, hEditboxWnd2, 192,0,drawwait);

 	hEditboxWnd3 =  GetDlgItem( hdwnd, ID_SRLINE);	// ---- sr wait ------
 	  /*udWnd3 = */ CreateUpDownControl( WS_CHILD | WS_BORDER | WS_VISIBLE | 
 	    UDS_SETBUDDYINT | UDS_ALIGNRIGHT, 
 	    5,100,20,10, hdwnd, ID_SRLINE_UPDOWN ,hInst, hEditboxWnd3, 80,0,srline);

#endif
/*
    hSoundRatebox = CreateWindow("Sound Rate", NULL ,WS_CHILD| WS_VISIBLE | CBS_SORT | CBS_SIMPLE,
     150,10,300,20, hwndMain , (HMENU)1,(HINSTANCE)GetWindowLong(hwndMain , GWL_HINSTANCE), NULL);

     LoadString((HINSTANCE)GetWindowLong(hwndMain , GWL_HINSTANCE) ,	ID_NEXTBOOTTIME ,nextboottime ,1024);
*/
										// ---- Extend Rom Name ---  2003/10/16
	SetDlgItemText( hdwnd, ID_EXTROMNAME ,Ext1Name);
	my_strncpy( Ext1Name_bak , Ext1Name, PATH_MAX);

	SetDlgItemText( hdwnd, ID_ROMPATH ,RomPath);// rom path 2007/5/12

	strcpy( RomPath_bak , RomPath);

	CPUclock_bak = CPUclock;

	if (OSD_getlocale() != OSDL_JP)	// 現在の言語設定を読み込み
	{
		changeDialogToEnglish(hdwnd);		// 設定ダイアログの日本語を、英語にかえる
	}
	return(1);
}

// 設定ダイアログの日本語を、英語にかえる

void changeDialogToEnglish(HWND hdwnd)
{
	HWND dlgHandle;
	dlgHandle = GetDlgItem(hdwnd, ID_MACHINE_TYPE);
	SetWindowText(dlgHandle, "Machine type (*)");

	dlgHandle = GetDlgItem(hdwnd, ID_FDD_NUM);
	SetWindowText(dlgHandle, "FDD numbers (*)");

	dlgHandle = GetDlgItem(hdwnd, ID_FASTTAPE);
	SetWindowText(dlgHandle, "Fast mode (*)");

	dlgHandle = GetDlgItem(hdwnd, ID_SCREEN4_COLOR);
	SetWindowText(dlgHandle, "screen 4 color(N60)");

	dlgHandle = GetDlgItem(hdwnd, ID_SCREEN_UPDATE);
	SetWindowText(dlgHandle, "Screen update");

	dlgHandle = GetDlgItem(hdwnd, ID_SCREEN_SIZE);
	SetWindowText(dlgHandle, "screen size");

	dlgHandle = GetDlgItem(hdwnd, ID_SCALE1);
	SetWindowText(dlgHandle, "x1");

	dlgHandle = GetDlgItem(hdwnd, ID_SCALE2);
	SetWindowText(dlgHandle, "x2");

	dlgHandle = GetDlgItem(hdwnd, ID_VIEW);
	SetWindowText(dlgHandle, "View");

	dlgHandle = GetDlgItem(hdwnd, ID_SCANLINE);
	SetWindowText(dlgHandle, "Scan Line");

	dlgHandle = GetDlgItem(hdwnd, ID_DISKLAMP);
	SetWindowText(dlgHandle, "Disk Lamp");


	// ---------------------------------------------------------
	dlgHandle = GetDlgItem(hdwnd, ID_CARTRIDGE);
	SetWindowText(dlgHandle, "Cartridge");

	dlgHandle = GetDlgItem(hdwnd, ID_ROM_PATH);
	SetWindowText(dlgHandle, "ROM PATH (*)");

	dlgHandle = GetDlgItem(hdwnd, ID_EXTRAM32);
	SetWindowText(dlgHandle, "PC-6001 RAM 32KB (*)");

	dlgHandle = GetDlgItem(hdwnd, ID_EXTRAM64);
	SetWindowText(dlgHandle, "Extended RAM 64KB (*)");

	dlgHandle = GetDlgItem(hdwnd, ID_EXTKANJION);
	SetWindowText(dlgHandle, "Extended KANJI ROM (*)");

	dlgHandle = GetDlgItem(hdwnd, ID_USE_COMPATIBLEROM);
	SetWindowText(dlgHandle, "Use Compatible ROM (*)");

	dlgHandle = GetDlgItem(hdwnd, ID_BROWSEROMPATH);
	SetWindowText(dlgHandle, "Ref..");



	dlgHandle = GetDlgItem(hdwnd, ID_STATUSBAR);
	SetWindowText(dlgHandle, "Status bar");

	dlgHandle = GetDlgItem(hdwnd, ID_ROMAJI_MODE);
	SetWindowText(dlgHandle, "Kana: Romaji mode");

	dlgHandle = GetDlgItem(hdwnd, ID_PSG_VOL);
	SetWindowText(dlgHandle, "PSG vol");

	dlgHandle = GetDlgItem(hdwnd, ID_FM_VOL);
	SetWindowText(dlgHandle, "FM vol");

	dlgHandle = GetDlgItem(hdwnd, ID_SOUNDON);
	SetWindowText(dlgHandle, "Use sound (*)");

}



// ****************************************************************************
//          configure_OnClickOK: コンフィグダイアログの OK ボタンを押された処理
// ****************************************************************************
// configureFunc から呼ばれる。
static int configure_OnClickOK(HWND hdwnd)
{
int  alert;
int  i;

//unsigned char nextboottime[]={"Please REBOOT 'iP6 Plus' to enable the change of options."};
unsigned char nextboottime[] = "MS_REBOOT_NEW_SETTING"; // MSG_ENABLE_CONFIG_REBOOT

	alert=0;
	psg_vol = SendMessage(hPSGVolWnd, TBM_GETPOS, 0, 0);
	fm_vol  = SendMessage(hFMVolWnd , TBM_GETPOS, 0, 0);

	ym2203_SetVolumePSG(psg_vol);						// set volume PSG
	ym2203_SetVolumeFM(fm_vol);							// set volume FM

	for(i=ID_RDPC60; i<=ID_RDPC68 ; i+=2)		// machine type
	  if(SendDlgItemMessage(hdwnd,i,BM_GETCHECK,0,0)==BST_CHECKED)
		{
		 newP6Version= (i- ID_RDPC60)/2;
		 if( newP6Version != P6Version)    /* 2003/5/24 */
		 	alert=1;
		 break;
		}
                                    		// -- sound --
	newUseSound = SendDlgItemMessage( hdwnd , ID_SOUNDON, BM_GETCHECK , 0,0);
	if( newUseSound != UseSound)
			alert=1;

	for(i=ID_RDFD0; i<=ID_RDFD2 ; i++)			// -- fd numbers --
	  if(SendDlgItemMessage(hdwnd,i,BM_GETCHECK,0,0)==BST_CHECKED)
		{
		new_disk_num = (i- ID_RDFD0);
		if( new_disk_num != disk_num)
			alert=1;
		break;
		}

	for(i=ID_SCALE1; i<=ID_SCALE2 ; i++)		// -- screen scale --
	  if(SendDlgItemMessage(hdwnd,i,BM_GETCHECK,0,0)==BST_CHECKED)
		{
		new_scale = (i- ID_SCALE1)+1;
		 if( new_scale != scale)
		   {
	        if( !isFullScreen())
	        	{
				scale= new_scale;
				resizewindow( scale, scale ,0); 
				setMenuCheck( IDM_SCREENSIZE , scale-1);	// menu check
	            }
		   }
		break;
		}
                                    // -- tape fast --
	if( !CasMode)                       // not moving tape
	    FastTape = SendDlgItemMessage( hdwnd , ID_FASTTAPE, BM_GETCHECK , 0,0);

    
	for(i=ID_60FPS; i<=ID_30FPS ; i++)			// -- screen update --
	  if(SendDlgItemMessage(hdwnd,i,BM_GETCHECK,0,0)==BST_CHECKED)
		{
		UPeriod = (i- ID_60FPS)+1;
		UPeriod_bak = UPeriod;
		//printf(" UPeriod == %d \n", UPeriod );
		break;
		}
	for(i=ID_SCR4MONO; i<=ID_SCR4COL ; i++)		// -- screen 4 color --
	  if(SendDlgItemMessage(hdwnd,i,BM_GETCHECK,0,0)==BST_CHECKED)
		{
		scr4col = (i- ID_SCR4MONO);
		break;
		}
    		                                // -- scan line --
	IntLac = SendDlgItemMessage( hdwnd , ID_SCANLINE, BM_GETCHECK , 0,0);
	setMenuCheck( IDM_SCANLINE, IntLac);
											// ----  status bar --
	UseStatusBar    = SendDlgItemMessage( hdwnd ,ID_STATUSBAR, BM_GETCHECK , 0,0);
	setMenuCheck( IDM_STATUSBAR, UseStatusBar);
											// ----  use disk lamp ---
	UseDiskLamp     = SendDlgItemMessage( hdwnd , ID_DISKLAMP, BM_GETCHECK , 0,0);
	setMenuCheck( IDM_DISK_LAMP, UseDiskLamp);
    		                                // -- extend kanjirom --
	new_extkanjirom = SendDlgItemMessage( hdwnd , ID_EXTKANJION, BM_GETCHECK,0,0);
	if( new_extkanjirom != extkanjirom)
	    alert=1;

	new_Use_CompatibleROM = SendDlgItemMessage(hdwnd, ID_USE_COMPATIBLEROM, BM_GETCHECK, 0, 0);
	if (new_Use_CompatibleROM != Use_CompatibleROM)
		alert = 1;

											// --- romaji mode ---
	romaji_mode    = SendDlgItemMessage(hdwnd, ID_ROMAJI_MODE, BM_GETCHECK, 0, 0);

                                            // -- extend RAM 32KB --
	if( P6Version == PC60)
		{
		new_use_extram32 = SendDlgItemMessage( hdwnd , ID_EXTRAM32, BM_GETCHECK , 0,0);
		if( new_use_extram32 != use_extram32)
			alert=1;
		}

											// -- extend RAM 64KB --
	if( P6Version != PC60)
		{
		new_use_extram64 = SendDlgItemMessage( hdwnd , ID_EXTRAM64, BM_GETCHECK , 0,0);
		if( new_use_extram64 != use_extram64)
			alert=1;
		}

											// -- key click sound
	//keyclick =     SendDlgItemMessage( hdwnd , ID_KEYCLICK_SOUND, BM_GETCHECK , 0,0);

											// -- stop 2ms timer
	TimerSWFlag = !SendDlgItemMessage( hdwnd , ID_STOP_2MSTIMER, BM_GETCHECK , 0,0);


	if( IntLac_bak != IntLac) ClearScr();	// スキャンライン切り替え  画面消す　2003/10/24

#if 0
	drawwait= GetDlgItemInt( hdwnd, ID_DRAWWAIT, NULL,0); // -- draw wait
	srline  = GetDlgItemInt( hdwnd, ID_SRLINE  , NULL,0); // -- sr   wait
	SetValidLine_sr( drawwait);
#endif

	CPUclock= GetDlgItemInt( hdwnd, ID_CPUCLOCK, NULL,0); // -- cpu clock
	if( CPUclock_bak != CPUclock)	// 違っていたら、適用
		{
		if( CPUclock<=0) CPUclock=4;
		SetClock(CPUclock*1000000);
		}

										// ---- Ext1Name ---  2003/10/16
	if( !isSpace( Ext1Name , sizeof( Ext1Name)) )       // space only?
		GetDlgItemText( hdwnd, ID_EXTROMNAME ,Ext1Name ,FILENAME_MAX);

	if( strncmp( Ext1Name_bak , Ext1Name , PATH_MAX)!=0)	// check changes
		alert=1;
	
	// ------------- rom path change check ---------------------   2007/5/12
	GetDlgItemText( hdwnd, ID_ROMPATH ,RomPath ,PATH_MAX);	// get rom path from editbox
	if( strncmp( RomPath  , RomPath_bak   , PATH_MAX)!=0)	// changed?
		{
		alert=1;
		}


/*	if(alert==1)
		OSD_MessageBox(nextboottime,"Information", OSDM_OK);
*/

	ConfigWrite();		// write to config file

    OSD_ClearWindow();		// clear window	2004/1/9
    return( alert);
}

//  ダイアログ表示時に特定のフォルダを選択状態にするには，コールバック関数を
//  定義して，BROWSEINFO 構造体の初期設定を以下のようにする．

int CALLBACK callback_brosedir(HWND hWnd, UINT uMsg, LPARAM lParam, LPARAM lpData) {
    if(uMsg == BFFM_INITIALIZED) {
        SendMessage(hWnd, BFFM_SETSELECTION, (WPARAM)TRUE, lpData);
    }
    return 0;
}

/*
    コールバック（ダイアログのプロシージャとだいたい同じ形)
*/
int CALLBACK BrowseCallbackProc(HWND hWnd, UINT uMsg, LPARAM lParam, LPARAM lpData)
{
    switch(uMsg)
    {
    case BFFM_INITIALIZED:
    // 初期化処理をココに書く
        SendMessage(hWnd, BFFM_SETSELECTION, (WPARAM)TRUE, lpData);
        //↑初期フォルダを選択する
        break;
    case BFFM_SELCHANGED:
    // 選択が選択されたら送られてくる
        break;
    case BFFM_VALIDATEFAILED:
    // エディットボックスに無効なアイテムが入力されたら送られる
        break;
    }
    return 0;
}

// ****************************************************************************
//          SelectFolder: フォルダの選択
// ****************************************************************************
int SelectFolder(HWND hdwnd, char *path)
{
    BROWSEINFO  binfo;
    LPITEMIDLIST idlist;

    binfo.hwndOwner=	hdwnd;
    binfo.pidlRoot=		NULL;
    binfo.pszDisplayName=path;
    binfo.lpszTitle=	"フォルダの指定";
    binfo.ulFlags=		BIF_RETURNONLYFSDIRS; 
    binfo.lpfn=			&BrowseCallbackProc;     //コールバック関数を指定する
    binfo.lParam=		(LPARAM)path;          
    //↑コールバック関数に初期フォルダをパラメータとして渡す
    binfo.iImage =		(int)NULL;

    idlist=SHBrowseForFolder(&binfo);
    if (idlist)
    {
        SHGetPathFromIDList(idlist,path);   //ITEMIDLISTからパスを得る
        CoTaskMemFree(idlist);              //ITEMIDLISTの解放(後処理)
        return TRUE;
    }
    return FALSE;
}


// ****************************************************************************
//          configure_OnClickBrowseRomPath: コンフィグダイアログの browse rom path を押された処理
// ****************************************************************************
// configureFunc から呼ばれる。
int OnClickBrowseRomPath( HWND hdwnd )
{
	int  ret=0;
	char path[PATH_MAX];
	char curdir[PATH_MAX];

	_getcwd( curdir, sizeof(curdir)); // get current directory

	if( RomPath[0]!=0)
		strcpy(path , RomPath);
	else
		strcpy( path , curdir);		// when RomPath is empty , set current directory

	if( SelectFolder(hdwnd, path))
		{
		chdir( curdir );			// back to current directory

		my_strncpy( RomPath , path , PATH_MAX);
		strcat( RomPath, "\\");
		SetDlgItemText( hdwnd, ID_ROMPATH ,RomPath);// rom path 2007/5/12
		ret=1;
		}
	return ret;
}


// ****************************************************************************
//          configure_OnClickOpenExtRom: コンフィグダイアログの Open extrom を押された処理
// ****************************************************************************
// configureFunc から呼ばれる。
int OnClickOpenExtRom( HWND hdwnd )
{
 int ret=0;
 static char  curdir[ FILENAME_MAX];
 static TCHAR fullpath[FILENAME_MAX];
 static TCHAR name[ FILENAME_MAX];
 static OPENFILENAME op = {0};


  // ---------------- open file structer ------------
    fullpath[0]=0;
    op.lStructSize        = sizeof( OPENFILENAME);
    op.hwndOwner          = hdwnd;
    op.lpstrFilter        = TEXT("All files {*.*}\0*.*\0\0");
    op.lpstrCustomFilter  = NULL;
    op.lpstrFile          = fullpath;
    op.lpstrFileTitle     = name;
    op.nMaxFile           = FILENAME_MAX;
    op.nMaxFileTitle      = FILENAME_MAX;
    op.Flags              = OFN_FILEMUSTEXIST | OFN_NONETWORKBUTTON | OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY;	// Overwrite check 2003/4/29
    op.lpstrInitialDir= NULL;
	
	op.lpstrFilter  = TEXT("rom files {*.rom;*.bin}\0*.rom;*.bin\0")
					  TEXT("All files {*.*}  \0*.*\0\0");
	op.lpstrInitialDir= ExtRomPath;
	if (Ext1Name[0])
		{
		strcpy(fullpath, ExtRomPath);
		strcat(fullpath, Ext1Name);
		}


	_getcwd( curdir, sizeof(curdir));	// backup curdir
	if(GetOpenFileName( &op))
		{
		chdir( curdir);					// resotre curdir
		my_strncpy( Ext1Name , name , FILENAME_MAX);
		fullpath[ op.nFileOffset] = 0;
		my_strncpy( ExtRomPath , fullpath , FILENAME_MAX);
		ConfigWrite();
		ret=1;

#if 0
		if( strncmp( Ext1Name , fullpath , PATH_MAX) !=0) // new file ? 
			{
			my_strncpy( Ext1Name , fullpath , PATH_MAX);
			SetDlgItemText( hdwnd, ID_EXTROMNAME ,Ext1Name);// ---- Ext1Name ---  2003/10/16
			ret=1;
			}
#endif
		}
	return(ret);
}

// ****************************************************************************
//          configureFunc: コンフィグダイアログの CALLBACK 関数
// ****************************************************************************
BOOL CALLBACK configureFunc(HWND hdwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
//HDC hdc;
//int i;
int alert=0;


    switch( message)
    {   			

	// ********************* INITIALIZE DIALOG ***********************
    case WM_INITDIALOG:
		configure_OnInitDialog(hdwnd);
		break;

	case WM_HSCROLL:		// track slider
		{
		int tmp;
		if( hPSGVolWnd == (HWND) lParam)
			{
			psg_vol = SendMessage( hPSGVolWnd, TBM_GETPOS , 0,0);
			
			//PRINTDEBUG(1,"PSG VOl.%d \n",tmp);
			}
		else if( hFMVolWnd == (HWND) lParam)
			{
			fm_vol = SendMessage( hFMVolWnd, TBM_GETPOS , 0,0);
			//PRINTDEBUG(1,"FM VOl.%d \n",tmp);
			}
		break;
		}

	case WM_COMMAND:
    	switch( LOWORD(wParam))
    	{
		// ******************** OK BUTTON DOWN *************************
	 	case IDOK:
			alert = configure_OnClickOK(hdwnd);
			EndDialog(hdwnd, alert); // close dialog     (return alert variable)
			return(1);

		// ******************** CANCEL BUTTON DOWN *************************
		 case IDCANCEL: 
	 		my_strncpy( RomPath , RomPath_bak , PATH_MAX);

			EndDialog(hdwnd,0); // close dialog
			return(1);

		 case ID_USE_COMPATIBLEROM:				// 互換ROM を使うをon/off するたびに機種選択をon/off
			 if (HIWORD(wParam) == BN_CLICKED)
			 {
				 new_Use_CompatibleROM = !new_Use_CompatibleROM;
				 SendDlgItemMessage(hdwnd, ID_USE_COMPATIBLEROM, BM_SETCHECK, new_Use_CompatibleROM, 0);

				 compatiblerom_onoff( hdwnd);
			 }
			 break;

#if 0
		// ******************** DEFAULT BUTTON DOWN *************************
		 case ID_DEFCLOCK:		// Default clocks
			SetDlgItemInt( hdwnd, ID_DRAWWAIT, 192,0); // -- draw wait
			SetDlgItemInt( hdwnd, ID_SRLINE  ,  80,0); // -- sr   wait
			SetDlgItemInt( hdwnd, ID_CPUCLOCK,   4,0); // -- cpu clock
		 	break;

		// ******************** OPEN BUTTON DOWN *************************
		 case ID_OPENEXTROM:	// Open extrom
			OnClickOpenExtRom( hdwnd );
			break;

		// ******************** CLEAR BUTTON DOWN *************************
		 case ID_CLEAREXTROM:	// Clear  extrom name editbox
			Ext1Name[0]=0;						// ---- Ext1Name ---  	2003/10/16
			Ext2Name[0]=0;
			SetDlgItemText( hdwnd, ID_EXTROMNAME ,Ext1Name);
	        break;
#endif

		 case ID_BROWSEROMPATH:
			 OnClickBrowseRomPath( hdwnd);
			 break;
		}
	    break;
   }
 return(0);
}

// ****************************************************************************
//          select_machineFunc: SELECT_MACHINEダイアログの CALLBACK 関数
// ****************************************************************************
BOOL CALLBACK select_machineFunc(HWND hdwnd, UINT message, WPARAM wParam, LPARAM lParam)
	{
	int ret=0;
	HDC hdc;

	if (message == WM_COMMAND)
		{
		switch (LOWORD(wParam))
			{
			case ID_PC60:
				ret = PC60;
				EndDialog(hdwnd, ret); // close dialog
				return(1);
			case ID_PC66:
				ret = PC66;
				EndDialog(hdwnd, ret); // close dialog
				return(1);
			}
		}
	return(0);
	}


// ****************************************************************************
//          OSD_SelectMachine  互換ROM用　機種選択
// ****************************************************************************
int OSD_SelectMachine(void)
{
	int ret=P6Version;

	ret = DialogBox(_hThisInst /*hInst*/, "SELECTMACHINE", hwndMain, (DLGPROC)select_machineFunc);
	
	return ret;
}

// ****************************************************************************
//          aboutFunc: ABOUT ダイアログの CALLBACK 関数
// ****************************************************************************
BOOL CALLBACK aboutFunc(HWND hdwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
//	char str[200];
	HDC hdc;

	hdc= GetDC(hdwnd);
	DrawIcon(hdc,10,25, hIcon); // draw ICON
	ReleaseDC( hdwnd, hdc);

	if( message==WM_COMMAND)
		{
		switch( LOWORD(wParam))
			{
			 case IDOK: 
			 		EndDialog(hdwnd,0); // close dialog
				    return(1);

	/*		 case IDM_URL: 
 					ShellExecute(_win32_getWindowHandle(),"open",HOMEPAGE_URL,NULL,NULL,SW_SHOWNORMAL);
		 			break; */
			}
		}
	else if (message == WM_INITDIALOG)
	{	
		HWND dlgHandle;

		dlgHandle = GetDlgItem(hdwnd, ID_VER);
		SetWindowText(dlgHandle, BUILD_VER);

		dlgHandle = GetDlgItem(hdwnd, ID_DATE);
		SetWindowText(dlgHandle, BUILD_DATE);

		dlgHandle = GetDlgItem(hdwnd, IDM_URL);
		SetWindowText(dlgHandle, HOMEPAGE_URL);

		{
			char comment[256];
			sprintf(comment, "%s   %s", getextkanjirom_type_msg(), OSD_getlocale_msg());
			dlgHandle = GetDlgItem(hdwnd, IDM_COMMENT);
			SetWindowText(dlgHandle, comment);
		}
	}
 
 return(0);
}


// ****************************************************************************
//          usageFunc: USAGE ダイアログの CALLBACK 関数
// ****************************************************************************
BOOL CALLBACK usageFunc(HWND hdwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
//	char str[200];
	HDC hdc;

	if( message==WM_COMMAND)
		{
		switch( LOWORD(wParam))
			{
			 case IDOK: 
			 		EndDialog(hdwnd,0); // close dialog
				    return(1);

	/*		 case IDM_URL: 
 					ShellExecute(_win32_getWindowHandle(),"open",HOMEPAGE_URL,NULL,NULL,SW_SHOWNORMAL);
		 			break; */
			}
		}
 
 return(0);
}



pushDebugStep()
{
	static int cnt=0;

	PostMessage(hDebugDlg , WM_COMMAND, ID_DEBUG_STEP,0);

	OSD_Delay(1);

}

// ****************************************************************************
//          debugFunc: debug ダイアログの CALLBACK 関数
// ****************************************************************************
BOOL CALLBACK debugFunc(HWND hdwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
//int alert;
#ifdef DEBUG

	switch( message)
    {   			

	// ********************* INITIALIZE DIALOG ***********************
    case WM_INITDIALOG:
		StopSound();			// stop sound 
		SetDlgItemText(hdwnd , ID_DEBUG_EDITBOX,"iP6 Plus Debugger \r\n");
		SetDlgItemText(hdwnd , ID_DEBUG_MEMORYADDRBOX,"0000");

		showRegisters( hdwnd);
		break;
		
    case WM_COMMAND:

		showRegisters( hdwnd);
    	switch( LOWORD(wParam))
    	{
		case ID_DEBUG_GO:
			//OnPushExecuteDebugCommand( hdwnd,"");
			break;
		case ID_DEBUG_STEP:
			OnPushExecuteDebugCommand( hdwnd,"S");	// step  execute
			SendMessage( hwndMain , WM_PAINT , 0,0);	// update main screen 
			break;
		case ID_DEBUG_ANIMATE:
			animatemode = !animatemode;
			break;
		case ID_DEBUG_CONTINUE:
			break;
		}
		break;

	case WM_PAINT:
		break;

	case WM_CLOSE:
		DestroyWindow( hdwnd);
		if( Trace ==1) {Trace=0; inTrace= DEBUG_END;}		// when close
		hDebugDlg =0;
		animatemode= 0;
		ResumeSound();			// resume sound
		return(1);
	}
#endif
 return(0);
}

/*
void open_debug_dialog(void)
{
	if( !hDebugDlg )
		hDebugDlg = CreateDialog(_hThisInst  ,"Debug",hwndMain,(DLGPROC) debugFunc );
}

void close_debug_dialog(void)
{
	SendMessage( hDebugDlg , WM_CLOSE,0,0);		// close a Debugger Dialog
	hDebugDlg =0;
}
*/

// show register , memory , cpu 
void showRegisters(HWND hdwnd)
{
	static int  firsttime =-1;
	static char pre_disasm[65536];

//	int tmp;
	char tmpstr1[256];
	char tmpstr2[256];

#ifdef DEBUG
	// **************** register *******************
	DebugStart( &R);
	SetDlgItemText(hdwnd , ID_DEBUG_REGISTERBOX, DebugResult);

	// **************** memory dump *******************
	GetDlgItemText( hdwnd, ID_DEBUG_MEMORYADDRBOX, tmpstr1, 4);	// get memory address
	sprintf(tmpstr2 ,"M%s",tmpstr1);
	DebugCommand( &R ,tmpstr2);
	SetDlgItemText(hdwnd , ID_DEBUG_MEMORYBOX  ,DebugResult);

	// **************** disasm  *******************
	sprintf( tmpstr1,"%04X:", R.PC);
	if( strstr( pre_disasm , tmpstr1 ) ==NULL || firsttime==-1)  // not found PC address ,update disasm
		DebugCommand( &R ,"D");
	else
		strcpy( DebugResult , pre_disasm);						// found PC address , pre result

	SetDlgItemText(hdwnd , ID_DEBUG_CPUBOX    ,DebugResult);
	strcpy( pre_disasm , DebugResult);

	firsttime=0;

#endif
}



// *********** debug command *******************
void OnPushExecuteDebugCommand(HWND hdwnd , char *command)
{
#ifdef DEBUG
	if (inTrace == DEBUG_DOING) 
		{
		if (!DebugCommand(&R, command)) 
			{
			if(DebugResult[0]) 
				{
				//printf("DebugResult=%s \n",DebugResult);
				SetDlgItemText(hdwnd , ID_DEBUG_EDITBOX, DebugResult );

				printf("%s",DebugResult);
				//	NSDictionary* attr = [NSDictionary dictionaryWithObject:
				//		[NSFont fontWithName: @"Courier" size: 10.0f] forKey: NSFontAttributeName];	
				//	NSAttributedString* dbgRes =[[[NSAttributedString alloc]
				//		initWithString: [NSString stringWithCString: DebugResult] attributes: attr] autorelease];
				//	[debugPrint insertText: dbgRes];		

				DebugResult[0]='\0';
				}
			} 
		else 
			{
			//[debugCommandField setSelectable: NO];
			inTrace = DEBUG_END;
			//[debugWindow saveFrameUsingName: @"DebugWindow"];
			//[debugWindow orderOut: self];
			}
		//[debugCommandField setStringValue: [NSString stringWithCString: ""]];
		//[debugCommandField becomeFirstResponder];
		}
#endif
}






// ****************************************************************************
//          OnMenuWriteRam: メニューの RAMメモリー出力
// ****************************************************************************
void OnMenuWriteRam(HWND hwnd)
{
	char  curdir[ FILENAME_MAX];
	TCHAR fullpath[FILENAME_MAX];
	TCHAR name[ FILENAME_MAX];
	OPENFILENAME op = {0};
	  // ---------------- open file structer ------------
	fullpath[0]=0;
	op.lStructSize        = sizeof( OPENFILENAME);
	op.hwndOwner          = hwnd;
	op.lpstrFilter        = TEXT("All files {*.*}\0*.*\0\0");
	op.lpstrCustomFilter  = NULL;
	op.lpstrFile          = fullpath;
	op.lpstrFileTitle     = name;
	op.nMaxFile           = FILENAME_MAX;
	op.nMaxFileTitle      = FILENAME_MAX;
	op.Flags              = OFN_NONETWORKBUTTON | OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY;	// Overwrite check 2003/4/29
	op.lpstrInitialDir= NULL;

	op.lpstrFilter  = TEXT("RAM image files {*.mem}\0*.mem\0")
					  TEXT("All files {*.*}  \0*.*\0\0");
	op.lpstrDefExt  = "*.mem";
	op.lpstrInitialDir= MemPath;			// initial directory
	_getcwd( curdir, sizeof(curdir));		// backup curdir

	if(GetSaveFileName( &op))
		{
		ramdump( fullpath);
		chdir( curdir);						// restore curdir
		}
	fullpath[ op.nFileOffset]=0;			// fullpath -> directory
	my_strncpy( MemPath , fullpath, FILENAME_MAX);	// directory only
	ConfigWrite();
}


// ****************************************************************************
//          OnMenuScreenshot: メニューの スクリーンショット出力
// ****************************************************************************
int OnMenuScreenshot(HWND hwnd)
{
	int   ret=0;
	char  curdir[ FILENAME_MAX];
	TCHAR fullpath[FILENAME_MAX];
	TCHAR name[ FILENAME_MAX];
	OPENFILENAME op = {0};
	  // ---------------- open file structer ------------
	fullpath[0]=0;
	op.lStructSize        = sizeof( OPENFILENAME);
	op.hwndOwner          = hwnd;
	op.lpstrFilter        = TEXT("All files {*.*}\0*.*\0\0");
	op.lpstrCustomFilter  = NULL;
	op.lpstrFile          = fullpath;
	op.lpstrFileTitle     = name;
	op.nMaxFile           = FILENAME_MAX;
	op.nMaxFileTitle      = FILENAME_MAX;
	op.Flags              = OFN_NONETWORKBUTTON | OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY;	// Overwrite check 2003/4/29
	op.lpstrInitialDir= NULL;

	op.lpstrFilter  = TEXT("PNG image files {*.png}\0*.png\0")
					  TEXT("All files {*.*}  \0*.*\0\0");
	op.lpstrInitialDir= ImgPath;			// initial directory
	op.lpstrDefExt  = "*.png";
	_getcwd( curdir, sizeof(curdir));	// backup curdir

	if(GetSaveFileName( &op))
		{
		 chdir( curdir);	// restore curdir
		 
		 savescreenshot( fullpath);
		 ret=1;
		}
	fullpath[ op.nFileOffset]=0;			// fullpath -> directory
	my_strncpy( ImgPath , fullpath, FILENAME_MAX);	// directory only
	ConfigWrite();
	return(ret);
}


// ****************************************************************************
//          OnMenuOpenLoadTape: メニューの OPEN LOAD TAPE
// ****************************************************************************
int OnMenuOpenLoadTape(HWND hwnd)
{
	int ret=0;
	char  curdir[ FILENAME_MAX];
	TCHAR fullpath[FILENAME_MAX];
	TCHAR name[ FILENAME_MAX];
	OPENFILENAME op = {0};

	memset(&op ,0,sizeof(op));
	  // ---------------- open file structer ------------
	fullpath[0]=0;
	name[0]=0;
	if( CasStream[0])		// set already opened file name
		{
		strcpy( fullpath , CasPath[0]);
		strcat( fullpath , CasName[0]);
		}
	op.lStructSize        = sizeof( OPENFILENAME);
	op.hwndOwner          = hwnd;
	op.lpstrFilter        = TEXT("All files {*.*}\0*.*\0\0");
	op.lpstrCustomFilter  = NULL;
	op.lpstrFile          = fullpath;
	op.lpstrFileTitle     = name;
	op.nMaxFile           = FILENAME_MAX;
	op.nMaxFileTitle      = FILENAME_MAX;
	op.Flags              = OFN_NONETWORKBUTTON | OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY;	// Overwrite check 2003/4/29
	op.lpstrInitialDir= NULL;


	op.Flags |= OFN_FILEMUSTEXIST;
	op.lpstrFilter  = TEXT("cas files {*.cas *.p6 *p6t *.hex}\0*.cas;*.p6;*.p6t;*.hex\0")
					  TEXT("All files {*.*}  \0*.*\0\0");
	op.lpstrInitialDir= CasPath[0];
	_getcwd( curdir, sizeof(curdir));	// backup curdir
	if(GetOpenFileName( &op))
		{
		chdir( curdir);					// resotre curdir
		my_strncpy( CasName[0] , name    , FILENAME_MAX);
		fullpath[ op.nFileOffset]=0;
		my_strncpy( CasPath[0] , fullpath, FILENAME_MAX);
		ConfigWrite();
		if (IsSameLoadSaveTape())		// Load Tape と Save Tapeが同じだったら、Save Tape をイジェクトする
			{
			*CasName[1] = 0;
			ConfigWrite();
			OpenFile1(FILE_SAVE_TAPE);
			}

		OpenFile1( FILE_LOAD_TAPE);

		ret=1;
		}
	return(ret);
}





// ****************************************************************************
//          OnMenuOpenSaveTape: メニューの OPEN SAVE TAPE
// ****************************************************************************
int OnMenuOpenSaveTape(HWND hwnd)
{
	int   ret=0;
	char  curdir[ FILENAME_MAX];
	TCHAR fullpath[FILENAME_MAX];
	TCHAR name[ FILENAME_MAX];
	OPENFILENAME op = {0};
	  // ---------------- open file structer ------------
	fullpath[0]=0;
	op.lStructSize        = sizeof( OPENFILENAME);
	op.hwndOwner          = hwnd;
	op.lpstrFilter        = TEXT("All files {*.*}\0*.*\0\0");
	op.lpstrCustomFilter  = NULL;
	op.lpstrFile          = fullpath;
	op.lpstrFileTitle     = name;
	op.nMaxFile           = FILENAME_MAX;
	op.nMaxFileTitle      = FILENAME_MAX;
	op.Flags              = OFN_NONETWORKBUTTON | OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY;	// Overwrite check 2003/4/29
	op.lpstrInitialDir= NULL;

	if( CasStream[1])		// set already opened file name
		{
		strcpy( fullpath,CasPath[1]);
		strcat( fullpath,CasName[1]);
		}
	op.Flags |=    OFN_CREATEPROMPT |OFN_OVERWRITEPROMPT; /* 作るか尋ねる。上書きするか尋ねる 2003/10/27 */
	op.lpstrFilter  = TEXT("cas files {*.cas *.p6 *p6t}\0*.cas;*.p6;*.p6t\0")
					  TEXT("All files {*.*}  \0*.*\0\0");
	op.lpstrInitialDir= CasPath[1];
	_getcwd( curdir, sizeof(curdir));	// backup curdir
	if(GetSaveFileName( &op))
		{
		chdir( curdir);					// resotre curdir
		my_strncpy( CasName[1] , name    , FILENAME_MAX);
		fullpath[ op.nFileOffset]=0;
		my_strncpy( CasPath[1] , fullpath, FILENAME_MAX);
		ConfigWrite();
		OpenFile1( FILE_SAVE_TAPE);
		ret=1;
		}
	return(ret);
}



// ****************************************************************************
//          OnMenuOpenDokodemoLoadTape: メニューの OPEN LOAD TAPE
// ****************************************************************************
int OnMenuOpenDokodemoLoad(HWND hwnd)
{
	int ret=0;
	char  curdir[ FILENAME_MAX];
	TCHAR fullpath[FILENAME_MAX];
	TCHAR name[ FILENAME_MAX];
	OPENFILENAME op = {0};
	  // ---------------- open file structer ------------
	fullpath[0]=0;
	op.lStructSize        = sizeof( OPENFILENAME);
	op.hwndOwner          = hwnd;
	op.lpstrFilter        = TEXT("All files {*.*}\0*.*\0\0");
	op.lpstrCustomFilter  = NULL;
	op.lpstrFile          = fullpath;
	op.lpstrFileTitle     = name;
	op.nMaxFile           = FILENAME_MAX;
	op.nMaxFileTitle      = FILENAME_MAX;
	op.Flags              = OFN_NONETWORKBUTTON | OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY;	// Overwrite check 2003/4/29
	op.lpstrInitialDir= NULL;

	op.Flags |= OFN_FILEMUSTEXIST;
	op.lpstrFilter  = TEXT("Dokodemo save file {*.ds }\0*.ds\0")
					  TEXT("All files {*.*}  \0*.*\0\0");

	_getcwd( curdir, sizeof(curdir));	// backup curdir
	if(GetOpenFileName( &op))
		{
		chdir( curdir);					// resotre curdir
		dokodemo_load( fullpath);

		ret=1;
		}
	return(ret);
}





// ****************************************************************************
//          OnMenuOpenDokodemoSave: メニューの OPEN SAVE TAPE
// ****************************************************************************
int OnMenuOpenDokodemoSave(HWND hwnd)
{
	int   ret=0;
	char  curdir[ FILENAME_MAX];
	TCHAR fullpath[FILENAME_MAX];
	TCHAR name[ FILENAME_MAX];
	OPENFILENAME op = {0};
	  // ---------------- open file structer ------------
	fullpath[0]=0;
	op.lStructSize        = sizeof( OPENFILENAME);
	op.hwndOwner          = hwnd;
	op.lpstrFilter        = TEXT("All files {*.*}\0*.*\0\0");
	op.lpstrCustomFilter  = NULL;
	op.lpstrFile          = fullpath;
	op.lpstrFileTitle     = name;
	op.nMaxFile           = FILENAME_MAX;
	op.nMaxFileTitle      = FILENAME_MAX;
	op.Flags              = OFN_NONETWORKBUTTON | OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY;	// Overwrite check 2003/4/29
	op.lpstrInitialDir= NULL;

	op.Flags |=    OFN_CREATEPROMPT |OFN_OVERWRITEPROMPT; /* 作るか尋ねる。上書きするか尋ねる 2003/10/27 */
	op.lpstrFilter  = TEXT("dds files {*.ds }\0*.ds\0")
					  TEXT("All files {*.*}  \0*.*\0\0");
	//op.lpstrInitialDir= CasPath[1];
	_getcwd( curdir, sizeof(curdir));	// backup curdir
	if(GetSaveFileName( &op))
		{
		chdir( curdir);					// resotre curdir
		dokodemo_save( fullpath);

		ret=1;
		}
	return(ret);
}



// ****************************************************************************
//          OnMenuOpenDisk: メニューの OPEN DISK
//    drive: 0 or 1
// ****************************************************************************
int OnMenuOpenDisk(HWND hwnd, int drive)
{
	int   ret=0;
	char  curdir[ FILENAME_MAX];
	TCHAR fullpath[FILENAME_MAX];
	TCHAR name[ FILENAME_MAX];
	OPENFILENAME op = {0};
	  // ---------------- open file structer ------------
	fullpath[0]=0;
	op.lStructSize        = sizeof( OPENFILENAME);
	op.hwndOwner          = hwnd;
	op.lpstrFilter        = TEXT("All files {*.*}\0*.*\0\0");
	op.lpstrCustomFilter  = NULL;
	op.lpstrFile          = fullpath;
	op.lpstrFileTitle     = name;
	op.nMaxFile           = FILENAME_MAX;
	op.nMaxFileTitle      = FILENAME_MAX;
	op.Flags              = OFN_NONETWORKBUTTON | OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY;	// Overwrite check 2003/4/29
	op.lpstrInitialDir= NULL;

	if (DskStream[drive])			// set already opened file name
		{
		strcpy(fullpath, DskPath[drive]);
		strcat(fullpath, DskName[drive]);
		}


	if( disk_num)
		{
		op.lpstrFilter  = TEXT("d88 files {*.d88}\0*.d88\0")
					     TEXT("All files {*.*}\0*.*\0\0");
	   	op.lpstrInitialDir= DskPath[0];			// initial directory

#if 0    // AUTO_FORMAT
		op.Flags |=    OFN_CREATEPROMPT |OFN_OVERWRITEPROMPT; /* 作るか尋ねる。  上書きするか尋ねる 2003/10/27 */
#endif   // AUTO_FORMAT
		op.Flags |=    OFN_FILEMUSTEXIST; /* 存在するファイルのみ */

		_getcwd( curdir, sizeof(curdir));		// backup curdir
		if(GetOpenFileName( &op))
			{
		 	chdir( curdir);						// restore curdir
			my_strncpy( DskName[drive] , name    , FILENAME_MAX);	// filename only
			my_strncpy( DskPath[drive] , fullpath, op.nFileOffset); // directory only
			if(drive ==0)
				{
				OpenFile1( FILE_DISK1);
				}
			else if( drive==1)
				{
				OpenFile1( FILE_DISK2);
				}
            ConfigWrite();
			ret=1;
			}
		}
	 return(ret);
	}

// ****************************************************************************
//          OnMenuOpenCapture: メニューの Capture
// ****************************************************************************
#if 0
int OnMenuOpenCapture(HWND hwnd)
{
	int   ret=0;
	char  curdir[ FILENAME_MAX];
	TCHAR fullpath[FILENAME_MAX];
	TCHAR name[ FILENAME_MAX];
	OPENFILENAME op = {0};
	  // ---------------- open file structer ------------
	fullpath[0]=0;
	op.lStructSize        = sizeof( OPENFILENAME);
	op.hwndOwner          = hwnd;
	op.lpstrFilter        = TEXT("All files {*.*}\0*.*\0\0");
	op.lpstrCustomFilter  = NULL;
	op.lpstrFile          = fullpath;
	op.lpstrFileTitle     = name;
	op.nMaxFile           = FILENAME_MAX;
	op.nMaxFileTitle      = FILENAME_MAX;
	op.Flags              = OFN_NONETWORKBUTTON | OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY;	// Overwrite check 2003/4/29
	op.lpstrInitialDir= NULL;

	op.lpstrFilter  = TEXT("AVI files {*.avi}\0*.avi\0")
				      TEXT("All files {*.*}\0*.*\0\0");
	op.lpstrInitialDir= DskPath[0];			// initial directory

	//op.Flags |=    OFN_FILEMUSTEXIST; /* 存在するファイルのみ */
#if 1    // AUTO_FORMAT
	op.Flags |=    OFN_CREATEPROMPT |OFN_OVERWRITEPROMPT; /* 作るか尋ねる。  上書きするか尋ねる 2003/10/27 */
#endif   // AUTO_FORMAT

	_getcwd( curdir, sizeof(curdir));		// backup curdir
	if(GetSaveFileName( &op))
		{
	 	chdir( curdir);						// restore curdir
		StartAVI( fullpath, Width , Height, 30, 22050, 24, FALSE );

		ret=1;
		}
	 return(ret);
	}
#endif


// ****************************************************************************
//          wm_command: メニュー選択の処理分け （ WM_COMMANDメッセージ用）
// ****************************************************************************
void wm_command( HWND hwnd, UINT message, WPARAM wParam , LPARAM lParam)
{

	switch( LOWORD( wParam))
	{
	case IDM_CONFIG:
			{
			if( DialogBox(_hThisInst /*hInst*/ , "Configure" ,hwnd,(DLGPROC) configureFunc)==1 )
				{
				/* ↑1 が返却されたら、再起動が必要です。*/
				if(OSD_MessageBox(MSG_REBOOT_ENABLE_CONFIG  ,"",OSDM_YESNO)==OSDR_YES) 
					{
					ConfigWrite();
				    if( !ResetPC(1))
						{
				        OSD_MessageBox( MSG_FAILED_RESET ,"",OSDM_OK);
						}
			        setMenuOnOff( IDM_OPEN_DISK1, disk_num);
			        setMenuOnOff( IDM_OPEN_DISK2, disk_num ==2);
					}
				}
			break;
			}
	case IDM_RESET: 
			if(OSD_MessageBox(MSG_RESET_CPU ,"",OSDM_YESNO)==OSDR_YES) 
				{
				 if( !ResetPC(0))
					{
					put_notfoundromfile();		
					}
				// if (!CPURunning) CPURunning = 1;
				// setMenuCheck(IDM_PAUSE, 0);
		        setMenuOnOff( IDM_OPEN_DISK1 , disk_num);
		        setMenuOnOff( IDM_OPEN_DISK2 , disk_num==2);
				}
			break;
	case IDM_WRITE_RAM:
			OnMenuWriteRam(hwnd);
			break;

	case IDM_NOWAIT:
			WaitFlag = !WaitFlag;
			setMenuCheck( IDM_NOWAIT, !WaitFlag);
			sw_nowait_mode( !WaitFlag);		// switch nowait mode / normal mode
			break;
/*	case IDM_KEYCLICK:
			keyclick= !keyclick;
			setMenuCheck( IDM_KEYCLICK, keyclick);
			break; */
	case IDM_SCREENSHOT:
            OnMenuScreenshot( hwnd);
			break;
	case IDM_EXIT: 
			PostMessage( hwnd , WM_CLOSE, 0,0);
			break;

	case IDM_PAUSE:
			CPURunning = !CPURunning;
			setMenuCheck(IDM_PAUSE, !CPURunning);
			break;

					/* *********************** TAPE (LOAD) ************************* */
	case IDM_OPEN_LOAD_TAPE: 
			{
			char fullpath[MAX_PATH];							// イジェクトする前にカウンターを覚えておく
			sprintf(fullpath, "%s%s", CasPath[0], CasName[0]);
			fpos_t pos;
			if( CasStream[0]!=NULL)
				{
				fgetpos(CasStream[0], &pos);
				SetTapeCounter(fullpath, pos);
				}

            if( OnMenuOpenLoadTape( hwnd)==1)				// ファイルオープンダイアログ
				{
				sprintf(fullpath,"%s%s",CasPath[0],CasName[0]);
				if( GetTapeCounter( fullpath , &pos))
					{
					if( OSD_MessageBox("MSG_RESTORE_TAPE_COUNTER","",OSDM_YESNO) == OSDR_YES)
						{
						fsetpos( CasStream[0] , &pos);
						}
					}
				  putStatusBar();                 // ステータスバー更新
				 }
			break;
			}
	case IDM_REWIND_LOAD_TAPE: 
			if( *CasName[0]!=0)
				if(OSD_MessageBox(MSG_REWIND_LOAD_TAPE ,"",OSDM_YESNO)== OSDR_YES) 
					OpenFile1( FILE_LOAD_TAPE);
			break;
	case IDM_EJECT_LOAD_TAPE:
			{
			char fullpath[MAX_PATH];							// イジェクトする前にカウンターを覚えておく
			sprintf(fullpath,"%s%s",CasPath[0],CasName[0]);
			fpos_t pos;
			fgetpos( CasStream[0], &pos);
			SetTapeCounter( fullpath ,pos);

			*CasName[0]=0;
			ConfigWrite();
			OpenFile1( FILE_LOAD_TAPE);
			putStatusBar();                 // ステータスバー更新
			break;
			}
					/* *********************** TAPE (SAVE)************************* */
	case IDM_OPEN_SAVE_TAPE:
            OnMenuOpenSaveTape( hwnd);
			break;
	case IDM_REWIND_SAVE_TAPE: 
			if( *CasName[1]!=0)
				if(OSD_MessageBox(MSG_REWIND_SAVE_TAPE ,"",OSDM_YESNO)==OSDR_YES) 
					OpenFile1( FILE_SAVE_TAPE);
			break;
	case IDM_EJECT_SAVE_TAPE: 
			*CasName[1]=0;
			ConfigWrite();
			OpenFile1( FILE_SAVE_TAPE);
			putStatusBar();                 // ステータスバー更新
			break;
   /* case IDM_USE_SAVE_TAPE:
            UseSaveTapeMenu = !UseSaveTapeMenu;
      		setMenuCheck( IDM_USE_SAVE_TAPE    , UseSaveTapeMenu);
       	    setMenuOnOff( IDM_OPEN_SAVE_TAPE   , UseSaveTapeMenu);
        	setMenuOnOff( IDM_REWIND_SAVE_TAPE , UseSaveTapeMenu);
        	setMenuOnOff( IDM_EJECT_SAVE_TAPE  , UseSaveTapeMenu);
            ConfigWrite();
            break;
	*/
	case IDM_OPEN_DISK1: 
			OnMenuOpenDisk( hwnd ,0);
			break;
	case IDM_EJECT_DISK1: 
			*DskName[0]=0;
			ConfigWrite();
			OpenFile1( FILE_DISK1);
			break;

	case IDM_OPEN_DISK2: 
			OnMenuOpenDisk( hwnd ,1);
			break;
	case IDM_EJECT_DISK2: 
			*DskName[1]=0;
			ConfigWrite();
			OpenFile1( FILE_DISK2);
			break;

	 case IDM_OPEN_EXTROM:	// Open extrom
			if( OnClickOpenExtRom( hwnd ) )
				{
								/* ↑1 が返却されたら、再起動が必要です。*/
				if(OSD_MessageBox(MSG_REBOOT_ENABLE_CONFIG ,"",OSDM_YESNO)==OSDR_YES) 
					{
					ConfigWrite();
				    if( !ResetPC(1))
				        OSD_MessageBox( MSG_FAILED_RESET  ,"",OSDM_OK);
					}
				}
			setMenuTitle( FILE_EXTROM);
			break;

	 case IDM_EJECT_EXTROM:	// Clear  extrom name editbox
			if(OSD_MessageBox(MSG_REBOOT_ENABLE_CONFIG ,"",OSDM_YESNO)==OSDR_YES) 
				{
				Ext1Name[0]=0;						// ---- Ext1Name ---  	2003/10/16
				Ext2Name[0]=0;
				ConfigWrite();
			    if( !ResetPC(1))
			        OSD_MessageBox( MSG_FAILED_RESET  ,"",OSDM_OK);
				}
			//SetDlgItemText( hdwnd, ID_EXTROMNAME ,Ext1Name);
			setMenuTitle( FILE_EXTROM);
	        break;

	case IDM_DOKOSAVE:
			OnMenuOpenDokodemoSave( hwnd);
		    break;
	case IDM_DOKOLOAD:
			OnMenuOpenDokodemoLoad( hwnd);
		    break;
	case IDM_MONITOR:
#ifdef DEBUG
			if( !Trace)
				{
		   		storeKeyrepeat();
				setMenuOnOff( IDM_WINDOW12  , 0);
	        	setMenuOnOff( IDM_FULLSCR   , 0);
				open_debug_dialog();
				Trace=1;
				}
			else
				{
		   		setKeyrepeat(DEFAULT_KEY_REPEAT);
	        	setMenuOnOff( IDM_WINDOW12  , 1);
				close_debug_dialog();
				Trace=0;
				}
			setMenuTitle( WINDOW12);
#endif
				break;
/*	case IDM_NO_TIMER:
			TimerSWFlag = !TimerSWFlag;
			setMenuCheck( IDM_NO_TIMER ,!TimerSWFlag);
			break; */
	case IDM_VER:  
			DialogBox(_hThisInst /*hInst*/ ,"about",hwnd,(DLGPROC) aboutFunc);
			break;
	case IDM_URL: 
 			ShellExecute(hwnd,"open",HOMEPAGE_URL,NULL,NULL,SW_SHOWNORMAL);
	 		break;
	case IDM_USAGE: 
			DialogBox(_hThisInst /*hInst*/ ,"usage",hwnd,(DLGPROC) usageFunc);
 			//ShellExecute(hwnd,"open",HELP_FILE,NULL,NULL,SW_SHOWNORMAL);
	 		break;
    case IDM_FULLSCR:

			resizewindow( 2.0 , 2.0, 0);
    		toggleFullScr();
			setMenuCheck( IDM_FULLSCR ,isFullScreen() );
			setMenuOnOff( IDM_WINDOW12  , !( isFullScreen() || Trace));		// fullscreen or debug mode : disable window x1 x2  
			if( !isFullScreen() && Trace)
				{
				resizewindow( scale ,win_scale ,0);			// debug mode: setup window size x3 
				}
			break;
	case IDM_WINDOW12:
			if( scale ==1)
				new_scale = 2;
			else
				new_scale = 1;
			setMenuTitle( WINDOW12);
			resizewindow( new_scale ,new_scale ,0);
			ConfigWrite();
			break;
	}

}



// ****************************************************************************
//          setMenuTitle: メニューアイテムの表示変更
//                        マウントしているファイル名など
// ****************************************************************************
/* set filename to open menu */
/* input: type = FILE_TAPE , FILE_DISK */
int setMenuTitle(int type)
{
	HMENU hmenu;
	UINT  itemId;
	TCHAR buff[FILENAME_MAX+20];
	MENUITEMINFO menuinfo={0};
	int   ret;
	int   i;

     switch(type)
 	    {
	    case FILE_LOAD_TAPE:
     				 itemId = IDM_EJECT_LOAD_TAPE;			// menu item id
	 				 if(*CasName[0])
		 				 sprintf(buff,"&Eject load tape (%s) ",CasName[0]); // new title
					 else
		 				 strcpy(buff,"&Eject load tape ");
	 				 break;
	    case FILE_SAVE_TAPE:
     				 itemId = IDM_EJECT_SAVE_TAPE;			// menu item id
	 				 if(*CasName[1])
		 				 sprintf(buff,"Eject save tape (%s) ",CasName[1]); // new title
					 else
		 				 strcpy(buff,"Eject save tape ");
	 				 break;
	    case FILE_DISK1: itemId = IDM_EJECT_DISK1;
	 				 if(*DskName[0])
		 				 sprintf(buff,"&Eject 1 (%s)",DskName[0]);
					 else
		 				 strcpy(buff,"&Eject 1");
	 				 break;
	    case FILE_DISK2: itemId = IDM_EJECT_DISK2;
	 				 if(*DskName[1])
		 				 sprintf(buff,"&Eject 2 (%s)",DskName[1]);
					 else
		 				 strcpy(buff,"&Eject 2");
	 				 break;
		case FILE_EXTROM: itemId = IDM_EJECT_EXTROM;
					 if(*Ext1Name)
						 sprintf(buff,"&Eject (%s)",Ext1Name);
					 else
						 strcpy(buff,"&Eject");
					 break;
		case WINDOW12:	itemId = IDM_WINDOW12;
					if( new_scale ==1)
						strcpy(buff, "&Window x2");
					else if( new_scale ==2)
						strcpy(buff, "&Window x1");
					break;
		case FILE_DOKOSAVE:	itemId = IDM_DOKOSAVE;
					//if (OSD_getlocale()!= OSDL_JP)
						strcpy(buff, "Save state");
					break;
		case FILE_DOKOLOAD:	itemId = IDM_DOKOLOAD;
					//if (OSD_getlocale() != OSDL_JP)
						strcpy(buff, "Load state");
					break;
		default: return 0;			// FILE_PRNT
	    }

	for(i=0; i<2;i++)
	 	{
		 menuinfo.cbSize = sizeof( MENUITEMINFO);
		 menuinfo.fMask = MIIM_TYPE;
		 menuinfo.dwTypeData = NULL;

		 switch( i) {
		   	case 0:	hmenu = GetMenu(hwndMain); break;
		    case 1: hmenu = getPopupMenu(); break;
		   }

		 ret= GetMenuItemInfo( hmenu , itemId ,0 ,&menuinfo);

		 menuinfo.dwTypeData = buff;
		 menuinfo.cch        = strlen(buff);
		 ret= SetMenuItemInfo( hmenu , itemId ,0 ,&menuinfo);
	     }
	 return(ret);
}


// ****************************************************************************
//          setMenuCheck: メニューにチェックを入れたり、外したり
// ****************************************************************************
/* input:  menu item number
   output: 1: checked  0: unchecked*/
int setMenuCheck(int type ,int sw)
{
	 HMENU hmenu;
	 UINT  itemId;
//	 TCHAR buff[FILENAME_MAX+20];
	 MENUITEMINFO menuinfo={0};
	 int   i;
//	 int   ret;
	 int   status;

	 switch( type)
	 	{
		case IDM_NO_TIMER:      itemId= IDM_NO_TIMER;break;
		case IDM_KEYCLICK:      itemId= IDM_KEYCLICK;break;
		case IDM_NOWAIT:        itemId= IDM_NOWAIT;  break;
        case IDM_USE_SAVE_TAPE: itemId= IDM_USE_SAVE_TAPE;  break;
        case IDM_FULLSCR:    itemId= IDM_FULLSCR; break;
		case IDM_PAUSE:		itemId = IDM_PAUSE; break;

#ifdef VIEW_MENU
        case IDM_SCANLINE:   itemId= IDM_SCANLINE; break;
        case IDM_STATUSBAR:  itemId= IDM_STATUSBAR; break;
        case IDM_DISK_LAMP:  itemId= IDM_DISK_LAMP; break; 
		case IDM_SCREENSIZE: itemId= IDM_SCREENSIZE;break;
#endif
		default: return(0);
		}
	
    
    for(i=0; i< 2; i++)
    	{
	 	menuinfo.cbSize = sizeof( MENUITEMINFO);
	 	menuinfo.fMask = MIIM_STATE;
        switch( i) {
        	case 0:	hmenu = GetMenu(hwndMain); break;
            case 1: hmenu = getPopupMenu(); break;
            }
        // if( hmenu==0) break;
		 /*ret= */ GetMenuItemInfo( hmenu , itemId ,0 ,&menuinfo);

		 if( sw )
	 		{
	 	 	menuinfo.fState =  MFS_CHECKED;
	 	 	status =1;
	 		}
	 	else
	 		{
		 	menuinfo.fState =  MFS_UNCHECKED;
	 	 	status =0;
	 		}
 		/*ret = */ SetMenuItemInfo( hmenu , itemId ,0 ,&menuinfo);
    	}
 	return( status );
}

// ****************************************************************************
//          setMenuOnOff: メニューを有効にしたり、無効にしたり
// ****************************************************************************
/* input:  menu item number
   output: 1: checked  0: unchecked*/
void setMenuOnOff(int type ,int sw)
{
	 HMENU hmenu;
	 UINT  itemId;
//	 TCHAR buff[FILENAME_MAX+20];
	 MENUITEMINFO menuinfo={0};
	 int   i;
//	 int   ret;
	 int   status;

	 itemId = type;
     for(i=0; i< 2; i++)
    	{
	 	menuinfo.cbSize = sizeof( MENUITEMINFO);
	 	menuinfo.fMask = MIIM_STATE;
        switch( i) {
        	case 0:	hmenu = GetMenu(hwndMain); break;
            case 1: hmenu = getPopupMenu(); break;
            }

		if( sw )
	 		{
	 	 	menuinfo.fState =  MFS_ENABLED;
	 	 	status =1;
	 		}
	 	else
	 		{
		 	menuinfo.fState =  MFS_DISABLED;
	 	 	status =0;
	 		}
 		/*ret = */ SetMenuItemInfo( hmenu , itemId ,0 ,&menuinfo);
        }
}




HMENU   prev_hmenu=0;
static HMENU hPopupMenu=0;


// ****************************************************************************
//          savemenu: メニューの情報を保存
// ****************************************************************************
void savemenu(void)
{
	prev_hmenu = GetMenu(hwndMain);
}


// ****************************************************************************
//          hidemenu: メニューを隠す
// ****************************************************************************
int hidemenu(void)
{
	int ret=0;
    if( prev_hmenu) 
	  	ret= SetMenu( hwndMain, NULL);
	return(ret);
}

// ****************************************************************************
//          hidemenu: メニューを元に戻す
// ****************************************************************************
int restoremenu(void)
{
	int ret=0;
    if( prev_hmenu)
		ret = SetMenu( hwndMain , prev_hmenu);
    prev_hmenu =0;
	return( ret);
}


// ****************************************************************************
//          loadPopupMenu: POPUP メニューを読み込む
// ****************************************************************************
void loadPopupMenu( HINSTANCE hInstance, char *menuname)
{
	HMENU htmpMenu;
	htmpMenu = LoadMenu( hInstance, menuname);
	hPopupMenu = GetSubMenu(htmpMenu,0);
}

// ****************************************************************************
//          openPopupMenu: POPUP メニューを開く
// ****************************************************************************
void openPopupMenu( HWND hwnd , int x, int y)
{
	if( hPopupMenu )
    	{
		POINT po;
		po.x = x; po.y = y;
		ClientToScreen( hwnd , &po);
		TrackPopupMenu( hPopupMenu , TPM_LEFTALIGN | TPM_BOTTOMALIGN, po.x , po.y ,0 ,hwnd ,NULL);
        }
}


// ****************************************************************************
//          getHpopupMenu: POPUP メニューのハンドルを返す
// ****************************************************************************
HMENU getPopupMenu( void)
{
	return( hPopupMenu);
}



// ****************************************************************************
//          drag_open: ドラッグされたファイルによってファイルをオープンする
// ****************************************************************************
int drag_open(char *path, int max)
{
	char drive[10];
	char dir[1024];
	char file[1024];
	char ext[20];

	_splitpath(path, drive, dir, file, ext);



	if (!stricmp(ext, ".CAS") || !stricmp(ext, ".P6") || !stricmp(ext, ".P6T"))	// テープ
	{
		strcpy(CasPath, drive);
		strcat(CasPath, dir);
		strcpy(CasName, file);
		strcat(CasName, ext);
		ConfigWrite();				// 設定ファイル書き込み
		OpenFile1(FILE_LOAD_TAPE);
	}
	else if (!stricmp(ext, ".D88"))			// ディスク
	{
		int drv_no = 0;
		strcpy(DskPath[drv_no], drive);
		strcat(DskPath[drv_no], dir);
		strcpy(DskName[drv_no], file);
		strcat(DskName[drv_no], ext);
		ConfigWrite();				// 設定ファイル書き込み
		OpenFile1(FILE_DISK1);
	}
#if 0
	else if (!stricmp(ext, ".TXT")|| !stricmp(ext, ".MD"))			// テキストファイル 流し込み
	{
		int size=512;
		FILE *fp;
		char *inbuff ,*outbuff;
		char *p;

		fp = fopen(path, "r"); 
		if (!fp) {
			MessageBox(hwndMain,"ファイルオープンエラー", "",MB_OK);
			return 0;
		 }

		inbuff  = malloc(size);
		outbuff = malloc(size);
		if (inbuff != NULL && outbuff !=NULL) {
			while (1){
				if (fgets(inbuff, size, fp) == NULL) {
					break;
					}
				if( inbuff[strlen(inbuff)-1] == 0xa) {
					strcat( inbuff, "\r");
				}
				convertSjis2p6key(inbuff, outbuff);
				putAutokeyMessage(outbuff);
				}
			fclose(fp);
			free(inbuff);
			free(outbuff);
			}
		else {
			return 0;
		}
	}
#endif
	return 1;

}



// ****************************************************************************
//          WindowFunc: Windowsのメッセージ処理のための CALLBACK 関数
// ****************************************************************************
LRESULT CALLBACK WindowFunc( HWND hwnd, UINT message, WPARAM wParam , LPARAM lParam)
{
	HDC hdc;
	PAINTSTRUCT paintstruct;
	// static BITMAP bitmap;
	// static HDC    hdcBmp;
	//hwndMain = hwnd;
	int id, i;
	HDROP hDrop;
	UINT uFileNo;
	char szFileName[PATH_MAX];

//    init_delay( 60 );

	 switch(message) {

	   case WM_CLOSE:
		    if( !UseCPUThread ) StopSound();		// Stop Sound
			if(OSD_MessageBox(MSG_EXIT ,"",OSDM_YESNO)==OSDR_YES)
				{
				DestroyWindow( hwndMain);
				CPURunning =0;
				}
			else
				{
				CPURunning =1;
				if( !UseCPUThread ) ResumeSound();		// Resume Sound
				}
			break;

	   case WM_DESTROY: 

			//UnprepareLayeredWindow(); // transparent
	        CPUThreadRun=0;         // CPU Thread aborting ... 
	        Sleep(200);
	
		   //	TrashP6();
			//TrashMachine();			// add 2002/10/15
			if(Verbose) printf("EXITED at PC=%04Xh.\n",ExitedAddr);
		   	if( Console) {
				FreeConsole();		// free console
		   	}
//			OSD_ReleaseSurface( surface1 );
//			OSD_ReleaseSurface( surface2 );

			PostQuitMessage(0);
			break;
	
	   case WM_RBUTTONDOWN:		// RIGHT CLICK
		    if( !UseCPUThread ) StopSound();
			if( isFullScreen())	// if fullscreen mode ,open popupmenu 
		   		openPopupMenu( hwnd , LOWORD(lParam) , HIWORD(lParam));
		    if( !UseCPUThread ) ResumeSound();
	   		break;
	
	   case WM_CREATE:
		   DragAcceptFiles(
			   hwnd,    // ドラッグ＆ドロップを登録するウィンドウ
			   TRUE			// アクセプトオプション
		   );
		   loadPopupMenu( ((LPCREATESTRUCT)(lParam))->hInstance, "SUBMENU");
	
			if( Console) {
				FreeConsole();
				AllocConsole();		// alloc console
			}

			//setMenuCheck( IDM_KEYCLICK   , keyclick);
			setMenuCheck( IDM_USE_SAVE_TAPE , UseSaveTapeMenu);
	
	        setMenuOnOff( IDM_OPEN_SAVE_TAPE   , UseSaveTapeMenu);
	        setMenuOnOff( IDM_REWIND_SAVE_TAPE , UseSaveTapeMenu);
	        setMenuOnOff( IDM_EJECT_SAVE_TAPE  , UseSaveTapeMenu);
	
	        setMenuOnOff( IDM_OPEN_DISK1        , disk_num);
	        setMenuOnOff( IDM_OPEN_DISK2        , disk_num ==2);

			//setMenuTitle( FILE_LOAD_TAPE);	/* MENU TITLE を変更 2005/7/17 */
			//setMenuTitle( FILE_SAVE_TAPE);
			//setMenuTitle( FILE_DISK);
			setMenuTitle( FILE_EXTROM);

			InvalidateRect( hwnd, NULL, TRUE);
	
			//PrepareLayeredWindow(hwnd);	// transparent
			//MySetLayeredWindowAttributes(hwnd,0,  210,LWA_ALPHA);

	        UseJoystick= JoysticOpen();
			break;


	   case WM_DROPFILES:		// ドラッグ&ドロップ
		   hDrop = (HDROP)wParam;
		   uFileNo = DragQueryFile((HDROP)wParam, 0xFFFFFFFF, NULL, 0);
		   for (i = 0; i < (int)uFileNo; i++) {
			   DragQueryFile(hDrop, i, szFileName, sizeof(szFileName));
			   drag_open(szFileName, sizeof(szFileName));
		   }
		   DragFinish(hDrop);
		   break;
#if 0
	   case WM_PAINT:
		   if( surface1 && surface2)
	   		{
			OSD_Surface * surface;
			int w,h;

			w= surface1->w;
			h= (bitmap ? lines :200) * scale; // text mode =200  graphics mode=204
			//top = (204-lines)*scale; 			// 200 line= 4     204 line= 0

			hdc= BeginPaint(hwnd, &paintstruct);
			if( bitpix ==8) realize_lplogpal( hdc,0);

			if( isScroll())				// scroll mode
				surface = surface2;
			else
				surface = surface1;		// normal mode

			//BitBlt( hdc, 0+paddingw,paddingh, w , h, surface->hdcBmp,0,0, SRCCOPY);

			StretchBlt( hdc , paddingw, paddingh , screen->w ,screen->h ,surface->hdcBmp ,0 ,0 , surface->w, surface->h , SRCCOPY);
	
			EndPaint(hwnd,&paintstruct);
			}
			break;
#endif
	   case WM_SIZE:
#if 1
		   if( wParam == SIZE_MAXIMIZED)
			   is_maximized = 1;
		   else
			   is_maximized = 0;

		   if( wParam == SIZE_MAXIMIZED || wParam == SIZE_RESTORED)	/* 最大化 ・元のサイズに戻す */
				{
				if( screen ) 
					{
					int w,h;
					w = LOWORD(lParam);			// ウィンドウの幅・高さを取得
					h = HIWORD(lParam);
		  			w -= (BORDERW+PADDINGW); 
					h -= (BORDERH+PADDINGH);
					//w -=50;
					//h -=99;
					
					screen->w = w;
					screen->h = h;
					OSD_ClearWindow();
					}
				} 
#endif
		   break;
	   case WM_SIZING:
		   {
			if( screen )
			   {
				RECT *rc = (RECT*) lParam;
				int w,h;

				w = rc->right  - rc->left+1;	/* ウィンドウの幅・高さを取得 */
				h = rc->bottom - rc->top +1;
		   		w -= (BORDERW+PADDINGW*2); 
				h -= (BORDERH+PADDINGH*2);

				if( !inTrace)
					{
					if( w < Width  ) w= Width;
					if( h < Height ) h= Height;		/* 最小値 */
				}else {
					if( w < M6WIDTH * DEBUG_WINDOW_RATE ) w= M6WIDTH * DEBUG_WINDOW_RATE;
					if( h < M6HEIGHT* DEBUG_WINDOW_RATE ) h= M6HEIGHT* DEBUG_WINDOW_RATE;		/* 最小値 */
					}
										
					/* ------- ウインドウの縦横比を一定にする ------- */
				switch( wParam)					
					{
					case WMSZ_RIGHT:
					case WMSZ_LEFT:     h= (int)(w* M5HEIGHT/ M5WIDTH); // = (w * 625) / 1000; break;
					case WMSZ_TOP:
					case WMSZ_BOTTOM:   w = (int)(h* M5WIDTH/ M5HEIGHT) ; break; // w = (h * 16 ) / 10; break;
					case WMSZ_BOTTOMRIGHT:
					case WMSZ_TOPRIGHT:
					case WMSZ_BOTTOMLEFT:
					case WMSZ_TOPLEFT:
										if( w >h ) w = (int)(h*M5WIDTH/ M5HEIGHT); else h=(int)(w* M5HEIGHT/ M5WIDTH); 
										break;
					}
				w -=12;
				screen->w = w;
				screen->h = h;

				/* ------------ Window の位置を設定 --------------- */
				switch( wParam )
					{
					case WMSZ_LEFT:
					case WMSZ_BOTTOMLEFT:
							rc->left  = rc->right  - (w +(BORDERW+PADDINGW*2));
							rc->bottom= rc->top    + (h +(BORDERH+PADDINGW*2));
							break;
					case WMSZ_TOP:
					case WMSZ_TOPRIGHT:
							rc->right = rc->left   + (w +(BORDERW+PADDINGW*2));
							rc->top   = rc->bottom - (h +(BORDERH+PADDINGH*2));
							break;
					case WMSZ_BOTTOM:
					case WMSZ_BOTTOMRIGHT:
					case WMSZ_RIGHT:
							rc->right = rc->left   + (w +(BORDERW+PADDINGW*2));
							rc->bottom= rc->top    + (h +(BORDERH+PADDINGH*2));
							break;
					case WMSZ_TOPLEFT:
							rc->left  = rc->right  - (w +(BORDERW+PADDINGW*2));
							rc->top   = rc->bottom - (h +(BORDERH+PADDINGH*2));
							break;							
					}
				OSD_ClearWindow();
				}
		    break;
		   }
       case WM_QUERYNEWPALETTE: 
       		{
       	    HDC hdc;
            hdc = GetDC( hwndMain);
            realize_lplogpal( hdc,TRUE);
            ReleaseDC( hwndMain , hdc);
			return TRUE;
			}
	   case WM_SYSKEYDOWN:
	   case WM_KEYDOWN:					// key in  buffering
			{
			if( wParam ==0 && lParam ==0) {
				break;
			}
			static int cnt=0;
			int isRepeat = HIWORD(lParam) & 0x4000 ? 1:0;
			PRINTDEBUG1(KEY_LOG,"repeat= %d \n", isRepeat);
			if( isRepeat )
				{
				 ++cnt;
				 if( ((cnt % 4) ==0) || ((cnt % 2) ==0))	// Slow down key repeat 
					{
					break;
					}
				}
			int keydown = !(HIWORD(lParam) & 0x8000);
			int scancode = wParam;
			int osdkeycode = OSD_transkey( scancode);
			if( osdkeycode == OSDK_PAGEDOWN && P6Version == PC60)
				{
				 break;
				}
			
			if (romaji_mode && kanaMode && inTrace == DEBUG_NONE)  // Exclusion debug mode
				{
				if(convert_romaji2kana(osdkeycode)!=HENKAN_CANCEL)
					break;
				}
				
			write_keybuffer( keybuffer ,0, keydown , scancode , osdkeycode);
			
			PRINTDEBUG3(KEY_LOG,"[WIN32GUI][WINDOWFUNC] keydown= %X scancode= %02X osdkeycode= %02X  \n",keydown ,scancode , osdkeycode);
			//write_keybuffer(0,!(HIWORD(lParam) & 0x8000),wParam);
			break;
			}
	
	   case WM_SYSKEYUP:
	   case WM_KEYUP:
	         {
 			 int keydown = !(HIWORD(lParam) & 0x8000);
			 int scancode = wParam;
			 int osdkeycode = OSD_transkey( scancode);
			 if (osdkeycode == OSDK_PAGEDOWN && P6Version == PC60)
					{
					break;
					}

			 write_keybuffer( keybuffer, 0, keydown , scancode , osdkeycode);
			 PRINTDEBUG3(KEY_LOG,"[WIN32GUI][WINDOWFUNC] keyup= %X  scancode= %02X osdkeycode= %02X \n",keydown ,scancode , osdkeycode);
			}
			break;
	   case WM_COMMAND:
			if( !UseCPUThread ) StopSound();
			wm_command(hwnd, message, wParam , lParam);	// menu selection
			if( !UseCPUThread ) ResumeSound();
			break;
	   //case WM_SETFOCUS: 		// set keyrepeat slowly  when getting focus  2003/10/18
	   //		setKeyrepeat(DEFAULT_KEY_REPEAT);
	   //		break;
	   case WM_KILLFOCUS: 		// set keyrepeat fastly when  killing focus  2003/10/18
	   //		storeKeyrepeat();
			kbFlagGraph=0;		// force GRAPH keyup
	   		break;
	   case WM_ENTERMENULOOP:	// when menu showing , stop sound  2005/12/31
		    if( !UseCPUThread ) StopSound();
		    break;
	   case WM_EXITMENULOOP:	// when menu exit, resume sound    2005/12/31
	        if( !UseCPUThread ) ResumeSound();
		    break;
	   case WM_MENUCHAR:		// ALT+ABC...　を押しても、チン　という音を出さないようにする。 2011/6/14
			return MNC_CLOSE << 16;
	   default: return DefWindowProc(hwnd, message,wParam,lParam);
	}
 return 0;
}


////////////////////////////////////////////////////////////////
// メッセージ表示
//
// 引数:	mes			メッセージ文字列へのポインタ
//			cap			ウィンドウキャプション文字列へのポインタ
//			type		表示形式指示のフラグ
// 返値:	int			押されたボタンの種類
//							OSDR_OK:     OKボタン
//							OSDR_CANCEL: CANCELボタン
//							OSDR_YES:    YESボタン
//							OSDR_NO:     NOボタン
////////////////////////////////////////////////////////////////
int OSD_MessageBox( const char *mes, const char *cap, int type )
{
	int Type = MB_OK;
	int res;
	
	// メッセージボックスのタイプ
	switch( type&0x000f ){
	case OSDM_OK:			Type = MB_OK;			break;
	case OSDM_OKCANCEL:		Type = MB_OKCANCEL;		break;
	case OSDM_YESNO:		Type = MB_YESNO;		break;
	case OSDM_YESNOCANCEL:	Type = MB_YESNOCANCEL;	break;
	}
	
	// メッセージボックスのアイコンタイプ
	switch( type&0x00f0 ){
	case OSDM_ICONERROR:	Type |= MB_ICONERROR;		break;
	case OSDM_ICONQUESTION:	Type |= MB_ICONQUESTION;	break;
	case OSDM_ICONWARNING:	Type |= MB_ICONWARNING;		break;
	case OSDM_ICONINFO:		Type |= MB_ICONINFORMATION;	break;
	}
	
	res = MessageBox( hwndMain, mgettext(mes), cap, Type | MB_TOPMOST );
	
	switch( res ){
	case IDOK:	return OSDR_OK;
	case IDYES:	return OSDR_YES;
	case IDNO:	return OSDR_NO;
	default:	return OSDR_CANCEL;
	}
}

// ****************************************************************************
//          messagebox: メッセージボックスを表示
// ****************************************************************************
int messagebox(char *str, char *title)
{
/*	int ret;
	 ret=0;
//	 if( !hwndMain)
//	 	hwndMain = _win32_getWindowHandle();
	 if( hwndMain)
		 ret=MessageBox( hwndMain, mgettext( str ),title, MB_OK);
	 return(ret);
*/
}


// ****************************************************************************
//          最大化されているかどうか　取得
// ****************************************************************************
int get_is_maximized(void)
{
	return is_maximized;
}


// ****************************************************************************
//          outputdebugstring: デバッガにメッセージを送る
// ****************************************************************************
void outputdebugstring(char *buff)
{
	OutputDebugString( buff);
}

// ****************************************************************************
//          putlasterror: 最後のエラーを表示
// ****************************************************************************
void putlasterror(void)
{
 LPVOID lpMsgBuf;

 //エラーコードを書式化
 FormatMessage((FORMAT_MESSAGE_ALLOCATE_BUFFER |
 FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS),
 NULL,GetLastError(),MAKELANGID(LANG_NEUTRAL,
 SUBLANG_DEFAULT),(LPTSTR)&lpMsgBuf,0,NULL);
 MessageBox(NULL, lpMsgBuf, TEXT("Error"), MB_OK | MB_ICONINFORMATION);

 LocalFree(lpMsgBuf);
}


// ********************************************************************************
// TO DO: 環境非依存コードにすること！

#if 0
// ****************************************************************************
//          PutDiskAccessLamp: ディスクのアクセスランプ　点灯/消灯
// ****************************************************************************
void PutDiskAccessLamp(int sw)
{
    if( UseDiskLamp)
        {
        HDC hdc;
        HBRUSH hbrush, hOldbrush;
        int r=0,g=0,b=0;
        int sx,sy,ex,ey;
    
        sx= Width-paddingw  -40;
        sy= Height+paddingh+10;
		ex= sx+30;
		ey= sy+10;
		
        switch(sw)
           {
            case 0:  r=  0; g=0;   b=0; break;
            case 1:  r=255; g=0;   b=0; break;
			case 255:r=255; g=255; b=255; break;
           }

        hdc = GetDC( hwndMain);
        hbrush = CreateSolidBrush( RGB(r,g,b));
        hOldbrush = (HBRUSH) SelectObject(hdc,hbrush);

		if( sw <2)
			{
		    Rectangle( hdc, sx   ,sy  , ex , ey); 
			}
		else
			{
			MoveToEx(  hdc, sx-1 ,sy-1 ,NULL);
			LineTo(    hdc, ex+1 ,sy-1);
			LineTo(    hdc, ex+1 ,ey+1);
			LineTo(    hdc, sx-1 ,ey+1);
			LineTo(    hdc, sx-1 ,sy-1);
			}

        SelectObject( hdc,hOldbrush);
        DeleteObject( hbrush);
        ReleaseDC( hwndMain , hdc);
        }
}
#endif










/* ******************************** not use ******************************************************* */


// ****************************************************************************
//          putStatusBarSub: ステータスバーの表示 (sub)
// ****************************************************************************
#if 0
void putStatusBarSub(void)
{
    HDC hdc;
//    int x,y;
  	int w,h;		// blt size:           width height
  	int top;		// display start line
  
  	w= hb1.Width;
  	h= (bitmap ? lines :200) * scale; // text mode =200  graphics mode=204
  	top = (204-lines)*scale; 			// 200 line= 4     204 line= 0
	hdc= GetDC( hwndMain);
    BitBlt( hdc, 0,top+paddingh+h+5, hb_statusbar.Width , hb_statusbar.Height, hb_statusbar.hdcBmp,0,0, SRCCOPY);  

    ReleaseDC( hwndMain, hdc);
}
#endif

#if 0
			{
			int w,h,top;
			w= hb1.Width;
			h= (bitmap ? lines :200) * scale; // text mode =200  graphics mode=204
			top = (204-lines)*scale; 			// 200 line= 4     204 line= 0

			hdc= BeginPaint(hwnd, &paintstruct);
			if( bitpix ==8) realize_lplogpal( hdc,0);

			if( isScroll())	// scroll mode
				BitBlt( hdc, 0+paddingw,top+paddingh, w , h, hb2.hdcBmp,0,0, SRCCOPY);
			else			// normal mode
				BitBlt( hdc, 0+paddingw,top+paddingh, w , h, hb1.hdcBmp,0,0, SRCCOPY);
	
			EndPaint(hwnd,&paintstruct);

			}

/*#if 0
#ifdef DEBUG
#if 1
			if( !Console) {
				FreeConsole();
				AllocConsole();		// alloc console
			}
			Trace=!Trace;
#else
			monitor_mode = !monitor_mode;
			if(monitor_mode)
                {
				resizewindow( 1,2);
                UPeriod_bak = UPeriod;
                UPeriod = 60;
                }
			else
                {
				resizewindow( new_scale, new_scale);
                UPeriod = UPeriod_bak;
                }
            Trace=!Trace;
#endif
#endif
#endif
			break;
*/
#endif
#endif 
//WIN32
