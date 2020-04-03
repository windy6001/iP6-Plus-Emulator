/** iP6: PC-6000/6600 series emualtor ************************/
/**                                                         **/
/**                       device.c                          **/
/**                                                         **/
/** by Windy 2002-2004                                      **/
/** by ISHIOKA Hiroshi 1998-2000                            **/
/** This code is based on fMSX written by Marat Fayzullin   **/
/*************************************************************/
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <time.h>
#include <assert.h>

#include "os.h"
#include "types.h"
#include "P6.h"
#include "mem.h"
#include "fdc.h"
#include "disk.h"
#include "d88.h"
#include "Refresh.h"
#include "cgrom.h"
#include "Option.h"
#include "Sound.h"
#include "fm.h"
#include "error.h"
#include "Sound.h"
#include "voice.h"

#include "dokodemo.h"

//#include "device.h"
//#include "mem.h"


//#include "Interrupt.h"  // sub cpu 周りが依存している
#include "Video.h"

void setBreakPointMessage( char *tmp);
byte JoystickGetState(int joy_no);

extern OSD_Surface *surface1;
extern OSD_Surface *surface2;

/* ****************** N66SR BASIC  ******************** add 2002/2 */
byte port40;					//I/O[40]     PALET NO.16
byte port41;					//I/O[41]     PALET NO.15
byte port42;					//I/O[42]     PALET NO.14
byte port43;					//I/O[43]     PALET NO.13
byte port60[16];				//I/O[60..67] READ  MEMORY MAPPING
								//I/O[68-6f]  WRITE MEMORY MAPPING
byte port93;					//I/O[93]     8255 MODE SET / BIT SET & RESET
byte port94;					//I/O[94]	  shadow of I/O[90]

byte portA0;					//I/O[A0]     YM-2203/8910 REGISTER ADDRESS
byte portA1;					//I/O[A1]     YM-2203/8910 DATA
byte portB0;					//I/O[B0]     SYSTEM LATCH

byte portB8;					//I/O[B8]     INTERRUPT ADDRESS of SUB CPU
byte portB9;					//I/O[B9]     INTERRUPT ADDRESS of JOY STICK
byte portBA;					//I/O[BA]     INTERRUPT ADDRESS of Timer
byte portBB;					//I/O[BB]     INTERRUPT ADDRESS of Voice
byte portBC;					//I/O[BC]     INTERRUPT ADDRESS of VRTC
byte portBD;					//I/O[BD]     INTERRUPT ADDRESS of RS-232C
byte portBE;					//I/O[BE]     INTERRUPT ADDRESS of Printer
byte portBF;					//I/O[BF]     INTERRUPT ADDRESS of EXT INT


byte portC0;                    //I/O[C0]     COLOR SET SELECT (CSS)
byte portC1= 0x00;				//I/O[C1]     CRT CONTROLLER MODE
byte portC2;                    //I/O[C2]     VOICE KANJIROM SELECT
byte portC3;                    //I/O[C3]     I/O C2h in/out
byte portC8= 0x00;				//I/O[C8]     CRT CONTROLLER TYPE
byte portC9;					//I/O[C9]     SR VRAM ADDRESS
byte portCA;					//I/O[CA]     X GEOMETORY low  HARDWARE SCROLL
byte portCB;					//I/O[CB]     X GEOMETORY high HARDWARE SCROLL
byte portCC;					//I/O[CC]     Y GEOMETORY      HARDWARE SCROLL

byte portCE;					//I/O[CE]     LINE SETTING  BITMAP (low) */
byte portCF;					//I/O[CF]     LINE SETTING  BITMAP (High) */

byte portDC;					//I/O[DC]     FDC status
byte portD1=0;					//I/O[D1]     MINI DISK  (CMD/DATA OUTPUT
byte portD2=0x00;				//I/O[D2]     MINI DISK  (CONTROL LINE INPUT)


byte portFA;					//I/O[FA]     INTERRUPT CONTROLLER
byte portFB;					//I/O[FB]     INTERRUPT ADDRESS CONTROLLER
byte portFC;					//I/O[FC]     EXT KANJIROM ADDRESS LATCH


byte portF0 = 0x11;				//I/O [F0]	MEMORY MAPPING [N60/N66]
byte portF1 = 0xDD;				//I/O [F1]	MEMORY MAPPING [N60/N66]
byte portF2;                    //I/O [F2]  MEMORY WRITE BLOCK [N60/N66]
byte portF3 = 0;				//I/O [F3]  WAIT CONTROLL
byte portF4;					//I/O [F4]  int1 address set
byte portF5;					//I/O [F5]  int2 address set
byte portF6;					//I/O [F6]  TIMER COUNTUP 
byte portF7 = 0x06;				//I/O [F7]  TIMER INT ADDRESS 
/* 初期値は0x06とする (PC-6001対応) */
byte portF8;					//I/O [F8]  CG ACCESS CONTROL


byte port_c_8255=0;				// 8255 port c
byte pre_port_c_8255 =0;        // 8255 port c (pre)

int  count_port_c_8255=0;       // 8255 port c counter



	// ************* PSG ***************************************

byte PSGReg=0;					// PSG REGISTER LATCH
//byte PSG[16];
byte PSGTMP[ 0xc0];				// PSG & FM REGISTER add 2002/10/15

	// ************* KEYBOARD / JOYSTICK ***********************

byte JoyState[2];
byte p6key = 0;
byte stick0 = 0;
byte keyGFlag = 0;
byte kanaMode = 0;
byte katakana = 0;
byte kbFlagGraph = 0;
byte kbFlagCtrl = 0;


	// ************** TIMER *********************************

byte TimerSW = 0;
byte TimerSW_F3 = 1;		/* 初期値は1とする（PC-6001対応） */

int IntSW_F3 = 1;
int Code16Count = 0;






int  extkanjirom_adr=0;			// EXT KANJIROM addr
/*int  extkanjirom=0;				// EXT KANJIROM 1:enable  0:disable
int  new_extkanjirom=0;

int  extram64     =0;				// EXT RAM   64KB   1:enable  0:disable
int  new_extram64 =0;
*/

/* palet datas 全ての色の配列を一応保有しているだけ */
int palet[ 16]      ={0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15}; // SR PALET  add 2002/4/21 (windy)


	// ****************** FLOPPY DISK  ********************

int disk_type;					// TRUE: PD765A   FALSE: mini disk
int disk_num;					// DRIVE NUMBERS  0:disk non  1-2:disk on line
int new_disk_num;				//                     (next boot time)
int UseDiskLamp;				// 1: use disk lamp  0: no use


int P6Version=PC66SR;			// CURRENT MACHINE TYPE
int newP6Version;				//     (next boot time)

int  PatchLevel = 1;
int CGSW93 = FALSE;


byte Verbose = 1;






	// *************** CPU **********************************

int   CPUclock;					// CPU clock
int   CPUclock_bak;				// CPU clock backup
int   drawwait;					// draw wait
int   drawwait_bak;				// draw wait backup

int	  busreq;						// busreq  1:ON  0:OFF
int   busreq_bak;

extern int CPUclock;
//extern int SaveCPU;

	// *********** SCREEN   *************************

int   scr4col;
int   Console=0;				// use console mode    1: use
//int   keyclick=0;				// use key click sound 1: use

int   UseSaveTapeMenu=0;		// save tape menu      1: on  0: off
int   UseStatusBar=0;           // status bar          1: on  0: off

byte  CSS1,CSS2,CSS3;			// CSS

byte  UPeriod     = 2;           // Interrupts/scr. update 1:60fps  2:30fps
byte  UPeriod_bak;				// UPeriod backup
byte  EndOfFrame=1;              // 1 when end of frame

byte CRTMode1,CRTMode2,CRTMode3;

int   bitmap=0;					// TRUE: BITMAP MODE   FALSE: TEXT MODE
int   cols=40;					// WIDTH COLUME 
int   rows=20;					// WIDTH LINES  

int   lines=200;				// graphics LINES


	// *********** N66SR BAIC  *************************
	// *********** Date Read Interrupt *****************
byte 	DateMode= DATE_NONE;	// Date mode
byte 	DateBuff[ 5];			// Date buffer
int		DateIdx;				// Index of Date buffer


