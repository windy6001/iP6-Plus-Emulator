/** iP6: PC-6000/6600 series emualtor ************************/
/**                                                         **/
/**                       device.h                          **/
/**                                                         **/
/** by Windy 2002-2004                                      **/
/** by ISHIOKA Hiroshi 1998-2000                            **/
/** This code is based on fMSX written by Marat Fayzullin   **/
/*************************************************************/


#ifndef _DEVICE_H
#define _DEVICE_H

/* ****************** N66SR BASIC  ******************** add 2002/2 */
extern byte port40;					//I/O[40]     PALET NO.16
extern byte port41;					//I/O[41]     PALET NO.15
extern byte port42;					//I/O[42]     PALET NO.14
extern byte port43;					//I/O[43]     PALET NO.13

extern byte port60[16];				//I/O[60..67] READ  MEMORY MAPPING
								//I/O[68-6f]  WRITE MEMORY MAPPING
extern byte port93;					//I/O[93]     8255 MODE SET / BIT SET & RESET
extern byte port94;					//I/O[94]	  shadow of I/O[90]

extern byte portA0;					//I/O[A0]     YM-2203/8910 REGISTER ADDRESS
extern byte portA1;					//I/O[A1]     YM-2203/8910 DATA

extern byte portB0;					//I/O[B0]     SYSTEM LATCH			
extern byte portB8;					//I/O[B8]     INTERRUPT ADDRESS of SUB CPU
extern byte portB9;					//I/O[B9]     INTERRUPT ADDRESS of JOY STICK
extern byte portBA;					//I/O[BA]     INTERRUPT ADDRESS of Timer
extern byte portBB;					//I/O[BB]     INTERRUPT ADDRESS of Voice
extern byte portBC;					//I/O[BC]     INTERRUPT ADDRESS of VRTC
extern byte portBD;					//I/O[BD]     INTERRUPT ADDRESS of RS-232C
extern byte portBE;					//I/O[BE]     INTERRUPT ADDRESS of Printer
extern byte portBF;					//I/O[BF]     INTERRUPT ADDRESS of EXT INT

extern byte portC0;                 //I/O[C0]     COLOR SET SELECT (CSS)
extern byte portC1;					//I/O[C1]     CRT CONTROLLER MODE
extern byte portC2;                 //I/O[C2]     VOICE KANJIROM SELECT
extern byte portC3;                 //I/O[C3]     I/O C2h in/out

extern byte portC8;					//I/O[C8]     CRT CONTROLLER TYPE
extern byte portC9;					//I/O[C9]     SR VRAM ADDRESS
extern byte portCA;					//I/O[CA]     X GEOMETORY low  HARDWARE SCROLL
extern byte portCB;					//I/O[CB]     X GEOMETORY high HARDWARE SCROLL
extern byte portCC;					//I/O[CC]     Y GEOMETORY      HARDWARE SCROLL

extern byte portCE;					//I/O[CE]     LINE SETTING  BITMAP (low) */
extern byte portCF;					//I/O[CF]     LINE SETTING  BITMAP (High) */

extern byte portDC;					//I/O[DC]     FDC status
extern byte portD1;					//I/O[D1]     MINI DISK  (CMD/DATA OUTPUT
extern byte portD2;					//I/O[D2]     MINI DISK  (CONTROL LINE INPUT)


extern byte portFA;					//I/O[FA]     INTERRUPT CONTROLLER
extern byte portFB;					//I/O[FB]     INTERRUPT ADDRESS CONTROLLER


extern byte portF0;					//I/O [F0]	MEMORY MAPPING [N60/N66]
extern byte portF1;					//I/O [F1]	MEMORY MAPPING [N60/N66]
extern byte portF2;                 //I/O [F2]  MEMORY WRITE BLOCK [N60/N66]
extern byte portF3;					//I/O [F3]  WAIT CONTROLL
extern byte portF4;					//I/O [F4]  int1 address set
extern byte portF5;					//I/O [F5]  int2 address set
extern byte portF6;					//I/O [F6]  TIMER COUNTUP 
extern byte portF7;					//I/O [F7]  TIMER INT ADDRESS 
/* 初期値は0x06とする (PC-6001対応) */
extern byte portF8;					//I/O [F8]  CG ACCESS CONTROL
extern byte portFC;					//I/O[FC]     EXT KANJIROM ADDRESS LATCH

