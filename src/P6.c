/** iP6: PC-6000/6600 series emualtor ************************/
/**                                                         **/
/**                           P6.c                          **/
/**                                                         **/
/** by Windy 2002-2004                                      **/
/** by ISHIOKA Hiroshi 1998-2000                            **/
/** This code is based on fMSX written by Marat Fayzullin   **/
/*************************************************************/
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#if HAVE_DIRECT_H
#include <direct.h>
#endif

#ifdef UNIX
#include <unistd.h>
#endif
#ifdef X11
#include "unix/Xconf.h"
#endif

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <time.h>
#include <assert.h>


#include "types.h"
#include "os.h"     

#include "P6.h"
#include "mem.h"
#include "Refresh.h"
#include "Sound.h"
#include "Option.h"
#include "pngrw.h"	// for write_png()
#include "SThread.h"

#include "Video.h"

#include "Build.h"

#include "keys.h"		/** scancode to keycode translate tables **/
#include "newkeydef.h"		/** Keyboard mapping **/
#include "schedule.h"
#include "fm.h"

#include "Debug.h"
#include "autokey.h"


#ifdef WIN32
#include "win/WinMenu.h"		// for IDM_FULLSCRN
#include "win/WinInput.h"
#endif

void BlitSurface(OSD_Surface * dst_surface, int ex, int ey, int w, int h, OSD_Surface * src_surface, int sx, int sy);


char *machinetype[5]={"PC-6001","PC-6001mkII","PC-6001mkIISR","PC-6601","PC-6601SR"};   // machinetype[ P6Version] で機種名が分かる

byte *Keys;

OSD_Surface *surface1 = NULL;
OSD_Surface *surface2 =NULL;
OSD_Surface *screen =NULL;
OSD_Surface *debug_surface =NULL;
OSD_Surface *status_surface = NULL;

#ifndef O_BINARY
#define O_BINARY 0
#endif

int paddingw = PADDINGW;  // move from Win32.c
int paddingh = PADDINGH;
int ignore_padding_flag =0; // 1:ignore padding variable of window

char *MsgOK      = "OK\n";
char *MsgFAILED  = "FAILED\n";

enum {NOT_USE_THREAD , USE_THREAD , USE_TIMER };

int UseCPUThread=0;         //   0: 単一スレッドで動かす。 1: CPU を別スレッドで動かす。2: タイマーで定期的に呼び出す
extern int CPUThreadRun;


int FastTape;	// use Fast Tape     1: high  0:normal

void putStatusBar(void);
OSD_Surface * getRefreshSurface( void);


char *Title=PROGRAM_NAME  BUILD_VER;		// title name

int refreshAll_flag;

int romaji_mode =0;   // romaji mode 1:true  0:false
int new_romaji_mode=0;

// ****************************************************************************
//           CPU RUN
// ****************************************************************************
int CpuRun(void)
{
	int ret=0;
#ifdef X11
	printf("-----------------------------------------------------------\n");
	printf("|    keyboard:  F12: exit  Scroll lock: kana              |\n");
	printf("-----------------------------------------------------------\n");
#endif

	// ************** ここから、エミュレータを実行 *********************
    if( UseCPUThread ==2)	// ********* 1/60秒ごとに、関数を呼ぶ ******************
      {

         OSD_InstallTimerEvent();                               // Timer イベントをインストール        
		 ret = OSD_RunApplicationEventLoop();					// メッセージループを回す
      }
    else if( UseCPUThread==1)		// *****  別スレッドでCPU を実行するか？ ***
        {					
		 SThread *cputhread;
         CPUThreadRun=1;

		 cputhread = OSD_CreateThread( CPUThreadProc ,NULL); // 別スレッドでCPUを実行しながら
		 OSD_DestroyThread( cputhread );

		 ret = OSD_RunApplicationEventLoop();					// メッセージループを回す
        }
    else
        {
		while (1)		// *** メッセージが来てない間を縫って、CPUを実行 ***
			{
			ret = OSD_ChkEvent();						// メッセージが来ていたら、メッセージを処理
			if (ret == 0 && Romfile_found) CPUThreadProc(NULL);			// メッセージが来てなかったら、CPUを実行する。
			if (ret == -1) break;						// 終了
			}
//		  ret = message.wParam;
        }
    return ret;
}


//  hdc= GetDC(hwndMain);
//  if( bitpix ==8)
//  		{
//    	 realize_lplogpal(hdc, FALSE);
//        }



// ****************************************************************************
//          OSD_BlitSurface: サーフェスからサーフェスに　blit する
// In:  dst_surface : 出力先サーフェス
//      ex,ey       : 出力先サーフェスの左上の座標
//      w,h         : 出力サーフェスのサイズ
//      src_surface : 転送元サーフェス
//      sx,sy       : 転送元サーフェスの左上の座標
// ****************************************************************************
void BlitSurface(OSD_Surface * dst_surface, int ex, int ey, int w, int h, OSD_Surface * src_surface, int sx, int sy)
{
	int y;
	int pitch = dst_surface->pitch;
	int bpp = dst_surface->format->BytesPerPixel;
	for (y = 0; y < h; y++)
	{
		memcpy((unsigned char*)dst_surface->pixels + (ey+y)* pitch + ex*bpp, (unsigned char*)src_surface->pixels + ((sy+y)*pitch) + sx*bpp, w*bpp);

	}
}



