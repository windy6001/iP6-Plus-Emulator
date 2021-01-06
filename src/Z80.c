/** Z80: portable Z80 emulator *******************************/
/**                                                         **/
/**                           Z80.c                         **/
/**                                                         **/
/** This file contains implementation for Z80 CPU. It can   **/
/** be used separately to emulate any Z80-based machine.    **/
/** In this case you will need: Z80.c, Z80.h, PTable.h,     **/
/** Codes.h, CodesED.h, CodesCB.h, CodesXX.h, CodesXCB.h.   **/
/** Don't forget to write Patch(), M_RDMEM(), and M_WRMEM() **/
/** functions to accomodate emulated machine architecture.  **/
/**                                                         **/
/** Copyright (C) Marat Fayzullin 1994-1998                 **/
/**     You are not allowed to distribute this software     **/
/**     commercially. Please, notify me, if you make any    **/
/**     changes to this file.                               **/
/*************************************************************/
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include "P6.h"
#include "mem.h"
#include "Debug.h"
#include "Tables.h"
#include "cycles.h"

#include "device.h"
#include "Timer.h"
#include "schedule.h"
#include "dokodemo.h"


/*** Registers ***********************************************/
/*** Z80 registers and running flag.                       ***/
/*************************************************************/
reg R;
byte CPURunning;

static long ClockCount=0;          /* Variable used to count CPU cycles   */

static long z80_clock = 0;
static long TimerInt_Clock = 0;
long BaseTimerClock = 0;
static long CmtInt_Clock = 0;

static int TintClockRemain;		// タイマ割り込み監視
static int CmtClockRemain;		// CMT割り込み監視
static int Valid_Line = 0;
static long TClock = 0;			// 1秒間に実行したクロック数
int TimerIntFlag = INTFLAG_NONE;
int CmtIntFlag = INTFLAG_NONE;
int StrigIntFlag = INTFLAG_NONE;
int KeyIntFlag = INTFLAG_NONE;


byte CasMode = CAS_NONE;

	// ------------------ for PC-6601SR  --------- add 2002/4/8
int TvrIntFlag;		// TV RESERVE-DATA   Read Interrupt
int VrtcIntFlag;		// VRTC Interrupt                  2002/4/21

int DateIntFlag = INTFLAG_NONE;	// DATE Interrupt  for PC-6601SR add 2002/4/8

int VoiceIntFlag = INTFLAG_NONE; // voice Interrupt for PC-6601SR add 2009/2/3


int code_log_flag;		// 1:op code logging  0:normal run

word save_intr=0xffff;


word getPC(void);
int exec1(void);
void IntrCtrl(long NowClock);


/*** Interrupts **********************************************/
/*** Interrupt-related variables.                          ***/
/*************************************************************/
#ifdef INTERRUPTS
byte IFlag = 0;       /* If IFlag==1, gen. int. and set to 0 */
#endif

/*** Trace and Trap ******************************************/
/*** Switches to turn tracing on and off in DEBUG mode.    ***/
/*************************************************************/
#ifdef DEBUG

byte Trace=0;       /* Tracing is on if Trace==1  */
word Trap=0xFFFF;   /* When PC==Trap, set Trace=1 */
int  monitor_mode=0;  /* 1: monitor_mode  0: normal_mode */
int  inTrace=DEBUG_NONE;		/* Tracing */
int  animatemode = 0;		/* 1: animate debug mode   0: normal debug mode */
int  enable_breakpoint=0;   /* bit b0: 1:bp0 enable   b1:  1:bp1 enable...*/

BREAKPOINT  breakpoint[ MAX_BREAKPOINT];
SAVEVRAM    savevram[MAX_SAVEVRAM];
#endif

/*** TrapBadOps **********************************************/
/*** When 1, print warnings of illegal Z80 instructions.   ***/
/*************************************************************/
byte TrapBadOps=0;

/*** nowait from PC ADDR to PC ADDR */
int nowait_start_addr = 0xffff;
int nowait_end_addr   = 0xffff;

 // all variable is static !

 static int  FastTapeRepeat=1;
 //static byte I, j;
 static pair J;

 static int HCount = 0;
 static int	NowClock;
 static int hline;

// ****************************************************************************
//			dokodemo save z80  (どこでもSAVEから呼ばれる）
// ****************************************************************************
void dokodemo_save_z80(void)
{
	dokodemo_putsection("[Z80]");
	dokodemo_putentry_word( "AF", R.AF.W);
	dokodemo_putentry_word( "BC", R.BC.W);
	dokodemo_putentry_word( "DE", R.DE.W);
	dokodemo_putentry_word( "HL", R.HL.W);
	dokodemo_putentry_word( "IX", R.IX.W);
	dokodemo_putentry_word( "IY", R.IY.W);
	dokodemo_putentry_word( "PC", R.PC.W);
	dokodemo_putentry_word( "SP", R.SP.W);

	dokodemo_putentry_word( "AF1", R.AF1.W);
	dokodemo_putentry_word( "BC1", R.BC1.W);
	dokodemo_putentry_word( "DE1", R.DE1.W);
	dokodemo_putentry_word( "HL1", R.HL1.W);
	dokodemo_putentry_byte( "IFF", R.IFF);
	dokodemo_putentry_byte( "I"  , R.I);


//	DOKODEMO_PUTENTRY_INT( CPURunning);
	DOKODEMO_PUTENTRY_INT( ClockCount);    /* long */      /* Variable used to count CPU cycles   */
	
	DOKODEMO_PUTENTRY_INT( z80_clock);	/* long */
	DOKODEMO_PUTENTRY_INT( TimerInt_Clock ); /* long */
	DOKODEMO_PUTENTRY_INT( BaseTimerClock ); /* long */
	DOKODEMO_PUTENTRY_INT( CmtInt_Clock );  /* long */
	
	DOKODEMO_PUTENTRY_INT( TintClockRemain);		// タイマ割り込み監視
	DOKODEMO_PUTENTRY_INT( CmtClockRemain);		// CMT割り込み監視
	DOKODEMO_PUTENTRY_INT( Valid_Line );
	DOKODEMO_PUTENTRY_INT( TClock );		/* long */	// 1秒間に実行したクロック数
	DOKODEMO_PUTENTRY_INT( TimerIntFlag );
	DOKODEMO_PUTENTRY_INT( CmtIntFlag );
	DOKODEMO_PUTENTRY_INT( StrigIntFlag );
	DOKODEMO_PUTENTRY_INT( KeyIntFlag );
	
	
	DOKODEMO_PUTENTRY_BYTE( CasMode );  /* byte */
	
	// ------------------ for PC-6601SR  --------- add 2002/4/8
	DOKODEMO_PUTENTRY_INT( TvrIntFlag);		// TV RESERVE-DATA   Read Interrupt
	DOKODEMO_PUTENTRY_INT( VrtcIntFlag);		// VRTC Interrupt                  2002/4/21
	DOKODEMO_PUTENTRY_INT( DateIntFlag);	// DATE Interrupt  for PC-6601SR add 2002/4/8
	DOKODEMO_PUTENTRY_INT( VoiceIntFlag ); // voice Interrupt for PC-6601SR add 2009/2/3
	
	
	DOKODEMO_PUTENTRY_INT(code_log_flag);		// 1:op code logging  0:normal run
	
	DOKODEMO_PUTENTRY_WORD( save_intr );	/* word */
	
	/*** Interrupts **********************************************/
	/*** Interrupt-related variables.                          ***/
	/*************************************************************/
#ifdef INTERRUPTS
	DOKODEMO_PUTENTRY_BYTE(  IFlag );       /* If IFlag==1, gen. int. and set to 0 */
#endif
	
	/*** Trace and Trap ******************************************/
	/*** Switches to turn tracing on and off in DEBUG mode.    ***/
	/*************************************************************/
#ifdef DEBUG
	DOKODEMO_PUTENTRY_BYTE( Trace);       /* Tracing is on if Trace==1  */
	DOKODEMO_PUTENTRY_WORD( Trap);   /* When PC==Trap, set Trace=1 */
	DOKODEMO_PUTENTRY_INT( monitor_mode);  /* 1: monitor_mode  0: normal_mode */
	DOKODEMO_PUTENTRY_INT( inTrace);		/* Tracing */
	DOKODEMO_PUTENTRY_INT( animatemode );		/* 1: animate debug mode   0: normal debug mode */
#endif
	
	/*** TrapBadOps **********************************************/
	/*** When 1, print warnings of illegal Z80 instructions.   ***/
	/*************************************************************/
	DOKODEMO_PUTENTRY_BYTE( TrapBadOps);


	  // all variable is static !
  DOKODEMO_PUTENTRY_INT( FastTapeRepeat);
  //static byte I, j;
  DOKODEMO_PUTENTRY_WORD( J.W);

  DOKODEMO_PUTENTRY_INT( HCount );
  DOKODEMO_PUTENTRY_INT( NowClock );
  DOKODEMO_PUTENTRY_INT( hline);

  DOKODEMO_PUTENTRY_INT( nowait_start_addr);
  DOKODEMO_PUTENTRY_INT( nowait_end_addr);
}