int   sr_mode;					// SR MODE TRUE: sr_mode  FALSE: not sr_mode
int   sr_mode_bak;


int   srline;					// sr line   enable when busreq=1
int   srline_bak;				// sr line   enable when busreq=1


int fm_vol;			// fm volume
int psg_vol;			// psg volume




// *********** TV Reserve Data Read Interrupt ***********
byte TvrMode= TVR_NONE;
byte TvrBuff[ 2];			// data buffer (dummy array    max length: Unknown)
int    TvrIdx;			// Index of data buffer


int cgrom_bank=3;		// CGROMのバンク先　デフォルトは、３　＝0x6000
						// I/O F8h C3: 0x6000  C2: 0x4000

void subcpuOUT(byte Value);
byte subcpuIN(void);



// ****************************************************************************
//			dokodemo_save_device()
// ****************************************************************************
void dokodemo_save_device(void)
{
	char tmp[256];
	int i;
	
	dokodemo_putsection("[DEVICE]");
	for(i=0; i< 16; i++)
		{
		sprintf( tmp, "port60[%d]",i);
		dokodemo_putentry_byte( tmp,port60[i]);
		}
	DOKODEMO_PUTENTRY_BYTE(port93);
	DOKODEMO_PUTENTRY_BYTE(port94);
	DOKODEMO_PUTENTRY_BYTE(portB0);

	DOKODEMO_PUTENTRY_BYTE(portB8);
	DOKODEMO_PUTENTRY_BYTE(portB9);
	DOKODEMO_PUTENTRY_BYTE(portBA);
	DOKODEMO_PUTENTRY_BYTE(portBB);
	DOKODEMO_PUTENTRY_BYTE(portBC);
	DOKODEMO_PUTENTRY_BYTE(portBD);
	DOKODEMO_PUTENTRY_BYTE(portBE);
	DOKODEMO_PUTENTRY_BYTE(portBF);

	DOKODEMO_PUTENTRY_BYTE(portC1);
	DOKODEMO_PUTENTRY_BYTE(portC2);
	DOKODEMO_PUTENTRY_BYTE(portC8);
	DOKODEMO_PUTENTRY_BYTE(portCA);
	DOKODEMO_PUTENTRY_BYTE(portCB);
	DOKODEMO_PUTENTRY_BYTE(portCC);
	DOKODEMO_PUTENTRY_BYTE(portCE);
	DOKODEMO_PUTENTRY_BYTE(portCF);
	DOKODEMO_PUTENTRY_BYTE(portDC);
	DOKODEMO_PUTENTRY_BYTE(portD1);
	DOKODEMO_PUTENTRY_BYTE(portD2);
	
	DOKODEMO_PUTENTRY_BYTE(portFA);
	DOKODEMO_PUTENTRY_BYTE(portFB);
	DOKODEMO_PUTENTRY_BYTE(portFC);

	DOKODEMO_PUTENTRY_BYTE(portF0);
	DOKODEMO_PUTENTRY_BYTE(portF1);
	DOKODEMO_PUTENTRY_BYTE(portF3);
	DOKODEMO_PUTENTRY_BYTE(portF4);
	DOKODEMO_PUTENTRY_BYTE(portF5);
	DOKODEMO_PUTENTRY_BYTE(portF6);
	DOKODEMO_PUTENTRY_BYTE(portF7);
	DOKODEMO_PUTENTRY_BYTE(portF8);

	DOKODEMO_PUTENTRY_BYTE(port_c_8255);
	DOKODEMO_PUTENTRY_BYTE(pre_port_c_8255);
	DOKODEMO_PUTENTRY_INT(count_port_c_8255);

	DOKODEMO_PUTENTRY_BYTE(PSGReg);

	for(i=0; i<0xc0; i++)
		{
		sprintf( tmp, "PSGTMP[%02X]",i);
		dokodemo_putentry_byte(tmp ,PSGTMP[i]);
		}
	DOKODEMO_PUTENTRY_BYTE(JoyState[0]);
	DOKODEMO_PUTENTRY_BYTE(JoyState[1]);
	DOKODEMO_PUTENTRY_BYTE(p6key );
	DOKODEMO_PUTENTRY_BYTE(stick0 );

	DOKODEMO_PUTENTRY_BYTE( keyGFlag );
	
	// ************* KEYBOARD / JOYSTICK ***********************
	DOKODEMO_PUTENTRY_BYTE(kanaMode);
	DOKODEMO_PUTENTRY_BYTE(katakana);
	DOKODEMO_PUTENTRY_BYTE(kbFlagGraph);
	DOKODEMO_PUTENTRY_BYTE(kbFlagCtrl);
	
	
	// ************** TIMER *********************************

	
	DOKODEMO_PUTENTRY_BYTE(TimerSW);
	DOKODEMO_PUTENTRY_BYTE(TimerSW_F3);		/* 初期値は1とする（PC-6001対応） */
	
	DOKODEMO_PUTENTRY_INT(IntSW_F3);
	DOKODEMO_PUTENTRY_INT(Code16Count);	
	

	DOKODEMO_PUTENTRY_INT(extkanjirom_adr);			// EXT KANJIROM addr
	
	
	/* palet datas 全ての色の配列を一応保有しているだけ */
	
	for(i=0; i<16;i++)
		{
		sprintf(tmp,"palet[%2d]",i);
		dokodemo_putentry_int(tmp , palet[i]);
		}		// SR PALET  add 2002/4/21 (windy)
	
	
	// ****************** FLOPPY DISK  ********************
	
	DOKODEMO_PUTENTRY_INT(disk_type);					// TRUE: PD765A   FALSE: mini disk
	DOKODEMO_PUTENTRY_INT(disk_num);					// DRIVE NUMBERS  0:disk non  1-2:disk on line
	DOKODEMO_PUTENTRY_INT(new_disk_num);				//                     (next boot time)
	DOKODEMO_PUTENTRY_INT(UseDiskLamp);				// 1: use disk lamp  0: no use
	
	
	DOKODEMO_PUTENTRY_INT(P6Version);	// CURRENT MACHINE TYPE
	DOKODEMO_PUTENTRY_INT(newP6Version);				//     (next boot time)
	
	DOKODEMO_PUTENTRY_INT(PatchLevel);
	DOKODEMO_PUTENTRY_INT(CGSW93);
	
	
	DOKODEMO_PUTENTRY_BYTE(Verbose);
	
	
	// *************** CPU **********************************
	
	DOKODEMO_PUTENTRY_INT(CPUclock);					// CPU clock
	DOKODEMO_PUTENTRY_INT(CPUclock_bak);				// CPU clock backup
	DOKODEMO_PUTENTRY_INT(drawwait);					// draw wait
	DOKODEMO_PUTENTRY_INT(drawwait_bak);				// draw wait backup
	
	DOKODEMO_PUTENTRY_INT(busreq);						// busreq  1:ON  0:OFF
	DOKODEMO_PUTENTRY_INT(busreq_bak);
	
	// *********** SCREEN   *************************
	
	DOKODEMO_PUTENTRY_INT(scr4col);
	DOKODEMO_PUTENTRY_INT(Console);				// use console mode    1: use
	//int   keyclick=0;				// use key click sound 1: use
	
	DOKODEMO_PUTENTRY_INT(UseSaveTapeMenu);		// save tape menu      1: on  0: off
	DOKODEMO_PUTENTRY_INT(UseStatusBar);           // status bar          1: on  0: off
	
	DOKODEMO_PUTENTRY_BYTE(CSS1);
	DOKODEMO_PUTENTRY_BYTE(CSS2);
	DOKODEMO_PUTENTRY_BYTE(CSS3);			// CSS
	
	DOKODEMO_PUTENTRY_BYTE(UPeriod);           // Interrupts/scr. update 1:60fps  2:30fps
	DOKODEMO_PUTENTRY_BYTE(UPeriod_bak);				// UPeriod backup
	DOKODEMO_PUTENTRY_BYTE(EndOfFrame);              // 1 when end of frame
	
	DOKODEMO_PUTENTRY_BYTE(CRTMode1);
	DOKODEMO_PUTENTRY_BYTE(CRTMode2);
	DOKODEMO_PUTENTRY_BYTE(CRTMode3);
	
	DOKODEMO_PUTENTRY_INT(bitmap);					// TRUE: BITMAP MODE   FALSE: TEXT MODE
	DOKODEMO_PUTENTRY_INT(cols);					// WIDTH COLUME 
	DOKODEMO_PUTENTRY_INT(rows);					// WIDTH LINES  
	
	DOKODEMO_PUTENTRY_INT(lines);				// graphics LINES
	
	
	// *********** N66SR BAIC  *************************
	// *********** Date Read Interrupt *****************
	DOKODEMO_PUTENTRY_BYTE(DateMode);	// Date mode


	dokodemo_putentry_buffer("DateBuff" ,DateBuff , 5);
	
	DOKODEMO_PUTENTRY_INT(DateIdx);				// Index of Date buffer
	
	
	DOKODEMO_PUTENTRY_INT(sr_mode);					// SR MODE TRUE: sr_mode  FALSE: not sr_mode
	DOKODEMO_PUTENTRY_INT(sr_mode_bak);
	
	
	DOKODEMO_PUTENTRY_INT(srline);					// sr line   enable when busreq=1
	DOKODEMO_PUTENTRY_INT(srline_bak);				// sr line   enable when busreq=1
	

	// *********** TV Reserve Data Read Interrupt ***********
	DOKODEMO_PUTENTRY_BYTE(TvrMode);
	DOKODEMO_PUTENTRY_BYTE(TvrBuff[0]);			// data buffer (dummy array    max length: Unknown)
	DOKODEMO_PUTENTRY_BYTE(TvrBuff[1]);
	DOKODEMO_PUTENTRY_INT(TvrIdx);			// Index of data buffer
}