// ****************************************************************************
//          PutImage: エミュレータ画面を、スクリーンに bitblt
// ****************************************************************************
/*
    normal:
	   screen <-- hb1
	
	scroll:
	   screen <-- hb2 <-- hb1

*/
void PutImage(void)
{
#if 1
  int x,y;
  int w,h;		// blt size:           width height
  OSD_Surface *src_surface = NULL;

  int _paddingw , _paddingh;

  if( ignore_padding_flag)
  {
	  _paddingw = paddingw;
	  _paddingh = paddingh;
	  paddingw = paddingh =0;
  }


  w= surface1->w;
  //h= (bitmap ? lines :200) * scale; // text mode =200  graphics mode=204
  h= lines *scale;
  //top = (204-lines)*scale; 			// 200 line= 4     204 line= 0
  

  if( isScroll())	// scroll mode 
  	{
	  int dx,dy;
	  x = portCB *256 + portCA;
	  y = portCC;

	  x *= scale;
	  y *= scale;

	  dx = surface1->w -x;
	  //dy = surface1->h -y;
	  dy = lines *scale -y;		// 縦スクロールで横線入るので

	  BlitSurface(surface2, dx, dy, x, y, surface1, 0, 0);
	  BlitSurface(surface2, 0, dy, dx, y, surface1, x, 0);
	  BlitSurface(surface2, dx, 0, x, dy, surface1, 0, y);
	  BlitSurface(surface2, 0, 0, dx, dy, surface1, x, y);



	  //OSD_BlitSurface( screen   ,0+paddingw ,paddingh, w       , h , surface2 ,0 ,0);  // to screen
	  src_surface = surface2;
	  }
  else
	  src_surface = surface1;
  




  if( !is_open_debug_dialog )	// normal
		{
		//                dest        dest_x      dest_y   w        h       src        src_x src_y
		//OSD_BlitSurface( screen     ,0+paddingw ,paddingh, Width , Height ,surface1 ,0 ,0);  // to screen
		int w,h, amari_w=0;
		float tmp;
		w = screen->w;
		h = screen->h;
		tmp = (float)h* 1.5686;
		w = (int)(float)(tmp+0.5);	// 四捨五入


		amari_w = (screen->w - w )/2;
		OSD_BlitSurface( screen     ,0+paddingw +amari_w ,paddingh, w       , h      ,src_surface ,0 ,0);  // to screen

	  }
  else if( inTrace)
		{			// debugger を開いたままステップ実行
	//	w = debug_surface->w;  h= debug_surface->h;
	//	OSD_BlitSurface( screen     ,0+paddingw ,paddingh, w      , h      ,debug_surface ,0 ,0);  // to screen
	
		OSD_BlitSurface( debug_surface ,0       ,0       , M6WIDTH,M6HEIGHT,src_surface ,0 ,0);  // to screen
		}
  else  {			// debugger を開いたままフル実行
		OSD_BlitSurface( debug_surface ,0       ,0       , M6WIDTH,M6HEIGHT,src_surface ,0 ,0);  // to screen
		RefreshDebugWindow();
	    }

  if( ignore_padding_flag)
  {
	  paddingw = _paddingw;
	  paddingh = _paddingh;
  }
	
  if( !is_open_debug_dialog)
	  OSD_BlitSurface( screen ,0       ,0 /*screen->h +paddingh*/  , w, STATUSBAR_HEIGHT,status_surface ,0 ,0);  // status bar to screen

#endif
}

// ****************************************************************************
//	keyboard_set_stick: set stick flag  
//                                          (keyboard()'s  internal function )
//   In: osdkeycode: OSDK keybode
//       keydown: 1: down  0:up
// ****************************************************************************
void keyboard_set_stick( int osdkeycode ,int keydown)
{
	if (osdkeycode == OSDK_SHIFT || osdkeycode == OSDK_LSHIFT || osdkeycode == OSDK_RSHIFT) 
		if( keydown)
			kbFlagShift = 1;
		else
			kbFlagShift = 0;

		/* for stick,strig */
    	{
    	byte tmp;
    	switch(osdkeycode) {
			case OSDK_SPACE  : tmp = STICK0_SPACE; break;
			case OSDK_LEFT   : tmp = STICK0_LEFT;  break;
			case OSDK_RIGHT  : tmp = STICK0_RIGHT; break;
			case OSDK_DOWN   : tmp = STICK0_DOWN;  break;
			case OSDK_UP     : tmp = STICK0_UP;    break;
			case OSDK_PAUSE  : tmp = STICK0_STOP;  break;
			//case OSDK_LEFTUP : tmp = STICK0_LEFT | STICK0_UP; break;
			//case OSDK_RIGHTUP: tmp = STICK0_RIGHT| STICK0_UP; break;
			//case OSDK_LEFTDOWN:tmp = STICK0_LEFT | STICK0_DOWN; break;
			//case OSDK_RIGHTDOWN:tmp= STICK0_RIGHT| STICK0_DOWN; break;
			case OSDK_SHIFT  :
			case OSDK_LSHIFT : 
			case OSDK_RSHIFT : tmp = STICK0_SHIFT;break;
			default: tmp = 0;
	      }
		if( keydown )
			stick0 |= tmp;
      	else
      		stick0 &=~tmp;
		}
}