// ****************************************************************************
//			dokodemo load z80  (どこでもloadから呼ばれる）
// ****************************************************************************
void dokodemo_load_z80(void)
{
	dokodemo_getentry_word( "AF", &R.AF.W);
	dokodemo_getentry_word( "BC", &R.BC.W);
	dokodemo_getentry_word( "DE", &R.DE.W);
	dokodemo_getentry_word( "HL", &R.HL.W);
	dokodemo_getentry_word( "IX", &R.IX.W);
	dokodemo_getentry_word( "IY", &R.IY.W);
	dokodemo_getentry_word( "PC", &R.PC.W);
	dokodemo_getentry_word( "SP", &R.SP.W);
	
	dokodemo_getentry_word( "AF1", &R.AF1.W);
	dokodemo_getentry_word( "BC1", &R.BC1.W);
	dokodemo_getentry_word( "DE1", &R.DE1.W);
	dokodemo_getentry_word( "HL1", &R.HL1.W);
	dokodemo_getentry_byte( "IFF", &R.IFF);
	dokodemo_getentry_byte( "I"  , &R.I);
	
	
//	DOKODEMO_GETENTRY_INT( CPURunning);
	DOKODEMO_GETENTRY_INT( ClockCount);    /* long */      /* Variable used to count CPU cycles   */
	
	DOKODEMO_GETENTRY_INT( z80_clock);	/* long */
	DOKODEMO_GETENTRY_INT( TimerInt_Clock ); /* long */
	DOKODEMO_GETENTRY_INT( BaseTimerClock ); /* long */
	DOKODEMO_GETENTRY_INT( CmtInt_Clock );  /* long */
	
	DOKODEMO_GETENTRY_INT( TintClockRemain);		// タイマ割り込み監視
	DOKODEMO_GETENTRY_INT( CmtClockRemain);		// CMT割り込み監視
	DOKODEMO_GETENTRY_INT( Valid_Line );
	DOKODEMO_GETENTRY_INT( TClock );		/* long */	// 1秒間に実行したクロック数
	DOKODEMO_GETENTRY_INT( TimerIntFlag );
	DOKODEMO_GETENTRY_INT( CmtIntFlag );
	DOKODEMO_GETENTRY_INT( StrigIntFlag );
	DOKODEMO_GETENTRY_INT( KeyIntFlag );
	
	
	DOKODEMO_GETENTRY_BYTE( CasMode );  /* byte */
	
	// ------------------ for PC-6601SR  --------- add 2002/4/8
	DOKODEMO_GETENTRY_INT( TvrIntFlag);		// TV RESERVE-DATA   Read Interrupt
	DOKODEMO_GETENTRY_INT( VrtcIntFlag);		// VRTC Interrupt                  2002/4/21
	DOKODEMO_GETENTRY_INT( DateIntFlag);	// DATE Interrupt  for PC-6601SR add 2002/4/8
	DOKODEMO_GETENTRY_INT( VoiceIntFlag ); // voice Interrupt for PC-6601SR add 2009/2/3
	
	
	DOKODEMO_GETENTRY_INT(code_log_flag);		// 1:op code logging  0:normal run
	
	DOKODEMO_GETENTRY_WORD( save_intr );	/* word */
	
	/*** Interrupts **********************************************/
	/*** Interrupt-related variables.                          ***/
	/*************************************************************/
#ifdef INTERRUPTS
	DOKODEMO_GETENTRY_BYTE(  IFlag );       /* If IFlag==1, gen. int. and set to 0 */
#endif
	
	/*** Trace and Trap ******************************************/
	/*** Switches to turn tracing on and off in DEBUG mode.    ***/
	/*************************************************************/
#ifdef DEBUG
	DOKODEMO_GETENTRY_BYTE( Trace);       /* Tracing is on if Trace==1  */
	DOKODEMO_GETENTRY_WORD( Trap);   /* When PC==Trap, set Trace=1 */
	DOKODEMO_GETENTRY_INT( monitor_mode);  /* 1: monitor_mode  0: normal_mode */
	DOKODEMO_GETENTRY_INT( inTrace);		/* Tracing */
	DOKODEMO_GETENTRY_INT( animatemode );		/* 1: animate debug mode   0: normal debug mode */
#endif
	
	/*** TrapBadOps **********************************************/
	/*** When 1, print warnings of illegal Z80 instructions.   ***/
	/*************************************************************/
	DOKODEMO_GETENTRY_BYTE( TrapBadOps);

	  // all variable is static !
  DOKODEMO_GETENTRY_INT( FastTapeRepeat);
  //static byte I, j;
  DOKODEMO_GETENTRY_WORD( J.W);

  DOKODEMO_GETENTRY_INT( HCount );
  DOKODEMO_GETENTRY_INT( NowClock );
  DOKODEMO_GETENTRY_INT( hline);

  DOKODEMO_GETENTRY_INT(nowait_start_addr);
  DOKODEMO_GETENTRY_INT(nowait_end_addr);
}




// ****************************************************************************
//			Z80命令のマクロ
// ****************************************************************************


#define S(Fl)        R.AF.B.l|=Fl
#define R(Fl)        R.AF.B.l&=~(Fl)
#define FLAGS(Rg,Fl) R.AF.B.l=Fl|ZSTable[Rg]

#define M_RLC(Rg)      \
  R.AF.B.l=Rg>>7;Rg=(Rg<<1)|R.AF.B.l;R.AF.B.l|=PZSTable[Rg]
#define M_RRC(Rg)      \
  R.AF.B.l=Rg&0x01;Rg=(Rg>>1)|(R.AF.B.l<<7);R.AF.B.l|=PZSTable[Rg]
#define M_RL(Rg)       \
  if(Rg&0x80)          \
  {                    \
    Rg=(Rg<<1)|(R.AF.B.l&C_FLAG); \
    R.AF.B.l=PZSTable[Rg]|C_FLAG; \
  }                    \
  else                 \
  {                    \
    Rg=(Rg<<1)|(R.AF.B.l&C_FLAG); \
    R.AF.B.l=PZSTable[Rg];        \
  }
#define M_RR(Rg)       \
  if(Rg&0x01)          \
  {                    \
    Rg=(Rg>>1)|(R.AF.B.l<<7);     \
    R.AF.B.l=PZSTable[Rg]|C_FLAG; \
  }                    \
  else                 \
  {                    \
    Rg=(Rg>>1)|(R.AF.B.l<<7);     \
    R.AF.B.l=PZSTable[Rg];        \
  }

#define M_SLA(Rg)      \
  R.AF.B.l=Rg>>7;Rg<<=1;R.AF.B.l|=PZSTable[Rg]
#define M_SRA(Rg)      \
  R.AF.B.l=Rg&C_FLAG;Rg=(Rg>>1)|(Rg&0x80);R.AF.B.l|=PZSTable[Rg]

#define M_SLL(Rg)      \
  R.AF.B.l=Rg>>7;Rg=(Rg<<1)|0x01;R.AF.B.l|=PZSTable[Rg]
#define M_SRL(Rg)      \
  R.AF.B.l=Rg&0x01;Rg>>=1;R.AF.B.l|=PZSTable[Rg]

#define M_BIT(Bit,Rg)  \
  R.AF.B.l=(R.AF.B.l&~(N_FLAG|Z_FLAG))|H_FLAG|(Rg&(1<<Bit)? 0:Z_FLAG)

#define M_SET(Bit,Rg) Rg|=1<<Bit
#define M_RES(Bit,Rg) Rg&=~(1<<Bit)

