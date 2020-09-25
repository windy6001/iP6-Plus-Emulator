/** iP6: PC-6000/6600 series emualtor ************************/
/**                                                         **/
/**                           P6.h                          **/
/**                                                         **/
/** by Windy 2002-2004                                      **/
/** by ISHIOKA Hiroshi 1998,1999                            **/
/** This code is based on fMSX written by Marat Fayzullin   **/
/*************************************************************/

#pragma warning(disable:4996)
//#define _POSIX_
#include <stdio.h>
#include <limits.h>

#include "Z80.h"            /* Z80 emulation declarations    */
#include "device.h"
//#include "mem.h"

#include "error.h"

/* #define UNIX  */         /* Compile iP6 for for Unix/X    */
/* #define MSDOS */         /* Compile iP6 for MSDOS/VGA     */
/* #define MITSHM */        /* Use MIT SHM extensions for X  */

#ifndef UNIX
#undef MITSHM
#endif

#define NORAM     0xFF      /* Byte to be returned from      */
                            /* non-existing pages and ports  */
extern int refreshAll_flag;



enum {PC60, PC60M2, PC60M2SR , PC66, PC66SR };	/* P6Version define */


/* -------------------------------- */
/* 99.06.02. */
/* size of ximage */
#define M1WIDTH   256		// mode 1
#define M1HEIGHT  192
#define M5WIDTH   320		// mode 5
#define M5HEIGHT  200

#define M6WIDTH   320		// mode 6      add 2003/3/16
#define M6HEIGHT  204

#define STATUSBAR_WIDTH  640  // status bar width
#define STATUSBAR_HEIGHT 50    // status bar height


#define PADDINGW   30  // 15		// PADDING   (ÉrÉbÉgÉ}ÉbÉvÇ∆ÅAÉEÉCÉìÉhÉEÇÃåÑä‘Åj
#define PADDINGH   50  // 40




//#define WIDTH     M5WIDTH*2
//#define HEIGHT    M5HEIGHT*2
//#define SCRWIDTH  WIDTH +PADDINGW	// SCREEN WIDTH
//#define SCRHEIGHT HEIGHT+PADDINGH

#define BORDERW    8        // BORDER    (ÉEÉCÉìÉhÉEògÇ‹Ç≈ÇÃåÑä‘)
#define BORDERH   70 // 56 // 46


/* -------------------------------- */

#define FILE_LOAD_TAPE 0
#define FILE_SAVE_TAPE 1
#define FILE_DISK1 2
#define FILE_DISK2 3
#define FILE_PRNT 4
#define FILE_EXTROM 5
//#define FILE_DISK2 6

#define WINDOW12 100

#define FILE_DOKOSAVE 10
#define FILE_DOKOLOAD 11


#define STICK0_SPACE 0x80
#define STICK0_LEFT  0x20
#define STICK0_RIGHT 0x10
#define STICK0_DOWN  0x08
#define STICK0_UP    0x04
#define STICK0_STOP  0x02
#define STICK0_SHIFT 0x01



void Keyboard(void);
void PSGOut(byte R,byte V);
void OpenFile1(unsigned int num);
int isScroll(void);

/****************************************************************/
/*** existROM(): exist ROM file                               ***/
/*** must call after calling StartP6()                        ***/
/****************************************************************/
int existROM(int version);
int existROMsub(int version, char *dir);


/****************************************************************/
/*** Allocate memory, load ROM images, initialize mapper, VDP ***/
/*** CPU and start the emulation. This function returns 0 in  ***/
/*** the case of failure.                                     ***/
/****************************************************************/
int StartP6(void);

/****************************************************************/
/*** Free memory allocated by StartP6().                     ***/
/****************************************************************/
void TrashP6(void);

/****************************************************************/
/*** Allocate resources needed by the machine-dependent code. ***/
/************************************** TO BE WRITTEN BY USER ***/
int InitMachine(void);

/****************************************************************/
/*** Deallocate all resources taken by InitMachine().         ***/
/************************************** TO BE WRITTEN BY USER ***/
void TrashMachine(void);

/** InitVariable ********************************************/
/* Initialize  Variable                                      */
/*     before calling InitMachine32                          */
/*************************************************************/
void InitVariable(void);

void InitColor(int screen_num);

char * getmodulepath(void);


int  UpdateScreen(void);
void enableCmtInt(void);
void SetTitle(fpos_t pos);