byte keyboard_get_stick(void)
{
		return stick0;
}


void keyboard_allclear_stick(void)
{
	stick0 = 0;
}

// ****************************************************************************
//	keyboard_set_key:
//                                          (keyboard()'s  internal function )
//   In:  osdkeycode : OSDK keycode
//   Out:_keyGFlag: graph keycode
//       _p6key:    p6 keycode
// ****************************************************************************
void keyboard_set_key(int osdkeycode,int *_keyGFlag ,int *_p6key)
{
		int osdkeycode_bak;
		if((P6Version==0)&&(osdkeycode==OSDK_F8)) osdkeycode=0; /* MODE key when 60 */

		osdkeycode_bak = osdkeycode;
    	osdkeycode&=0x1FF;
    	
    	 if (kbFlagGraph && inTrace == DEBUG_NONE)
			 Keys = Keys7[ osdkeycode ];
         else if (kanaMode && inTrace == DEBUG_NONE)
			if (katakana)
	  			if (kbFlagShift ) 
                	Keys = Keys6[ osdkeycode ];
	  			else 
                	Keys = Keys5[ osdkeycode ];
			else if (kbFlagShift)
                	Keys = Keys4[ osdkeycode ];
	  			else 
                	Keys = Keys3[ osdkeycode ];
      		else if (kbFlagShift)
            		Keys = Keys2[ osdkeycode ];
				else 
            		Keys = Keys1[ osdkeycode ];
      
	  	  *_keyGFlag = Keys[0]; *_p6key = Keys[1];

		  if(( osdkeycode_bak >= 0x61)&&  osdkeycode_bak <=0x7a)  
			  osdkeycode_bak -= 0x20;	// convert alphabet a-z to A-Z


		    /* control key + alphabet key */
		  if ((kbFlagCtrl == 1) && ( osdkeycode_bak  >= 0x41) && ( osdkeycode_bak  <= 0x5a))
				{*_keyGFlag = 0; *_p6key = osdkeycode_bak - 0x41 + 1;}
		  /*if (p6key != 0x00) IFlag = 1;*/
		    //if (Keys[1] != 0x00) 
}



// ****************************************************************************
//	keyboard_func_key
//
//   In: OSDK keycode
// ****************************************************************************

void keyboard_func_key( int osdkeycode)
{
    	switch(osdkeycode)
			{
			case OSDK_F6:
				if( kbFlagGraph)		// ALT+F6 : debugger
					{
					 open_debug_dialog();
					 Trace=1;
					}
				break;
			case OSDK_F9:
				printf("** f1 ＼n");
				//code_log_flag= !code_log_flag;	// op code logging
				break;

        	case OSDK_F10:
#ifdef SOUND
		  		StopSound();          /* Silence the sound        */
#endif
#ifdef X11
		 		run_conf();
		 		ClearScr();
				//usleep(100);
#endif
#ifdef SOUND
		  		ResumeSound();        /* Switch the sound back on */
#endif
		  		break;
#ifdef DEBUG
	        case OSDK_F11: if( Console) Trace=!Trace;break;
#endif
			case OSDK_F4:
				 if(!kbFlagGraph) break;	// ALT+f4 ?
#ifdef X11
			case OSDK_F12: CPURunning=0;
							exit(0); 
#endif

			case OSDK_CTRL:
			case OSDK_LCTRL:  kbFlagCtrl=1;break;	// ctrl
	        case OSDK_ALT:
			case OSDK_LALT:	kbFlagGraph=1;break;	// graph

			//	        case VK_INSERT:   osdkeycode=XK_F13;break; /* Ins -> F13 */
	      }

}