#define M_POP(Rg)      \
  R.Rg.B.l=M_RDMEM(R.SP.W++);R.Rg.B.h=M_RDMEM(R.SP.W++)
#define M_PUSH(Rg)     \
  M_WRMEM(--R.SP.W,R.Rg.B.h);M_WRMEM(--R.SP.W,R.Rg.B.l)

#define M_CALL         \
  J.B.l=M_RDMEM(R.PC.W++);J.B.h=M_RDMEM(R.PC.W++);       \
  M_WRMEM(--R.SP.W,R.PC.B.h);M_WRMEM(--R.SP.W,R.PC.B.l); \
  R.PC.W=J.W

#define M_JP  J.B.l=M_RDMEM(R.PC.W++);J.B.h=M_RDMEM(R.PC.W);R.PC.W=J.W
#define M_JR  R.PC.W+=(offset)M_RDMEM(R.PC.W)+1
#define M_RET R.PC.B.l=M_RDMEM(R.SP.W++);R.PC.B.h=M_RDMEM(R.SP.W++)

#define M_RST(Ad)      \
  M_WRMEM(--R.SP.W,R.PC.B.h);M_WRMEM(--R.SP.W,R.PC.B.l);R.PC.W=Ad

#define M_LDWORD(Rg)   \
  R.Rg.B.l=M_RDMEM(R.PC.W++);R.Rg.B.h=M_RDMEM(R.PC.W++)

#define M_ADD(Rg)      \
  J.W=R.AF.B.h+Rg;     \
  R.AF.B.l=            \
    (~(R.AF.B.h^Rg)&(Rg^J.B.l)&0x80? V_FLAG:0)| \
    J.B.h|ZSTable[J.B.l]|                       \
    ((R.AF.B.h^Rg^J.B.l)&H_FLAG);               \
  R.AF.B.h=J.B.l       

#define M_SUB(Rg)      \
  J.W=R.AF.B.h-Rg;     \
  R.AF.B.l=            \
    ((R.AF.B.h^Rg)&(R.AF.B.h^J.B.l)&0x80? V_FLAG:0)| \
    N_FLAG|-J.B.h|ZSTable[J.B.l]|                    \
    ((R.AF.B.h^Rg^J.B.l)&H_FLAG);                    \
  R.AF.B.h=J.B.l

#define M_ADC(Rg)      \
  J.W=R.AF.B.h+Rg+(R.AF.B.l&C_FLAG); \
  R.AF.B.l=                          \
    (~(R.AF.B.h^Rg)&(Rg^J.B.l)&0x80? V_FLAG:0)| \
    J.B.h|ZSTable[J.B.l]|            \
    ((R.AF.B.h^Rg^J.B.l)&H_FLAG);    \
  R.AF.B.h=J.B.l

#define M_SBC(Rg)      \
  J.W=R.AF.B.h-Rg-(R.AF.B.l&C_FLAG); \
  R.AF.B.l=                          \
    ((R.AF.B.h^Rg)&(R.AF.B.h^J.B.l)&0x80? V_FLAG:0)| \
    N_FLAG|-J.B.h|ZSTable[J.B.l]|    \
    ((R.AF.B.h^Rg^J.B.l)&H_FLAG);    \
  R.AF.B.h=J.B.l

#define M_CP(Rg)       \
  J.W=R.AF.B.h-Rg;     \
  R.AF.B.l=            \
    ((R.AF.B.h^Rg)&(R.AF.B.h^J.B.l)&0x80? V_FLAG:0)| \
    N_FLAG|-J.B.h|ZSTable[J.B.l]|                    \
    ((R.AF.B.h^Rg^J.B.l)&H_FLAG)

#define M_AND(Rg) R.AF.B.h&=Rg;R.AF.B.l=H_FLAG|PZSTable[R.AF.B.h]
#define M_OR(Rg)  R.AF.B.h|=Rg;R.AF.B.l=PZSTable[R.AF.B.h]
#define M_XOR(Rg) R.AF.B.h^=Rg;R.AF.B.l=PZSTable[R.AF.B.h]
#define M_IN(Rg)  Rg=DoIn(R.BC.B.l);R.AF.B.l=PZSTable[Rg]|(R.AF.B.l&C_FLAG)

#define M_INC(Rg)       \
  Rg++;                 \
  R.AF.B.l=             \
    (R.AF.B.l&C_FLAG)|ZSTable[Rg]|           \
    (Rg==0x80? V_FLAG:0)|(Rg&0x0F? 0:H_FLAG)

#define M_DEC(Rg)       \
  Rg--;                 \
  R.AF.B.l=             \
    N_FLAG|(R.AF.B.l&C_FLAG)|ZSTable[Rg]|    \
    (Rg==0x7F? V_FLAG:0)|((Rg&0x0F)==0x0F? H_FLAG:0)

#define M_ADDW(Rg1,Rg2) \
  J.W=(R.Rg1.W+R.Rg2.W)&0xFFFF;                        \
  R.AF.B.l=                                            \
    (R.AF.B.l&~(H_FLAG|N_FLAG|C_FLAG))|                \
    ((R.Rg1.W^R.Rg2.W^J.W)&0x1000? H_FLAG:0)|          \
    (((long)R.Rg1.W+(long)R.Rg2.W)&0x10000? C_FLAG:0); \
  R.Rg1.W=J.W

#define M_ADCW(Rg)      \
  I=R.AF.B.l&C_FLAG;J.W=(R.HL.W+R.Rg.W+I)&0xFFFF;            \
  R.AF.B.l=                                                  \
    (((long)R.HL.W+(long)R.Rg.W+(long)I)&0x10000? C_FLAG:0)| \
    (~(R.HL.W^R.Rg.W)&(R.Rg.W^J.W)&0x8000? V_FLAG:0)|        \
    ((R.HL.W^R.Rg.W^J.W)&0x1000? H_FLAG:0)|                  \
    (J.W? 0:Z_FLAG)|(J.B.h&S_FLAG);                          \
  R.HL.W=J.W

#define M_SBCW(Rg)      \
  I=R.AF.B.l&C_FLAG;J.W=(R.HL.W-R.Rg.W-I)&0xFFFF;            \
  R.AF.B.l=                                                  \
    N_FLAG|                                                  \
    (((long)R.HL.W-(long)R.Rg.W-(long)I)&0x10000? C_FLAG:0)| \
    ((R.HL.W^R.Rg.W)&(R.HL.W^J.W)&0x8000? V_FLAG:0)|         \
    ((R.HL.W^R.Rg.W^J.W)&0x1000? H_FLAG:0)|                  \
    (J.W? 0:Z_FLAG)|(J.B.h&S_FLAG);                          \
  R.HL.W=J.W