// ****************************************************************************
//		dokodemo_load_device()
// ****************************************************************************



void dokodemo_load_device(void)
{
	char tmp[256];
	int i;

	DOKODEMO_GETENTRY_INT(sr_mode);					// SR MODE TRUE: sr_mode  FALSE: not sr_mode
	DOKODEMO_GETENTRY_INT(sr_mode_bak);

	for(i=0; i< 16; i++)
	{
		sprintf( tmp, "port60[%d]",i);
		dokodemo_getentry_byte( tmp,(byte*)&port60[i]);
		DoOut( 0x60+i, port60[i]);
	}
	DOKODEMO_GETENTRY_BYTE(portF0);
	DoOut( 0xF0,portF0);
	DOKODEMO_GETENTRY_BYTE(portF1);
	DoOut( 0xF1,portF1);
	DOKODEMO_GETENTRY_BYTE(portF3);
	DoOut( 0xF3,portF3);
	DOKODEMO_GETENTRY_BYTE(portF4);
	DoOut( 0xF4,portF4);
	DOKODEMO_GETENTRY_BYTE(portF5);
	DoOut( 0xF5,portF5);
	DOKODEMO_GETENTRY_BYTE(portF6);
	DoOut( 0xF6,portF6);
	DOKODEMO_GETENTRY_BYTE(portF7);
	DoOut( 0xF7,portF7);
	DOKODEMO_GETENTRY_BYTE(portF8);
	DoOut( 0xF8,portF8);
	
	DOKODEMO_GETENTRY_BYTE(port93);
	DoOut( 0x93,port93);
	DOKODEMO_GETENTRY_BYTE(port94);
	DoOut( 0x94,port94);
//	DOKODEMO_GETENTRY_BYTE(portB0);
//	DoOut( 0xB0,portB0);

	DOKODEMO_GETENTRY_BYTE(portB8);
	DoOut( 0xB8,portB8);
	DOKODEMO_GETENTRY_BYTE(portB9);
	DoOut( 0xB9,portB9);
	DOKODEMO_GETENTRY_BYTE(portBA);
	DoOut( 0xBA,portBA);
	DOKODEMO_GETENTRY_BYTE(portBB);
	DoOut( 0xBB,portBB);
	DOKODEMO_GETENTRY_BYTE(portBC);
	DoOut( 0xBC,portBC);
	DOKODEMO_GETENTRY_BYTE(portBD);
	DoOut( 0xBD,portBD);
	DOKODEMO_GETENTRY_BYTE(portBE);
	DoOut( 0xBE,portBE);
	DOKODEMO_GETENTRY_BYTE(portBF);
	DoOut( 0xBF,portBF);

	DOKODEMO_GETENTRY_BYTE(portC1);
	DoOut( 0xC1,portC1);
//	DOKODEMO_GETENTRY_BYTE(portC2);
//	DoOut( 0xC2,portC2);
	DOKODEMO_GETENTRY_BYTE(portC8);
	DoOut( 0xC8,portC8);
//	DOKODEMO_GETENTRY_INT(sr_mode);					// SR MODE TRUE: sr_mode  FALSE: not sr_mode
//	DOKODEMO_GETENTRY_INT(sr_mode_bak);

	DOKODEMO_GETENTRY_BYTE(portCA);
	DoOut( 0xCA,portCA);
	DOKODEMO_GETENTRY_BYTE(portCB);
	DoOut( 0xCB,portCB);
	DOKODEMO_GETENTRY_BYTE(portCC);
	DoOut( 0xCC,portCC);
	DOKODEMO_GETENTRY_BYTE(portCE);
	DoOut( 0xCE,portCE);
	DOKODEMO_GETENTRY_BYTE(portCF);
	DoOut( 0xCF,portCF);
	DOKODEMO_GETENTRY_BYTE(portDC);
//	DoOut( 0xDC,portDC);
	DOKODEMO_GETENTRY_BYTE(portD1);
	DoOut( 0xD1,portD1);
	DOKODEMO_GETENTRY_BYTE(portD2);
	DoOut( 0xD2,portD2);
	
	DOKODEMO_GETENTRY_BYTE(portFA);
	DoOut( 0xFA,portFA);
	DOKODEMO_GETENTRY_BYTE(portFB);
	DoOut( 0xFB,portFB);
	DOKODEMO_GETENTRY_BYTE(portFC);
	DoOut( 0xFC,portFC);
	
	
	
	
	DOKODEMO_GETENTRY_BYTE(port_c_8255);
	DOKODEMO_GETENTRY_BYTE(pre_port_c_8255);
	DOKODEMO_GETENTRY_INT(count_port_c_8255);
	
	DOKODEMO_GETENTRY_BYTE(PSGReg);
	
	for(i=0; i<0xc0; i++)
	{
		sprintf( tmp, "PSGTMP[%02X]",i);
		dokodemo_getentry_byte(tmp ,&PSGTMP[i]);
		PSGOut(i , PSGTMP[i]);		
	}
	DOKODEMO_GETENTRY_BYTE(JoyState[0]);
	DOKODEMO_GETENTRY_BYTE(JoyState[1]);
	DOKODEMO_GETENTRY_BYTE(p6key );
	DOKODEMO_GETENTRY_BYTE(stick0 );
	
	DOKODEMO_GETENTRY_BYTE( keyGFlag );
	
	// ************* KEYBOARD / JOYSTICK ***********************
	DOKODEMO_GETENTRY_BYTE(kanaMode);
	DOKODEMO_GETENTRY_BYTE(katakana);
	DOKODEMO_GETENTRY_BYTE(kbFlagGraph);
	DOKODEMO_GETENTRY_BYTE(kbFlagCtrl);
	
	// ************** TIMER *********************************
	
	DOKODEMO_GETENTRY_BYTE(TimerSW);
	DOKODEMO_GETENTRY_BYTE(TimerSW_F3);		/* 初期値は1とする（PC-6001対応） */
	
	DOKODEMO_GETENTRY_INT(IntSW_F3);
	DOKODEMO_GETENTRY_INT(Code16Count);	
	
	
	DOKODEMO_GETENTRY_INT(extkanjirom_adr);			// EXT KANJIROM addr
	
	/* palet datas 全ての色の配列を一応保有しているだけ */
	
	for(i=0; i<16;i++)
	{
		sprintf(tmp,"palet[%2d]",i);
		dokodemo_getentry_int(tmp , &palet[i]);
	}		// SR PALET  add 2002/4/21 (windy)
	
	
	// ****************** FLOPPY DISK  ********************
	
	DOKODEMO_GETENTRY_INT(disk_type);					// TRUE: PD765A   FALSE: mini disk
	DOKODEMO_GETENTRY_INT(disk_num);					// DRIVE NUMBERS  0:disk non  1-2:disk on line
	DOKODEMO_GETENTRY_INT(new_disk_num);				//                     (next boot time)
	DOKODEMO_GETENTRY_INT(UseDiskLamp);				// 1: use disk lamp  0: no use
	
	
	DOKODEMO_GETENTRY_INT(P6Version);	// CURRENT MACHINE TYPE
	DOKODEMO_GETENTRY_INT(newP6Version);				//     (next boot time)
	
	DOKODEMO_GETENTRY_INT(PatchLevel);
	DOKODEMO_GETENTRY_INT(CGSW93);
	
	
	DOKODEMO_GETENTRY_BYTE(Verbose);
	
	
	// *************** CPU **********************************
	
	DOKODEMO_GETENTRY_INT(CPUclock);					// CPU clock
	DOKODEMO_GETENTRY_INT(CPUclock_bak);				// CPU clock backup
	DOKODEMO_GETENTRY_INT(drawwait);					// draw wait
	DOKODEMO_GETENTRY_INT(drawwait_bak);				// draw wait backup
	
	DOKODEMO_GETENTRY_INT(busreq);						// busreq  1:ON  0:OFF
	DOKODEMO_GETENTRY_INT(busreq_bak);
	
	// *********** SCREEN   *************************
	
	DOKODEMO_GETENTRY_INT(scr4col);
	DOKODEMO_GETENTRY_INT(Console);				// use console mode    1: use
	//int   keyclick=0;				// use key click sound 1: use
	
	DOKODEMO_GETENTRY_INT(UseSaveTapeMenu);		// save tape menu      1: on  0: off
	DOKODEMO_GETENTRY_INT(UseStatusBar);           // status bar          1: on  0: off
	
	DOKODEMO_GETENTRY_BYTE(CSS1);
	DOKODEMO_GETENTRY_BYTE(CSS2);
	DOKODEMO_GETENTRY_BYTE(CSS3);			// CSS
	
	DOKODEMO_GETENTRY_BYTE(UPeriod);           // Interrupts/scr. update 1:60fps  2:30fps
	DOKODEMO_GETENTRY_BYTE(UPeriod_bak);				// UPeriod backup
	DOKODEMO_GETENTRY_BYTE(EndOfFrame);              // 1 when end of frame
	
	DOKODEMO_GETENTRY_BYTE(CRTMode1);
	DOKODEMO_GETENTRY_BYTE(CRTMode2);
	DOKODEMO_PUTENTRY_BYTE(CRTMode3);
	
	DOKODEMO_GETENTRY_INT(bitmap);					// TRUE: BITMAP MODE   FALSE: TEXT MODE
	DOKODEMO_GETENTRY_INT(cols);					// WIDTH COLUME 
	DOKODEMO_GETENTRY_INT(rows);					// WIDTH LINES  
	
	DOKODEMO_GETENTRY_INT(lines);				// graphics LINES
	
	
	// *********** N66SR BAIC  *************************
	// *********** Date Read Interrupt *****************
	DOKODEMO_GETENTRY_BYTE(DateMode);	// Date mode
	
	dokodemo_getentry_buffer("DateBuff" ,DateBuff , 5);

	DOKODEMO_GETENTRY_INT(DateIdx);				// Index of Date buffer
	
	
	
	
	DOKODEMO_GETENTRY_INT(srline);					// sr line   enable when busreq=1
	DOKODEMO_GETENTRY_INT(srline_bak);				// sr line   enable when busreq=1
	
	
	// *********** TV Reserve Data Read Interrupt ***********
	DOKODEMO_GETENTRY_BYTE(TvrMode);
	DOKODEMO_GETENTRY_BYTE(TvrBuff[0]);			// data buffer (dummy array    max length: Unknown)
	DOKODEMO_GETENTRY_BYTE(TvrBuff[1]);
	DOKODEMO_GETENTRY_INT(TvrIdx);			// Index of data buffer

	DOKODEMO_GETENTRY_BYTE(portB0);
	DoOut( 0xB0,portB0);

}