// ****************************************************************************
//          Keyboard: キーボードの走査処理
//  （Z80 CPU core から、定期的に呼び出されている。）
// ****************************************************************************
void Keyboard(void)
{
	static int convert_length=0;
	static int convert_counter=0;
	int keydown;
	int scancode;
	int osdkeycode;

#ifdef DEBUG
	if( inTrace != DEBUG_NONE) return;		// debug mode:  do nothing
#endif

//	if( KeyIntFlag != INTFLAG_NONE) return;		// keyin interrupt requesting , do nothing  2010/3/5 -> ゲームキーだめになる削除 2021/2/27

#ifdef SOUND
	FlushSound();  /* Flush sound stream on each interrupt */
#endif

	if( read_keybuffer( keybuffer ,NULL ,&keydown , &scancode ,&osdkeycode) )
		{
		PRINTDEBUG2(KEY_LOG,"[P6][keyboard] osdkeycode=%02X (%c)" ,osdkeycode, (osdkeycode < 0x20) ? 0: osdkeycode);
		if( osdkeycode == OSDK_SHIFT) PRINTDEBUG(KEY_LOG," SHIFT KEY ");

		keyboard_set_stick(osdkeycode ,keydown);		// for stick

		if( keydown || osdkeycode==OSDK_CAPSLOCK)       // caps lock のみ、keydown /up ともに押したことにする。
			{
			int  _keyGFlag ,_p6key;

			keyboard_func_key( osdkeycode);					// function key
			keyboard_set_key( osdkeycode ,&_keyGFlag,&_p6key);		// normal key

			keyGFlag = _keyGFlag;
			p6key = _p6key;
			PRINTDEBUG2(KEY_LOG,"-->p6key=%02X (%c) KEYDOWN\n", p6key,(p6key <0x20) ? 0:p6key);
			
			if (_p6key != 0x00) 
				{
				KeyIntFlag = INTFLAG_REQ;		// request keyin interrupt..to P6 system
				PRINTDEBUG(KEY_LOG,"[P6][keyboard]  keyintflag REQUEST.... \n");
				}
			}
		else 						// keyup ?
			{
			if (osdkeycode==OSDK_ALT || osdkeycode==OSDK_LALT)  kbFlagGraph=0;
			if (osdkeycode==OSDK_CTRL|| osdkeycode==OSDK_LCTRL ) kbFlagCtrl=0;
			PRINTDEBUG(KEY_LOG,"KEYUP \n");
     		}
		if( p6key == 0x40)
			{
			 code_log_flag = 1;
			}
		}
}


static unsigned char rgb[256][4];		// エミュレータで使用するパレットの rgb値

/* ******************************* */
/* Below are the routines related to allocating and deallocating
   colors */
// ****************************************************************************
//          InitColor: カラーデータの初期化
//    　　（エミュレータ画面に直接書き込むデータ）
// ****************************************************************************
/* set up coltable, alind8? etc. */
void InitColor(int screen_num)
{
	byte *Xtab;
	//XID col;
	ColTyp col;
//	unsigned char rgb[256][4];
	byte i,j;
	word R,G,B,H;


// param Pal11  Pal12  Pal13  Pal14  Pal15  Pal53については、Refresh.c に移動しました。

//	if( bitpix ==8) 
//		if(!create_lplogpal()) return;		// 256 カラーだと、論理パレットを作成
	Xtab = OSD_GetXtab();

//#ifdef WIN32
//	for(i=0;i<6;i++)
//		if( trans[i] >256)
//			trans[i] /= 256;
//#endif

	for(H=0;H<2;H++)
      for(B=0;B<2;B++)
        for(G=0;G<2;G++)
          for(R=0;R<2;R++)
		  {
          i=R|(G<<1)|(B<<2)|(H<<3);

		  col = OSD_Getcolor(trans[param[i][0]] ,trans[param[i][1]] ,trans[param[i][2]] ,0);

		  rgb[i][3] = col.ct_byte[3] ;
		  rgb[i][2] = col.ct_byte[2] ;
		  rgb[i][1] = col.ct_byte[1] ;
		  rgb[i][0] = col.ct_byte[0] ;


		  if (bitpix == 8) 			// ======== 256 color mode ============
      	 	{
			BPal[i].ct_byte[0]=col.ct_xid;		// 実際に描画するのは、パレットのインデックス
#ifdef WIN32
			BPal[i].ct_byte[0]=i;		// 実際に描画するのは、パレットのインデックス
#endif
			}
	      else 
		    {						// ======== full color mode ============
	        for(j=0; j<4; j++)
	             BPal[i].ct_byte[j]= col.ct_byte[Xtab[j]]; // 実際に描画するのは、RGBの値

            }
         }

#ifdef WIN32
	OSD_SetPalete(rgb);
#endif

	/* setting color list for each screen mode */
	for(i=0;i<4;i++) BPal11[i] = BPal[Pal11[i]];
	for(i=0;i<8;i++) BPal12[i] = BPal[Pal12[i]];
	for(i=0;i<8;i++) BPal13[i] = BPal[Pal13[i]];
	for(i=0;i<4;i++) BPal14[i] = BPal[Pal14[i]];
	for(i=0;i<8;i++) BPal15[i] = BPal[Pal15[i]];
	for(i=0;i<32;i++) BPal53[i] = BPal[Pal53[i]];
	
	// ******************* fast palet      add 2002/9/27 *********
	for(i=0;i<32;i++) BPal62[i] = BPal53[i];  // for RefreshScr62/63
	for(i=0;i<16;i++) BPal61[i] = BPal[i];    // for RefreshScr61

//  for(i=0;i<32;i++) BPalet[i] = BPal53[i];    // for Palet Color 2003/2/4
}