enum Codes
{
  NOP,LD_BC_WORD,LD_xBC_A,INC_BC,INC_B,DEC_B,LD_B_BYTE,RLCA,
  EX_AF_AF,ADD_HL_BC,LD_A_xBC,DEC_BC,INC_C,DEC_C,LD_C_BYTE,RRCA,
  DJNZ,LD_DE_WORD,LD_xDE_A,INC_DE,INC_D,DEC_D,LD_D_BYTE,RLA,
  JR,ADD_HL_DE,LD_A_xDE,DEC_DE,INC_E,DEC_E,LD_E_BYTE,RRA,
  JR_NZ,LD_HL_WORD,LD_xWORD_HL,INC_HL,INC_H,DEC_H,LD_H_BYTE,DAA,
  JR_Z,ADD_HL_HL,LD_HL_xWORD,DEC_HL,INC_L,DEC_L,LD_L_BYTE,CPL,
  JR_NC,LD_SP_WORD,LD_xWORD_A,INC_SP,INC_xHL,DEC_xHL,LD_xHL_BYTE,SCF,
  JR_C,ADD_HL_SP,LD_A_xWORD,DEC_SP,INC_A,DEC_A,LD_A_BYTE,CCF,
  LD_B_B,LD_B_C,LD_B_D,LD_B_E,LD_B_H,LD_B_L,LD_B_xHL,LD_B_A,
  LD_C_B,LD_C_C,LD_C_D,LD_C_E,LD_C_H,LD_C_L,LD_C_xHL,LD_C_A,
  LD_D_B,LD_D_C,LD_D_D,LD_D_E,LD_D_H,LD_D_L,LD_D_xHL,LD_D_A,
  LD_E_B,LD_E_C,LD_E_D,LD_E_E,LD_E_H,LD_E_L,LD_E_xHL,LD_E_A,
  LD_H_B,LD_H_C,LD_H_D,LD_H_E,LD_H_H,LD_H_L,LD_H_xHL,LD_H_A,
  LD_L_B,LD_L_C,LD_L_D,LD_L_E,LD_L_H,LD_L_L,LD_L_xHL,LD_L_A,
  LD_xHL_B,LD_xHL_C,LD_xHL_D,LD_xHL_E,LD_xHL_H,LD_xHL_L,HALT,LD_xHL_A,
  LD_A_B,LD_A_C,LD_A_D,LD_A_E,LD_A_H,LD_A_L,LD_A_xHL,LD_A_A,
  ADD_B,ADD_C,ADD_D,ADD_E,ADD_H,ADD_L,ADD_xHL,ADD_A,
  ADC_B,ADC_C,ADC_D,ADC_E,ADC_H,ADC_L,ADC_xHL,ADC_A,
  SUB_B,SUB_C,SUB_D,SUB_E,SUB_H,SUB_L,SUB_xHL,SUB_A,
  SBC_B,SBC_C,SBC_D,SBC_E,SBC_H,SBC_L,SBC_xHL,SBC_A,
  AND_B,AND_C,AND_D,AND_E,AND_H,AND_L,AND_xHL,AND_A,
  XOR_B,XOR_C,XOR_D,XOR_E,XOR_H,XOR_L,XOR_xHL,XOR_A,
  OR_B,OR_C,OR_D,OR_E,OR_H,OR_L,OR_xHL,OR_A,
  CP_B,CP_C,CP_D,CP_E,CP_H,CP_L,CP_xHL,CP_A,
  RET_NZ,POP_BC,JP_NZ,JP,CALL_NZ,PUSH_BC,ADD_BYTE,RST00,
  RET_Z,RET,JP_Z,PFX_CB,CALL_Z,CALL,ADC_BYTE,RST08,
  RET_NC,POP_DE,JP_NC,OUTA,CALL_NC,PUSH_DE,SUB_BYTE,RST10,
  RET_C,EXX,JP_C,INA,CALL_C,PFX_DD,SBC_BYTE,RST18,
  RET_PO,POP_HL,JP_PO,EX_HL_xSP,CALL_PO,PUSH_HL,AND_BYTE,RST20,
  RET_PE,LD_PC_HL,JP_PE,EX_DE_HL,CALL_PE,PFX_ED,XOR_BYTE,RST28,
  RET_P,POP_AF,JP_P,DI,CALL_P,PUSH_AF,OR_BYTE,RST30,
  RET_M,LD_SP_HL,JP_M,EI,CALL_M,PFX_FD,CP_BYTE,RST38
};

enum CodesCB
{
  RLC_B,RLC_C,RLC_D,RLC_E,RLC_H,RLC_L,RLC_xHL,RLC_A,
  RRC_B,RRC_C,RRC_D,RRC_E,RRC_H,RRC_L,RRC_xHL,RRC_A,
  RL_B,RL_C,RL_D,RL_E,RL_H,RL_L,RL_xHL,RL_A,
  RR_B,RR_C,RR_D,RR_E,RR_H,RR_L,RR_xHL,RR_A,
  SLA_B,SLA_C,SLA_D,SLA_E,SLA_H,SLA_L,SLA_xHL,SLA_A,
  SRA_B,SRA_C,SRA_D,SRA_E,SRA_H,SRA_L,SRA_xHL,SRA_A,
  SLL_B,SLL_C,SLL_D,SLL_E,SLL_H,SLL_L,SLL_xHL,SLL_A,
  SRL_B,SRL_C,SRL_D,SRL_E,SRL_H,SRL_L,SRL_xHL,SRL_A,
  BIT0_B,BIT0_C,BIT0_D,BIT0_E,BIT0_H,BIT0_L,BIT0_xHL,BIT0_A,
  BIT1_B,BIT1_C,BIT1_D,BIT1_E,BIT1_H,BIT1_L,BIT1_xHL,BIT1_A,
  BIT2_B,BIT2_C,BIT2_D,BIT2_E,BIT2_H,BIT2_L,BIT2_xHL,BIT2_A,
  BIT3_B,BIT3_C,BIT3_D,BIT3_E,BIT3_H,BIT3_L,BIT3_xHL,BIT3_A,
  BIT4_B,BIT4_C,BIT4_D,BIT4_E,BIT4_H,BIT4_L,BIT4_xHL,BIT4_A,
  BIT5_B,BIT5_C,BIT5_D,BIT5_E,BIT5_H,BIT5_L,BIT5_xHL,BIT5_A,
  BIT6_B,BIT6_C,BIT6_D,BIT6_E,BIT6_H,BIT6_L,BIT6_xHL,BIT6_A,
  BIT7_B,BIT7_C,BIT7_D,BIT7_E,BIT7_H,BIT7_L,BIT7_xHL,BIT7_A,
  RES0_B,RES0_C,RES0_D,RES0_E,RES0_H,RES0_L,RES0_xHL,RES0_A,
  RES1_B,RES1_C,RES1_D,RES1_E,RES1_H,RES1_L,RES1_xHL,RES1_A,
  RES2_B,RES2_C,RES2_D,RES2_E,RES2_H,RES2_L,RES2_xHL,RES2_A,
  RES3_B,RES3_C,RES3_D,RES3_E,RES3_H,RES3_L,RES3_xHL,RES3_A,
  RES4_B,RES4_C,RES4_D,RES4_E,RES4_H,RES4_L,RES4_xHL,RES4_A,
  RES5_B,RES5_C,RES5_D,RES5_E,RES5_H,RES5_L,RES5_xHL,RES5_A,
  RES6_B,RES6_C,RES6_D,RES6_E,RES6_H,RES6_L,RES6_xHL,RES6_A,
  RES7_B,RES7_C,RES7_D,RES7_E,RES7_H,RES7_L,RES7_xHL,RES7_A,  
  SET0_B,SET0_C,SET0_D,SET0_E,SET0_H,SET0_L,SET0_xHL,SET0_A,
  SET1_B,SET1_C,SET1_D,SET1_E,SET1_H,SET1_L,SET1_xHL,SET1_A,
  SET2_B,SET2_C,SET2_D,SET2_E,SET2_H,SET2_L,SET2_xHL,SET2_A,
  SET3_B,SET3_C,SET3_D,SET3_E,SET3_H,SET3_L,SET3_xHL,SET3_A,
  SET4_B,SET4_C,SET4_D,SET4_E,SET4_H,SET4_L,SET4_xHL,SET4_A,
  SET5_B,SET5_C,SET5_D,SET5_E,SET5_H,SET5_L,SET5_xHL,SET5_A,
  SET6_B,SET6_C,SET6_D,SET6_E,SET6_H,SET6_L,SET6_xHL,SET6_A,
  SET7_B,SET7_C,SET7_D,SET7_E,SET7_H,SET7_L,SET7_xHL,SET7_A
};
  