/****************************************************************/
/*** Send a character to the printer.                         ***/
/****************************************************************/
void Printer(byte V) { fputc(V,PrnStream); }



// *****************************************************************
//    PC-6601SR のDATE$ TIME$   タイマーに設定されるデータ構造
//
//    5バイトのデータに下記を格納
//
//    +00   +01 +02 +03 +04
//   月/曜   日　時　分　秒
//
//   BCDフォーマットで保存される 
// 　先頭だけ 上の位が月をあらわし、下位は、曜日をあらわす。
//   曜日は、0:月曜　1:火曜　2:水曜　3:木曜　4:金曜　5:土曜　6:日曜  (Ｃ言語と並びが違う）
//
//   例） 56 17  13 14 05 の場合　　→ 5月、日曜日、17日、13時　14分、05秒のこと
//
//
// *****************************************************************


struct tm *tm_now= NULL;		// エミュレータ内の現在日時


// ****************************************************************************
//          savetime: 現在日時を保存
// ****************************************************************************
void saveDateTime(void)
{
	time_t  t;
	time(&t);
	tm_now = localtime(&t);

}

// ****************************************************************************
//           updatetime: 日時を 1秒進める
// ****************************************************************************
void updateDateTime(void)
{
	if (tm_now == NULL) return;

	tm_now->tm_sec ++;
	if (tm_now->tm_sec >= 60)
		{
		tm_now->tm_sec = 0;
		tm_now->tm_min++;
		if (tm_now->tm_min >= 60)
			{
			tm_now->tm_min = 0;
			tm_now->tm_hour++;
			if (tm_now->tm_hour >= 24)
				{
				tm_now->tm_hour = 0;
				}
			}
		}
}



// ****************************************************************************
//          fmtdate: 現在日時を PC-6601SR のタイマー用の文字列にセットして返却
// ****************************************************************************
byte *fmtdate(void)
{
	char  tmpbuff[100 + 1];
	static byte outbuff[5 + 1];
	int i;
	int wday;

	time_t  t;
	//struct tm *tms;
	//time(&t);
	//tms = localtime(&t);

	// fixed wday (PC-6601SR's wday is 0:Mon 1:Tue ... 6:Sun), add windy 2002/8/16
	wday = (tm_now->tm_wday >0) ? tm_now->tm_wday - 1 : 6;
	outbuff[0] = ((tm_now->tm_mon) + 1) << 4 | (wday);

	sprintf(tmpbuff, "%02d%02d%02d%02d", tm_now->tm_mday, tm_now->tm_hour, tm_now->tm_min, tm_now->tm_sec);
	for (i = 0; i<4; i++)
	{
		outbuff[1 + i] = (tmpbuff[i * 2] - '0') << 4 | tmpbuff[i * 2 + 1] - '0';
	}
	return (outbuff);
}


// ****************************************************************************
//          setfmtdate: PC-6601SR のタイマー用の文字列から、日時を設定
// ****************************************************************************
setfmtdate( char *inbuff)
{
	int i;
	char tmpbuff[100];
	int wday;

	for (i = 0; i < 5; i++)
	{
		tmpbuff[i * 2] = (inbuff[i] >> 4) & 0xf;
		tmpbuff[i * 2 + 1] = inbuff[i] & 0xf;
	}
	tm_now->tm_mon = tmpbuff[0]-1;					// 月
	wday = tmpbuff[1];								// 曜日
	tm_now->tm_wday = (wday > 0) ? wday - 1 : 6;	// 曜日
	tm_now->tm_mday = tmpbuff[2] * 10 + tmpbuff[3];	// 日
	tm_now->tm_hour = tmpbuff[4] * 10 + tmpbuff[5];	// 時
	tm_now->tm_min  = tmpbuff[6] * 10 + tmpbuff[7];	// 分
	tm_now->tm_sec  = tmpbuff[8] * 10 + tmpbuff[9];	// 秒


}