// ****************************************************************************
//          savescreenshot: スクリーンショットを出力
//  In: path : 出力先パス名
// ****************************************************************************
int savescreenshot(char *path)
{
	OSD_Surface *surface;
	int ret=1;
	int i;
 
#ifdef __APPLE__
	surface = OSD_GetVideoSurface();
#else
	if( isScroll())	// scroll ? 
		surface = surface2;	// scroll mode
	else
		surface = surface1;	// normal mode
#endif 
 
	int len = strlen(path);
	char *p=  path+len-4;
	int png = !stricmp(p, ".png");
	int bmp = !stricmp(p, ".bmp");

	if( png || ( !png && !bmp) )	// png or 拡張子がついてないとき
		{
		if( surface->format->BitsPerPixel ==8) 
	 		{
			 for(i=0 ; i< 16 ; i++)		/* 8bpp だと、パレットを渡す */
				 set_png_palette( i, rgb[i][2], rgb[i][1] ,rgb[i][0]);
			 for (i = 16; i < 256; i++)
				 set_png_palette(i, 0, 0, 0);
			 set_png_maxpalette( 256);
	 		}
 
	// 	ret = write_png(path, surface->pixels, surface->w 
	//		,lines*scale,  surface->format->BitsPerPixel );
		write_png(path , surface );
		}
	if ( bmp)					// bmp のとき
		{
		if (surface->format->BitsPerPixel == 8)
			{
			}

		}
	return( ret);
}



/****************************************************************/
/*** INTERNAL FUNCTION  PROTOTYPE                             ***/
/****************************************************************/



void Patch(reg *R)
{
  printf("**********iP6: patch at PC:%4X**********\n",R->PC.W);
}


// ****************************************************************************
// 		UpdateScreen:  画面の更新処理 
// 						Z80.cから呼ばれる
// ****************************************************************************
int UpdateScreen(void)
{
	static int count  = 0;
	static int UCount=1;
	int    updated =0;

	if (!Romfile_found) return 0;		// ROM なしだと何もしない

//	if ((CasStream[0] && count++ == 60))
	if( count++ == 60)
		{
#ifdef __linux__
		fpos_t pos;
#else
		fpos_t pos =0;
#endif
    	count = 0;
    
		if (CasStream[0])
			fgetpos(CasStream[0], &pos);    // カセットの読み込み位置取得
		//else
			//pos = -1;					// カセットなし
    	SetTitle(pos);                  // タイトル更新
		}

	if (UseStatusBar && (count==0) )
		{
		putStatusBar();                 // ステータスバー更新
   		}
  
	if((!--UCount)||!EndOfFrame)      // 画面描画処理
  		{
  		updated = 1;				// refreshed screen
		UCount=UPeriod;
    	/* Refreshing screen: */
     	/*printf("ref:%d,%d,%d \n",CRTMode1,CRTMode2,CRTMode3); */

#ifdef CARBON
        ChangeSurface_traditional();
#endif

        OSD_LockSurface( getRefreshSurface() );

		if( sr_mode) {			/* SR-BASIC   (add 2002/2) by Windy*/
			SCR[CRTMode1+2][CRTMode2 ? (CRTMode3 ? 3 : 2) : 0]();
     	}else {
			SCR[CRTMode1][CRTMode2 ? (CRTMode3 ? 3 : 2) : 0]();
     	}

        OSD_UnlockSurface( getRefreshSurface() );
  	}
 return (updated);
}


/* CMT割り込みの許可の外部インタフェース */
#if 0
void enableCmtInt(void)
{ if (CasMode) CmtIntFlag = INTFLAG_REQ; }
#endif