enum CodesED
{
  DB_00,DB_01,DB_02,DB_03,DB_04,DB_05,DB_06,DB_07,
  DB_08,DB_09,DB_0A,DB_0B,DB_0C,DB_0D,DB_0E,DB_0F,
  DB_10,DB_11,DB_12,DB_13,DB_14,DB_15,DB_16,DB_17,
  DB_18,DB_19,DB_1A,DB_1B,DB_1C,DB_1D,DB_1E,DB_1F,
  DB_20,DB_21,DB_22,DB_23,DB_24,DB_25,DB_26,DB_27,
  DB_28,DB_29,DB_2A,DB_2B,DB_2C,DB_2D,DB_2E,DB_2F,
  DB_30,DB_31,DB_32,DB_33,DB_34,DB_35,DB_36,DB_37,
  DB_38,DB_39,DB_3A,DB_3B,DB_3C,DB_3D,DB_3E,DB_3F,
  IN_B_xC,OUT_xC_B,SBC_HL_BC,LD_xWORDe_BC,NEG,RETN,IM_0,LD_I_A,
  IN_C_xC,OUT_xC_C,ADC_HL_BC,LD_BC_xWORDe,DB_4C,RETI,DB_,LD_R_A,
  IN_D_xC,OUT_xC_D,SBC_HL_DE,LD_xWORDe_DE,DB_54,DB_55,IM_1,LD_A_I,
  IN_E_xC,OUT_xC_E,ADC_HL_DE,LD_DE_xWORDe,DB_5C,DB_5D,IM_2,LD_A_R,
  IN_H_xC,OUT_xC_H,SBC_HL_HL,LD_xWORDe_HL,DB_64,DB_65,DB_66,RRD,
  IN_L_xC,OUT_xC_L,ADC_HL_HL,LD_HL_xWORDe,DB_6C,DB_6D,DB_6E,RLD,
  IN_F_xC,OUT_xC_F,SBC_HL_SP,LD_xWORDe_SP,DB_74,DB_75,DB_76,DB_77,
  IN_A_xC,OUT_xC_A,ADC_HL_SP,LD_SP_xWORDe,DB_7C,DB_7D,DB_7E,DB_7F,
  DB_80,DB_81,DB_82,DB_83,DB_84,DB_85,DB_86,DB_87,
  DB_88,DB_89,DB_8A,DB_8B,DB_8C,DB_8D,DB_8E,DB_8F,
  DB_90,DB_91,DB_92,DB_93,DB_94,DB_95,DB_96,DB_97,
  DB_98,DB_99,DB_9A,DB_9B,DB_9C,DB_9D,DB_9E,DB_9F,
  LDI,CPI,INI,OUTI,DB_A4,DB_A5,DB_A6,DB_A7,
  LDD,CPD,IND,OUTD,DB_AC,DB_AD,DB_AE,DB_AF,
  LDIR,CPIR,INIR,OTIR,DB_B4,DB_B5,DB_B6,DB_B7,
  LDDR,CPDR,INDR,OTDR,DB_BC,DB_BD,DB_BE,DB_BF,
  DB_C0,DB_C1,DB_C2,DB_C3,DB_C4,DB_C5,DB_C6,DB_C7,
  DB_C8,DB_C9,DB_CA,DB_CB,DB_CC,DB_CD,DB_CE,DB_CF,
  DB_D0,DB_D1,DB_D2,DB_D3,DB_D4,DB_D5,DB_D6,DB_D7,
  DB_D8,DB_D9,DB_DA,DB_DB,DB_DC,DB_DD,DB_DE,DB_DF,
  DB_E0,DB_E1,DB_E2,DB_E3,DB_E4,DB_E5,DB_E6,DB_E7,
  DB_E8,DB_E9,DB_EA,DB_EB,DB_EC,DB_ED,DB_EE,DB_EF,
  DB_F0,DB_F1,DB_F2,DB_F3,DB_F4,DB_F5,DB_F6,DB_F7,
  DB_F8,DB_F9,DB_FA,DB_FB,DB_FC,DB_FD,DB_FE,DB_FF
};

// ****************************************************************************
//			CB コードの処理
// ****************************************************************************

static void CodesCB(void)
{
  register byte I;

  ClockCount-=cycles_cb[I=M_RDMEM(R.PC.W++)];
  switch(I)
  {
#include "CodesCB.h"
    default:
      if(TrapBadOps)
        printf
        (   
          "Unrecognized instruction: CB %X at PC=%hX\n",
          M_RDMEM(R.PC.W-1),R.PC.W-2
        );
  }
}

// ****************************************************************************
//			DD CB コードの処理
// ****************************************************************************
static void CodesDDCB(void)
{
  register pair J;
  register byte I;

#define XX IX    
  J.W=R.XX.W+(offset)M_RDMEM(R.PC.W++);

  ClockCount-=cycles_xx_cb[I=M_RDMEM(R.PC.W++)];
  switch(I)
  {
#include "CodesXCB.h"
    default:
      if(TrapBadOps)
        printf
        (
          "Unrecognized instruction: DD CB %X %X at PC=%hX\n",
          M_RDMEM(R.PC.W-2),M_RDMEM(R.PC.W-1),R.PC.W-4
        );
  }
#undef XX
}

// ****************************************************************************
//			FD CB コードの処理
// ****************************************************************************
static void CodesFDCB(void)
{
  register pair J;
  register byte I;

#define XX IY
  J.W=R.XX.W+(offset)M_RDMEM(R.PC.W++);

  ClockCount-=cycles_xx_cb[I=M_RDMEM(R.PC.W++)];
  switch(I)
  {
#include "CodesXCB.h"
    default:
      if(TrapBadOps)
        printf
        (
          "Unrecognized instruction: FD CB %X %X at PC=%hX\n",
          M_RDMEM(R.PC.W-2),M_RDMEM(R.PC.W-1),R.PC.W-4
        );
  }
#undef XX
}

// ****************************************************************************
//			ED コードの処理
// ****************************************************************************
static void CodesED(void)
{
  register byte I;
  register pair J;

  /*ClockCount-=cycles_xx[I=M_RDMEM(R.PC.W++)];*/
  ClockCount-=cycles_ed[I=M_RDMEM(R.PC.W++)];
  switch(I)
  {
#include "CodesED.h"
    case PFX_ED:
      R.PC.W--;break;
    default:
      if(TrapBadOps)
        printf
        (
          "Unrecognized instruction: ED %X at PC=%hX\n",
          M_RDMEM(R.PC.W-1),R.PC.W-2
        );
  }
}

// ****************************************************************************
//			DD コードの処理
// ****************************************************************************
static void CodesDD(void)
{
  register byte I;
  register pair J;

#define XX IX
  ClockCount-=cycles_xx[I=M_RDMEM(R.PC.W++)];
  switch(I)
  {
#include "CodesXX.h"
    case PFX_FD:
    case PFX_DD:
      R.PC.W--;break;
    case PFX_CB:
      CodesDDCB();break;
    case HALT: 
#ifdef INTERRUPTS
      if(R.IFF&0x01) { R.PC.W--;R.IFF|=0x80; }
#else
      printf("CPU HALTed and stuck at PC=%hX\n",R.PC.W-=2);
      CPURunning=0;
#endif
      break;
    default:
      if(TrapBadOps)
        printf
        (
          "Unrecognized instruction: DD %X at PC=%hX\n",
          M_RDMEM(R.PC.W-1),R.PC.W-2
        );
  }
#undef XX
}

// ****************************************************************************
//			FDコードの処理
// ****************************************************************************
static void CodesFD(void)
{
  register byte I;
  register pair J;

#define XX IY
  ClockCount-=cycles_xx[I=M_RDMEM(R.PC.W++)];
  switch(I)
  {
#include "CodesXX.h"
    case PFX_FD:
    case PFX_DD:
      R.PC.W--;break;
    case PFX_CB:
      CodesFDCB();break;
    case HALT:
#ifdef INTERRUPTS
      if(R.IFF&0x01) { R.PC.W--;R.IFF|=0x80; }
#else
      printf("CPU HALTed and stuck at PC=%hX\n",R.PC.W-=2);
      CPURunning=0;
#endif
      break;
    default:
        printf
        (
          "Unrecognized instruction: FD %X at PC=%hX\n",
          M_RDMEM(R.PC.W-1),R.PC.W-2
        );
  }
#undef XX
}

// ****************************************************************************
//          Z80 CPU をリセットする
// ****************************************************************************
void ResetZ80(void)
{
	R.PC.W=0x0000;R.SP.W=0xF000;
	R.AF.W=R.BC.W=R.DE.W=R.HL.W=0x0000;
	R.AF1.W=R.BC1.W=R.DE1.W=R.HL1.W=0x0000;
	R.IX.W=R.IY.W=0x0000;
	R.I=0x00;R.IFF=0x00;

	ClockCount = 0;
	save_intr=0xffff;

	CPURunning=1;
	TClock = 0;

	TintClockRemain = TimerInt_Clock;		// タイマ割り込み監視
	CmtClockRemain = CmtInt_Clock;			// CMT割り込み監視
}

// ****************************************************************************
//			クロック周波数を得る
// ****************************************************************************
long GetClock(void)
{
	return z80_clock;
}

// ****************************************************************************
//			Z80が動作する事ができる水平ライン数
// ****************************************************************************
void SetValidLine(long Line)
{
	Valid_Line = 262 - Line;
}


// ****************************************************************************
//			Z80が動作する事ができる水平ライン数    (SR専用)
// ****************************************************************************
void SetValidLine_sr(long Line)
{
	SetValidLine( Line);
	if( !busreq && (P6Version== PC60M2SR || P6Version== PC66SR))
		{
		 Valid_Line += srline;
		 if( Valid_Line >262)
		 	 Valid_Line =262;
		}
}