/****************************************************************/
/*** Write number into given IO port.                         ***/
/****************************************************************/
void DoOut(register byte Port,register byte Value)
{
   int i;
  if (Verbose&0x02) printf("--- DoOut : %02X, %02X ---\n", Port, Value);

  if( enable_breakpoint)
	for(i=1; i< MAX_BREAKPOINT ; i++)
		if(Port==breakpoint[i].addr && breakpoint[i].action == B_OUT  &&  breakpoint[i].enable ==1)
			{
			char tmp[256];
			sprintf( tmp ," *** BREAK at PC:%04X #%d (OUT 0x%02X<---DATA 0x%02X) ***\n",getPC(), i , Port , Value);
			setBreakPointMessage( tmp);
			Trace=1; 
			 break; 
			}


  switch(Port) {
		// ----------- voice -------------
   case 0xE0: //voice_Out( Value);
  			//test_put_voice_data(Value);	/* output voice data */
			OutE0H( Value);
  			break;
   case 0xE2: //voice_SetMode( Value);
			OutE2H( Value);
  			break;
   case 0xE3: //voice_SetCommand( Value);
			OutE3H( Value);
  			break;

  
		// ------------- PALET ----------------
   case 0x40:
   case 0x41:
   case 0x42:
   case 0x43: 
		  if( P6Version == PC60M2SR  || P6Version == PC66SR )
		    {
			int reg,val;
			reg= 15-(Port-0x40);
			val= 15-Value;
			
			if( palet[reg] !=val)
				{
				PRINTDEBUG2(DEV_LOG ,"[P6.c][DoOut] palet port=%x   Value=%x \n",Port,Value);
				}
			palet[ reg]= val;	// 一応、保持しておく
			do_palet( reg,val);	// fast palet  2002/7/29 

			switch( Port)
				{
				case 0x40: port40= Value;break;
				case 0x41: port41= Value;break;
				case 0x42: port42= Value;break;
				case 0x43: port43= Value;break;
				}
			}
			break;	

		// --------- SR  READ MEMORY MAPPING -------------- add 2002/2
   case 0x60:
   case 0x61:
   case 0x62:
   case 0x63:
   case 0x64:
   case 0x65:
   case 0x66:
   case 0x67:
			memorymap_mode6_readblock( Port , Value);
            return;

		// --------- SR WRITE  MEMORY MAPPING -------------- add 2002/2
   case 0x68:
   case 0x69:
   case 0x6a:
   case 0x6b:
   case 0x6c:
   case 0x6d:
   case 0x6e:
   case 0x6f:
   /* printf("write map [%X]=%X  EnWrite=%X\n",Port,Value,((Port&0xe)-8)/2); */
			memorymap_mode6_writeblock(Port ,Value);
            return;
           
		// --------- 8251 (RS-232C) --------------
   case 0x80: return;
   case 0x81: return;                   /* 8251 status          */

		// --------- 8255 PORT A   (SUB CPU) --------------
   case 0x90:                           /* subCPU               */
			subcpuOUT(Value);
			return;

		// --------- 8255 PORT B   (SUB CPU) --------------
   case 0x91: 
			Printer(~Value); return;  /* printer data         */

		// --------- 8255 PORT C   (SUB CPU) --------------
   case 0x92: return;                   /* printer,CRT,CG,sub   */

		// --------- MODE SET / BIT SET&RESET (PORT C)   --------------
   case 0x93:
            pre_port_c_8255 = port_c_8255;      // store previous value
            
    		port93 = Value;
    		if( Value &1)
    				port_c_8255 |=   1<<((Value>>1)&0x07);
			else
    				port_c_8255 &= ~(1<<((Value>>1)&0x07));

			switch(Value) {
		    	 /*
		    	case 0x02: EndOfFrame=0; break;
		    	case 0x03: EndOfFrame=1; break;
		    	*/
	        	case 0x04: CGSW93 = TRUE; RdMem[cgrom_bank /*3*/]=CGROM; break;
	        	case 0x05: CGSW93 = FALSE; if(!sr_mode){ DoOut(0xF0,portF0);}
	        	case 0x08: port_c_8255 |= 0x88; break;	// 送信許可
	         	case 0x09: port_c_8255 &= 0xf7; break;	// 送信準備完了
	         	case 0x0c: port_c_8255 |= 0x28; break;	// 受信許可申請
	         	case 0x0d: port_c_8255 &= 0xf7; break;	// 受信準備完了
				}
			port_c_8255 |= 0xa8;
		    return;

		// --------- SOUND REGISTER ADDRESS LATCH   --------------
   case 0x70:							/* FM SOUND CARTRIDGE */
   case 0xA0:
			portA0 = Value;
			PSGReg=Value;			/* YM-2203 reg addr */
			return; 

		// --------- SOUND WRITE DATA               --------------
   case 0x71:							/* FM SOUND CARTRIDGE */
   case 0xA1: 
			portA1 = Value;
			PSGTMP[PSGReg]=Value;        /* 8910 /YM-2203 data            */
			PSGOut(PSGReg,Value);
	        return;

   case 0xA3: return;                   /* 8910 inactive ?      */

		// --------- SYSTEM LATCH              --------------
   case 0xB0:
			portB0 = Value;
            if( !sr_mode)    /*  not VRAM address in the SR MODE  add 2002/2*/
                {
                 VRAM=RAM+VRAMHead[CRTMode1][(Value&0x06)>>1];
                }
            TimerSW=(Value&0x01)?0:1;
			PRINTDEBUG3(DEV_LOG ,"[P6.c][DoOut] port B0  Value=%02X  PC=%04X  TimerSW=%d  \n",Value ,getPC(),TimerSW );
            return;

		// --------- INTERRUPT ADDRESS  (B8 - BF) ----------
   case 0xB8:
            portB8= Value;  		// SUB CPU Interrupt address 
            PRINTDEBUG1(DEV_LOG ,"[P6.c][DoOut] Port B8  Value=%2X \n",Value); 
            return; 
   case 0xB9:
            portB9= Value;  		// JOY STICK Interrupt address 
            PRINTDEBUG1(DEV_LOG ,"[P6.c][DoOut] Port B9  Value=%2X \n",Value); 
            return; 
   case 0xBA:
            portBA= Value;  		// Timer Interrupt address 
            PRINTDEBUG1(DEV_LOG ,"[P6.c][DoOut] Port BA  Value=%2X \n",Value); 
            return; 
   case 0xBB:
            portBB= Value;  		// Voice Interrupt address 
            PRINTDEBUG1(DEV_LOG ,"[P6.c][DoOut] Port BB  Value=%2X \n",Value); 
            return; 
   case 0xBC:
            portBC= Value;  		// VRTC Interrupt address 2002/4/21
            PRINTDEBUG1(DEV_LOG ,"[P6.c][DoOut] Port BC  Value=%2X \n",Value); 
            return; 
   case 0xBD:
            portBD= Value;  		// RS-232C Interrupt address 
            PRINTDEBUG1(DEV_LOG ,"[P6.c][DoOut] Port BD  Value=%2X \n",Value); 
            return; 
   case 0xBE:
            portBE= Value;  		// Printer Interrupt address 
            PRINTDEBUG1(DEV_LOG ,"[P6.c][DoOut] Port BE  Value=%2X \n",Value); 
            return; 
   case 0xBF:
            portBF= Value;  		// EXT INT Interrupt address 
            PRINTDEBUG1(DEV_LOG ,"[P6.c][DoOut] Port BF  Value=%2X \n",Value); 
            return; 

		// --------- Color Set Select        --------------
   case 0xC0:                           /* CSS                  */
			portC0= Value;
			CSS3=(Value&0x04)<<2;CSS2=(Value&0x02)<<2;CSS1=(Value&0x01)<<2;
            return;
		// --------- CRT CONTROLLER MODE     --------------
   case 0xC1:
   			refreshAll_flag=1;
			if( Value != portC1)
               {
			   if(sr_mode)
					{PRINTDEBUG5(DEV_LOG ,"[P6.c][DoOut] Port C1  Value=%2X  PC=%04X cols=%4d ,scrnmode=%d  ,g_width=%d \n"
					,Value ,getPC() ,(Value&2)?40:80  ,(Value&4)?1:2 ,(Value&8)?320:640 );}
			   else
					{PRINTDEBUG5(DEV_LOG ,"[P6.c][DoOut] Port C1  Value=%2X  PC=%04X  cols=%4d ,scrnmode=%s  ,color=%d  \n"
					,Value ,getPC() ,(Value&2)?32:40  ,(Value&4)?"character":"graphics" ,(Value&8)?16:4 ); }
					
			   }
            portC1= Value;

            if( sr_mode_bak   != sr_mode)  ClearScr();// 2002/9/3  くり抜き対策
            if((CRTMode1)&&(Value&0x02)) ClearScr();
            
            CRTMode1=(Value&0x02) ? 0 : 1;
            CRTMode2=(Value&0x04) ? 0 : 1;
            CRTMode3=(Value&0x08) ? 0 : 1;
			
			if( sr_mode)	// lines    (sr mode only)	2006/11/4
	            {
				 if( Value & 0x04)
					lines=(Value&0x01) ? 200 : 204; // graphics mode
				 else
					lines= 200;						// character mode
				}
			
            if( !sr_mode)	// use CGROM 
                CGROM = ((CRTMode1 == 0) ? CGROM1 : CGROM5);
            else
                CGROM = CGROM6;    // N66SR BASIC use CGROM6  add 2002/2/20

			if( sr_mode && CRTMode3 && CRTMode2 && bitmap) // N66SR BASIC & screen 3
				{
				if( scale==1)  resizewindow( 2.0,2.0,0);	// scale 1 -> resize scale 2
				}

		// ********* select  width 80  / width 40    2002/7/14 ***********
 		  if( sr_mode)	
		    {
		     if(CRTMode1==1      && CRTMode2==0 && !bitmap) /* width 80 */
		 	 	{
				 cols=80;
				 if( scale==1) resizewindow(2.0,2.0,0); // scale 1 -> resize scale 2 2003/4/27
				}
 		     else if(CRTMode1==0 && CRTMode2==0 && !bitmap) /* Width 40 */
			 	{
				cols=40;
				}
			 }
			
		  /* キャラクター文字 が切り替わった時も、PortB0 により、VRAMアドレスは変更される */
			if( !sr_mode)    /*  not VRAM address in the SR MODE  add 2002/2*/
                {
                 VRAM=RAM+VRAMHead[CRTMode1][(portB0 &0x06)>>1];
                }

		    sr_mode_bak  = sr_mode;	// add windy 2002/9/3 くり抜き対策
            return;

		// --------- ROM SELECT     --------------
   case 0xC2:
			portC2 = Value;
			memorymap_voice_kanjirom(Port, Value);
			PRINTDEBUG2(DEV_LOG ,"[P6.c][DoOut] port C2  Value=%02X  PC=%04X  \n", Value ,getPC());

            return;

		// --------- I/O C2H IN/OUT     --------------
		// FFH: OUT   00H: IN
   case 0xC3: portC3 = Value; 
			return;                   /* C2H in/out switch    */

		// --------- CRT CONTROLLER TYPE --------------
   case 0xc8:
			if( P6Version != PC66SR && P6Version != PC60M2SR ) return;
   			refreshAll_flag=1;

			//printf("c8==%02X\n",Value);
		   if( Value != portC8)
		   	{
		     PRINTDEBUG3(DEV_LOG, "[P6.c][DoOut] Port 0xc8  vram=%4X ,bitmap=%d    , rows=%d ",(Value & 0x10)?0x8000:0x0,  (Value &8)? 0:1, (Value&4)?20:25);
		     PRINTDEBUG2(DEV_LOG, ",bus_req=%d, sr_mode=%d\n",(Value & 0x2)?0:1,((Value&1)==1)?0:1);
		   	}
		   busreq_bak = busreq;
		   
		   portC8  = Value;        /* crt contoller type  SR BASIC add 2002/2 */
		   bitmap  = (Value & 8)? 0:1;
		   rows    = (Value & 4)? 20:25;
		   busreq  = (Value & 2)? 0:1;
		   sr_mode = ((Value & 1)==1) ? 0 : 1;                 /* sr_mode */

#if 1
		   if( busreq_bak != busreq)	// busreq change ?
		   	{
	 		SetValidLine_sr( drawwait);	// re-calc drawwait (再計算)
			SetClock(CPUclock*1000000);
			}
#endif

		   if( bitmap && sr_mode)	/* VRAMがビットマップモードになっているか？　かつ、SRモードか？*/
          	{                 /* VRAM address (bitmap) VRAM自体をどのメモリーを使うか選択 */
			 //VRAM_BITMAP =(Value & 0x10) ? RAM+0x8000:RAM+0x0000; /* リセットがかかるので、実装しない方がいい? */
		    }

    /* 1)sr_mode になったら MODE 5から6に移行するとき、CGROMが前のままなのに
        RefreshScr61 を呼ばれるといけないので CGROMを初期化しておく    2003/11/9
       2)sr_mode になったら MODE 5から6に戻って、6から5になるときもあるので
         portF0を初期化しておく。
       */
           if(sr_mode)
            {
             CGROM=CGROM6; 
             portF0=0x11;
            }
		 return;
		
		// --------- TEXT VRAM ADDRESS -------------- add 2002/2
  case 0xC9:
		portC9 = Value;
	      //if( sr_mode && !bitmap )
		  if( sr_mode ) 
			{
			  if( !CRTMode2)    // TEXT mode か？
	            {		
	            VRAM=RAM+(Value & 0xf)*0x1000;
	            //printf("VRAM= %4X  \n",  (Value & 0xf)*0x1000);
	           }
		     else				// Graphics mode 
			   {
			    VRAM=((Value & 0x8)==0x8) ? RAM+0x8000 : RAM+0x0000;
		       }
		    }
	       return;
	       
		// --------- HARDWARE SCROLL  -------------- add 2002/2
  case 0xCA:
  case 0xCB:
  case 0xCC:
             if( P6Version == PC60M2SR  || P6Version == PC66SR )
	  		   {
	  		    switch( Port)
	  		       {
	  		       case 0xCA: portCA= Value; break;
	  		       case 0xCB: portCB= Value; break;
	  		       case 0xCC: portCC= Value; break;
	  		       }	  		       
	  		   }
	  		   return;

	// --------- BITMAP ACCESS LINE  -------------- add 2002/2
  case 0xCE: if( P6Version ==PC60M2SR  || P6Version == PC66SR )
				portCE=Value; /* Graphics Y zahyou SR-BASIC add 2002/2 */
  			 return;
  case 0xCF: if( P6Version ==PC60M2SR  || P6Version == PC66SR ) 
  					{
				    //portCF=Value; 	// port CF 未使用にしました。2003/10/22
					portCF=0;
                    }
			 return;

	// *********   DISK DRIVE  ***************  add 2002/2
	/* ---------------------------------
	mini floppy disk unit:
		D0h: data input
		D1h: data output
		D2h: control input
		D3h: control output

	FDC:
		D0h: FDC buffer
		D1h: FDC buffer
		D2h: FDC buffer
		D3h: FDC buffer
          --------------------------------*/
    	
  case 0xD0:
  case 0xD2: if(disk_type) {fdc_push_buffer( Port,Value);}/* FDC fd buffer */
                       return;
  case 0xD1:
  case 0xD3: if(disk_type)
	             {fdc_push_buffer( Port,Value);}
	       else
	             {disk_out( Port, Value);} /* disk command,data 2002/3/13*/
	       return;

  case 0xDA: fdc_outDA( Value);		/* FDC read/write sectors */
	       return;
  case 0xDD: fdc_outDD( Value);		/* FDC command */
           return;


  
	/* ------------------------------------------
		N60m/N66 BASIC  のメモリーマッピング
       ------------------------------------------ */

  case 0xF0:                         		/* read block set       */
		memorymap_mode5_readblock0( Port , Value);

		//PRINTDEBUG2(DEV_LOG ,"[P6.c][DoOut] port F0  Value=%02X  PC=%04X  \n", Value ,getPC());

        return;
  case 0xF1:                           /* read block set       */
		memorymap_mode5_readblock1( Port , Value);

		//PRINTDEBUG2(DEV_LOG ,"[P6.c][DoOut] port F1  Value=%02X  PC=%04X  \n", Value ,getPC());
          return;
   case 0xF2:                           /* write ram block set  */
		portF2 = Value;
	   memorymap_mode5_writeblock( Port , Value);

		//PRINTDEBUG2(DEV_LOG ,"[P6.c][DoOut] port F2  Value=%02X  PC=%04X  \n", Value ,getPC());
		return;

   case 0x7F:
   		memorymap_mode1_extmegarom( Port , Value);         /* bank change for MEGAROM */
   		return;

  case 0xF3:                           /* wait,int control     */
	    portF3 = Value;
	    TimerSW_F3=(Value&0x04)?0:1;
        IntSW_F3=(Value&0x01)?0:1;
		PRINTDEBUG4(DEV_LOG ,"[P6.c][DoOut] port F3  Value=%02X  PC=%04X  TimerSW_F3=%d   IntSW_F3=%d\n", Value ,getPC(), TimerSW_F3 ,IntSW_F3);
         return;
  case 0xF4:portF4 = Value;
			return;                   /* int1 addr set        */
  case 0xF5:portF5 = Value;
			return;                   /* int2 addr set        */
  case 0xF6:                           /* timer countup value  */
           portF6 = Value;
           SetTimerIntClock(Value);
           return;
  case 0xF7:                           /* timer int addr set    */
           portF7 = Value;
           return;
  case 0xF8:							/* CG access control    */
		   portF8 = Value;
		   if( 0xc0 <= Value  && Value <= 0xc8)			/* CGROM のバンク先を変更できる */
				cgrom_bank = Value - 0xc0;				/* c3: bank3 = 0x6000  c2: bank2 = 0x4000 */
		   return;                   

		/* ---------------------------------  (SR)
		FAh: interrupt control 
		    7    6      5     4     3     2       1      0
		    EXT  PRINT  232C  VRTC VOICE  TIMER  STICK  SUB CPU
		    
		    1:DI   0:EI
		 おそらく、B8H-BFH で指定するアドレスと　関係がありそうです。
		      --------------------------------*/
  case 0xFA:
  		  if( P6Version ==PC60M2SR  || P6Version == PC66SR )
		    {
	       	portFA= Value;   	// Interrupt DI/EI controll 2002/4/21
	       	PRINTDEBUG1(DEV_LOG, "[P6.c][DoOut] Port 0xFA  Value=%02X\n",portFA);
	       	}
	       return; 

		/* ---------------------------------   (SR)
		FBh: interrupt vector address control
		    7    6      5     4     3     2       1      0
		    EXT  PRINT  232C  VRTC VOICE  TIMER  STICK  SUB CPU
		    
		    1:Enable  0: Disable
		 おそらく、B8H-BFH で指定するアドレスと　関係があると思われる。
	      --------------------------------*/
  case 0xFB:
  		  if( P6Version ==PC60M2SR  || P6Version == PC66SR )
		    {
	       	portFB= Value; 		// Interrupt address controll 2002/4/21
	       	PRINTDEBUG1(DEV_LOG ,"[P6.c][DoOut] Port 0xFB  Value=%02X\n",portFB);
	       	}
	       return;   

	/* ---------------------------------   (SR)
		拡張漢字ROM　アドレスラッチ

        このポートは、拡張漢字ROMをさしたときのみ、有効になります。
		FCh に上位８ビットを出力すると、Bレジスタをどうにかして読み出して、
        下位アドレスとする様です。
	   --------------------------------*/
  case 0xFC:
			 portFC = Value;
			 extkanjirom_adr= Value*256 + getB();

			//printf("%X \n",extkanjirom_adr);
			//if( !(getB() & 0xf)) {printf("%04X \n", Value*256 + getB() );}
			break;					// address latch
  case 0xFF:break;					// ext kanjirom  00h: enable / ffh:disable
  default: //PRINTDEBUG3("[P6.c][DoOut] Unused  port=%02X  value=%02x \n",Port,Value);
			break;
  }
}