// ****************************************************************************
//	InitVariable:
//     before calling InitMachine32
// ****************************************************************************
void InitVariable(void)
{
	int i;

	init_tapeCounter();	// テープカウンターの保存先を初期化

#ifdef TARGET_OS_IPHONE
	UseCPUThread=2;
#endif
	refreshAll_flag =1;

	WaitFlag =1;
  	TrapBadOps=1;		// 未定義命令を出力する
  	Verbose=1;
	PatchLevel=1;

#ifdef MITSHM
//  UseSHM=1;
#endif

#ifdef DEBUG
	inTrace = DEBUG_NONE;
	enable_breakpoint = 0;
#endif	

	UPeriod =2;
	UPeriod_bak= UPeriod;
	CasMode =0;			// Casset stop	2003/8/24

	UseSound=1;

	newUseSound= UseSound;
	UseSaveTapeMenu=1;
	
	SaveCPU=1;
	scale=2;
	new_scale = scale;
	win_scale = scale;
	backup_scale = scale;

	IntLac=0;
	P6Version=PC60M2;
	newP6Version= P6Version;
 
	new_use_extram64 = use_extram64 =0;
	
	new_use_extram32 = use_extram32 =0;

	extkanjirom =0;
	new_extkanjirom = extkanjirom;
 
	FastTape=0;

	disk_num=0;
	new_disk_num = disk_num;
	UseDiskLamp=1;
 
	//CasName[0]=0;
	//DskName[0]=0;
	//PrnName[0]=0;

	CPUclock=4;
	//drawwait=192;
	drawwait=175;      // 10000ループのベンチマークでは、このぐらいの速度になる 2008/6/29
	
	//drawwait =192;	// 192 でないと、音楽が安定しない
	
	srline  = 100;
	srline_bak = srline;
	//keyclick= 1;
	TimerSWFlag = 1;

 
 
 /* ********************************************************************** */
 
	port60[0]= 0xf8; 				//I/O[60..67] READ  MEMORY MAPPING
	for(i=1;i<15;i++) port60[i]=0;	//I/O[68-6f]  WRITE MEMORY MAPPING
	//port93;						//I/O[93]     8255 MODE SET / BIT SET & RESET
	//port94;						//I/O[94]	  shadow of I/O[90]
	//portBC;						//I/O[BC]     INTERRUPT ADDRESS of VRTC

	portC1= 0x00;					//I/O[C1]     CRT CONTROLLER MODE
	portC8= 0x00;					//I/O[C8]     CRT CONTROLLER TYPE
	//portCA;						//I/O[CA]     X GEOMETORY low  HARDWARE SCROLL
	//portCB;						//I/O[CB]     X GEOMETORY high HARDWARE SCROLL
	//portCC;						//I/O[CC]     Y GEOMETORY      HARDWARE SCROLL
	//portCE;						//I/O[CE]     LINE SETTING  BITMAP (low) */
	//portCF;						//I/O[CF]     LINE SETTING  BITMAP (High) */
	//portFA;						//I/O[FA]     INTERRUPT CONTROLLER
	//portFB;						//I/O[FB]     INTERRUPT ADDRESS CONTROLLER


	port_c_8255=0;					// 8255 port c

    /* ****************** FLOPPY DISK  ******************** */
	// portDC;						//I/O[DC]     FDC status
	portD1=0;						//I/O[D1]     MINI DISK  (CMD/DATA OUTPUT
	portD2=0x00;					//I/O[D2]     MINI DISK  (CONTROL LINE INPUT)

    /* ************************************************* */

				
	portF0 = 0x11;				//I/O [F0]	MEMORY MAPPING [N60/N66]
	portF3 = 0;					//I/O [F3]  WAIT CONTROLL
	portF6 = 3;					//I/O [F6]  TIMER COUNTUP 
	portF7 = 0x06;				//I/O [F7]  TIMER INT ADDRESS 

	PatchLevel = 1;
	CGSW93 = FALSE;

	p6key = 0;
	stick0 = 0;
	keyGFlag = 0;
	kanaMode = 0;
	katakana = 0;
	kbFlagGraph = 0;
	kbFlagCtrl = 0;
	kbFlagShift = 0;
	TimerSW = 0;
	TimerSW_F3 = 1;		/* 初期値は1とする（PC-6001対応） */

	IntSW_F3 = 1;
	Code16Count = 0;

	CRTMode1=0;			/* 初期値は０とする   2003/10/20 */
	CRTMode2=0;
	CRTMode3=0;

	sound_rate = SOUND_RATE;
	code_log_flag=0;	/* 1: logging  all run code. very heavy! :-<)*/


	fm_vol=17;			// fm volume
	psg_vol=15;			// psg volume


	//strncpy(RomPath , getmodulepath(),PATH_MAX-10);	/* RomPath */
	OSD_GetModulePath( RomPath ,PATH_MAX-10);
	if( RomPath[0] =='/')
		strcat( RomPath , "/rom/");
	else 
		strcat( RomPath , "\\rom\\");

	CSS1=1;
	CSS2=0;
	CSS3=0;

	initAutokey();		// init auto key

	if (debugWorkPath[0] == 0) {
		OSD_GetModulePath(debugWorkPath, PATH_MAX);
	}

}


// ****************************************************************************
//          SetTitle: ウインドウのタイトルバーの変更
// ****************************************************************************
void SetTitle(fpos_t pos)
{
	static char TitleBuf[256];
	char *p= TitleBuf;


	p+= sprintf(TitleBuf, "%s ", Title);
	p+= sprintf(p, "(%s) ", machinetype[ P6Version]);
	if (CasStream[0])
		{
		p += sprintf(p, "  [%ldcnt.] ", pos);
		}
	p+= sprintf(p, "%d fps",Z80_Getfps());
	if (!CPURunning) p += sprintf(p, "  << PAUSE >>");

	OSD_SetWindowTitle( TitleBuf );
}

// ****************************************************************************
//     my_strncpy: strncpy  (add string with terminated)  
// ****************************************************************************
char* my_strncpy(char *dest, char *src, int max)
{
	int i;
	for(i=0; i< max;i++)
	    {
	    *(dest+i) = *(src+i);
	    *(dest+i+1)=0;
	    if( *(src+i)==0) break;
		}
	return( dest);
}


// ****************************************************************************
//  isScroll:    SR screen is scrolling ?
//   Out:  1: scroll   0: normal
// ****************************************************************************
int isScroll(void)
{
	 int x,y;
	  x = portCB *256 + portCA;
	  y = portCC;
	  return(( x !=0 || y !=0) && bitmap && sr_mode);
}

// ****************************************************************************
//          sw_nowait_mode: NO WAITモード/ NORMAL モードの切り替え
//  In: mode TRUE: nowait mode     FALSE: normal mode
// ****************************************************************************
void sw_nowait_mode(int mode)
{
	if( mode)		// no wait and high speed
		{
		CPUclock_bak = CPUclock;
		drawwait_bak = drawwait;
		CPUclock     = 20;
		drawwait     = 10;
		}
	else				// normal speed
		{
		CPUclock     = CPUclock_bak;
		drawwait     = drawwait_bak;
		}
}