extern byte port_c_8255;			// 8255 port c




	// ************* PSG ***************************************

extern byte PSGReg;					// PSG REGISTER LATCH
//byte PSG[16];
extern byte PSGTMP[ 0xc0];				// PSG & FM REGISTER add 2002/10/15

	// ************* KEYBOARD / JOYSTICK ***********************

extern byte JoyState[2];
extern byte p6key;
extern byte stick0 ;
extern byte keyGFlag ;
extern byte kanaMode ;
extern byte katakana ;
extern byte kbFlagGraph ;
extern byte kbFlagCtrl ;
extern byte kbFlagShift;


	// ************** TIMER *********************************

extern byte TimerSW ;
extern byte TimerSW_F3 ;		/* 初期値は1とする（PC-6001対応） */

extern int IntSW_F3;
extern int Code16Count;






extern int  extkanjirom_adr;			// EXT KANJIROM addr


/* palet datas 全ての色の配列を一応保有しているだけ */
extern int palet[ 16] ;


	// ****************** FLOPPY DISK  ********************

extern int disk_type;					// TRUE: PD765A   FALSE: mini disk
extern int disk_num;					// DRIVE NUMBERS  0:disk non  1-2:disk on line
extern int new_disk_num;				//                     (next boot time)
extern int UseDiskLamp;				// 1: use disk lamp  0: no use


extern int P6Version;				// CURRENT MACHINE TYPE
extern int newP6Version;				//     (next boot time)

extern int  PatchLevel ;
extern int CGSW93;


extern byte Verbose ;






	// *************** CPU **********************************

extern int   CPUclock;					// CPU clock
extern int   CPUclock_bak;				// CPU clock backup
extern int   drawwait;					// draw wait
extern int   drawwait_bak;				// draw wait backup

extern int	  busreq;						// busreq  1:ON  0:OFF
extern int   busreq_bak;

extern int CPUclock;
extern int SaveCPU;

	// *********** SCREEN   *************************

extern int   scr4col;
extern int   Console;				// use console mode    1: use
//extern int   keyclick;				// use key click sound 1: use

extern int   UseSaveTapeMenu;		// save tape menu      1: on  0: off
extern int   UseStatusBar;           // status bar          1: on  0: off

extern byte  CSS1,CSS2,CSS3;			// CSS

extern byte  UPeriod     ;           // Interrupts/scr. update 1:60fps  2:30fps
extern byte  UPeriod_bak;				// UPeriod backup
extern byte  EndOfFrame;              // 1 when end of frame

extern byte CRTMode1,CRTMode2,CRTMode3;

extern int   bitmap;					// TRUE: BITMAP MODE   FALSE: TEXT MODE
extern int   cols;					// WIDTH COLUME 
extern int   rows;					// WIDTH LINES  

extern int   lines;				// graphics LINES


	// *********** N66SR BAIC  *************************
	// *********** Date Read Interrupt *****************
extern byte 	DateMode;	// Date mode
extern byte 	DateBuff[ 5];			// Date buffer
extern int		DateIdx;				// Index of Date buffer


extern int   sr_mode;					// SR MODE TRUE: sr_mode  FALSE: not sr_mode
extern int   sr_mode_bak;


extern int   srline;					// sr line   enable when busreq=1
extern int   srline_bak;				// sr line   enable when busreq=1


extern int fm_vol;			// fm volume
extern int psg_vol;			// psg volume




// *********** TV Reserve Data Read Interrupt ***********
extern byte TvrMode;
extern byte TvrBuff[ 2];			// data buffer (dummy array    max length: Unknown)
extern int    TvrIdx;			// Index of data buffer

void dokodemo_save_device(void);
void dokodemo_load_device(void);
void saveDateTime(void);

#endif