/****************************************************************/
/*** Read number from given IO port.                          ***/
/****************************************************************/
byte DoIn(register byte Port)
{
  int i;
  /*static byte Last90Data = 0;*/
  byte Value;
  switch(Port) {
		/* ------------ voice -------------- */   /* 音声合成を少しでも速くするために */
	case 0xE0: Value = InE0H();
				break;
				//Value= (byte) voice_GetStatus(); break;				
				//Value=0x40;break;         /* uPD7752              */
    case 0xE2: Value = InE2H(); break;
    case 0xE3: Value = InE3H(); break;

		/* ------------------------------
		    memory mapping (SR)		2002/2 Windy
	   ------------------------------ */
    case 0x60:case 0x61:case 0x62:case 0x63:case 0x64:case 0x65:case 0x66:case 0x67:
    case 0x68:case 0x69:case 0x6a:case 0x6b:case 0x6c:case 0x6d:case 0x6e:case 0x6f:
    		  if( P6Version ==PC60M2SR  || P6Version == PC66SR )
		    	{
                 Value=port60[ Port-0x60 ];
                }
               else
                {
				 Value= NORAM;
				}
               break;

	/* ------------------------------
		    8251 
	   ------------------------------ */
    case 0x80: Value=NORAM;break;		 /* 8251 data */
    case 0x81: Value=0x85;break;         /* 8251 ACIA status     */

	/* ------------------------------
		    sub CPU
	   ------------------------------ */
    case 0x90:                           /* subCPU input */
    case 0x94:
			  Value= subcpuIN();
			  break;
			  
    case 0x92: Value=NORAM;break;
#if 0
    		   count_port_c_8255++;
    		   if(count_port_c_8255 > 5)
    		     { 
    		      Value=port_c_8255; 	/* return 8255 port c (sub cpu)*/
    		      count_port_c_8255 =0;
    		      }
    		   else
    		      {
    		      Value= pre_port_c_8255;  /* if until n count , return previous value  2009/9/7 */
    		      }
    		  break;
#endif

    case 0x93: Value=NORAM;break;
    		   //Value=port93;		/* 8255 port c bit set/reset (sub cpu)*/
    /* FFhを返却しないと、csaveできない。詳細は、CSAVE のREADY CHECK参照のこと */

	/* ------------------------------
		    8910 / YM-2203 
	   ------------------------------ */
	case 0x72:							/* FM SOUND CARTRIDGE */
    case 0xA2:                           /* 8910 data */
			  if(PSGReg!=14) 
			  		Value=(PSGReg>13? 0xff:PSGTMP[PSGReg]);
		      else
               {
				JoyState[0]= ~JoystickGetState(0);	/* add JOYSTICK 2003/8/31 */
			    JoyState[1]= ~JoystickGetState(1);
			  	if(PSGTMP[15]==0xC0) 
			  		Value=JoyState[0];
				else
				    Value=JoyState[1];
                }
		      break;
	// JoyStat[]に、STICKの結果の反転したデータを書き込んでおく。

	/* ---------------------------------
	A3h: YM-2203 status
		7     6 5 4 3 2    1    0
		SBC               OVB   OVA
		|                  |    |--TIMER A  1:overflow
		|                  --------TIMER B  1:overflow
		-------------- 1:busy  0:ready
          --------------------------------*/

	case 0x73:							/* FM SOUND CARTRIDGE */
    case 0xA3: /*if(sr_mode)*/
			  // if(P6Version ==2 || P6Version ==4)		/* 2009/8/4 Windy */
				{
//				 Value=2+1; 	/* YM-2203 status */
				 Value= ym2203_ReadStatus();	// read from fmgen  2003/10/13
				}
	    	 break;

	/* ---------------------------------
	B2h: floppy interrupt / machine type 
	    7 6 5 4 3 2   1   0
	                 CHK  INT
	                  |    |------ floppy intterupt    1:kaijyo  0:warikomicyuu
	                  ------------ machine type        1:66SR    0:mk2SR
          --------------------------------*/
    case 0xB2:
	          Value=((P6Version== PC66SR )? 2:0)| 1; break; 

    case 0xC0: Value=NORAM;break;        /* pr busy,rs carrier   */
    case 0xC2: Value=NORAM;break;        /* ROM switch           */


	// *************************** DISK ****************  by Windy
    case 0xD1:
    case 0xD3:if( disk_type)
 	             Value= fdc_pop_buffer( Port);  	/* FDC buffer */
 	          else
	             Value= disk_inp(Port);	/* support for mk2 DISK add 2002/4/4 */
	          break;

    case 0xD2:
    case 0xD0: if( disk_type)
	             {Value= fdc_pop_buffer( Port);}
	           else
	             {Value=disk_inp(Port);}     /* disk data  add 2002/3/13 */
	           break;
    case 0xD4: Value=0;    break;	// test

    case 0xDC: Value=fdc_inpDC();break;    /* FDC status */
    case 0xDD: Value=fdc_inpDD();break;    /* FDC data */
 	// *********************************************************

    case 0xF0: if(sr_mode || P6Version== PC60 )// sr_mode and PC-6001
					Value=NORAM;
			   else
					Value=portF0; 
			   break; 
    case 0xF1: if(sr_mode || P6Version== PC60 )// sr_mode and PC-6001
					Value=NORAM;
			   else
					Value=portF1; 
			   break;
    case 0xF3: Value=portF3;break;
    case 0xF6: Value=portF6;break;
    case 0xF7: Value=portF7;break;

	/* ---------------------------------   (SR)
		拡張漢字ROM
	   --------------------------------*/
	case 0xFD:Value= NORAM;
			    if( EXTKANJIROM)
			       Value= EXTKANJIROM[ extkanjirom_adr*2]; 	// read left font
			  break;
  	case 0xFE:Value= NORAM;
			    if( EXTKANJIROM)
			       Value= EXTKANJIROM[ extkanjirom_adr*2+1];// right font
			  break;
//    default: printf("Unused  port=%02X  value=%02x \n",Port,Value);
    default:   Value=NORAM;break;        /* If no port, ret FFh  */
  }

  if( enable_breakpoint)
	for(i=1; i< MAX_BREAKPOINT ; i++)
		if(Port==breakpoint[i].addr && breakpoint[i].action == B_IN  &&  breakpoint[i].enable ==1)
			{
			char tmp[256];
			sprintf( tmp ," *** BREAK at PC:%04X #%d (IN 0x%02X--->DATA 0x%02X) ***\n",getPC(), i , Port , Value);
			setBreakPointMessage( tmp);
			Trace=1; 
			 break; 
			}

  if (Verbose&0x02) printf("--- DoIn  : %02X, %02X ---\n", Port, Value);
  return(Value);
}