// ****************************************************************************
//          sw_srline: 
//  In: mode TRUE: tape moving      FALSE: normal mode
// ****************************************************************************
void sw_srline(int mode)
{
	if( mode)
    	{
         srline     = 0;
        }
    else
    	{
         srline     = srline_bak;
        }

     SetValidLine_sr( drawwait);	// re-calc drawwait (再計算)

     Event_Del(  EVENT_PSG);			// 2009/10/3
	if( !Event_Add( EVENT_PSG, 1000, EV_LOOP|EV_MS ,NULL) )  return;


}

// ****************************************************************************
//          ResetPC:
//  In:  reboot   1:force reboot
//  Out: non zero : successfull
//       zero     : failed
// ****************************************************************************
int ResetPC(int reboot)
{
 //if( !existROM( newP6Version))  // exist rom ?
//	 { return(0);}

	if(P6Version != newP6Version || reboot)
		{			// --- reboot ---
        CPURunning =0;      // stop CPU
        OSD_Delay(1);
		TrashP6();
#ifdef SOUND
		//StopSound();			// add 2002/10/15
		//TrashSound();
#endif
		InitVariable();
		ConfigRead();
		ClearScr();
		if (StartP6())
			CPURunning = 1;          // run CPU
		else
			CPURunning = 0;
#ifdef SOUND
		//ResumeSound();			// add 2002/10/15
		//InitSound();
#endif
		ym2203_Reset();
		}
	else
		{			// --- soft reset ---
		if( CPURunning)		// check CPU running ... 2003/4/26
			{
	        CPURunning =0;      // stop CPU
			InitVariable();		// initialize variable 2003/8/24
#ifdef SOUND
		StopSound();			// add 2002/10/15
#endif
			ConfigRead();

			InitMemmap();		// initialize memory map
			ResetZ80();
	        CPURunning =1;      // run CPU
			ClearScr();
			ym2203_Reset();
			}
		}
 OSD_ClearWindow();
 return(1);
}

// ****************************************************************************
//          putStatusBar: ステータスバーの表示
// ****************************************************************************
void putStatusBar(void)
{
	int  fontsize;
	char tmp[256];
	char str[256];
	int  xx, yy;
	fpos_t pos[3];

	fontsize = 16;

	xx = paddingw;
	//yy= screen->h +paddingh;
	yy = 0;

	OSD_Setcolor(C_BLUESKY);
	sprintf(str, "%-15s  ", CasName[0]);
	if (CasStream[0]!=NULL)
	{
		fgetpos(CasStream[0], &pos[0]);
		sprintf(tmp, "[%05lld/%05lld]", pos[0], CasSize[0]);
	}
	else
	{
		strcpy(tmp, "               ");
	}
	strcat(str, tmp);
	OSD_textout(status_surface, xx, yy, str, fontsize);    // write to screen

#if 0
	sprintf(str, "Save Tape: %-15s  ", CasName[1]);
	if (CasStream[1])
	{
		fgetpos(CasStream[1], &pos[1]);
		sprintf(tmp, "[%05ld]", pos[1]);
	}
	else
		tmp[0] = 0;
	strcat(str, tmp);
	OSD_textout(status_surface, xx, yy + fontsize * 1, str, fontsize);    // write to screen
#endif

	
	char tmp2[2][21];

	my_strncpy(tmp2[0], DskName[0], 20);
	my_strncpy(tmp2[1], DskName[1], 20);
	sprintf(str, "Disk1:%-20s                 2:%-20s", tmp2[0], tmp2[1]);
	OSD_textout(status_surface, xx, yy + fontsize * 1, str, fontsize);    // write to screen
	
	//OSD_Setcolor( 15 );
}

// ****************************************************************************
//          PutDiskAccessLamp: ディスクのアクセスランプ　点灯/消灯
// ****************************************************************************
void PutDiskAccessLamp(int sw)
{
    if( UseDiskLamp)
        {
		int col;
        //int r=0,g=0,b=0;
        int sx,sy,w,h;
    
        sx= screen->w - paddingw-40;

        sy= screen->h + paddingh;
		//sy = paddingh;

		w= 30;
		h= 10;
		
		if(sw ==1) 
			col=9;	// red
		else
			col=0;

		OSD_Setcolor( col);
		OSD_Rectangle( screen ,sx ,sy ,w ,h ,1);
		OSD_Setcolor( 15 );
        }
}