// ****************************************************************************
//					クロック周波数を設定
// ****************************************************************************
// 合わせてタイマ・CMT割り込み周期も算出する
void SetClock(long clock)
{
	z80_clock = clock;

	// 4MHzの場合、2083に近い値になる?
	BaseTimerClock = (long)( ((float)(clock/(60*262)) * (float)(Valid_Line)) / 8.533332992f);
	//  TimerInt_Clock = BaseTimerClock;
	SetTimerIntClock( portF6);	// TimerIntClock の初期化 (上記の代替処理)

	//printf("basetimerclock=%d \n",BaseTimerClock);
	//printf("TimerInt_Clock %d\n",TimerInt_Clock);
	//	CmtInt_Clock = ((clock/(60*262)) * Valid_Line) / 2;
	//	CmtInt_Clock = CmtInt_Clock * 2;		// ← 暫定的処理！

	CmtInt_Clock = 33333; 	//  (常に固定)
}

// ****************************************************************************
//				タイマー割り込みのカウント
// ****************************************************************************
// 推測:初期値は3で、この時2msec。たぶん、4count = 2msec。
// SR では、初期値が　127なので、おそらく、127で、2msになる。
void SetTimerIntClock(long pow)
{
	static int sr_mode_bak=-1;

	if(sr_mode != sr_mode_bak)	/* sr_mode が変わっていたら、一応 powを初期化する 2004/2/3*/
		{
		 if(!sr_mode) 
		 	pow= 3;
		}

	if(!sr_mode)
	  TimerInt_Clock = BaseTimerClock * (pow + 1) / 4 ;
	else
	  TimerInt_Clock = BaseTimerClock * (double)(pow + 1) / 128.0 ; // 暫定的処理!! 2002/10/18
//    TimerInt_Clock *=1.15;

#if 1
	if(!sr_mode)
        TimerInt_Clock *= 1.073;    // 10000 回ループでは、このぐらいの数字になる 2008/6/29
    //else
      //  TimerInt_Clock *= 0.94;     // 10000 回ループでは、このぐらいの数字になる 2008/7/28
        
#endif

	sr_mode_bak = sr_mode;
	printf("TimerInt_Clock %d\n",TimerInt_Clock);
}


// ****************************************************************************
//			これまでに実行したマシンカウントを取得する
// ****************************************************************************
long GetTClock(void)
{
	long retValue = TClock;
	TClock = 0;
	return retValue;
}

/*extern int TimerSWFlag;*/
int TimerSWFlag = 1;		/* 0: TIMER 割り込みを強制的に発生しないようにする.*/
int WaitFlag = 1;
void unixIdle();
extern int IntSW_F3;
long StartCount;

// ****************************************************************************
//			Z80 CPU を一画面分だけ実行する
//		Out: 現在のPCアドレス
// ****************************************************************************
word Z80(void)
{
  // all variable is static !
  static int i;
/*  static int  FastTapeRepeat=1;
  //static byte I, j;
  static pair J;

  static int HCount = 0;
  static int	NowClock;
  static int hline;
*/
  PRINTDEBUG(CPU_LOG, "[Z80] ------------------------ 1 screen start ---------------------------------------------- \n");

#ifdef DEBUG
  if( inTrace ) goto AFTER_TRACE;	// return from debbuging mode
#endif

	CPURunning=1;
    FastTapeRepeat = 1;

	if( UseCPUThread == 2)         // 60fps Timer で駆動する
		{
		if(CasMode && FastTape)
			FastTapeRepeat = 100;
		}
	else 
		{
    	if( CasMode && FastTape)
	   	   UPeriod = 59;		// FAST TAPE
	    else
		   UPeriod = UPeriod_bak;	// Restore UPeriod
		}


    for (i=0 ; i< FastTapeRepeat ; i++)
	{
		
    // 一画面分の水平走査線 - Z80がBUSREQで停止している走査線数 = Z80稼動走査線数
	ClockCount += z80_clock/(60*262);	// 1水平ライン分のクロック数


	for (hline = 0; hline < Valid_Line; /*hline++*/) 
		{

		// 1水平ライン分の処理を開始する
      	while(ClockCount > 0)
			{
#ifdef DEBUG
			//	if(R.PC.W == Trap)
			//		{Trace=1; }
			/* Turn tracing on when reached trap address */
				// ブレークポイント＝PCなら、デバッガ開く
			int i;
			if( enable_breakpoint)
				for(i=1; i< MAX_BREAKPOINT ; i++)
					if(R.PC.W==breakpoint[i].addr && breakpoint[i].action == B_PC  &&  breakpoint[i].enable ==1)
						{Trace=1; break; }


			/* Debugging start */
			if (Trace) {
				inTrace = DEBUG_START;
				return R.PC.W;
				}


AFTER_TRACE:
			inTrace = DEBUG_NONE;
#endif

			NowClock = exec1();   /* 命令を１つデコードして、実行 */
			Event_Update( NowClock);


			IntrCtrl( NowClock);  /* 処理クロックの減算と、割り込み要求の発生 */
			
//			ym2203_Count( 15);
			
			if ((TimerIntFlag == INTFLAG_REQ) ||
		    	(CmtIntFlag == INTFLAG_REQ) ||
		    	(KeyIntFlag == INTFLAG_REQ))
					break;

			} /* while(ClockCount > 0) { */


      /* １画面分の有効スキャンラインにかかる処理時間分を実行したら
	 	画面の更新処理 */
      /* 262は表示する画面のスキャンライン数 */
		if (ClockCount <= 0)
			{
			hline++;
			ClockCount += z80_clock/(60*262);
			  // キーボード状態をチェックする
	    	Keyboard();
			if (++HCount == 262) 
			   {
				HCount = 0;
				// キーボード状態をチェックする
		    	/*Keyboard();*/
		  // ------------- VRTC  interrupt ----------------- 2002/4/21
				if( sr_mode )
					{
				 	if( portFA & 0x10 )	// VRTC Enable ?
						VrtcIntFlag = INTFLAG_NONE;           // Disable
				 	else
						VrtcIntFlag =INTFLAG_REQ;  // VRTC Interrupt request
    				}
				/* 割り込みを発生させる
				（他の割り込み処理を帰線期間で検出させるため） */
			    IFlag=1;
			   }
			}

		if(IFlag)	/* 割り込み判定する？ */
      		{
			if(!CPURunning) 
				return(R.PC.W); /*break;*/ /* add */
			IFlag=0;
			J.W=Interrupt();	/* get Interrupt vector */

			if(((J.W!=0xFFFF)&&(R.IFF&0x01))||(J.W==0x0066)) /* 割り込み有り */
				{
				/* which interrupt? */
				if (J.W == INTADDR_TIMER) 
					TimerIntFlag = INTFLAG_NONE;
				else if (J.W == INTADDR_CMTREAD) 
					CmtIntFlag = INTFLAG_EXEC;
				else if (J.W == INTADDR_STRIG) 
					StrigIntFlag = INTFLAG_EXEC;
				else if ((J.W == INTADDR_KEY1) || (J.W == INTADDR_KEY2))
				    KeyIntFlag = INTFLAG_EXEC;
			    // *************** PC-6601SR ********************** 2002/4/8
				else if(J.W == INTADDR_DATE) 
					DateIntFlag = INTFLAG_EXEC;  // date
				else if(J.W == INTADDR_TVR )  
					TvrIntFlag = INTFLAG_EXEC;  // TV Reserve
				else if(J.W == INTADDR_VOICE) 
					VoiceIntFlag = INTFLAG_EXEC; // voice
		     	// ********************************************************
				else if(J.W == INTADDR_VRTC) 
					VrtcIntFlag = INTFLAG_NONE; // VRTC 優先でない
		  		VrtcIntFlag=INTFLAG_NONE;

				/* Experimental V Shouldn't disable all interrupts? */
				R.IFF=(R.IFF&0xBE)|((R.IFF&0x01)<<6);
				if(R.IFF&0x80)		// HALT命令実行中のときは、PCを進めてから、割り込み処理へ
					{ R.PC.W++;R.IFF&=0x7F; }

#if (CPU_LOG && defined( DEBUG))
				if( code_log_flag)
					{
					PRINTDEBUG( CPU_LOG,"[Z80][Z80]  Interrupt Execute :");
					if( 0) {}
					else if( J.W == INTADDR_TIMER)   {PRINTDEBUG( CPU_LOG,"(Timer) \n");}
					else if( J.W == INTADDR_CMTREAD) {PRINTDEBUG( CPU_LOG,"(CMT) \n");}
					else if( J.W == INTADDR_STRIG)   {PRINTDEBUG( CPU_LOG,"(STRIG) \n");}
					else if( J.W == INTADDR_KEY1)    {PRINTDEBUG( CPU_LOG,"(KEY1) \n");}
					else if( J.W == INTADDR_KEY2)    {PRINTDEBUG( CPU_LOG,"(KEY2) \n");}
					else if( J.W == INTADDR_VRTC)    {PRINTDEBUG( CPU_LOG,"(VRTC) \n");}
					else if( J.W == INTADDR_VOICE)   {PRINTDEBUG( CPU_LOG,"(VOICE) \n");}
					else PRINTDEBUG( CPU_LOG,"\n");
					}
#endif

				M_PUSH(PC);		// 戻りアドレスを push

				if(J.W==0x0066)
					 R.PC.W=0x0066; /* NMI */
				else
		    		if(R.IFF&0x04) /* IM 2 */
						{
		    			J.W&=0xFE;J.B.h=R.I;
						R.PC.B.l=M_RDMEM(J.W++);	// 割り込み処理ルーチンにJP
		    			R.PC.B.h=M_RDMEM(J.W);
		    			}
		    		else
		      			if(R.IFF&0x02) 
							R.PC.W=0x0038; /* IM 1 */
		      			else 
							R.PC.W=J.W; /* IM 0 */
		 		} /* if(((J.W!=0xFFFF)&&(R.IFF&0x01))||(J.W==0x0066))*/
      		}   /*  if(IFlag) */
    	} /* for (hline = 0; hline < Valid_Line; hline++) */

  	} /* for(;;) */
	PRINTDEBUG(CPU_LOG, "[Z80] ------------------------ 1 screen end ---------------------------------------------- \n");

  return(R.PC.W);
}