/****************************************************************/
/*** SUB CPU OUTPUT                                           ***/
/****************************************************************/
void subcpuOUT(byte Value)
{
	// ------------ TAPE --------------------
                /* CMT SAVE */
      if (CasMode==CAS_SAVEBYTE)
		{ 
		 fputc(Value,CasStream[1]);
		 CasMode=CAS_NONE; 
		 return; 
		}

                /* CMT LOAD STOP */
      if ((Value==0x1a)&&(CasMode==CAS_LOADING)) 
		{ 
		 CasMode=CAS_NONE;
		 return;
		}

                /* CMT SAVE STOP */
      if ((Value==0x3a)&&(CasMode==CAS_SAVEBYTE)) // add Windy 2008/10/5
		{ 
		 CasMode=CAS_NONE;
		 return;
		}
		
		
		/* CMT LOAD OPEN(0x1E,0x19(1200baud)/0x1D,0x19(600baud)) */
      if ((Value==0x19)&&(CasStream[0]))
		{
		 CasMode=CAS_LOADING;
		 return; 
		}

                /* CMT SAVE OPEN */
      if ((Value==0x38)&&(CasStream[1])) /* CMT SAVE DATA */
		{
		 CasMode=CAS_SAVEBYTE; 
		 //printf("CMT SAVE BYTE\n");
		 return;
	 	}
        

	// ------------ KEYIN  --------------------
      if (Value==0x04) 
      	{ kanaMode = kanaMode ? 0 : 1; return; }
      if (Value==0x05) 
      	{ katakana = katakana ? 0 : 1; return; }
      if (Value==0x06) /* strig,stick */
		{ StrigIntFlag = INTFLAG_REQ; return; }	// STRIG Interrupt REQUEST

	// ------------ TIMER (PC-6601SR)  --------------------
      if( DateMode ==DATE_WRITE && P6Version == PC66SR )// DATE Write    2002/4/8
		{								// SUB CPUに、5 byte渡す
         if( DateIdx <5)
			{
			 DateBuff[ DateIdx++]= Value;
			}
		 else 
			{
			 DateMode = DATE_NONE;
			 DateIdx = 0;
			 setfmtdate(DateBuff);		// set date&time 
			}
		}

      if (Value==0x32 && P6Version == PC66SR )	// タイマーからの読み込み  コマンド32 を投げられると、割り込み処理で５バイト受け渡す
		{ 
		    DateIntFlag = INTFLAG_REQ;   // DATE READ Interrupt REQUEST
		    DateMode  = DATE_READ;
		    DateIdx      = 0;
		    memcpy( DateBuff, fmtdate(),5);  // 日時データを用意しておく
	        return;
		}
      else if (Value==0x33  && P6Version == PC66SR )	// タイマーへの書き込み
		{
		   DateMode = DATE_WRITE;      // DATE WRITE
		   DateIdx      = 0;
		   return;
		}
      else if (Value==0x31 &&  P6Version == PC66SR )
		{		     // TV Reserve-DATA READ  Interrupt REQUEST
		   TvrIntFlag = INTFLAG_REQ;
		   TvrMode   = TVR_READ;
		   TvrIdx       = 0;
		   return;
		}
}