// **********************************************************
//        resize a window
//     窓の大きさを 動的に変更する。
//
// In:  bitmap_scale: エミュレータ画面の大きさ 1 or 2
//      win_scale:    ウインドウの大きさ       1 or 2
// Written by Windy
// **********************************************************
int resizewindow( float _bitmap_scale , float _win_scale , int flags)
{
	OSD_ClearWindow();						// clear window
#ifdef WIN32
//	setMenuCheck( IDM_SCREENSIZE , _bitmap_scale-1);	// menu check /uncheck
	setMenuTitle( WINDOW12);
#endif

#ifdef UNIX
    if( UseCPUThread ) {            // multi thread or timer event ?
        CPURunning =0;              // for thread safe
        sleep(1);
        }
#endif

	OSD_ReleaseSurface( screen);
	if( screen != surface1) {OSD_ReleaseSurface( surface1); surface1 =NULL;}
	if( screen != surface2) {OSD_ReleaseSurface( surface2); surface2 =NULL;}
	if( status_surface )    {OSD_ReleaseSurface( status_surface ); status_surface = NULL;}
	screen = NULL;

    // ********* resize window ***************
	Width = (M6WIDTH  * _win_scale);		// change size
	Height= (M6HEIGHT * _win_scale);
	printf("resizewindow ***** %d   %d  %d \n", Width , Height ,_win_scale);

    screen= OSD_setwindowsurface(Width ,Height ,bitpix ,WINDOW_SOFTWARE | flags);


    // ******** resize bitmap ******************
	Width = M6WIDTH * _bitmap_scale;		// change size
	Height= M6HEIGHT* _bitmap_scale;

    surface1 = OSD_CreateSurface(Width ,Height ,bitpix ,SURFACE_BITMAP);
	surface2 = OSD_CreateSurface(Width ,Height ,bitpix ,SURFACE_OFFSCREEN);
	status_surface = OSD_CreateSurface( STATUSBAR_WIDTH , STATUSBAR_HEIGHT , bitpix , SURFACE_BITMAP);

	//XBuf = surface1->pixels;
    setRefreshSurface( surface1 );      // set Refresh Surface
    
	setwidth( _bitmap_scale-1);			// setting new scale

	new_scale= scale;
	scale= _bitmap_scale;
	win_scale = _win_scale;
	
	if( UseCPUThread ) 
	   CPURunning =1;
	return 1;
}


// ****************************************************************************
//          isSpace: 文字列が全て空白であるか？
// ****************************************************************************
int isSpace(char *str ,unsigned int max)
{
	unsigned int  i;
	int ret;
	ret =1;
	for(i=0; i< strlen(str); i++)
	   	{
	   	if( *(str+i)!=' ') ret= 0;
	    if( i > max ) break;
	    }
	return( ret);
}


// ****************************************************************************
//          test_clear_voice_data: ホルマントデータのファイルを空にする
// Out: 1: success  0: failed
// ****************************************************************************
int test_clear_voice_data(void)
{
	FILE *fp;
	int  ret=0;

	fp = fopen("testvoice.dat","w");	// ファイルを空にする。
	if( fp != NULL)
	  	{
	     fclose( fp);
	     ret=1;
	    }
	return(ret);
}

// ****************************************************************************
//          test_put_voice_data: ホルマントデータをファイルに書き出す
// In:  Value: data to write
// Out: 1: success  0: failed
// ****************************************************************************
int test_put_voice_data(byte Value)
{
	FILE *fp;
	int  ret=0;
  
	fp = fopen("testvoice.dat","ab+");
	if( fp != NULL)
  		{
    	fputc( Value, fp);
    	fclose( fp);
    	ret=1;
    	}
	return(ret);
}



// ****************************************************************************
//          conv_argv: Windowsの lpszArgs から、argc, argv に変換する
// ****************************************************************************
/**  bug fixed: set the first option to argv[1]     2003/5/11 */
void conv_argv( char *lpszArgs , int *argc ,char *argv[])
{
	static char aaa[100][31];
	static char tmp[2000];
	char *p;
	int i;
 
	my_strncpy( tmp , lpszArgs ,1999);
	

	*argc=0;	 /* start argc==0 */
	for(i=0; i <100;i++)
		argv[i]= NULL;

	p =strtok( tmp , " ");
	while(p)		//  sepalate cmdline to argv
	     {
		  argv[*argc]= aaa[*argc];
		  my_strncpy( argv[*argc], p,30);
		  p= strtok(NULL," ");
		  //if( p ==NULL) break;
		  if( ++(*argc) > 99) break;
		 }
}

/* **************************************************** */



// 2002/2         first version --  N66SR-BASIC can  run ...
// 2002/2/20  Implimented CGROM for SR
// 2002/2/23  Implimented Width 40,25 for SR
// 2002/3/16  Implimented fdd (Inteligent)
// 2002/3/21  Implimented fdd (PD765)
// 2002/3/24  Fixed screen 2,2,1 bug for SR
// 2002/3/30  Support option -64 and -68 for PC-6001mk2SR and PC-6601SR
// 2002/4/8   Implimented TV reserver ,Date$ and TIMe$  (PC-6601SR)
// 2002/4/21  Implimented PALET , VRTC Interrupt ,menu's color OK! (PC-6601SR)
// 2002/4/29  Implimented WIDTH 80, and 640x200 dots
// 2002/5/3   Implimented SCREEN 3  (BUGGY)
// 2002/7/7   Support 16kb CGROM and 8kb CGROM, and against double free()
// 2002/7/14  Fixed  WIDTH 80 and SCREEN 3 !   (640x400 dots large window)
// 2002/8/16  Fixed wday on TV reserver , wday is 0:Mon .... 6:Sun
// 2002/9/29  Improved performance of draw routine (mode 6)
// 2002/10/17 Fixed wait timer (WIN32)
// 2002/10/24 Implimented sound (WIN32)
// 2003/11/23 Release 4.0
// 2003/12    Implimented fm sound (WIN32)
// 2003/12/31 Implimented tape faster