word getPC(void)
{
 return ( R.PC.W);
}

word getB(void)
{
 return ( R.BC.B.h);
}



// ****************************************************************************
//          exec1: 命令を1つデコードして、実行する
//   Out: 実行したクロック数を返却
// ****************************************************************************
int exec1(void)
{
	static int flag= 0;
	
	register byte I, j;
	register pair J;
	long NowClock;
  
	StartCount = ClockCount;
	if (R.PC.W == nowait_start_addr) WaitFlag = 0;
	if (R.PC.W == nowait_end_addr)   WaitFlag = 1;


//	if( peek_memory(0xfa5e)*256+peek_memory(0xfa5d)==390 ) { code_log_flag = 1;}
//	static int ADA1=0;
//	if( R.PC.W == 0x1B06) {code_log_flag =1; ADA1=1;}
//	if( R.PC.W == 0x1A6D) {code_log_flag =0; ADA1=0;}
//	if( R.PC.W == 0xADA1) {code_log_flag =1; ADA1=1;}
//	if( R.PC.W == 0xADA0 && ADA1==1) code_log_flag=1;	// CPUの動きを記録するようにする for PACMAN
//	if( R.PC.W == 0xAD94 && ADA1==1) code_log_flag=0;	// 
//	if( R.PC.W == 0xf903) { code_log_flag=1; Trap=0x6b11;}	// 
//	if( R.PC.W == 0xf9b2) set_fd_send(1);    // こら娘で、データ送る前に0EHを送ってないため動かない。強制的に、fd_send を１にすると、動く。
//    if(R.PC.W == 0x442) 		dokodemo_load("/Users/windy/aaa.dds");
	
	//if( R.PC.W == 0xdf4d) code_log_flag = 1;
//	if( R.PC.W >= 0xe000 && R.PC.W <=0xefff ) code_log_flag = 1;
//	if( R.PC.W == 0xf903) code_log_flag = 1;
//	if (R.PC.W == 0xe400) code_log_flag = 1;
//	if (R.PC.W == 0xab00) code_log_flag = 0;
//	if (R.PC.W == 0x5a05) code_log_flag = 1;
//	if (R.PC.W == 0x59d4) code_log_flag = 0;

//	if (R.PC.W == 0x4f85) code_log_flag = 1;
//	if (R.PC.W == 0x442)  code_log_flag = 0;

#if CPU_LOG && defined(DEBUG)
	if( R.PC.W == 0xEB5) { PRINTDEBUG(KEY_LOG,"[Z80][exec1] keyin Interrupt start\n"); flag =1;}
	if( R.PC.W == 0xEB9) { PRINTDEBUG(KEY_LOG,"[Z80][exec1] get key from SUB CPU\n");}
	if( R.PC.W == 0xF15) { PRINTDEBUG2(KEY_LOG,"[Z80][exec1] send keydata to p6 keybuffer [%02X (%c)] \n", R.DE.B.l &0xff ,R.DE.B.l &0xff);}
	if( R.PC.W == 0xF30) { if(flag) PRINTDEBUG(KEY_LOG,"[Z80][exec1] keyin Interrupt END \n"); flag=0;}

	//if( R.PC.W == 0x64b9) { R.AF.B.h = 0x0b; }
	//if( R.PC.W== 0x6441) { code_log_flag = 1; }

	if(code_log_flag)
		{
		static char buff[256];
		static char machineCode[256];
		static char comment[20];
		buff[0]=0;
		DAsm( buff, machineCode, comment , R.PC.W);
		PRINTDEBUG2(CPU_LOG,"[Z80][exec1] PC:0x%04X  %-15s ",R.PC.W ,buff);
		PRINTDEBUG5(CPU_LOG,"AF:%04X BC:%04X DE:%04X HL:%04X SP:%04X",R.AF.W ,R.BC.W ,R.DE.W ,R.HL.W ,R.SP.W);
		PRINTDEBUG1(CPU_LOG," Line:%d\n", peek_memory(0xfa5e)*256+peek_memory(0xfa5d));
	}
#endif

	switch((j=M_RDMEM(R.PC.W++)))	// 命令 fetch
		{
#include "Codes.h"							// 通常命令

		case PFX_CB: CodesCB();break; // 2バイト命令
		case PFX_ED: CodesED();break;
		case PFX_FD: CodesFD();break;
		case PFX_DD: CodesDD();break;
		case HALT:
		   		ClockCount=0;
#ifdef INTERRUPTS
		   		/*if(R.IFF&0x01)*/
			    { R.PC.W--;R.IFF|=0x80; }
#else
		   		printf("CPU HALTed and stuck at PC=%hX\n",--R.PC.W);
		   		CPURunning=0;
#endif
		   		R.IFF|=0x80;
		   		break;
		default:
		   		if(TrapBadOps)
		   			printf
					(
		 			"Unrecognized instruction: %X at PC=%hX\n",
		 			M_RDMEM(R.PC.W-1),R.PC.W-1
		 			);
		}


	ClockCount-=cycles_main[j]+1;			/* 残クロック数 */
	NowClock = StartCount - ClockCount;		/* 実行したクロック数 */
	return( NowClock);						/* 実行したクロック数　返す */
}

// ****************************************************************************
//          IntrCtrl: 処理クロックの減算と、割り込み要求の発生
// ****************************************************************************
void IntrCtrl(long NowClock)
{
	
	/* 実行したクロック数を保存する */
	TClock += NowClock;
	
	/* タイマー割り込みクロック数の計算 */
	TintClockRemain-= NowClock;
	
	/* CMT割り込みクロック数の計算 */
	CmtClockRemain-= NowClock;

		/* CMT割り込みの発生許可？ */
	if (CmtClockRemain <= 0) 
		{
		/* 割り込み許可状態？ */
		if (R.IFF&0x01)
			{
	   		IFlag = 1;
			CmtClockRemain = CmtInt_Clock;
	   		if (CasMode) CmtIntFlag = INTFLAG_REQ;
	 			// enableCmtInt();
	   		}
		}

	/* タイマー割り込みの発生？ */
	if (TintClockRemain <= 0 && CmtIntFlag != INTFLAG_REQ)
		{	// テープ動作中のみ、割り込みをマスク可能にする、かなり強引な処置
		if (((R.IFF&0x01)))
			{
			/*TintClockRemain += TimerInt_Clock;*/
			TintClockRemain = TimerInt_Clock;

			if(TimerSW && (TimerSWFlag && TimerSW_F3))
	   			{
#if CPU_LOG
			if( code_log_flag) {PRINTDEBUG1(CPU_LOG,"[Z80][Z80] TIMER interrupt REQUEST  PC:%04X \n",getPC());}
#endif
		 		TimerIntFlag = INTFLAG_REQ;
				IFlag=1;
       			}
			else
				{
#if CPU_LOG				
			if( code_log_flag) PRINTDEBUG1(CPU_LOG,"[Z80][Z80] TIMER interrupt can't REQUEST (Masked Interrupt) PC:%04X\n",getPC());
#endif
				}
    		}
		}

	// テープ割り込みと、タイマー割り込みが重なったときに、テープ割り込みを優先しないと、
	// ドアドアmk2でロードできなくなるので、
	// 順番を入れ替えました。 2005/6/6 
}