/****************************************************************/
/*** SUB CPU INPUT                                            ***/
/****************************************************************/
byte subcpuIN(void)
{
byte Value=0;
     if ((CmtIntFlag == INTFLAG_EXEC) && (CasMode==CAS_LOADBYTE)) {
		/* -------- CMT Load 1 Byte ------ */
		CmtIntFlag = INTFLAG_NONE;
		CasMode=CAS_LOADING;
    	Value=fgetc(CasStream[0]);
		//printf("CMT DATA INP  %02X\n", Value);

      } else if (StrigIntFlag == INTFLAG_EXEC) {
		/* ------- stick,strig ------- */
		StrigIntFlag = INTFLAG_NONE;
          Value = stick0; //stick0=0;
          printf("stick0=%d \n",Value);
		/*
		if (ExecStringInt) {
			Value=stick0;
			ExecStringInt = 0;
			Code16Count = 0;
		} else {
			Code16Count++;
			if (10 == Code16Count) {
				Value = 0x16;
				Code16Count = 0;
			} else {
				Value = stick0;
			}
		}
		*/
      } else if (KeyIntFlag == INTFLAG_EXEC) {
		/* keyboard */
		KeyIntFlag = INTFLAG_NONE;
		if ((p6key == 0xFE) && (keyGFlag == 1)) kanaMode = kanaMode ? 0 : 1;
		if ((p6key == 0xFC) && (keyGFlag == 1)) katakana = katakana ? 0 : 1;
		Value = p6key;
		/*
		if (0 != p6key) Last90Data = Value;
		p6key = 0;
		*/

		// ------------ TIMER  (PC-6601SR)   -------------- 2002/4/8
      } else if ( DateIntFlag==INTFLAG_EXEC && DateMode == DATE_READ) {	
		if( DateIdx <5)						// DATE Interrupt
		   Value= DateBuff[ DateIdx++];		// SUB CPUから、データを渡す
		else
		 {
          Value=0xff;
		  DateMode = DATE_NONE;
		  DateIntFlag=INTFLAG_NONE;
		 }
	  } else if( TvrIntFlag ==INTFLAG_EXEC && TvrMode == TVR_READ) {
		if( TvrIdx <1)					// TV RESERVE-DATA Interrupt
		   Value = TvrBuff[ TvrIdx++];		// dummy dataを渡す。
		else
		 {
		    Value=0xff;
		    TvrMode   = TVR_NONE;
		    TvrIntFlag = INTFLAG_NONE;
		} 
     } else {
        /*if (0 == p6key) Value = Last90Data; else Value = p6key;*/
		Value = p6key;
        }
 return(Value);
}



	/*  VRAM から TEXTVRAM に変更。VRAM先頭アドレスを、グラフィックと、テキストで分けた。
                 screen 2,2,1対策  add 2003/10/25 */
	            //TEXTVRAM=RAM+(Value & 0xf)*0x1000;
	    	 //VRAM = (Value & 0x10) ? RAM+0x8000:RAM+0x0000; /*vram address*/