void sw_nowait_mode(int mode);
void sw_srline(int mode);
int ResetPC(int reboot);

int isSpace(char *str ,unsigned int max);
char* my_strncpy(char *dest, char *src, int max);

void PutDiskAccessLamp(int sw);
void putStatusBar(void);

int CpuRun(void);
int resizewindow( float bitmap_scale , float win_scale , int flags);

void keyboard_set_stick( int osdkeycode ,int keydown);
void keyboard_set_key(int osdkeycode,int *_keyGFlag ,int *_p6key);

// -----------------------------------------------------------------------
/******** Variables used to control emulator behavior ********/
extern byte Verbose;                  /* Debug msgs ON/OFF   */
extern int P6Version;            /* 0=60,1=62,2=64,3=66,4=68 */
extern int newP6Version;	/* new P6Version */
/*************************************************************/


#define PRINTOK     if(Verbose) printf("%s",MsgOK)
#define PRINTFAILED if(Verbose) printf("%s",MsgFAILED)

extern char *MsgOK;
extern char *MsgFAILED;


extern int UseCPUThread;       //   0: $BC10l%9%l%C%I$GF0$+$9!#(B 1: CPU $B$rJL%9%l%C%I$GF0$+$9!#(B2: $B%?%$%^!<$GDj4|E*$K8F$S=P$9(B

extern int FastTape;	// use Fast Tape     1: high  0:normal

int test_put_voice_data(byte Value);
int test_clear_voice_data(void);

byte keyboard_get_stick(void);

void conv_argv( char *lpszArgs , int *argc ,char *argv[]);

#if HAVE_GETCWD
#define _getcwd getcwd
#endif


// *************************************************************
//          ÉçÅ[É}éöïœä∑
// *************************************************************
#define HENKAN_SUCCESS  1		// ÉçÅ[É}éöïœä∑ê¨å˜
#define HENKAN_FAILED   0		// ÉçÅ[É}éöïœä∑é∏îs
#define HENKAN_DOING   -1		// ÉçÅ[É}éöïœä∑íÜ
#define HENKAN_CANCEL   2		// ÉçÅ[É}éöïœä∑ÇµÇ»Ç¢Å@& ÉLÉÉÉìÉZÉã

extern int romaji_mode;		// 1:true  0:false
extern int new_romaji_mode;

// *************************************************************
//          debug output
// *************************************************************

#define VOI_LOG  0		// for VOICE
#define DSK_LOG  0		// for DISK
#define DEV_LOG  0		// for DEVICE
#define SND_LOG  0		// for SOUND
#define CPU_LOG  1		// for CPU
#define KEY_LOG  0      // for key
#define MEM_LOG  0      // for memory

#if (VOI_LOG | DSK_LOG | DEV_LOG | SND_LOG | CPU_LOG | KEY_LOG | MEM_LOG)

#define PRINTDEBUG( m, s)                 { if(m) debug_printf(s); }
#define PRINTDEBUG1( m, s, a)             { if(m) debug_printf(s , a); }
#define PRINTDEBUG2( m, s, a,b)           { if(m) debug_printf(s , a ,b); }
#define PRINTDEBUG3( m, s, a,b,c)         { if(m) debug_printf(s , a ,b ,c); }
#define PRINTDEBUG4( m, s, a,b,c,d)       { if(m) debug_printf(s , a ,b ,c ,d); }
#define PRINTDEBUG5( m, s, a,b,c,d,e)     { if(m) debug_printf(s , a ,b ,c, d, e); }
#define PRINTDEBUG6( m, s, a,b,c,d,e,f)   { if(m) debug_printf(s , a ,b ,c ,d ,e ,f); }
#define PRINTDEBUG7( m, s, a,b,c,d,e,f,g) { if(m) debug_printf(s , a ,b ,c ,d ,e ,f ,g); }


#else

#define PRINTDEBUG( m, s)                 
#define PRINTDEBUG1( m, s, a)             
#define PRINTDEBUG2( m, s, a,b)           
#define PRINTDEBUG3( m, s, a,b,c)         
#define PRINTDEBUG4( m, s, a,b,c,d)       
#define PRINTDEBUG5( m, s, a,b,c,d,e)     
#define PRINTDEBUG6( m, s, a,b,c,d,e,f)   
#define PRINTDEBUG7( m, s, a,b,c,d,e,f,g) 

#endif