/****************************************************************/
/*** Refresh screen, check keyboard and sprites. Call this    ***/
/*** function on each interrupt.                              ***/
/****************************************************************/

// ****************************************************************************
//          Interrupt: 割り込みが発生しているか検出して、対応する割り込みインデックスを返却する
//  Out: 割り込み発生していたら、割り込みインデックス
// ****************************************************************************
word Interrupt(void)
{
	/* interrupt priority (PC-6601SR) */
	/* 1.Timer     , 2.subCPU    , 3.Voice     , 4.VRTC      */
	/* 5.RS-232C   , 6.Joy Stick , 7.EXT INT   , 8.Printer   */

			// IntSW_F3 と TimerSW_F3 を条件に追加 2003/9/16 Windy
	if (TimerIntFlag == INTFLAG_REQ  && IntSW_F3 && TimerSW_F3)
	  	return(INTADDR_TIMER); /* timer interrupt */
	else if (CasMode && (p6key == 0xFA) && (keyGFlag == 1))
    	return(INTADDR_CMTSTOP); /* Press STOP while CMT Load or Save */
	else if ((CasMode==CAS_LOADING) && (CmtIntFlag == INTFLAG_REQ)) 
	    {
	    /* CMT Loading */
	    CmtIntFlag = INTFLAG_NONE;
	    if(!feof(CasStream[0])) 
	       { 				/* if not EOF then Interrupt to Load 1 byte */
	        CasMode=CAS_LOADBYTE;
			//printf("CMT Interrupt requested \n");	// test
	        return(INTADDR_CMTREAD);
	       } else { 		/* if EOF then Error */
            printf("tape file reached EOF\n");
			/* テープが最後まで行っても、実際にはそのまま待ち続ける。。*/
            //CasMode=CAS_NONE;
            //return(INTADDR_CMTSTOP); /* Break */  /* do nothing!  Windy 2008/10/5 */
       	    return(INT_NONE);
       	   }
		}
    else if ((StrigIntFlag == INTFLAG_REQ) && IntSW_F3) /* if command 6 */
	    return(INTADDR_STRIG);
	else if ((KeyIntFlag == INTFLAG_REQ) && IntSW_F3) /* if any key pressed */
	    if (keyGFlag == 0) 
        	return(INTADDR_KEY1); /* normal key */
	    else 
        	return(INTADDR_KEY2); /* special key (graphic key, etc.) */

  // ***************** PC-6601SR ************************* 2002/4/8 ( by Windy)
	else if( DateIntFlag == INTFLAG_REQ && DateMode ==DATE_READ) //date interrupt
    	return( INTADDR_DATE);
	else if( TvrIntFlag   == INTFLAG_REQ && TvrMode ==TVR_READ) 
    	return( INTADDR_TVR);
	else if( VrtcIntFlag == INTFLAG_REQ ) 	// VRTC interrupt 2002/4/21
		return( INTADDR_VRTC);
	else if( VoiceIntFlag == INTFLAG_REQ)	// voice interrupt
		return  INTADDR_VOICE;
  // ***************************************************************
	else /* none */
  		return(INT_NONE);
}


word  ExitedAddr;						// exit code from Z80()
int   CPUThreadRun;         			// CPU Thread  1: run
static int  fps1;				// frame per second

extern int VoiceFlag;

// ****************************************************************************
//        CPUThreadProc: CPU スレッドのループ 
//     ・メッセージの来てない間を縫って、一画面のみ動かす
//     ・別スレッドでずっと動かす
//     ・60msタイマーで一画面のみ動かす。
// ****************************************************************************
//static DWORD WINAPI CPUThreadProc(LPVOID lpParameter)
dword CPUThreadProc(char * lpParameter)
    {
   // static int  delayed;
    static int  cnt=60;
    static int  frame_numbers=0;	// frame numbers
	static int  skip_frames=0;
	
	//int   interval;
	double next;
	double step= 1000.0/60.0;


	next  = OSD_GetTicks()+ step;
    do {
		//printf("UPeriod =%d \n",UPeriod );
		//printf("next=%f   %d  waitflag %d  UseCPUThread %d ＼n",next ,OSD_GetTicks() ,WaitFlag ,UseCPUThread);
		

        // -------- CPURunning が０だと、CPUは実行しない -----------
        if(CPURunning)
			{
			if( cnt-- )
				{				
#ifdef DEBUG
				if( inTrace != DEBUG_DOING )
#endif
		    		ExitedAddr= Z80();
#if CPU_LOG
					if( code_log_flag)
						PRINTDEBUG1( CPU_LOG, "[Z80][CPUThreadProc]    Z80()  EXITED ADDR = 0x%04X \n",ExitedAddr );
#endif
#ifdef DEBUG

				if( inTrace==DEBUG_DOING && animatemode )	// animate mode , push step button.
					{
					DebugCommand(&R , "s");
					}
				
				if( inTrace == DEBUG_START)
					{
					open_debug_dialog();  
					DebugStart(&R);
					if (DebugResult[0]) {
						DebugPutResult();
					    DisplayDisasm( );

						DebugResult[0]='\0';
						}
					inTrace = DEBUG_DOING;
					}
				else if( inTrace == DEBUG_DOING )
					{
						// debug command prompt  (1 moji nyuryoku  ,command execute) 2009/7/27
					DebugDo();
					}
#endif

				if(( cnt==1 && UPeriod==60)|| (UPeriod!=60)) 	// FAST TAPE  UPeriod=60 is 1fps
			 		{
                   	 if( OSD_GetTicks() < next && WaitFlag)  // WaitFlag=0 is no wait mode 
						{
						if( UpdateScreen())	// draw to hidden surface
						      {
								frame_numbers++;
							  }

						 if( UseCPUThread !=2) 
						    if( OSD_GetTicks() < next)
							{
								int s= next - OSD_GetTicks();
								//printf("sleep s = %d＼n",s);
#if CPU_LOG
								PRINTDEBUG1(CPU_LOG,"[Z80][CPUThreadProc] Sleep %d ms \n",s);
#endif
								if (!(CasMode && FastTape))
   									OSD_Delay(s);   // normal sleep
#if CPU_LOG
								PRINTDEBUG(CPU_LOG,"[Z80][CPUThreadProc] Sleep end \n");
#endif							
							}
						}
					  else
						{
						 skip_frames++;
						 if( skip_frames == 48 /*28　10*/)
							{
							 //printf("skip_frames =%d \n",skip_frames);
							 //skip_frames=0;
							 UpdateScreen();	// draw to hidden surface
							}
						}

					}
				next += step;
				}
			else
				{
				cnt=60;
		   		//printf("Updated=%d fps  UPeriod =%d \n",frame_numbers, UPeriod );
				fps1 = frame_numbers;
				printf("fps = %d  (skip:%d)  %02X \n" ,fps1, skip_frames ,stick0);
		   		frame_numbers=0;
		   		skip_frames=0;
		  		}

			}
		else
			{
			UpdateScreen();
			OSD_Delay(1);
			}
		} 
    while(  CPUThreadRun );

//	OSD_TrashTimer();



    return(0);
}


int Z80_Getfps(void)
{
	return (fps1);
}



//#ifndef WIN32
	/* 画面を更新する */
//	UpdateScreen();
//#endif
	/*	if( monitor_mode)	
			monitor(&R); */

    /* idle */
//	if (WaitFlag && UPeriod!=60) unixIdle();
