/**                                                         **/
/**                         Debug.c                         **/
/**                                                         **/
/** This file contains the built-in debugging routine for   **/
/** the Z80 emulator which is called on each Z80 step when  **/
/** Trap!=0.                                                **/
/**                                                         **/
/** Copyright (C) Marat Fayzullin 1995-1998                 **/
/**     You are not allowed to distribute this software     **/
/**     commercially. Please, notify me, if you make any    **/
/**     changes to this file.                               **/
/**                                                         **/
/** modified by Windy                                       **/
/*************************************************************/
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef DEBUG

#include "P6.h"
#include "types.h"
#include "Z80.h"
#include "keys.h"
#include "mem.h"
#include "Refresh.h"
#include "device.h"
#include "Video.h"
#include "Debug.h"
#include "voice.h"
#include "os.h"

#if HAVE_UNISTD_H
#include <unistd.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

int my_atoi( char *str);
void set_breakpoint( int action , int addr ,int idx ,int enable);
void chk_backlog(void);


//void ramdump(void);

// 

#define MAX_BACK_LOG 300
#define MAX_COMMANDLINES (30+12)
#define MAX_COMMANDCOWS  (61) // 63

#define  COMMANDLINE_TOP_YY  21

#define  MAX_DISASM_LINES 16


#define  MAX_DUMP_LINES   16
#define  DUMP_TOP_XX      (62)  
#define  DUMP_TOP_YY	  22

#define  STATUS_TOP_XX    (71+14+18)
#define  STATUS_TOP_YY	   1

#define  DISASM_TOP_XX    (40+14)
#define  DISASM_TOP_YY	   1

#define  IO_TOP_XX        62
#define  IO_TOP_YY	      40

#define  FUNCTION_TOP_XX  0
#define  FUNCTION_TOP_YY  (52+11)

#define MAX_CMD_HISTORY      20	/* 命令のヒストリー　最大数 */

static int com_xx=0;		/* コマンドライン　見た目上の座標　debug screen cursor's  XY */
static int com_yy=0;

static int com_top_yy =0;  		/* バックログ配列の　表示開始Y座標　left top Y */
static int com_back_log_yy = 0;    /* バックログ配列の　現在のY座業 */

static int com_top_yy_bak = 0;     /* com_top_yy のバックアップ　（バックログをたどる前の座標）*/
static int com_is_backscroll=0;		/* 1:バックスクロール中  0:通常*/


enum {DEFAULT_BANK=0 , RAM_BANK ,EXTRAM_BANK};

static int dump_is_ram= DEFAULT_BANK;	 /* dump list mode  0:DEFAULT_BANK 1:RAM_BANK  2:EXTRAM_BANK  */
static int dump_is_ram_bak=DEFAULT_BANK;	  
extern int ignore_padding_flag;   /* 1:ignore padding variable */

static int  dump_xx=0;			  /* dump xx */
static int  dump_yy=0;			  /* dump yy */
static word dump_start_adr=0;     /* dump start address */



static word disasm_start_adr=0;   /* disasm start address */
static word disasm_current_adr=0; /* disasm current address (PC) */

static int  disasm_is_backscroll=0; /* disasm is backscroll 1: backscroll 0:normal */

static int  disasm_pre_adr=0;    /* disasm pre line address */
static int  disasm_pre2_adr=0;   /* disasm pre line address */
static int  disasm_next_adr=0;	 /* disasm next line address */
static int  disasm_next2_adr=0;	 /* disasm next line address */

static int  disasm_xx=0;
static int  disasm_yy=0;

static word  disasm_cursor_adr=0;

byte debug_key;			/* debug_key */
int   debug_keydown=0; 


enum {P_COMMAND, P_DISASM , P_DUMP};	/* 各ペイン */
int current_pain = P_COMMAND;		/* 現在のペイン */

OSD_Surface* debug_surface = NULL;



unsigned char DebugResult[10000];

static char DebugResult_sub[1000];

static int at;      // index for strings

static int at2;     // index for attribute



static char commandLine[ MAX_BACK_LOG][256];	// command line

static char commandLineAttr[ MAX_BACK_LOG][256];	// command line Attribute

static char commandHistory[ MAX_CMD_HISTORY][256];	// command line history


int is_open_debug_dialog=0;  /* 1: open  0:close */

static int is_debug_rdmem_ram=0;    /* 1: read memmory from RAM  0: read memory from current bank */


char debugWorkPath[PATH_MAX]="";		// デバッグの作業パス

static reg pre_reg;		// 前回のレジスター


void  DebugCommandPrompt( void);
void  DebugDisasmPrompt( void);
void  DebugDumpPrompt( void);


void DisplayMemDump(void);
void DisplayStatus(reg *R);
void DisplayIO(void);
int  make_memdump( reg *R ,const char *S , int spc_flag);

void RefreshDebugString(void);
void DebugPutResult(void);
void DisplayDisasm(void);
void DebugPutString( int x, int y , unsigned char *str ,int max_x , char *attr);

void ClearAttr(void);
//void make_disasm(reg *R, const char *S);
void make_disasm(word start_adr, const char *S);
static void DisplayRegisters(reg *R);
void push_stacks(word stack_addr , int cmd ,int reg , word value ,word addr);
void pop_stacks(void);
void debug_savevram(int argc, char*argv[]);
void debug_settape(int argc, char *argv[]);
void debug_setnowait(int argc, char* argv[]);
void help_set(void);
void do_stacks( byte opcode1 , byte opcode2 , byte opcode3);
void debug_loadvram(int argc, char* argv[]);


static char *Mnemonics[256] =
{
  "NOP","LD BC,#h","LD (BC),A","INC BC","INC B","DEC B","LD B,*h","RLCA",
  "EX AF,AF'","ADD HL,BC","LD A,(BC)","DEC BC","INC C","DEC C","LD C,*h","RRCA",
  "DJNZ @h","LD DE,#h","LD (DE),A","INC DE","INC D","DEC D","LD D,*h","RLA",
  "JR @h","ADD HL,DE","LD A,(DE)","DEC DE","INC E","DEC E","LD E,*h","RRA",
  "JR NZ,@h","LD HL,#h","LD (#h),HL","INC HL","INC H","DEC H","LD H,*h","DAA",
  "JR Z,@h","ADD HL,HL","LD HL,(#h)","DEC HL","INC L","DEC L","LD L,*h","CPL",
  "JR NC,@h","LD SP,#h","LD (#h),A","INC SP","INC (HL)","DEC (HL)","LD (HL),*h","SCF",
  "JR C,@h","ADD HL,SP","LD A,(#h)","DEC SP","INC A","DEC A","LD A,*h","CCF",
  "LD B,B","LD B,C","LD B,D","LD B,E","LD B,H","LD B,L","LD B,(HL)","LD B,A",
  "LD C,B","LD C,C","LD C,D","LD C,E","LD C,H","LD C,L","LD C,(HL)","LD C,A",
  "LD D,B","LD D,C","LD D,D","LD D,E","LD D,H","LD D,L","LD D,(HL)","LD D,A",
  "LD E,B","LD E,C","LD E,D","LD E,E","LD E,H","LD E,L","LD E,(HL)","LD E,A",
  "LD H,B","LD H,C","LD H,D","LD H,E","LD H,H","LD H,L","LD H,(HL)","LD H,A",
  "LD L,B","LD L,C","LD L,D","LD L,E","LD L,H","LD L,L","LD L,(HL)","LD L,A",
  "LD (HL),B","LD (HL),C","LD (HL),D","LD (HL),E","LD (HL),H","LD (HL),L","HALT","LD (HL),A",
  "LD A,B","LD A,C","LD A,D","LD A,E","LD A,H","LD A,L","LD A,(HL)","LD A,A",
  "ADD B","ADD C","ADD D","ADD E","ADD H","ADD L","ADD (HL)","ADD A",
  "ADC B","ADC C","ADC D","ADC E","ADC H","ADC L","ADC (HL)","ADC A",
  "SUB B","SUB C","SUB D","SUB E","SUB H","SUB L","SUB (HL)","SUB A",
  "SBC B","SBC C","SBC D","SBC E","SBC H","SBC L","SBC (HL)","SBC A",
  "AND B","AND C","AND D","AND E","AND H","AND L","AND (HL)","AND A",
  "XOR B","XOR C","XOR D","XOR E","XOR H","XOR L","XOR (HL)","XOR A",
  "OR B","OR C","OR D","OR E","OR H","OR L","OR (HL)","OR A",
  "CP B","CP C","CP D","CP E","CP H","CP L","CP (HL)","CP A",
  "RET NZ","POP BC","JP NZ,#h","JP #h","CALL NZ,#h","PUSH BC","ADD *h","RST 00h",
  "RET Z","RET","JP Z,#h","PFX_CB","CALL Z,#h","CALL #h","ADC *h","RST 08h",
  "RET NC","POP DE","JP NC,#h","OUTA (*h)","CALL NC,#h","PUSH DE","SUB *h","RST 10h",
  "RET C","EXX","JP C,#h","INA (*h)","CALL C,#h","PFX_DD","SBC *h","RST 18h",
  "RET PO","POP HL","JP PO,#h","EX HL,(SP)","CALL PO,#h","PUSH HL","AND *h","RST 20h",
  "RET PE","LD PC,HL","JP PE,#h","EX DE,HL","CALL PE,#h","PFX_ED","XOR *h","RST 28h",
  "RET P","POP AF","JP P,#h","DI","CALL P,#h","PUSH AF","OR *h","RST 30h",
  "RET M","LD SP,HL","JP M,#h","EI","CALL M,#h","PFX_FD","CP *h","RST 38h"
};

static char *MnemonicsCB[256] =
{
  "RLC B","RLC C","RLC D","RLC E","RLC H","RLC L","RLC (HL)","RLC A",
  "RRC B","RRC C","RRC D","RRC E","RRC H","RRC L","RRC (HL)","RRC A",
  "RL B","RL C","RL D","RL E","RL H","RL L","RL (HL)","RL A",
  "RR B","RR C","RR D","RR E","RR H","RR L","RR (HL)","RR A",
  "SLA B","SLA C","SLA D","SLA E","SLA H","SLA L","SLA (HL)","SLA A",
  "SRA B","SRA C","SRA D","SRA E","SRA H","SRA L","SRA (HL)","SRA A",
  "SLL B","SLL C","SLL D","SLL E","SLL H","SLL L","SLL (HL)","SLL A",
  "SRL B","SRL C","SRL D","SRL E","SRL H","SRL L","SRL (HL)","SRL A",
  "BIT 0,B","BIT 0,C","BIT 0,D","BIT 0,E","BIT 0,H","BIT 0,L","BIT 0,(HL)","BIT 0,A",
  "BIT 1,B","BIT 1,C","BIT 1,D","BIT 1,E","BIT 1,H","BIT 1,L","BIT 1,(HL)","BIT 1,A",
  "BIT 2,B","BIT 2,C","BIT 2,D","BIT 2,E","BIT 2,H","BIT 2,L","BIT 2,(HL)","BIT 2,A",
  "BIT 3,B","BIT 3,C","BIT 3,D","BIT 3,E","BIT 3,H","BIT 3,L","BIT 3,(HL)","BIT 3,A",
  "BIT 4,B","BIT 4,C","BIT 4,D","BIT 4,E","BIT 4,H","BIT 4,L","BIT 4,(HL)","BIT 4,A",
  "BIT 5,B","BIT 5,C","BIT 5,D","BIT 5,E","BIT 5,H","BIT 5,L","BIT 5,(HL)","BIT 5,A",
  "BIT 6,B","BIT 6,C","BIT 6,D","BIT 6,E","BIT 6,H","BIT 6,L","BIT 6,(HL)","BIT 6,A",
  "BIT 7,B","BIT 7,C","BIT 7,D","BIT 7,E","BIT 7,H","BIT 7,L","BIT 7,(HL)","BIT 7,A",
  "RES 0,B","RES 0,C","RES 0,D","RES 0,E","RES 0,H","RES 0,L","RES 0,(HL)","RES 0,A",
  "RES 1,B","RES 1,C","RES 1,D","RES 1,E","RES 1,H","RES 1,L","RES 1,(HL)","RES 1,A",
  "RES 2,B","RES 2,C","RES 2,D","RES 2,E","RES 2,H","RES 2,L","RES 2,(HL)","RES 2,A",
  "RES 3,B","RES 3,C","RES 3,D","RES 3,E","RES 3,H","RES 3,L","RES 3,(HL)","RES 3,A",
  "RES 4,B","RES 4,C","RES 4,D","RES 4,E","RES 4,H","RES 4,L","RES 4,(HL)","RES 4,A",
  "RES 5,B","RES 5,C","RES 5,D","RES 5,E","RES 5,H","RES 5,L","RES 5,(HL)","RES 5,A",
  "RES 6,B","RES 6,C","RES 6,D","RES 6,E","RES 6,H","RES 6,L","RES 6,(HL)","RES 6,A",
  "RES 7,B","RES 7,C","RES 7,D","RES 7,E","RES 7,H","RES 7,L","RES 7,(HL)","RES 7,A",
  "SET 0,B","SET 0,C","SET 0,D","SET 0,E","SET 0,H","SET 0,L","SET 0,(HL)","SET 0,A",
  "SET 1,B","SET 1,C","SET 1,D","SET 1,E","SET 1,H","SET 1,L","SET 1,(HL)","SET 1,A",
  "SET 2,B","SET 2,C","SET 2,D","SET 2,E","SET 2,H","SET 2,L","SET 2,(HL)","SET 2,A",
  "SET 3,B","SET 3,C","SET 3,D","SET 3,E","SET 3,H","SET 3,L","SET 3,(HL)","SET 3,A",
  "SET 4,B","SET 4,C","SET 4,D","SET 4,E","SET 4,H","SET 4,L","SET 4,(HL)","SET 4,A",
  "SET 5,B","SET 5,C","SET 5,D","SET 5,E","SET 5,H","SET 5,L","SET 5,(HL)","SET 5,A",
  "SET 6,B","SET 6,C","SET 6,D","SET 6,E","SET 6,H","SET 6,L","SET 6,(HL)","SET 6,A",
  "SET 7,B","SET 7,C","SET 7,D","SET 7,E","SET 7,H","SET 7,L","SET 7,(HL)","SET 7,A"
};

static char *MnemonicsED[256] =
{
  "DB EDh,00h","DB EDh,01h","DB EDh,02h","DB EDh,03h",	// 00
  "DB EDh,04h","DB EDh,05h","DB EDh,06h","DB EDh,07h",
  "DB EDh,08h","DB EDh,09h","DB EDh,0Ah","DB EDh,0Bh",
  "DB EDh,0Ch","DB EDh,0Dh","DB EDh,0Eh","DB EDh,0Fh",
  "DB EDh,10h","DB EDh,11h","DB EDh,12h","DB EDh,13h",	// 10
  "DB EDh,14h","DB EDh,15h","DB EDh,16h","DB EDh,17h",
  "DB EDh,18h","DB EDh,19h","DB EDh,1Ah","DB EDh,1Bh",
  "DB EDh,1Ch","DB EDh,1Dh","DB EDh,1Eh","DB EDh,1Fh",
  "DB EDh,20h","DB EDh,21h","DB EDh,22h","DB EDh,23h",	// 20
  "DB EDh,24h","DB EDh,25h","DB EDh,26h","DB EDh,27h",
  "DB EDh,28h","DB EDh,29h","DB EDh,2Ah","DB EDh,2Bh",
  "DB EDh,2Ch","DB EDh,2Dh","DB EDh,2Eh","DB EDh,2Fh",
  "DB EDh,30h","DB EDh,31h","DB EDh,32h","DB EDh,33h",	// 30
  "DB EDh,34h","DB EDh,35h","DB EDh,36h","DB EDh,37h",
  "DB EDh,38h","DB EDh,39h","DB EDh,3Ah","DB EDh,3Bh",
  "DB EDh,3Ch","DB EDh,3Dh","DB EDh,3Eh","DB EDh,3Fh",
  "IN B,(C)",  "OUT (C),B", "SBC HL,BC", "LD (#h),BC",	// 40
  "NEG"     ,  "RETN",      "IM 0",      "LD I,A",
  "IN C,(C)",  "OUT (C),C", "ADC HL,BC", "LD BC,(#h)",
  "DB EDh,4Ch","RETI",      "DB EDh,4Eh","LD R,A",
  "IN D,(C)",  "OUT (C),D", "SBC HL,DE","LD (#h),DE",	// 50
  "DB EDh,54h","DB EDh,55h","IM 1","LD A,I",
  "IN E,(C)",  "OUT (C),E", "ADC HL,DE","LD DE,(#h)",
  "DB EDh,5Ch","DB EDh,5Dh","IM 2","LD A,R",
  "IN H,(C)",  "OUT (C),H", "SBC HL,HL","LD (#h),HL",	// 60
  "DB EDh,64h","DB EDh,65h","DB EDh,66h","RRD",
  "IN L,(C)",  "OUT (C),L", "ADC HL,HL","LD HL,(#h)",
  "DB EDh,6Ch","DB EDh,6Dh","DB EDh,6Eh","RLD",
  "IN (HL),(C)","OUT (C),(HL)","SBC HL,SP","LD (#h),SP",	// 70 
  "DB EDh,74h","DB EDh,75h","DB EDh,76h","DB EDh,77h",
  "IN A,(C)",  "OUT (C),A", "ADC HL,SP","LD SP,(#h)",
  "DB EDh,7Ch","DB EDh,7Dh","DB EDh,7Eh","DB EDh,7Fh",
  "DB EDh,80h","DB EDh,81h","DB EDh,82h","DB EDh,83h",	// 80
  "DB EDh,84h","DB EDh,85h","DB EDh,86h","DB EDh,87h",
  "DB EDh,88h","DB EDh,89h","DB EDh,8Ah","DB EDh,8Bh",
  "DB EDh,8Ch","DB EDh,8Dh","DB EDh,8Eh","DB EDh,8Fh",
  "DB EDh,90h","DB EDh,91h","DB EDh,92h","DB EDh,93h",	// 90
  "DB EDh,94h","DB EDh,95h","DB EDh,96h","DB EDh,97h",
  "DB EDh,98h","DB EDh,99h","DB EDh,9Ah","DB EDh,9Bh",
  "DB EDh,9Ch","DB EDh,9Dh","DB EDh,9Eh","DB EDh,9Fh",
  "LDI"       ,"CPI",       "INI",       "OUTI",		// A0
  "DB EDh,A4h","DB EDh,A5h","DB EDh,A6h","DB EDh,A7h",
  "LDD"       ,"CPD",       "IND",       "OUTD",
  "DB EDh,ACh","DB EDh,ADh","DB EDh,AEh","DB EDh,AFh",
  "LDIR"      ,"CPIR",      "INIR",      "OTIR",		// B0
  "DB EDh,B4h","DB EDh,B5h","DB EDh,B6h","DB EDh,B7h",
  "LDDR"      ,"CPDR",      "INDR",      "OTDR",
  "DB EDh,BCh","DB EDh,BDh","DB EDh,BEh","DB EDh,BFh",
  "DB EDh,C0h","DB EDh,C1h","DB EDh,C2h","DB EDh,C3h",	// C0
  "DB EDh,C4h","DB EDh,C5h","DB EDh,C6h","DB EDh,C7h",
  "DB EDh,C8h","DB EDh,C9h","DB EDh,CAh","DB EDh,CBh",
  "DB EDh,CCh","DB EDh,CDh","DB EDh,CEh","DB EDh,CFh",
  "DB EDh,D0h","DB EDh,D1h","DB EDh,D2h","DB EDh,D3h",	// D0
  "DB EDh,D4h","DB EDh,D5h","DB EDh,D6h","DB EDh,D7h",
  "DB EDh,D8h","DB EDh,D9h","DB EDh,DAh","DB EDh,DBh",
  "DB EDh,DCh","DB EDh,DDh","DB EDh,DEh","DB EDh,DFh",
  "DB EDh,E0h","DB EDh,E1h","DB EDh,E2h","DB EDh,E3h",	// E0
  "DB EDh,E4h","DB EDh,E5h","DB EDh,E6h","DB EDh,E7h",
  "DB EDh,E8h","DB EDh,E9h","DB EDh,EAh","DB EDh,EBh",
  "DB EDh,ECh","DB EDh,EDh","DB EDh,EEh","DB EDh,EFh",
  "DB EDh,F0h","DB EDh,F1h","DB EDh,F2h","DB EDh,F3h",	// F0
  "DB EDh,F4h","DB EDh,F5h","DB EDh,F6h","DB EDh,F7h",
  "DB EDh,F8h","DB EDh,F9h","DB EDh,FAh","DB EDh,FBh",
  "DB EDh,FCh","DB EDh,FDh","DB EDh,FEh","DB EDh,FFh"
};

static char *MnemonicsXX[256] =
{
  "NOP",      "LD BC,#h", "LD (BC),A", "INC BC", "INC B",      "DEC B",    "LD B,*h",        "RLCA",	// 00
  "EX AF,AF'","ADD I%,BC","LD A,(BC)", "DEC BC", "INC C",      "DEC C",    "LD C,*h",        "RRCA",
  "DJNZ @h",  "LD DE,#h", "LD (DE),A", "INC DE", "INC D",      "DEC D",    "LD D,*h",        "RLA",		// 10
  "JR @h",    "ADD I%,DE","LD A,(DE)", "DEC DE", "INC E",      "DEC E",    "LD E,*h",        "RRA",
  "JR NZ,@h", "LD I%,#h", "LD (#h),I%","INC I%", "INC I%h",    "DEC I%h",  "LD I%h,*h",      "DAA",		// 20
  "JR Z,@h",  "ADD I%,I%","LD I%,(#h)","DEC I%", "INC I%l",    "DEC I%l",  "LD I%l,*h",      "CPL",
  "JR NC,@h", "LD SP,#h", "LD (#h),A", "INC SP", "INC (I%+^h)","DEC (I%+^h)","LD (I%+^h),*h","SCF",		// 30
  "JR C,@h",  "ADD I%,SP","LD A,(#h)", "DEC SP", "INC A",      "DEC A",    "LD A,*h",        "CCF",
  "LD B,B",   "LD B,C",   "LD B,D",    "LD B,E", "LD B,I%h",   "LD B,I%l", "LD B,(I%+^h)",   "LD B,A",	// 40
  "LD C,B",   "LD C,C",   "LD C,D",    "LD C,E", "LD C,I%h",   "LD C,I%l", "LD C,(I%+^h)",   "LD C,A",
  "LD D,B",   "LD D,C",   "LD D,D",    "LD D,E", "LD D,I%h",   "LD D,I%l", "LD D,(I%+^h)",   "LD D,A",	// 50
  "LD E,B",   "LD E,C",   "LD E,D",    "LD E,E", "LD E,I%h",   "LD E,I%l", "LD E,(I%+^h)",   "LD E,A",
  "LD I%h,B", "LD I%h,C", "LD I%h,D",  "LD I%h,E","LD I%h,I%h","LD I%h,I%l","LD H,(I%+^h)", "LD I%h,A",	// 60
  "LD I%l,B", "LD I%l,C", "LD I%l,D",  "LD I%l,E","LD I%l,I%h","LD I%l,I%l","LD L,(I%+^h)", "LD I%l,A",
  "LD (I%+^h),B","LD (I%+^h),C",       "LD (I%+^h),D","LD (I%+^h),E","LD (I%+^h),H","LD (I%+^h),L","HALT","LD (I%+^h),A",	 // 70
  "LD A,B",   "LD A,C",   "LD A,D",    "LD A,E", "LD A,I%h",   "LD A,I%l",  "LD A,(I%+^h)", "LD A,A",
  "ADD B",    "ADD C",    "ADD D",     "ADD E",  "ADD I%h",    "ADD I%l",  "ADD (I%+^h)",   "ADD A",	// 80
  "ADC B",    "ADC C",    "ADC D",     "ADC E",  "ADC I%h",    "ADC I%l",  "ADC (I%+^h)",   "ADC,A",
  "SUB B",    "SUB C",    "SUB D",     "SUB E",  "SUB I%h",    "SUB I%l",  "SUB (I%+^h)",   "SUB A",	// 90
  "SBC B",    "SBC C",    "SBC D",     "SBC E",  "SBC I%h",    "SBC I%l",  "SBC (I%+^h)",   "SBC A",
  "AND B",    "AND C",    "AND D",     "AND E",  "AND I%h",    "AND I%l",  "AND (I%+^h)",   "AND A",	// A0
  "XOR B",    "XOR C",    "XOR D",     "XOR E",  "XOR I%h",    "XOR I%l",  "XOR (I%+^h)",   "XOR A",
  "OR B",     "OR C",     "OR D",      "OR E",   "OR I%h",     "OR I%l",   "OR (I%+^h)",    "OR A",		// B0
  "CP B",     "CP C",     "CP D",      "CP E",   "CP I%h",     "CP I%l",   "CP (I%+^h)",    "CP A",
  "RET NZ",   "POP BC",   "JP NZ,#h",  "JP #h",  "CALL NZ,#h", "PUSH BC",  "ADD *h",        "RST 00h",	// C0
  "RET Z",    "RET",      "JP Z,#h",   "PFX_CB", "CALL Z,#h",  "CALL #h",  "ADC *h",        "RST 08h",
  "RET NC",   "POP DE",   "JP NC,#h",  "OUTA (*h)","CALL NC,#h","PUSH DE", "SUB *h",        "RST 10h",	// D0
  "RET C",    "EXX",      "JP C,#h",   "INA (*h)","CALL C,#h", "PFX_DD",   "SBC *h",        "RST 18h",
  "RET PO",   "POP I%",   "JP PO,#h",  "EX I%,(SP)","CALL PO,#h","PUSH I%","AND *h",        "RST 20h",	// E0
  "RET PE",   "LD PC,I%", "JP PE,#h",  "EX DE,I%","CALL PE,#h","PFX_ED",   "XOR *h",        "RST 28h",
  "RET P",    "POP AF",   "JP P,#h",   "DI",     "CALL P,#h",  "PUSH AF",  "OR *h",         "RST 30h",	// F0
  "RET M",    "LD SP,I%", "JP M,#h",   "EI",     "CALL M,#h",  "PFX_FD",   "CP *h",         "RST 38h"
};

static char *MnemonicsXCB[256] =
{
  "LD B,RLC (I%@h)","LD C,RLC (I%@h)","LD D,RLC (I%@h)","LD E,RLC (I%@h)","LD H,RLC (I%@h)","LD L,RLC (I%@h)","RLC (I%@h)","LD A,RLC (I%@h)",	// 00
  "LD B,RRC (I%@h)","LD C,RRC (I%@h)","LD D,RRC (I%@h)","LD E,RRC (I%@h)","LD H,RRC (I%@h)","LD L,RRC (I%@h)","RRC (I%@h)","LD A,RRC (I%@h)",
  "LD B,RL (I%@h) ","LD C,RL (I%@h) ","LD D,RL (I%@h) ","LD E,RL (I%@h) ","LD H,RL (I%@h) ","LD L,RL (I%@h) ","RL (I%@h)","LD A,RL (I%@h) ",	// 10
  "LD B,RR (I%@h) ","LD C,RR (I%@h) ","LD D,RR (I%@h) ","LD E,RR (I%@h) ","LD H,RR (I%@h) ","LD L,RR (I%@h) ","RR (I%@h)","LD A,RR (I%@h) ",
  "LD B,SLA (I%@h)","LD C,SLA (I%@h)","LD D,SLA (I%@h)","LD E,SLA (I%@h)","LD H,SLA (I%@h)","LD L,SLA (I%@h)","SLA (I%@h)","LD A,SLA (I%@h)",	// 20
  "LD B,SRA (I%@h)","LD C,SRA (I%@h)","LD D,SRA (I%@h)","LD E,SRA (I%@h)","LD H,SRA (I%@h)","LD L,SRA (I%@h)","SRA (I%@h)","LD A,SRA (I%@h)",
  "LD B,SLL (I%@h)","LD C,SLL (I%@h)","LD D,SLL (I%@h)","LD E,SLL (I%@h)","LD H,SLL (I%@h)","LD L,SLL (I%@h)","SLL (I%@h)","LD A,SLL (I%@h)",	// 30
  "LD B,SRL (I%@h)","LD C,SRL (I%@h)","LD D,SRL (I%@h)","LD E,SRL (I%@h)","LD H,SRL (I%@h)","LD L,SRL (I%@h)","SRL (I%@h)","LD A,SRL (I%@h)",
  
  "BIT 0,(I%@h)","BIT 0,(I%@h)","BIT 0,(I%@h)","BIT 0,(I%@h)","BIT 0,(I%@h)","BIT 0,(I%@h)","BIT 0,(I%@h)","BIT 0,(I%@h)",		// 40
  "BIT 1,(I%@h)","BIT 1,(I%@h)","BIT 1,(I%@h)","BIT 1,(I%@h)","BIT 1,(I%@h)","BIT 1,(I%@h)","BIT 1,(I%@h)","BIT 1,(I%@h)",
  "BIT 2,(I%@h)","BIT 2,(I%@h)","BIT 2,(I%@h)","BIT 2,(I%@h)","BIT 2,(I%@h)","BIT 2,(I%@h)","BIT 2,(I%@h)","BIT 2,(I%@h)",		// 50
  "BIT 3,(I%@h)","BIT 3,(I%@h)","BIT 3,(I%@h)","BIT 3,(I%@h)","BIT 3,(I%@h)","BIT 3,(I%@h)","BIT 3,(I%@h)","BIT 3,(I%@h)",
  "BIT 4,(I%@h)","BIT 4,(I%@h)","BIT 4,(I%@h)","BIT 4,(I%@h)","BIT 4,(I%@h)","BIT 4,(I%@h)","BIT 4,(I%@h)","BIT 4,(I%@h)",		// 60
  "BIT 5,(I%@h)","BIT 5,(I%@h)","BIT 5,(I%@h)","BIT 5,(I%@h)","BIT 5,(I%@h)","BIT 5,(I%@h)","BIT 5,(I%@h)","BIT 5,(I%@h)",
  "BIT 6,(I%@h)","BIT 6,(I%@h)","BIT 6,(I%@h)","BIT 6,(I%@h)","BIT 6,(I%@h)","BIT 6,(I%@h)","BIT 6,(I%@h)","BIT 6,(I%@h)",		// 70
  "BIT 7,(I%@h)","BIT 7,(I%@h)","BIT 7,(I%@h)","BIT 7,(I%@h)","BIT 7,(I%@h)","BIT 7,(I%@h)","BIT 7,(I%@h)","BIT 7,(I%@h)",

  "LD B,RES 0,(I%@h)","LD C,RES 0,(I%@h)","LD D,RES 0,(I%@h)","LD E,RES 0,(I%@h)","LD H,RES 0,(I%@h)","LD L,RES 0,(I%@h)","RES 0,(I%@h)","LD A,RES 0,(I%@h)",	// 80
  "LD B,RES 1,(I%@h)","LD C,RES 1,(I%@h)","LD D,RES 1,(I%@h)","LD E,RES 1,(I%@h)","LD H,RES 1,(I%@h)","LD L,RES 1,(I%@h)","RES 1,(I%@h)","LD A,RES 1,(I%@h)",
  "LD B,RES 2,(I%@h)","LD C,RES 2,(I%@h)","LD D,RES 2,(I%@h)","LD E,RES 2,(I%@h)","LD H,RES 2,(I%@h)","LD L,RES 2,(I%@h)","RES 2,(I%@h)","LD A,RES 2,(I%@h)",	// 90
  "LD B,RES 3,(I%@h)","LD C,RES 3,(I%@h)","LD D,RES 3,(I%@h)","LD E,RES 3,(I%@h)","LD H,RES 3,(I%@h)","LD L,RES 3,(I%@h)","RES 3,(I%@h)","LD A,RES 3,(I%@h)",
  "LD B,RES 4,(I%@h)","LD C,RES 4,(I%@h)","LD D,RES 4,(I%@h)","LD E,RES 4,(I%@h)","LD H,RES 4,(I%@h)","LD L,RES 4,(I%@h)","RES 4,(I%@h)","LD A,RES 4,(I%@h)",	// A0
  "LD B,RES 5,(I%@h)","LD C,RES 5,(I%@h)","LD D,RES 5,(I%@h)","LD E,RES 5,(I%@h)","LD H,RES 5,(I%@h)","LD L,RES 5,(I%@h)","RES 5,(I%@h)","LD A,RES 5,(I%@h)",
  "LD B,RES 6,(I%@h)","LD C,RES 6,(I%@h)","LD D,RES 6,(I%@h)","LD E,RES 6,(I%@h)","LD H,RES 6,(I%@h)","LD L,RES 6,(I%@h)","RES 6,(I%@h)","LD A,RES 6,(I%@h)",	// B0
  "LD B,RES 7,(I%@h)","LD C,RES 7,(I%@h)","LD D,RES 7,(I%@h)","LD E,RES 7,(I%@h)","LD H,RES 7,(I%@h)","LD L,RES 7,(I%@h)","RES 7,(I%@h)","LD A,RES 7,(I%@h)",

  "LD B,SET 0,(I%@h)","LD C,SET 0,(I%@h)","LD D,SET 0,(I%@h)","LD E,SET 0,(I%@h)","LD H,SET 0,(I%@h)","LD L,SET 0,(I%@h)","SET 0,(I%@h)","LD A,SET 0,(I%@h)",	// C0
  "LD B,SET 1,(I%@h)","LD C,SET 1,(I%@h)","LD D,SET 1,(I%@h)","LD E,SET 1,(I%@h)","LD H,SET 1,(I%@h)","LD L,SET 1,(I%@h)","SET 1,(I%@h)","LD A,SET 1,(I%@h)",
  "LD B,SET 2,(I%@h)","LD C,SET 2,(I%@h)","LD D,SET 2,(I%@h)","LD E,SET 2,(I%@h)","LD H,SET 2,(I%@h)","LD L,SET 2,(I%@h)","SET 2,(I%@h)","LD A,SET 2,(I%@h)",	// D0
  "LD B,SET 3,(I%@h)","LD C,SET 3,(I%@h)","LD D,SET 3,(I%@h)","LD E,SET 3,(I%@h)","LD H,SET 3,(I%@h)","LD L,SET 3,(I%@h)","SET 3,(I%@h)","LD A,SET 3,(I%@h)",
  "LD B,SET 4,(I%@h)","LD C,SET 4,(I%@h)","LD D,SET 4,(I%@h)","LD E,SET 4,(I%@h)","LD H,SET 4,(I%@h)","LD L,SET 4,(I%@h)","SET 4,(I%@h)","LD A,SET 4,(I%@h)", // E0
  "LD B,SET 5,(I%@h)","LD C,SET 5,(I%@h)","LD D,SET 5,(I%@h)","LD E,SET 5,(I%@h)","LD H,SET 5,(I%@h)","LD L,SET 5,(I%@h)","SET 5,(I%@h)","LD A,SET 5,(I%@h)",
  "LD B,SET 6,(I%@h)","LD C,SET 6,(I%@h)","LD D,SET 6,(I%@h)","LD E,SET 6,(I%@h)","LD H,SET 6,(I%@h)","LD L,SET 6,(I%@h)","SET 6,(I%@h)","LD A,SET 6,(I%@h)",	// F0
  "LD B,SET 7,(I%@h)","LD C,SET 7,(I%@h)","LD D,SET 7,(I%@h)","LD E,SET 7,(I%@h)","LD H,SET 7,(I%@h)","LD L,SET 7,(I%@h)","SET 7,(I%@h)","LD A,SET 7,(I%@h)",

};




//*************************************************************/
//			debug start
//*************************************************************/
void DebugStart(reg *R)
{
	at=0;
    
	if( DebugResult_sub[0]!=0)
		{
		 at+=sprintf(DebugResult+at,"%s",DebugResult_sub);
		 DebugResult_sub[0]=0;
		}

	ClearAttr();
	DisplayRegisters(R);
	DisplayMemDump();

	DisplayStatus( R);
	DisplayIO();
	disasm_current_adr = R->PC.W;

	chdir( debugWorkPath);		// デバッグパスに cd する

}



//*************************************************************/
//			D_RDMEM    debug memory read
//*************************************************************/
byte D_RDMEM( word A)
{
	byte V;
	if( is_debug_rdmem_ram )
		V = peek_memory( A);	// READ RAM
	else
		V = M_RDMEM( A);		// READ BANK MEMORY
	return V;
}


/* S: disasm output 
   MCode: machine code
   A: memory address
*/
#define ADD_MachineCode(B)   idx+=sprintf(MachineCode+idx,"%02X",D_RDMEM(B))	   // MachineCode


//*************************************************************/
//		Dasm　一命令を、逆アセンブルする		
//  Input  A: スタートアドレス
//         S: ニーモニックなどの結果 を返す
//         MachineCode: マシン語コードの結果　を返す
//         comment: コメントを返す
//  Output （逆アセンブルした）一命令のバイト数
//*************************************************************/
/** DAsm() ***************************************************/
/** DAsm() will disassemble the code at adress A and put    **/
/** the output text into S. It will return the number of    **/
/** bytes disassembled.                                     **/
/*************************************************************/

int DAsm(char *S, char *MachineCode ,char *comment , word A)
{
  char R[128],H[10],C,*T,*P;
  byte J,Offset=0;
  word B;

  int  idx=0;	// MachinCode idx

  comment[0]=0;

  B=A;C='\0';J=0;

  ADD_MachineCode(B);	// add MachineCode

  switch(D_RDMEM(B))
  {
    case 0xCB: B++;ADD_MachineCode(B); T=MnemonicsCB[D_RDMEM(B++)];  break;
    case 0xED: B++;ADD_MachineCode(B); T=MnemonicsED[D_RDMEM(B++)];  break;
    case 0xDD: B++;C='X';
		       if(D_RDMEM(B)!=0xCB) { ADD_MachineCode(B); T=MnemonicsXX[D_RDMEM(B++)];}
               else
               { ADD_MachineCode(B); B++;ADD_MachineCode(B); Offset=D_RDMEM(B++); J=1;ADD_MachineCode(B); T=MnemonicsXCB[D_RDMEM(B++)];  }
               break;
    case 0xFD: B++;C='Y';
         	   if(D_RDMEM(B)!=0xCB) { ADD_MachineCode(B); T=MnemonicsXX[D_RDMEM(B++)];}
               else
               { ADD_MachineCode(B); B++;ADD_MachineCode(B); Offset=D_RDMEM(B++); J=1;ADD_MachineCode(B); T=MnemonicsXCB[D_RDMEM(B++)]; }
               break;
    default:   T=Mnemonics[D_RDMEM(B++)]; 
  }

  if((P=strchr(T,'^')))					// IX+d  or IY+d 's  +d
  {
    strncpy(R,T,P-T);R[P-T]='\0';
    sprintf(H,"%02X",D_RDMEM(B++));  ADD_MachineCode(B);
    strcat(R,H);strcat(R,P+1);
  }
  else strcpy(R,T);
  if((P=strchr(R,'%'))) *P=C;			// IX or IY

  if((P=strchr(R,'*')))					// 8bit sokuti
  {
    strncpy(S,R,P-R);S[P-R]='\0';    ADD_MachineCode(B);
    sprintf(H,"%02X",D_RDMEM(B++)); 
    strcat(S,H);strcat(S,P+1);
  }
  else
    if((P=strchr(R,'@')))				// 8bit soutai sokuti
    {
      strncpy(S,R,P-R);S[P-R]='\0';
	  if(!J) {ADD_MachineCode(B);  Offset=D_RDMEM(B++);	}	 
      //strcat(S,Offset&0x80? "-":"+");
      J=Offset&0x80? 256-Offset:Offset;
      //sprintf(H,"%02X",J);
      //strcat(S,H);strcat(S,P+1);
	  sprintf(H,"%04X", Offset&0x80? B-J: B+J);			// JR命令のアドレス計算 相対表記やめて、絶対表記にする　windy
	  strcat(S,H);strcat(S,P+1);
    }
    else
      if((P=strchr(R,'#')))				// 16bit sokuti
      {
		word adr = D_RDMEM(B)+256*D_RDMEM(B+1);
		strncpy(S,R,P-R);S[P-R]='\0';
        sprintf(H,"%04X",adr); ADD_MachineCode(B); ADD_MachineCode(B+1);
        strcat(S,H);strcat(S,P+1);
        B+=2;

		if( strstr(S,"CALL") || strstr(S,"JP"))		// CALL ,JP のみ
			{
			strcpy(comment ,read_disasm_comment(adr));	// comment
			if( comment[0]=='.') comment[0]= 0;
			}
      }
      else strcpy(S,R);


  if( strstr(S,"RST"))
	{
	 int adr;
	 sscanf(S+4,"%02XH",&adr);
	 strcpy(comment ,read_disasm_comment(adr));	// comment
	 if( comment[0]=='.') comment[0]= 0;
	}
  return(B-A);
}



char* cnvCommandName(int cmd)
{
	char* name[] = {
	"END","FOR","NEXT","DATA","INPUT","DIM","READ","LET",
	"GOTO","RUN","IF","RESTORE","GOSUB","RETURN","REM",
	"STOP","OUT","ON","LPRINT","DEF","POKE","PRINT",
	"CONT","LIST","LLIST","CLEAR","COLOR","PSET",
	"PRESET","LINE","PAINT","SCREEN","CLS","LOCATE",
	"CONSOLE","CLOAD","CSAVE","EXEC","SOUND","PLAY","KEY",
	"LCOPY","NEW",
	"RENUM","CIRCLE", "GET","PUT","BLOAD","BSAVE","FILES",
	"LFILES","LOAD","MERGE", "NAME","SAVE","FIELD","LSET","RSET",
	"OPEN","CLOSE","DSKO$","KILL","TALK","MON","KANJI","DELETE",  // DELETE = C1
	"TAB","TO","FN","SPC","INKEY$","THEN","NOT","STEP","+","-","*","/","^","AND","OR",">","=","<",
	"SGN","INT","ABS","USR","FRE","INP","LPOS","POS","SQR","RND","LOG",
	"EXP","COS","SIN","TAN","PEEK","LEN","HEX$","STR$","VAL","ASC",
	"CHR$","LEFT$","RIGHT$","MID$","POINT","CSRLIN","STICK","STRIG","TIME",
	"PAD", "DSKI$","LOF","LOC","EOF","DSKF","CVS","MKS$","ATN","CVI","MKI$",
	};
	
	if( 0x80 <= cmd  && cmd <=0xfc)
		return (name[cmd-0x80]);
	else
		return "";
}


char* getOperand( word addr)
{
	static byte buff[512];
	byte *out = buff;
	memset(buff,0,sizeof(buff));

	byte a = peek_memory(addr);
	while (a != ':' && a != 0x00)		// TO FIX  ダブルコーテーション処理を追加するべき
		{
		if( a >= 0x80) {				// 中間言語
			byte *p = cnvCommandName(a);
			int len = strlen(p);
			my_strncpy(out , p , len);
			out+=len;
			addr++;
		} else {
			*out= a;
			out++;
			addr++;
			}
		a = peek_memory(addr);
		}

	return buff;
}


/** Debug() **************************************************/
/** This function should exist if DEBUG is #defined. When   **/
/** Trace!=0, it is called after each command executed by   **/
/** the CPU, and given the Z80 registers.                   **/
/*************************************************************/
static void DisplayRegisters(reg *R)
{
	char T[10];
	static char Flags[8] = "SZ.H.PNC";
	static char S[128];		// Disasm list
	static char MachineCode[128]; // machine code
	static char comment[20]; // comment
	byte J, I;

    ClearAttr();

	DAsm(S, MachineCode ,comment, R->PC.W);
	for(J=0,I=R->AF.B.l;J<8;J++,I<<=1) T[J]=I&0x80? Flags[J]:'.';
	T[8]='\0';


	at += sprintf
	(
		DebugResult+at, "AF:%04X BC:%04X DE:%04X HL:%04X SP:%04X IX:%04X IY:%04X\r\n",
		R->AF.W,R->BC.W,R->DE.W,R->HL.W,R->SP.W,R->IX.W,R->IY.W
	);
	at += sprintf
	( 
		DebugResult+at, "PC:%04X [%-8s - %-13s] SP:[%04X] F[%s]\r\n",
		R->PC.W , MachineCode ,S,M_RDMEM(R->SP.W)+M_RDMEM(R->SP.W+1)*256,T
	);

	if( !sr_mode)	// 非SRの実行中のBASICのコマンドを表示する
		{
		 if( R->PC.W == 0x71c)
			{
			at += sprintf(DebugResult+at,"[%d] ",peek_memory(0xfa5e)*256+peek_memory(0xfa5d));	// 行番号
			at += sprintf(DebugResult+at,"%s ", cnvCommandName(R->AF.B.h+0x80));				// 命令
			at += sprintf(DebugResult+at,"%s \r\n", getOperand(R->HL.W+1));						// オペランド
		 }
		 if(R->PC.W == 0x7f6)
			{
			 at += sprintf(DebugResult + at, "[%d] ", peek_memory(0xfa5e) * 256 + peek_memory(0xfa5d));	// 行番号
			 at += sprintf(DebugResult + at, "%s \r\n", getOperand(R->HL.W ));						// オペランド
		 }
	}

#if 0
	at += sprintf
	(
		DebugResult+at, "AF:%04X HL:%04X DE:%04X BC:%04X SP:%04X IX:%04X IY:%04X\r\n",
		R->AF.W,R->HL.W,R->DE.W,R->BC.W,R->SP.W,R->IX.W,R->IY.W
	);
	at += sprintf
	( 
		DebugResult+at, "PC:%04X [%02X - %-13s] SP:[%04X] FLAGS:[%s]\r\n\r\n",
		R->PC.W , M_RDMEM(R->PC.W),S,M_RDMEM(R->SP.W)+M_RDMEM(R->SP.W+1)*256,T
	);
#endif
}



//*************************************************************/
//			display usage
//*************************************************************/
static void DisplayUsage(void)
{
	at += sprintf(DebugResult+at, "***** Built-in Z80 Debugger Commands *****\r\n");
	at += sprintf(DebugResult + at, "break <addr>  : Set Breakpoint\r\n");
	at += sprintf(DebugResult + at, "dump <addr>   : Memory dump at addr\r\n");
	at += sprintf(DebugResult + at, "disasm <addr> : Disassembly at addr of current BANK\r\n");
	at += sprintf(DebugResult + at, "ur <addr>     : Disassembly at addr of RAM\r\n");
	at += sprintf(DebugResult + at, "go(g)         : Continue with debugger window\r\n");
	at += sprintf(DebugResult + at, "G             : Continue without debugger window\r\n");
	at += sprintf(DebugResult + at, "loadmem       : Load memory \r\n");
	at += sprintf(DebugResult + at, "savemem       : Save memory \r\n");
	at += sprintf(DebugResult + at, "setbin        : Load memory \r\n");
	at += sprintf(DebugResult + at, "reg           : Display registers\r\n");
	at += sprintf(DebugResult + at, "step(s)       : Break at next instruction\r\n");
	at += sprintf(DebugResult + at, "reset         : reset  (PC address =0) \r\n");
	at += sprintf(DebugResult + at, "set           : setting \r\n");
	at += sprintf(DebugResult + at, "?,h           : Show this help text\r\n\r\n");

#if 0
	at += sprintf(DebugResult+at, "***** Built-in Z80 Debugger Commands *****\r\n");
	at += sprintf(DebugResult+at, "r          : Display registers\r\n");
	at += sprintf(DebugResult+at, "s          : Break at next instruction\r\n");
	at += sprintf(DebugResult+at, "= <addr>   : Break at addr\r\n");
	at += sprintf(DebugResult+at, "+ <offset> : Break at PC + offset\r\n");
	at += sprintf(DebugResult+at, "c          : Continue without break\r\n");
	at += sprintf(DebugResult+at, "j <addr>   : Continue from addr\r\n");
	at += sprintf(DebugResult+at, "m <addr>   : Memory dump at addr\r\n");
	at += sprintf(DebugResult+at, "d <addr>   : Disassembly at addr\r\n");
	at += sprintf(DebugResult+at, "?,h        : Show this help text\r\n\r\n");
#endif
}

//*************************************************************/
//			clear attr
//*************************************************************/
void ClearAttr(void)
{
	int i,j;
    //memset( Attr_DebugResult , 0x0f , 10000);

	for(i=0; i< MAX_BACK_LOG; i++)
		for(j=0; j< 256; j++)
			commandLineAttr[ i][ j] =0x0f;
}


//*************************************************************/
//			display memory dump  (右側のずっと表示している部分）
//*************************************************************/
void DisplayMemDump(void)
	{
	char tmp[10];
	static char pre_result[2048];
    static char Attr[2048];
	unsigned char DebugResult_bak[10000];

    strcpy( DebugResult_bak , DebugResult);
    memset( Attr , 0x0F, 2048); 
    
    sprintf( tmp , "0x%X", dump_start_adr);
    
    at=0;
    make_memdump( NULL , tmp , 1);
    
	if(dump_is_ram ==EXTRAM_BANK){		// 拡張RAMだと、[EXT] の色を変える
		memset( Attr , 11 , 5);
	}

        // --------  カーソル位置を、反転表示 -------
	if( current_pain == P_DUMP)
		{
		byte *p1 ,*p2;
		int   s1 ,s2;
		int   i;
    
		p1 = DebugResult;
		for(i=0; i< dump_yy+1; i++)
			{
			p1 = strstr( p1+1 , "\n");          // search same address
			}

		if( p1)		// found !
			{
		    //p2 = strstr( p1+1 , "\n");                // search next \n
			s1 = p1 - DebugResult;
			if( DebugResult[ s1]=='\n') s1++;
		    //s2 = p2 - DebugResult;
		    if( p1 )
				for(i=0 ; i<2; i++)
			        Attr[ (5+s1) + dump_xx *3+i ] = 0xF0;
			}
		}




    DebugPutString( DUMP_TOP_XX , DUMP_TOP_YY ,DebugResult ,20 ,Attr );
    strcpy( pre_result , DebugResult );
	RefreshDebugString();

    strcpy( DebugResult , DebugResult_bak);
	}
   


//*************************************************************/
//			display i/o		（下側のずっと表示されている部分）
//*************************************************************/
void DisplayIO(void)
{
	int portE1=0;

	int port90=0;
	int port91=0;
	int port92=0;

	int portD0=0;
	int portD3=0;



	int    i,j,k;
    //char   S[256];
	static char pre_result[1024];
    static char Attr[1024];
    char DebugResult_bak[10000];
    //char *p;
    int   f;
    
    int   col= 0x0E;
 
    strcpy( DebugResult_bak , DebugResult);
    memset( Attr , 0x0F, 1024);

    at=0;
	at+= sprintf(DebugResult+at,"[I/O PORT]  \n");
	for(k=0; k<11; k++) 
		for(j=0;j<7;j++) 
			{
			if( k>7 && j >3) break;		/* 右下には、色をつけない */
			for(i=0;i<4;i++) Attr[ at+i+j*7+k*49]= col;
			}
	
	at+= sprintf(DebugResult+at,"       B0H:%02X E2H:%02X 60H:%02X 6BH:%02X B8H:%02X FAH:%02X\n"         ,portB0,InE2H(),port60[0],port60[0xB],portB8,portFA);
	at+= sprintf(DebugResult+at,"       C0H:%02X E3H:%02X 61H:%02X 6CH:%02X B9H:%02X FBH:%02X\n"         ,portC0,InE3H(),port60[1],port60[0xC],portB9,portFB);
	at+= sprintf(DebugResult+at,"       C1H:%02X F0H:%02X 62H:%02X 6DH:%02X BAH:%02X FCH:%02X\n"         ,portC1, portF0,port60[2],port60[0xD],portBA,portFC);
	at+= sprintf(DebugResult+at,"90H:%02X C2H:%02X F1H:%02X 63H:%02X 6EH:%02X BBH:%02X 40H:%02X\n"  ,port90,portC2, portF1,port60[3],port60[0xE],portBB ,port40);
	at+= sprintf(DebugResult+at,"91H:%02X C3H:%02X F2H:%02X 64H:%02X 6FH:%02X BCH:%02X 41H:%02X\n"  ,port91,portC3, portF2,port60[4],port60[0xF],portBC ,port41);
	at+= sprintf(DebugResult+at,"92H:%02X D0H:%02X F3H:%02X 65H:%02X        BDH:%02X 42H:%02X\n"    ,port92,portD0, portF3,port60[5]            ,portBD ,port42);
	at+= sprintf(DebugResult+at,"93H:%02X D1H:%02X F4H:%02X 66H:%02X C8H:%02X BEH:%02X 43H:%02X\n"  ,port93,portD1, portF4,port60[6],portC8     ,portBE ,port43);
	at+= sprintf(DebugResult+at,"A0H:%02X D2H:%02X F5H:%02X 67H:%02X C9H:%02X BFH:%02X       \n"  ,portA0,portD2, portF5,port60[7],portC9     ,portBF);
	at+= sprintf(DebugResult+at,"A1H:%02X D3H:%02X F6H:%02X 68H:%02X "                            ,portA1,portD3, portF6,port60[8] );
	for(k=0;k<8;k++) Attr[at+k] = col; at+= sprintf(DebugResult+at,"CBH,CAH:%02X%02X        \n" ,portCB      ,portCA);
	at+= sprintf(DebugResult+at,"A2H:%02X E0H:%02X F7H:%02X 69H:%02X "                     ,DoIn(0xA2)  ,InE0H(), portF7 ,port60[9]);
	for(k=0;k<8;k++) Attr[at+k] = col; at+= sprintf(DebugResult+at,"CDH,CCH:%02X%02X        \n" ,0 ,portCC);
	at+= sprintf(DebugResult+at,"A3H:%02X E1H:%02X F8H:%02X 6AH:%02X "                     ,DoIn(0xA3)  ,portE1, portF8,port60[0xA]);
	for(k=0;k<8;k++) Attr[at+k] = col; at+= sprintf(DebugResult+at,"CFH.CEH:%02X%02X        "   ,portCF      ,portCE);


    DebugPutString( IO_TOP_XX, IO_TOP_YY ,DebugResult ,23 ,Attr );
    strcpy( pre_result , DebugResult );
   	RefreshDebugString();

    strcpy( DebugResult , DebugResult_bak);
	}



//*************************************************************/
//			display function key		(一番下）
//*************************************************************/
void DisplayFunction(void)
{
	int    i,j,k;
    //char   S[256];
	static char pre_result[1024];
    static char Attr[1024];
    char DebugResult_bak[10000];
    //char *p;
    int   f;
    
    int   col= 0x0E;
 
    strcpy( DebugResult_bak , DebugResult);
    memset( Attr , 0x0F, 1024);

    at=0;
	at+= sprintf(DebugResult+at,"                       f5:                           f9:BREAK   f10:         f11: STEP-In   " );
	//for(j=0;j<4;j++) for(i=0;i<4;i++) Attr[ at+i+j*9]= col;
	//for(j=0;j<2;j++) for(i=0;i<5;i++) Attr[ at+i+j*11+36] = col;


    DebugPutString( FUNCTION_TOP_XX, FUNCTION_TOP_YY ,DebugResult ,23 ,Attr );
    strcpy( pre_result , DebugResult );
   	RefreshDebugString();

    strcpy( DebugResult , DebugResult_bak);
	}



//*************************************************************/
//			display status		（右側のずっと表示されている部分）
//*************************************************************/
void DisplayStatus(reg *R)
{
	int    i,j,k;
    //char   S[256];
	static char pre_result[1024];
    static char Attr[1024];
    char DebugResult_bak[10000];
    //char *p;
    int   f;
    
    int   col= 0x0E;
 
    strcpy( DebugResult_bak , DebugResult);
    memset( Attr , 0x0F, 1024);

    at=0;
	at+= sprintf(DebugResult+at,"[REGISTERS]\n" );
	for(j=0;j<4;j++) for(i=0;i<4;i++) Attr[ at+i+j*9]= col;

	at+= sprintf(DebugResult+at,"AF :%04X BC :%04X DE :%04X HL :%04X \n",R->AF.W, R->BC.W , R->DE.W , R->HL.W);

	for(j=0;j<4;j++) for(i=0;i<4;i++) Attr[ at+i+j*9]= col;

	at+= sprintf(DebugResult+at,"AF':%04X BC':%04X DE':%04X HL':%04X \n",R->AF1.W, R->BC1.W , R->DE1.W , R->HL1.W);

	for(j=0;j<4;j++) for(i=0;i<4;i++) Attr[ at+i+j*9]= col;

	at+= sprintf(DebugResult+at,"IX :%04X IY :%04X PC :%04X SP :%04X \n",R->IX.W, R->IY.W , R->PC.W , R->SP.W);
	at+= sprintf(DebugResult+at,"FLAGS:[");
	
	f =R->AF.B.l;		// CPU flag 
	for(i=0;i<8;i++)
		{ 
		if( (f & 0x80) ==0)		// flag off ?
			 Attr[ at+i] = 0x07;	// off -> gray color
		f<<=1;		// next flag
		}
	Attr[ at+0xa] = col;  Attr[ at+0xb] = col;

	at+= sprintf(DebugResult+at,"SZ.H.PNC] I:%02X\n",R->I);
		

	/* memory map */
	at+= sprintf(DebugResult+at,"[MEMORY]   -READ-   -WRITE-    ");

	for(k=0; k<5; k++) Attr[ at+k] = col;
    at+= sprintf(DebugResult+at,"(BC):%02X,%02X\n",M_RDMEM(R->BC.W) ,M_RDMEM(R->BC.W+1));

	for(i=0; i<8; i++)
		{
		for(k=0; k<10; k++) Attr[ at+k] =col;
		at+= sprintf(DebugResult+at,"%04X-%04X:",i*0x2000,i*0x2000+0x1fff);
		at+= sprintf(DebugResult+at,"%-9s:%-9s  ", read_b[i] , write_b[i]);


		     if( i==0) {for(k=0; k<5; k++) Attr[ at+k] = col; at+= sprintf(DebugResult+at,"(DE):%02X,%02X\n",M_RDMEM(R->DE.W) ,M_RDMEM(R->DE.W+1));}
		else if( i==1) {for(k=0; k<5; k++) Attr[ at+k] = col; at+= sprintf(DebugResult+at,"(HL):%02X,%02X\n",M_RDMEM(R->HL.W) ,M_RDMEM(R->HL.W+1));}
		else if( i==2) {for(k=0; k<5; k++) Attr[ at+k] = col; at+= sprintf(DebugResult+at,"(SP):%02X,%02X\n",M_RDMEM(R->SP.W) ,M_RDMEM(R->SP.W+1));}
		else if( i==3) {for(k=0; k<5; k++) Attr[ at+k] = col; at+= sprintf(DebugResult+at,"(IX):%02X,%02X\n",M_RDMEM(R->IX.W) ,M_RDMEM(R->IX.W+1));}
		else if( i==4) {for(k=0; k<5; k++) Attr[ at+k] = col; at+= sprintf(DebugResult+at,"(IY):%02X,%02X\n",M_RDMEM(R->IY.W) ,M_RDMEM(R->IY.W+1));}
		else at+= sprintf(DebugResult+at,"\n");
		}

    DebugPutString( STATUS_TOP_XX, STATUS_TOP_YY ,DebugResult ,23 ,Attr );
    strcpy( pre_result , DebugResult );
   	RefreshDebugString();

    strcpy( DebugResult , DebugResult_bak);
	}



int cmd_length1[]={
0x00,0x03,0x04,0x05,0x07,0x08,0x09,0x0a,0x0b,0x0c,0x0d,0x0f,
0x12,0x13,0x14,0x15,0x17,0x19,0x1a,0x1b,0x1c,0x1d,0x1f,
0x23,0x24,0x25,0x27,0x29,0x2B,0x2C,0x2D,0x2F,
0x33,0x34,0x35,0x36,0x37,0x38,0x39,0x3B,0x3C,0x3D,
0xC5,0xC7,0xC8,0xC9,0xCF,
0xD0,0xD1,0xD5,0xD7,0xD8,0xD9,0xDF,
0xE0,0xE1,0xE5,0xE7,0xE8,0xE9,0xEB,0xEE,0xEF,
0xF0,0xF1,0xF3,0xF5,0xF7,0xF8,0xF9,0xFB,0xFF,
-1
};

//*************************************************************/
//				1バイト命令かどうか確認
//*************************************************************/
int is_length1( byte op1 )
{
	int ret=0;
	int i=0;

	if( op1 >= 0x3F && op1 <= 0xC1)
		return 1;

	do {
		if( (byte)cmd_length1[i] == op1)
			{
			ret =1;
			break;
			}
	} while(cmd_length1[i] != -1);
	return ret;
}

//*************************************************************/
//				2バイト命令かどうか確認
//*************************************************************/
int is_length2( byte op1 ,byte op2)
{
	int i=0;
	int ret=0;

	if(op1==0x3E || op1==0x06 || op1==0x0E || op1==0x16 || op1==0x1E || op1==0x26 || op1==0x2E || op1==0x36 
	|| op1==0xC6 || op1==0xCE || op1==0xD6 || op1==0xDE || op1==0xE6 || op1==0xEE || op1==0xF6 || op1==0xFE
    || op1==0x18 || op1==0x38 || op1==0x30 || op1==0x28 || op1==0x20 || op1==0x10
	|| op1==0xDB || op1==0xD3 )   
			ret = 1;

	if(op1 == 0xED)
		if( op2==0x57 || op2==0x5F || op2==0xA0 || op2==0xB0 || op2==0xA8 || op2==0xB8 || op2==0xA1 || op2==0xB1 || op2==0xA9 || op2==0xB9
		 || op2==0x46 || op2==0x56 || op2==0x5E || op2==0x44 || op2==0x4D || op2==0x45
		 || op2==0x78 || op2==0x40 || op2==0x48 || op2==0x50 || op2==0x58 || op2==0x60 || op2==0x68 || op2==0xA2 || op2==0xB2 || op2==0xAA|| op2==0xBA
		 || op2==0x79 || op2==0x41 || op2==0x49 || op2==0x51 || op2==0x59 || op2==0x61 || op2==0x69 || op2==0xA3 || op2==0xB3 || op2==0xAB|| op2==0xBB)
				ret =1;

	if(op1 == 0xDD || op1==0xFD)
		if( op2==0xF9 || op2==0xE5 || op2==0xE1
		 || op2==0x09 || op2==0x19 || op2==0x29 || op2==0x39 || op2==0x23 || op2==0x2B  || op2==0xE3 || op2==0xE9 )
				ret= 1;

	if(op1 == 0xCB)
		ret=1;
	return ret;
}


//*************************************************************/
//				3バイト命令かどうか確認
//*************************************************************/
int is_length3( byte op1, byte op2 , byte op3)
{
	int ret=0;

	if( op1==0xDD || op1==0xFD)
		 if( op2 == 0x7E || op2 ==0x46 || op2== 0x4E ||op2 == 0x56 || op2 == 0x5E  || op2 == 0x66 || op2 == 0x6E
			  || (op2 >=0x70 && op2 <= 0x77) 
			  || op2 ==0x86 || op2 ==0x8E || op2 ==0x96 || op2 ==0x9E || op2 ==0xA6|| op2 ==0xAE || op2 ==0xB6 || op2 ==0xBE
			  || op2 ==0x34 || op2 ==0x35)
			ret =1;

	if( op1==0x01 || op1==0x11 || op1==0x21  || op1==0x2A || op1==0x31  || op1==0x22 || op1==0x32 )
			ret =1;
	
	if( op1==0xC3 || op1==0xDA|| op1==0xD2 || op1==0xCA || op1==0xC2 || op1==0xEA || op1==0xE2 || op1==0xFA || op1==0xF2
	 || op1==0xCD || op1==0xDC|| op1==0xD4 || op1==0xCC || op1==0xC4 || op1==0xEC || op1==0xE4 || op1==0xFC || op1==0xF4)
			ret = 1;

	return ret;
}

//*************************************************************/
//				4バイト命令かどうか確認
//*************************************************************/
int is_length4( byte op1, byte op2 , byte op3 ,byte op4)
{
	int ret=0;

	if( op1==0xDD || op1==0xFD)
		 if( op2 == 0x36 || op2 ==0x21 || op2== 0x2A ||op2 == 0x22 || op2 == 0xCB )
			ret =1;

	if( op1==0xED )
		 if( op2 == 0x43 || op2 ==0x53 || op2== 0x73 )
			ret =1;
	
	return ret;
}


//*************************************************************/
//				直前のOPコードの長さを取得
//*************************************************************/
int get_prev_cmd_length(word addr)
{
	int ret= 0;
	word s;
	byte op1,op2,op3,op4;
	s = addr;

	op1 = M_RDMEM( addr-1);
	op2 = M_RDMEM( addr-2);
	op3 = M_RDMEM( addr-3);
	op4 = M_RDMEM( addr-4);
	if( is_length1( op1))
		ret =1;
	else if( is_length2( op2 , op1))
		ret =2;

	return ret;
}


//*************************************************************/
//				display Disasm      上の方でずっと表示されている部分
//*************************************************************/
void DisplayDisasm(void)
{
    char   S[256];
	static char pre_disasm[1024];
    static byte Attr[1024];
    char DebugResult_bak[10000];
 
    strcpy( DebugResult_bak , DebugResult);
    memset( Attr , 0x0F, 1024);

    at=0;

	if( !disasm_is_backscroll ) {
		//sprintf( S,"%04X:", R->PC.W );
		sprintf( S,"%04X:", disasm_current_adr );
	
		if( strstr( pre_disasm , S ) ==NULL )  // not found PC address ,update disasm
			{
			S[0]=0;
			make_disasm( disasm_current_adr ,S );
			disasm_start_adr= disasm_current_adr;
			}
		else
			{
			strcpy( DebugResult , pre_disasm);						// found PC address , use pre result
			}
	}else {
		S[0]=0;
		make_disasm( disasm_start_adr ,S );
	}

        // --------  現在のアドレスを、反転表示 -------
    {
    byte *p1 ,*p2;
    int   s1 ,s2;
    int   i;
    
	//sprintf( S,"%04X:", R->PC.W );
	sprintf( S,"%04X:", disasm_current_adr );
    p1 = strstr( DebugResult , S);          // search same address

	if( p1)		// found !
		{
	    p2 = strstr( p1 , "\n");                // search next \n
		s1 = p1 - DebugResult;
	    s2 = p2 - DebugResult;
	    if( p1 )
	        for(i=s1 ; i < s2; i++)             // set reverse color
		        Attr[ i ] = 0x90;
		}
    }


        // --------  ブレークポイントを、反転表示 -------
	{
	int idx;
	for(idx=1; idx< MAX_BREAKPOINT; idx++)
	    {
		byte *p1 ,*p2;
		int   s1 ,s2;
		int   action, adr , enable;
		int   i;

		action = breakpoint[idx].action;
		adr=     breakpoint[idx].addr;
		enable = breakpoint[idx].enable;

		if( action == B_PC && enable==1)	// breakpoint あるか？
			{
			sprintf( S,"%04X:", adr );
			p1 = strstr( DebugResult , S);          // search same address

			if( p1)		// found !
				{
				p2 = strstr( p1 , "\n");                // search next \n
				s1 = p1 - DebugResult;
				s2 = p2 - DebugResult;
				if( p1 )
					for(i=s1 ; i < s1+4; i++)             // set reverse color
						 Attr[ i ] = 0xe0;
				}
			}
	    }
	}






        // --------  カーソル位置を、反転表示 -------
	if( current_pain == P_DISASM)
		{
		byte *p1 ,*p2;
		int   s1 ,s2;
		int   i;
    
		//sprintf( S,"%04X:", disasm_current_adr );
	
		p1 = DebugResult;
		for(i=0; i< disasm_yy; i++)
			{
			p1 = strstr( p1+1 , "\n");          // search \n
			}

		if( p1)		// found !
			{
			char tmp[256];
		    p2 = strstr( p1+1 , "\n");                // search next \n

			s1 = p1 - DebugResult;
			if( DebugResult[ s1]=='\n') s1++;	// 改行なら、次の文字へ
			strncpy( tmp, DebugResult+s1,4);				// カーソルのアドレス取得
			sscanf( tmp,"%04X",&disasm_cursor_adr);


			s2 = p2 - DebugResult;
		    if( p1 )
				for(i=s1; i<s2; i++)
				{
				if( Attr[i] == 0x90)	// PC ?
					Attr[i] = 0xD0;
				else if( Attr[i]==0xe0)	// break point ?
					Attr[i] = 0x30;
				else
					Attr[i] = 0xF0;
				}
			}
		}



	/* 2行目のアドレスを取得　（下へ移動用）*/
	{
	char tmp[10];
	byte  *p3;
	p3 = strstr( DebugResult ,"\n");		// search \n
	if( p3 )
		{
		 memcpy( tmp, p3+1,4);	// copy address
		 tmp[4]=0;
		 sscanf( tmp ,"%04X",&disasm_next_adr );
		}
	}

    DebugPutString( DISASM_TOP_XX, DISASM_TOP_YY ,DebugResult ,23+8 ,Attr );
	strcpy( pre_disasm , DebugResult);

	RefreshDebugString();

    strcpy( DebugResult , DebugResult_bak );
}


void make_disasm_pre(word start_adr)
{
	char S[128];
	word Addr;
	word Addr2[256];
	word endAddr;
	byte J=0;
	byte currentJ=0;
	static char MachineCode[128]; // machine code
	static char comment[20];	// comment

	Addr2[J++] = Addr = start_adr-32;

	endAddr = start_adr+32; if( endAddr >=0xff00) endAddr=0;	// はまりこみ対策！？

	if( start_adr == 0xffc0)
		{ printf("ffc0");}

		{
		while( 0xff80 <= Addr && Addr <= 0xffff)
			{
			Addr+=DAsm(S,MachineCode ,comment , Addr);
			if( Addr == start_adr) 
				{ 
				currentJ = J;
				if( currentJ ==0) 
					printf("currentJ 0 ");
				}
			Addr2[J++] = Addr;
			}
		}

	do {
		Addr+=DAsm(S,MachineCode ,comment , Addr);
		if( Addr == start_adr) 
			{ 
			currentJ = J;
			if( currentJ ==0) 
				printf("currentJ 0 ");
			}
		Addr2[J++] = Addr;
		}
	while( Addr <= endAddr);


	if( currentJ !=0)
		{
		disasm_pre_adr  = Addr2[currentJ-1]; 	// 1行前のアドレスを取得
		disasm_pre2_adr = Addr2[currentJ-6];	// n行前のアドレス
		disasm_next_adr = Addr2[currentJ+1];	//  次のアドレス
		disasm_next2_adr= Addr2[currentJ+6];	// n行次のアドレス
		}
	else
		{
		disasm_pre2_adr  = disasm_pre_adr  = start_adr-1;
		disasm_next2_adr = disasm_next_adr  = start_adr+1;
		}
}


//*************************************************************/
//			make disasm list	逆アセンブルリストを作成する
//*************************************************************/
//void make_disasm(reg *R, const char *S)
void make_disasm(word start_adr, const char *S)
{
	word Addr;
	byte J;
	static char MachineCode[128]; // machine code
	static char comment[20];	// comment

	//if (strlen(S) > 1) sscanf(S+1, "%hX", &Addr ); else Addr=R->PC.W;
	if (strlen(S) > 1) Addr = my_atoi( S+1); else Addr=start_adr;

	for (J = 0; J < 16; J++) {
		at += sprintf(DebugResult+at, "%04X:",Addr);         // Address
		Addr+=DAsm(S,MachineCode ,comment , Addr);
		at += sprintf(DebugResult+at, "%-8s ",MachineCode);	  // Machinecode
		at += sprintf(DebugResult+at, "%-14s", S);           // Disasm
		at += sprintf(DebugResult+at, "%-20s\r\n", comment);  // comment
	   }
	at += sprintf(DebugResult+at, "\r\n");

	make_disasm_pre(start_adr);
}


static byte getData(word Addr)
{
	byte tmp;
	if (dump_is_ram == RAM_BANK)
		tmp = peek_memory(Addr);
	else if (dump_is_ram == EXTRAM_BANK)
		tmp = peek_ext_memory(Addr);
	else
		tmp = M_RDMEM(Addr);
	return tmp;
}


//*************************************************************/
//				make memdump list
//  R: Register
//  S: parameter string
//  spc_flag 1: space on    0: space no
//  return: next address
//*************************************************************/
int make_memdump( reg *R ,const char *S , int spc_flag)
{
	word Addr;	// start address
	byte I,J;
	byte tmp[20];
	int max;

	int  amari;
	char mode[6];

	char headerBuff[16*3+100];
	char hexBuff[16 * 3+100];
	char charBuff[16 * 2+100];

	memset(headerBuff,0,sizeof(headerBuff));

	if (strlen(S) >=1) Addr = my_atoi( S); else Addr= R->PC.W;

	amari = Addr & 0xf;
	if( dump_is_ram ==RAM_BANK) 
		strcpy(mode ,"[RAM]"); 
	else if( dump_is_ram==EXTRAM_BANK)
		strcpy(mode, "[EXT]");
	else
		strcpy(mode ,"[MEM]");

	// ------------------ +0 to +F --------------------
	strcat(headerBuff, mode);
	for (J = 0; J < 16; J++)
	{
		sprintf(tmp, "+%X", J);
		strcat(headerBuff, tmp);
		if (spc_flag) strcat(headerBuff, " ");
	}
	at += sprintf(DebugResult + at, "%s\r\n", headerBuff);

	for (J = 0; J < 16; J++) {
		memset(hexBuff, 0, sizeof(hexBuff));
		memset(charBuff, 0, sizeof(charBuff));
		sprintf(tmp, "%04X:",Addr );	// アドレス
		strcat( hexBuff, tmp);
		
		// ------ data -----------
		if(J==0)							// 冒頭のスペースが必要なら挿入する
			{
			for(I=0; I<amari ; I++)
				{
				strcat(hexBuff,"  ");
				if( spc_flag && I < 15) strcat(hexBuff," ");
				}
			}
		I=0;
		max =16;
		do {
			if( J==0 && amari) {
				max = 16-amari;
			}

			sprintf(tmp, "%02X", getData(Addr));
			strcat(hexBuff, tmp);
			if( spc_flag && I <15 ) strcat(hexBuff ," ");


			byte val = getData(Addr);
			sprintf(tmp, "%c", ((val) >= 0x20) ? val : '.');
			strcat(charBuff, tmp);

			Addr++;
			I++;
			}
		while(I< max);

		at += sprintf(DebugResult+at, "%s|%s\r\n",hexBuff, charBuff);

	}
//	at += sprintf(DebugResult+at, "\r\n");
	return Addr;
}



enum cmd_defs { 
D_HELP ,D_GO     ,D_GOFULL  , D_TRACE, D_STEP,
D_BREAK ,D_READ ,D_WRITE  , D_FILL   , D_MOVE,
D_SEARCH,D_OUT,  D_LOADMEM ,D_SAVEMEM , D_RESET ,
D_REG , D_DISASM ,D_DUMP ,D_ANIMATE , D_UR,
D_IN , D_SAVEVRAM,D_LOADVRAM, D_VRAMS,D_NOWAIT , D_PWD , D_DIR ,
D_CD , D_SET     ,D_SETBIN , D_BT    ,D_PRINT, 
D_EDIT,D_HOGE,
};

static char   cmd[][10]= {
"help"   , "?"          , "go"       ,"g"	    ,"G",
"trace"  , "t"          , "step"     ,"s"       ,"break"    ,
"b"      , "read"       , "write"    ,"out"     ,"loadmem"  ,
"savemem",  "reset"     , "reg"      , "disasm" , "dump"    ,
"out"    ,"animate"     , "ur"       ,"in"      , "savevram" ,
"loadvram","vrams"      , "nowait"   ,"pwd"     , "dir",    
"cd",     "set"			,"setbin",    "bt"      ,"print",
"edit",   "fill"		,"hoge",
};

static char   cmdNo[]   ={
D_HELP   ,D_HELP       , D_GO       , D_GO       ,D_GOFULL,
D_TRACE  ,D_TRACE      , D_STEP     , D_STEP     ,D_BREAK ,
D_BREAK,  D_READ       , D_WRITE    , D_OUT      ,D_LOADMEM,
D_SAVEMEM,D_RESET      ,D_REG       , D_DISASM   ,D_DUMP,
D_OUT ,   D_ANIMATE,   D_UR         , D_IN       ,D_SAVEVRAM,
D_LOADVRAM , D_VRAMS,  D_NOWAIT     , D_PWD      ,D_DIR,
D_CD,	  D_SET,       D_SETBIN     , D_BT		 ,D_PRINT,
D_EDIT,   D_FILL,	   D_HOGE,
-1}; 



//*************************************************************/
//				Debug Conv Cmd   （コマンド文字列を、コマンド番号に変換）
//  Input:  cmd string
//  Output: cmd no.
//*************************************************************/
int DebugConvCmd(char *str )
{
	int found=0;
	int i=0;

	do {
		if( !strcmp( cmd[i], str))
			{
			found =1;
			break;
			}
		i++;
	}while( cmdNo[i]!=-1 );

	if( found ==1)
		return cmdNo[i];
	else
		return 255;
}

//*************************************************************/
//			is_hex    16進数か？
//*************************************************************/
int is_hex( char  value)
{
	int ret;
	value = toupper( value);
	if( isdigit( value) || value =='A' ||value =='B' ||value =='C' ||value =='D' ||value =='E' ||value =='F')
		ret= 1;
	else
		ret= 0;
	return ret;
}


//*************************************************************/
//			chk_hex    16進数か？
//*************************************************************/
int chk_hex( char *str)
{
	int i;
	int ret=1;
	for(i=0; i< strlen(str); i++)
		{
		 if( !is_hex( *(str+i)) )
			ret =0;
			break;
		}
	return ret;
}


//*************************************************************/
//			chk_num   10進数　数字か？
//*************************************************************/
int chk_num( char *str)
{
	int i;
	int ret=1;
	for(i=0; i< strlen(str); i++)
		{
		 if( !isdigit( *(str+i)) )
			ret =0;
			break;
		}
	return ret;
}

//*************************************************************/
//				my_atoi　文字列を数値にする
//    Input:　数字変換したい文字列
//    Output: 変換された数字
//            -1: 失敗
//*************************************************************/
int my_atoi( char *str)
{
	int tmp=0;

	if((*str=='0' && ( *(str+1)=='x' || *(str+1)=='X')) ||
	   (*str=='&' && ( *(str+1)=='h' || *(str+1)=='H')))
		{
		if( chk_hex( str+2 )) 
			sscanf(str+2,"%X",&tmp);
		else
			tmp = -1;
		}
	else
		if( chk_num( str))
			sscanf(str,"%d",&tmp);
		else
			tmp = -1;

	return tmp;
}

enum                {R_AF  ,R_BC ,R_DE ,R_HL ,R_IX ,R_IY, R_AF1 ,R_BC1 , R_DE1 ,R_HL1, R_PC ,R_SP,
					 R_A   ,R_B  ,R_C  ,R_D ,R_E   ,R_H  ,R_L   ,R_A1  ,R_B1   ,R_C1 ,R_D1 ,R_E1 ,R_H1 , R_L1,R_NON};
char *regname[]   = {"AF"  ,"BC" ,"DE" ,"HL" ,"IX" ,"IY", "AF'" ,"BC'" ,"DE'"  ,"HL'","PC" ,"SP",
					 "A"   ,"B"  ,"C"  ,"D" ,"E"   ,"H"  ,"L"   ,"A'"  ,"B'"   ,"C'" ,"D'" ,"E'" ,"H'","L'" , ""};

#define MAX_REG   26
#define REG16     11


#define MAX_STACKS 15

enum { S_PUSH , S_CALL ,S_LD , S_EX};

struct _stacks {
	word stack_addr; // スタックのアドレス
	word value;		 // 積んだデータ
	int  cmd;		 // 命令： S_PUSH / S_CALL ...
	int  reg;		 // レジスタ: R_AF / R_BC ...
	word addr;		 // CALL 飛び先アドレス
} stacks[ MAX_STACKS];

static int stack_idx = 0;



//*************************************************************/
//			conv_regname    レジスター名変換
//*************************************************************/
int conv_regname( char *str ,int *reg)
{
	int i=0;
	int found=0;

	do {
		if( *regname[i] == 0) break;
		if( strcmp( regname[i] ,str)==0)
		{
			*reg = i;
			found =1;
			break;
		}
		i++;
	} while( 1);

	return found;
}


//*************************************************************/
//			debug_reg   REG命令
//*************************************************************/
void debug_reg( int argc ,char *argv[])
{
	if( argc >2)
		{
		 int reg,value;
		 if( conv_regname(argv[1],&reg))
			{
			 value = my_atoi( argv[2]);
			 if( value != -1) 
				{
				if( reg < MAX_REG)
					{
					switch( reg)
						{
						case R_AF:  R.AF.W  = value; break;
						case R_BC:  R.BC.W  = value; break;
						case R_DE:  R.DE.W  = value; break;
						case R_HL:  R.HL.W  = value; break;
						case R_IX:  R.IX.W  = value; break;
						case R_IY:  R.IY.W  = value; break;
						case R_AF1: R.AF1.W = value; break;
						case R_BC1: R.BC1.W = value; break;
						case R_DE1: R.DE1.W = value; break;
						case R_HL1: R.HL1.W = value; break;
						case R_PC:  R.PC.W  = value; disasm_current_adr= value; DisplayDisasm(); break;
						case R_SP:  R.SP.W  = value; break;
						case R_A:   R.AF.B.h= (byte)value & 0xff; break;
						case R_B:   R.BC.B.h= (byte)value & 0xff; break;
						case R_C:   R.BC.B.l= (byte)value & 0xff; break;
						case R_D:   R.DE.B.h= (byte)value & 0xff; break;
						case R_E:   R.DE.B.l= (byte)value & 0xff; break;
						case R_H:   R.HL.B.h= (byte)value & 0xff; break;
						case R_L:   R.HL.B.l= (byte)value & 0xff; break; 
						case R_A1:  R.AF1.B.h=(byte)value & 0xff; break;
						case R_B1:  R.BC1.B.h=(byte)value & 0xff; break;
						case R_C1:  R.BC1.B.l=(byte)value & 0xff; break;
						case R_D1:  R.DE1.B.h=(byte)value & 0xff; break;
						case R_E1:  R.DE1.B.l=(byte)value & 0xff; break;
						case R_H1:  R.HL1.B.h=(byte)value & 0xff; break;
						case R_L1:  R.HL1.B.l=(byte)value & 0xff; break;
						}
					if( reg <= REG16)
						at += sprintf(DebugResult+at, " %s  <-- %04X \r\n",regname[reg] ,value);
					else
						at += sprintf(DebugResult+at, " %s  <-- %02X \r\n",regname[reg] ,value & 0xff);

					}
				else
					{
					at += sprintf(DebugResult+at, "Invalid  argument  arg:2 '%s'\r\n",argv[1]);
					}
				}
			else
				at += sprintf(DebugResult+at, "Invalid  argument  arg:3 '%s'\r\n",argv[2]);

			}
		 else
			at += sprintf(DebugResult+at, "Invalid  argument  arg:2 '%s'\r\n",argv[1]);

		}
	 else
		at += sprintf(DebugResult+at, "Invalid  argument  arg:2 \r\n");
}

//*************************************************************/
//			debug_out    out命令
//*************************************************************/
void debug_out( int argc ,char *argv[])
{			// I/O output
	if( argc >2)
		{
		int reg,value;
		reg   = my_atoi( argv[1] );
		value = my_atoi( argv[2] );
		
		if( reg >=0 && value >=0)
			{
			reg &= 0xff;
			value &= 0xff;
			//printf(" out( %X ,%X ) \n", reg, value);
			DoOut( reg , value );
			at += sprintf(DebugResult+at, "   I/O OUT 0x%X <- 0x%X \r\n",reg,value);
			DisplayMemDump();
			}
		
		if( reg ==-1)
			at += sprintf(DebugResult+at, "Invalid  argument  arg:2 '%s'\r\n",argv[1]);
		else if( value ==-1)
			at += sprintf(DebugResult+at, "Invalid  argument  arg:3 '%s'\r\n",argv[2]);
			
		}
	else
		at += sprintf(DebugResult+at, "Invalid  argument  \r\n");
}

//*************************************************************/
//			debug_in    IN命令
//*************************************************************/
void debug_in( int argc ,char *argv[])
{			// I/O output
	if( argc >1)
		{
		int reg,value;
		reg   = my_atoi( argv[1] );
		
		if( reg >=0 )
			{
			reg &= 0xff;
			//printf(" out( %X ,%X ) \n", reg, value);
			value = DoIn( reg );
			value &= 0xff;
			at += sprintf(DebugResult+at, "   I/O IN 0x%02X -> 0x%02X (%d)\r\n",reg,value ,value);
			}
		else if( reg ==-1)
			at += sprintf(DebugResult+at, "Invalid  argument  arg:2 '%s'\r\n",argv[1]);
			
		}
	else
		at += sprintf(DebugResult+at, "Invalid  argument  \r\n");
}


char *break_action[]={"", "PC",  "IN", "OUT",  "ON", "OFF", "CLEAR", 0};


//*************************************************************/
//			search_action	命令を探す
//*************************************************************/
int search_action( char *str ,int *action)
	{
	int i=0,j=0;
	int found=0;
	char tmp[256];
	int len;

	len= strlen(str);
	for(j=0; j< len; j++)		/* 大文字にする */
		{
		 tmp[j] = toupper( *str);
		 tmp[j+1]=0;
		 str++;
		 if( j>254) break;
		}
	do  {
		if(strcmp( tmp , break_action[i])==0) {
			found=1;
			*action = i;
			break;
			}
		i++;
		}
	while ( break_action[i]);
	return found;
	}

//*************************************************************/
//			debug_break    break命令
//*************************************************************/
void debug_break( int argc , char *argv[])
{
	int action = B_PC;	// action
	int addr = 0;		// addr
	int idx = 1;		// idx
	int ii=1;
	int enable=1;		// enable
	int invalid = 0;	// invalid operand


	if( argc>1 )						// ---------------- アクションのチェック ------------------
		if( (*argv[ii] >='a' && *argv[ii] <='z') || (*argv[ii] >='A' && *argv[ii] <='Z') )
			if( search_action( argv[ii] ,&action))	// action
				{
				ii++;
				if(action ==B_ON)  enable = 1;
				if(action ==B_OFF) enable = 0;
				}
			else
				{
				invalid= 1;
				}
											// ---------------	アドレス　のチェック --------------
	if((action == B_PC || action ==B_IN || action ==B_OUT) && argc >ii)
		{
		if( *argv[ii] !='#' )		// address
			{
			addr = my_atoi( argv[ii]);
			ii++;
			}
		else
			{
			invalid= 1;
			}
		}


	if( argc > ii) {					// -------------------- index のチェック -----------------
		if( *argv[ii] =='#' )		// index
			{
			idx = my_atoi( argv[ii]+1);
			if( idx <1  ||  idx > MAX_BREAKPOINT ) 
				{
				 invalid= 1;
				}
			else if( action == B_CLEAR)		// clear
				{
				 int i;
				 breakpoint[idx].action = B_NONE;
				 breakpoint[idx].addr = 0;
				 breakpoint[idx].enable = 0;

				// ------ ブレークポイント　１つでも有効なら、enable_breakpoint 1にする
		 		enable_breakpoint = 0;
				 for(i=0; i< MAX_BREAKPOINT ; i++)
					if( breakpoint[i].action != B_NONE)
						enable_breakpoint = 1;

				 return ;
				}
			}
		else
			{
			 invalid= 1;
			}
		}
	else {									// 設定されていないスロットを探す
		for(int i=1; i<MAX_BREAKPOINT; i++) {
			if(breakpoint[i].action == B_NONE) {
				idx = i;
				break;
			}
		}
	}

	if( action == B_CLEAR && argc==2)	// ---------------- BREAK CLEAR （オールクリア) -------------
		{
		int i;
		for(i=0; i<MAX_BREAKPOINT ; i++)
			{
			breakpoint[ i].action = B_NONE;
			breakpoint[ i].addr = 0;
			breakpoint[ i].enable = 0;
			}
		enable_breakpoint = 0;

		 return;
		}

	if( argc==1)
		{
		int i;						// -------- すべてのブレークポイント表示する ----------
		for(i=1; i< MAX_BREAKPOINT; i++)
			{
			int action = breakpoint[i].action;
			int addr   = breakpoint[i].addr;
			int enable = breakpoint[i].enable;

			if( action != B_NONE)
				{
				at += sprintf(DebugResult+at, " #%2d  %-4s ",i ,break_action[ action] );
				if( action ==B_PC)                   at += sprintf(DebugResult+at, "0x%04X ",addr );
				if( action ==B_IN || action== B_OUT) at += sprintf(DebugResult+at, "0x%02X ",addr );
				at += sprintf(DebugResult+at, " (%s)\r\n" ,(enable ? "ON":"OFF"));
				}
			else
				at += sprintf(DebugResult+at, " #%2d  -- NONE -- \r\n",i);
				
			}
		return;
		}

	if( !invalid ) {		// ----------- 設定されたブレークポイントを表示する -------
		set_breakpoint( action , addr ,idx, enable);
	}
	else {
		at += sprintf(DebugResult+at, "Invalid operand.\r\n");
		return;
	}

}


//*************************************************************/
//				get break point 条件に合うブレークポイントのインデックスの取得
//    Input:　
//*************************************************************/
int get_breakpoint( int action , int addr )
{
	int i;
	int idx=-1;
	for(i =1; i< MAX_BREAKPOINT; i++)
		{
		if( breakpoint[ i].addr   == addr && breakpoint[i].enable && breakpoint[i].action == action)
			{
			idx    = i;
			break;
			}
		}
	return idx;
}

//*************************************************************/
//				get break point disable 無効なブレークポイントのインデックスの取得
//   Output: 0 - : found
//           -1  : not found　
//*************************************************************/
int get_breakpoint_disable( void )
{
	int idx=-1;
	int i;
	for(i=1; i< MAX_BREAKPOINT; i++)
		{
		 if( breakpoint[ i].enable==0)
			{
			 idx = i;
			 break;
			}
		}
	return idx;
}


//*************************************************************/
//				set break point ブレークポイントの設定
//    Input:　
//*************************************************************/
void set_breakpoint( int action , int addr ,int idx ,int enable)
{
	char tmp[256];
	int  a=0;

	if( enable ==255)						// 255だと、反転する 
		enable = !breakpoint[ idx].enable;

	breakpoint[ idx].action = action;
	breakpoint[ idx].addr   = addr;
	breakpoint[ idx].enable = enable;

	a += sprintf(tmp+a, "SET BREAKPOINT #%d  %-4s ",idx ,break_action[ action] );
	if( action ==B_PC)                   a += sprintf(tmp+a, "0x%04X ",addr );
	if( action ==B_IN || action== B_OUT) a += sprintf(tmp+a, "0x%02X ",addr );
	a += sprintf(tmp+a, " (%s)\r\n" ,(enable ? "ON":"OFF"));

	strcpy( DebugResult, tmp);

	enable_breakpoint = 1;
}


//*************************************************************/
//				set break point message   ブレークポイントで止まった時のメッセージを設定しておく
//    Input:　
//*************************************************************/
setBreakPointMessage( char *str)
{
	strcpy( DebugResult_sub ,str);	
}



//*************************************************************/
//				debug_saveram  VRAMの内容を拡張メモリーに保存する
//    Input:　
//*************************************************************/
void debug_savevram(int argc, char*argv[])
{
	byte *vram_addr;
	byte *extram_addr;
	int start_addr;
	int end_addr;
	int x, y, w, h;
	int no;
	int max_no;

	if( argc >4) {
		x = my_atoi(argv[1]);
		y = my_atoi(argv[2]);
		w = my_atoi(argv[3]);
		h = my_atoi(argv[4]);

		// 拡張メモリー、どこまで記録したかを取得する  （savevram 0から探して enable ０のやつを探す）
		max_no = 0;
		for (int i = 0; i < MAX_SAVEVRAM; i++) {
			if (savevram[i].enable == 0) {
				max_no = i;
				break;
			}
		}

		no = max_no;
		start_addr = 0;											// １回目の開始アドレス＝０

		if( no >=1 ) start_addr = savevram[no-1].end_addr+1;	// 既に１個でも記録されていると、前回の終了+1 を開始アドレスにする
		end_addr = start_addr;

		vram_addr = RAM + 0x1a00 + y * 256 / 2 + x;
		extram_addr = EXTRAM64+start_addr;

		if( (start_addr + w * h/2 ) > 0xffff) {
			at += sprintf(DebugResult + at, "error: EXTRAM capacity over \n\r");
			return;
		}

		// VRAMから拡張メモリーにコピーする
		for (int yy = 0; yy < h / 2; yy++)
			{
			memcpy(extram_addr, vram_addr, w);
			vram_addr += 256;
			extram_addr += w;
			end_addr += w;
			}
	

		savevram[no].start_addr = start_addr;
		savevram[no].end_addr = end_addr;
		savevram[no].x = x;
		savevram[no].y = y;
		savevram[no].w = w;
		savevram[no].h = h;
		savevram[no].enable = 1;

		at += sprintf(DebugResult + at, "Saved vram %d %d %d %d -> #%2d \n\r", x,y,w,h,no);
	}
	else {
		at += sprintf(DebugResult + at, "parameter error \n\r");
	}
}

//*************************************************************/
//				debug_loadram  拡張メモリーからVRAMにロードする
//    Input:　
//*************************************************************/
void debug_loadvram(int argc, char* argv[])
{
	byte* vram_addr;
	byte* extram_addr;
	int start_addr = 0;
	int x, y, w, h;
	int no;
	int max_no;

	if( argc != 4 ) {
		at += sprintf(DebugResult + at, "parameter error \n\r");
		return;
	}
	no = my_atoi(argv[1]);
	x = my_atoi(argv[2]);
	y = my_atoi(argv[3]);
	w = savevram[no].w;
	h = savevram[no].h;

	// 拡張メモリー、どこまで記録したかを取得する  （savevram 上から探して enable ０のやつを探す）
	max_no =0;
	for (int i = 0; i < MAX_SAVEVRAM; i++) {
		if( savevram[i].enable == 0 ) {
			max_no = i;
			break;
		}
	}
	if( no > max_no ) {
		at += sprintf(DebugResult + at, "error: max no : %d \n\r", max_no);

	}

	vram_addr = RAM + 0x1a00 + y * 256 / 2 + x;
	extram_addr = EXTRAM64+savevram[no].start_addr;

	// 拡張メモリーからVRAMにコピーする
	for (int yy = 0; yy < h / 2; yy++)
	{
		memcpy(vram_addr, extram_addr,  w);
		vram_addr += 256;
		extram_addr += w;
	}

}




//*************************************************************/
//				vrams 
//    saveしたVRAM の内容を表示する
//*************************************************************/
int debug_vrams()
{
	int i;						// -------- すべての保存したVRAMの内容を表示する ----------
	for (i = 0; i < MAX_SAVEVRAM; i++)
	{

		int start_addr = savevram[i].start_addr;
		int end_addr   = savevram[i].end_addr;
		int x = savevram[i].x;
		int y = savevram[i].y;
		int w = savevram[i].w;
		int h = savevram[i].h;
		int enable     = savevram[i].enable;

		if (enable )
		{
			at += sprintf(DebugResult + at, " #%2d  ", i);
			at += sprintf(DebugResult + at, "0x%04X ", start_addr);
			at += sprintf(DebugResult + at, "0x%04X ", end_addr);
			at += sprintf(DebugResult + at, "%d ", x);
			at += sprintf(DebugResult + at, "%d ", y);
			at += sprintf(DebugResult + at, "%d ", w);
			at += sprintf(DebugResult + at, "%d ", h);
			at += sprintf(DebugResult + at, "\t\n");
		}
		else
			at += sprintf(DebugResult + at, " #%2d  -- NONE -- \r\n", i);

	}
	return 0;
}


//*************************************************************/
//				loadmem
//*************************************************************/
void debug_loadmem(int argc, char* argv[])
{
	if( argc !=4) {
		at += sprintf(DebugResult + at, "Invalid!\r\n"); 
		return;
		}
	word start_addr , end_addr;
	start_addr = my_atoi(argv[2]);
	if( argv[3][0]!='#') {
		end_addr = my_atoi(argv[3]);			// end_addr
		}
	else{
		end_addr = start_addr+ my_atoi(argv[3]+1);	// size
	}


	if (start_addr > end_addr) {
		at += sprintf(DebugResult + at, "Invalid : start_addr is bigger than end_addr \r\n");
		return;
	}

	char *path=NULL;
	int  len = strlen( argv[1])+2;
	path = malloc( len);
	if (path == NULL) {
		at += sprintf(DebugResult + at, "memory allocate failed \r\n");
		return;
	}
	sprintf(path ,"%s", argv[1] );
	
	

	FILE *fp = fopen(path,"rb");
	if( fp !=NULL) {
        int size=0;
		for(word i= start_addr; i<=end_addr; i++) {
			int v = fgetc(fp);
			if( v== EOF) break;
			poke_memory(i, v);
			size++;
			}
		fclose(fp);
		at += sprintf(DebugResult + at, "loaded from '%s' %d (0x%4X) bytes  \r\n",argv[1] , size,size);
	}
	else {
		at += sprintf(DebugResult + at, "file '%s' open error  \r\n", argv[1]);
	}
	free( path);
}

//*************************************************************/
//				loadmem
//*************************************************************/
void debug_setbin(int argc, char* argv[])
{
	if (argc != 3) {
		at += sprintf(DebugResult + at, "Invalid!\r\n");
		return;
	}
	int start_addr, end_addr;
	start_addr = my_atoi(argv[2]);



	char* path = NULL;
	int  len = strlen(argv[1]) + 2;
	path = malloc(len);
	if (path == NULL) {
		at += sprintf(DebugResult + at, "memory allocate failed \r\n");
		return;
	}
	sprintf(path, "%s", argv[1]);



	FILE* fp = fopen(path, "rb");
	if (fp != NULL) {
		fseek(fp , 0 ,SEEK_END);
		end_addr = start_addr + ftell(fp);
		fseek(fp, 0, SEEK_SET);
		if( end_addr > 0xffff) {
			at += sprintf(DebugResult + at, "end addres is overflow!\r\n");
			fclose(fp);
			return;
		}
		if (start_addr > end_addr) {
			at += sprintf(DebugResult + at, "Invalid : start_addr is bigger than end_addr \r\n");
			return;
		}

		int size = 0;
		for (word i = start_addr; i <= end_addr; i++) {
			int v = fgetc(fp);
			if (v == EOF) break;
			poke_memory(i, v);
			size++;
		}
		fclose(fp);
		at += sprintf(DebugResult + at, "loaded from '%s' %d (0x%4X) bytes  \r\n", argv[1], size, size);
	}
	else {
		at += sprintf(DebugResult + at, "file '%s' open error  \r\n", argv[1]);
	}
	free(path);
}


//*************************************************************/
//				savemem
//*************************************************************/
void debug_savemem(int argc, char* argv[])
{
	if (argc != 4) {
		at += sprintf(DebugResult + at, "Invalid!\r\n");
		return;
	}
	word start_addr, end_addr;
	start_addr = my_atoi(argv[2]);
	if (argv[3][0] != '#') {
		end_addr = my_atoi(argv[3]);			// end_addr
	}
	else {
		end_addr = start_addr + my_atoi(argv[3] + 1);	// size
	}


	if (start_addr > end_addr) {
		at += sprintf(DebugResult + at, "Invalid : start_addr is bigger than end_addr \r\n");
		return;
	}

	char* path = NULL;
	int  len = strlen(argv[1]) + 2;
	path = malloc(len);
	if (path == NULL) {
		at += sprintf(DebugResult + at, "memory allocate failed \r\n");
		return;
	}
	sprintf(path, "%s",  argv[1]);



	FILE* fp = fopen(path, "w+b");
	if (fp != NULL) {
		int size = 0;
		for (word i = start_addr; i <= end_addr; i++) {
			int v = peek_memory(i);
			fputc(v, fp);
			size++;
		}
		fclose(fp);
		at += sprintf(DebugResult + at, "Save memory [%04X]->'%s' (0x%X) bytes  \r\n", start_addr, argv[1], size, size);
	}
	else {
		at += sprintf(DebugResult + at, "file '%s' open error  \r\n", argv[1]);
	}
	free(path);
}

//*************************************************************/
//				reset
//*************************************************************/
void debug_reset(int argc, char* argv[])
{
	R.PC.W = 0;
	InitMemmap();
	at += sprintf(DebugResult + at, "reset ok  \r\n");
}


//*************************************************************/
//				pwd
//*************************************************************/
void debug_pwd(int argc, char* argv[])
{
	char curdir[PATH_MAX];
	getcwd(curdir, PATH_MAX);		// save current directory
	at += sprintf(DebugResult + at, "'%s' ", curdir);
	at += sprintf(DebugResult + at, "\r\n");
}

//*************************************************************/
//				dir
//*************************************************************/
void debug_dir(int argc, char*argv[] )
{
	OSD_OpenFiler( debugWorkPath);		// debug work path のフォルダを、エクスプローラーで開く

}

//*************************************************************/
//				cd
//*************************************************************/
void debug_cd(int argc,char *argv[])
{
	char curdir[PATH_MAX];
	getcwd( curdir, PATH_MAX);		// save current directory
	if( argc ==2) {
		if( chdir( argv[1] )==0) {
			getcwd(curdir, PATH_MAX);		// save current directory
			at += sprintf(DebugResult + at, "cd to '%s'... SUCCESS", argv[1]);
			at += sprintf(DebugResult + at, "\r\n");
			my_strncpy(debugWorkPath, curdir, PATH_MAX);
		}
		else {
			at += sprintf(DebugResult + at, "cd to '%s'... FAILED", argv[1]);
			at += sprintf(DebugResult + at, "\r\n");
		}
	} else {
		at += sprintf(DebugResult + at, "'%s' ", curdir);
		at += sprintf(DebugResult + at, "\r\n");
	}
}

//*************************************************************/
//				set
//*************************************************************/
void debug_set(int argc, char* argv[])
{
	if(argc >1) {
		if (strcmp(argv[1], "tape") == 0) {
			debug_settape(argc, argv);
		} else if(strcmp(argv[1], "nowait")==0) {
			debug_setnowait(argc, argv);
		} else {
			at += sprintf(DebugResult + at, "Invalid");
			at += sprintf(DebugResult + at, "\r\n");
		}
	}
	else {
		help_set();
	}
}


//*************************************************************/
//				set tape
//*************************************************************/
void debug_settape(int argc, char *argv[])
{
	if( argc ==3) {
		long current = ftell(CasStream[0]);

		fseek(CasStream[0], 0, SEEK_END);
		long size = ftell(CasStream[0]);
		long pos = atoi(argv[2]);
		if (size < pos) {
			at += sprintf(DebugResult + at, "new position: %d is bigger than file size", pos);
			at += sprintf(DebugResult + at, "\r\n");
			fseek(CasStream[0], current, SEEK_SET);  // restore tape position
			return;
		}
		fseek(CasStream[0], pos, SEEK_SET);
	}
}


//*************************************************************/
//				set no wait
//*************************************************************/
void debug_setnowait(int argc, char* argv[])
{
	if (argc == 2) {
		at += sprintf(DebugResult + at, "nowait: 0x%04X - 0x%04X \r\n", nowait_start_addr, nowait_end_addr);;
		return;
	}
	if (argc != 4) {
		at += sprintf(DebugResult + at, "parameter error\r\n");
		return;
	}
	nowait_start_addr = my_atoi(argv[2]);
	nowait_end_addr = my_atoi(argv[3]);

	at += sprintf(DebugResult + at, "set nowait 0x%04X - 0x%04X \r\n", nowait_start_addr, nowait_end_addr);;
}


//*************************************************************/
//			debug bt バックトレース　表示
//*************************************************************/
void debug_bt(int argc, char* argv[])
{
	at += sprintf(DebugResult + at, "    ADDR VALUE \r\n");
	for (int i = stack_idx-1; i >=0 ; i--) {
		at += sprintf(DebugResult + at, "%2d) %04X %04X ", i, stacks[i].stack_addr, stacks[i].value);
		if (stacks[i].cmd == S_PUSH)
			at += sprintf(DebugResult + at, " PUSH ");
		else if( stacks[i].cmd == S_CALL)
			at += sprintf(DebugResult + at, " CALL %04XH ",stacks[i].addr);
		else if( stacks[i].cmd == S_LD)
			at += sprintf(DebugResult + at, " LD SP,%04XH ", stacks[i].addr);
		else
			at += sprintf(DebugResult + at, " EX (SP),HL ");

		at += sprintf(DebugResult + at, " %s ", regname[stacks[i].reg]);
		at += sprintf(DebugResult + at, "\r\n");
	}


//	for (int i = 0x2122; i < 0x2150; i++) {		// test
//		putOneKanji(0, 500, i, 0x6f);
//	}


}

void debug_save_reg( reg r)
{
	pre_reg = r;
}

void do_stacks( byte opcode1 , byte opcode2 , byte opcode3)
{

	switch (opcode1) {
		case 0x31:  push_stacks(R.SP.W , S_LD ,  R_NON, 0, opcode3*256+opcode2); break;
		case 0xE3:  push_stacks(R.SP.W , S_EX  , R_HL, R.HL.W ,0); break;

		//case 0xF5:  push_stacks(R.SP.W , S_PUSH, R_AF, R.AF.W ,0); break;
		//case 0xE5:  push_stacks(R.SP.W , S_PUSH, R_HL, R.HL.W, 0); break;
		//case 0xD5:  push_stacks(R.SP.W , S_PUSH, R_DE, R.DE.W, 0); break;
		//case 0xC5:  push_stacks(R.SP.W , S_PUSH, R_BC, R.BC.W, 0); break;
					break;
					// ----- CALL  -----------
		case 0xCD:  push_stacks(R.SP.W, S_CALL, R_NON, pre_reg.PC.W+3, opcode3 * 256 + opcode2); 
					break;
		case 0xDC:	
		case 0xD4:
		case 0xCC:
		case 0xC4:
		case 0xEC:
		case 0xE4:
		case 0xFC:
		case 0xF4:  if (pre_reg.SP.W != R.SP.W) {
						push_stacks(R.SP.W, S_CALL, R_NON, 0, opcode3 * 256 + opcode2); break;
					}
					break;

					// --------- RET ------------
		case 0xC9:  pop_stacks(); break;

		case 0xD8:
		case 0xD0:
		case 0xC8:
		case 0xC0:
		case 0xE8:
		case 0xE0:
		case 0xF8:
		case 0xF0:  if (pre_reg.SP.W != R.SP.W) {
						pop_stacks();
					}
					break;
		//case 0xF1:  
		//case 0xE1:  
		//case 0xD1:  
		//case 0xC1:  
		//			pop_stacks();
		//			break;
		case 0xDD:
					if (opcode2 == 0xE5)
						push_stacks(R.SP.W, S_PUSH, R_IX, R.IX.W, 0);
					else if (opcode2 == 0xE1)
						pop_stacks();
					break;
		case 0xFD:
					if (opcode2 == 0xE5)
						push_stacks(R.SP.W, S_PUSH, R_IY, R.IY.W, 0);
					else if (opcode2 == 0xE1)
						pop_stacks();
					break;
		case 0xED:
					if (opcode2 == 0x4D || opcode2 == 0x45)
						pop_stacks();
					break;
	}
}



//*************************************************************/
//			push stacks スタック情報をpush
//*************************************************************/
void push_stacks(word stack_addr , int cmd ,int reg , word value ,word addr)
{
	if (stack_idx == MAX_STACKS) {
		for (int i = 1; i < MAX_STACKS - 1; i++) {
			stacks[i - 1] = stacks[i];
		}
		stack_idx--;
	}
	stacks[stack_idx].stack_addr = stack_addr;
	stacks[stack_idx].cmd = cmd;
	stacks[stack_idx].reg = reg;
	stacks[stack_idx].value = value;
	stacks[stack_idx].addr = addr;
	stack_idx++;
}
// stack_idx ==0 だとデータなし、１だと、[0]にデータが書きこまれている


//*************************************************************/
//			pop stacks スタック情報からデータ削除
//*************************************************************/
void pop_stacks(void)
{
	if (stack_idx <= 0) {
		stack_idx =0;
		return;
	}
	stack_idx--;
	stacks[stack_idx].stack_addr = 0;
	stacks[stack_idx].cmd = 0;
	stacks[stack_idx].reg = 0;
	stacks[stack_idx].value = 0;
	stacks[stack_idx].addr = 0;
}


//*************************************************************/
//			debug_print: BASICの変数を表示  またかえるかも？
//*************************************************************/
void debug_print(int argc, char* argv[])
{
	if( sr_mode ) return;

	byte str[256];		// 文字列変数の文字列を格納する

	word var_start   = peek_memory(0xff57)*256+peek_memory(0xff56);
	word array_start = peek_memory(0xff59)*256+peek_memory(0xff58);
	if( var_start == array_start) return;		// 同じだと、変数領域なし

	word p = var_start;
	byte var_name[4];
	var_name[0] = peek_memory(p);  p++; 
	var_name[1] = peek_memory(p);  p++;
	var_name[2] = '$';
	var_name[3] = 0;
	if( (var_name[1] & 0x80)==0x0 )
		{						// 単精度浮動小数点
								 
		}
	else
		{						// 文字列
		memset(str,0,sizeof(str));
		byte len =    peek_memory(p); p+=2;		// 長さを読み込んで、ダミーを飛ばす
		word str_po = peek_memory(p)+peek_memory(p+1) * 256; p+=2;
		for(int i=0; i<len; i++)
			{
			 byte a = peek_memory(str_po + i);
			 if( a<' ') { a=' ';}
			 str[i]= a;
			}
		}

	at += sprintf(DebugResult + at, "%s="  , var_name);
	at += sprintf(DebugResult + at, "'%s' ", str);
	at += sprintf(DebugResult + at, "\r\n");
}


//*************************************************************/
//			debug_dump: ダンプ命令
/*************************************************************/
void debug_dump(int argc, char* argv[])
{
	byte *S;
	byte tmp[20];
	static word addr=0;		// 前回の続き
	if( argc >1)			
		{
		 S= argv[1];
		}
	else
		{					// パラメータなしだと、前回の続きから再開
		 sprintf(tmp,"0x%X",addr);
		 S= tmp;
		}
	addr = make_memdump(NULL, S, 0);
	
}


//*************************************************************/
//			debug_edit
/*************************************************************/
void debug_edit(int argc, char* argv[])
{
	word addr=0;
	if (argc > 1)
		{
		 addr = my_atoi( argv[1]);
		 dump_xx = addr & 0xf;
		 dump_yy = 0;
		 dump_start_adr = addr & 0xfff0;
		 
		}
	
	current_pain = P_DUMP;		// ダンプリストに移動する
	DisplayDisasm();
	DisplayMemDump();

}


//*************************************************************/
//			debug_fill
/*************************************************************/
void debug_fill(int argc, char* argv[])
{
	if (argc < 4) {
		at += sprintf(DebugResult + at, "Invalid!\r\n");
		return;
	}
	int start_addr = my_atoi(argv[1]);
	int end_addr   = my_atoi(argv[2]);
	int value      = my_atoi(argv[3]);

	if (start_addr >= 0xffff) {
		at += sprintf(DebugResult + at, "Invalid start_address\r\n");
	}
	if (end_addr >= 0xffff) {
		at += sprintf(DebugResult + at, "Invalid end_address\r\n");
	}

	if (start_addr > end_addr) {
		at += sprintf(DebugResult + at, "Invalid : start_addr is bigger than end_addr \r\n");
		return;
	}

	for (word i = start_addr; i <= end_addr; i++) {
		poke_memory(i, value);
	}
	at += sprintf(DebugResult + at, "filled %04X - %04X %02X \r\n",start_addr, end_addr ,value);
}




void help_set(void)
{
	at += sprintf(DebugResult + at, "setting  \r\n");	// 色んな設定ができる
	at += sprintf(DebugResult + at, " - set tape <new tape position> \r\n");
	at += sprintf(DebugResult + at, " - set nowait <start addr> <end addr> \r\n");
}


//*************************************************************/
//				HELP
//*************************************************************/
void debug_help(int argc, char* argv[])
{
	if (argc == 2) {
		int cmd_no = DebugConvCmd(argv[1]);
		switch( cmd_no) {
			case D_ANIMATE: break;
			case D_REG:     at += sprintf(DebugResult + at, "reg <name> <value>\r\n");
							at += sprintf(DebugResult + at, "set to register\r\n");
							at += sprintf(DebugResult + at, "\r\n");
							at += sprintf(DebugResult + at, "no parameter : show all registers\r\n");
							at += sprintf(DebugResult + at, "name: register name \r\n");
							at += sprintf(DebugResult + at, "      AF BC DE HL IX IY SP PC AF'BC'DE'HL'\r\n");
							at += sprintf(DebugResult + at, "      A B C D E H L  A'B'C'D'E'H'L'\r\n");
							at += sprintf(DebugResult + at, "value: setting value \r\n");
							break;
			case D_STEP:    break;
			case D_GO:      
			case D_GOFULL:  at += sprintf(DebugResult + at, "g:    continue with monitor\r\n");
							at += sprintf(DebugResult + at, "G:    continue without monitor\r\n");
							break;
			case D_DUMP:    at += sprintf(DebugResult + at, "dump <addr>\r\n");
							at += sprintf(DebugResult + at, "dump list\r\n");
							break;
			case D_DISASM:  at += sprintf(DebugResult + at, "disasm <addr>\r\n");
							at += sprintf(DebugResult + at, "disasmble on CURRENT memory bank\r\n");
							at += sprintf(DebugResult + at, "\r\n");
							break;
			case D_UR:      at += sprintf(DebugResult + at, "ur <addr>\r\n");
							at += sprintf(DebugResult + at, "disasmble MAIN RAM\r\n");
							at += sprintf(DebugResult + at, "\r\n");
							break;
			case D_OUT:     at += sprintf(DebugResult + at, "out <i/o port> <value>\r\n");
							at += sprintf(DebugResult + at, "output to i/o port\r\n");
							break;
			case D_IN:      at += sprintf(DebugResult + at, "in <i/o port> <\r\n");
							at += sprintf(DebugResult + at, "input from i/o port\r\n");
							break;
			case D_BREAK:	at += sprintf(DebugResult + at, "break <action> <addr/port> #<slot no>\r\n");
							at += sprintf(DebugResult + at, "set break points\r\n");
							at += sprintf(DebugResult + at, "\r\n");
							at += sprintf(DebugResult + at, "no parameter:   show all breakpoints\r\n");
							at += sprintf(DebugResult + at, "break <addr>:   break at address\r\n");
							at += sprintf(DebugResult + at, "\r\n");
							at += sprintf(DebugResult + at, "action:in:      break at in \r\n");
							at += sprintf(DebugResult + at, "       out:     break at out \r\n");
							at += sprintf(DebugResult + at, "       on:      enable break point \r\n");
							at += sprintf(DebugResult + at, "       off:     disable break point \r\n");
							at += sprintf(DebugResult + at, "slot no:        #1-#10\r\n");
							break;

			case D_SAVEVRAM:at += sprintf(DebugResult + at, "savevram <x> <y> <w> <h>\r\n");
							at += sprintf(DebugResult + at, "save vram to EXT-RAM (mode 6 SCREEN 2)\r\n");
							break;
			case D_LOADVRAM:at += sprintf(DebugResult + at, "loadram <slot no> <x> <y> \r\n");
							at += sprintf(DebugResult + at, "load vram from EXT-RAM (mode 6 SCREEN 2)\r\n");
							break;
			case D_VRAMS:   break;
			case D_NOWAIT:  at += sprintf(DebugResult + at, "nowait <start addr> <end addr>\r\n");
							at += sprintf(DebugResult + at, "Accelerate only specific routines\r\n");
							break;
			case D_LOADMEM: at += sprintf(DebugResult + at, "loadmem <filename> <start addr> <end addr>\r\n");
							break;
			case D_SAVEMEM: at += sprintf(DebugResult + at, "savemem <filename> <start addr> <end addr>\r\n");
							break;
			case D_SETBIN:  at += sprintf(DebugResult + at, "setbin  <filename> <start addr>\r\n");
							break;
			case D_RESET:   break;
			case D_PWD:     break;
			case D_SET:     help_set(); break;
			case D_PRINT:	break;	   
		}
		return;
	}
}


//*************************************************************/
//				Debug Command   デバッグコマンドの実行
//    Input:　
//    Output: 1: go  2:go full
//*************************************************************/
int DebugCommand(reg *R, const char *Command)
{
	size_t i;
	char tmp[128];
	char *S;
	int  cmd_no;

	int argc;
	char *argv[100];

    ClearAttr();
	DebugResult[0]=0; at=0;	// 存在しないコマンドを入力した場合、レスポンスなし
    
    DisplayDisasm();
	DisplayStatus(R);
	DisplayMemDump();
	DisplayIO();
	DisplayFunction();

	if( !*Command) return 0;
	conv_argv( Command , &argc , argv);
	if( argc ==0) return 0; 
	cmd_no = DebugConvCmd( argv[0] );

	//printf("Command argv= %s %s %s  argc =%d \n",argv[0], argv[1], argv[2] ,argc);

	at2 = at = 0;


	my_strncpy(tmp, Command ,127);
	S= tmp;
	for(i=0;i<strlen(tmp);i++)
		{
		if( *S ==' ')
			break;
		S++;
		}
	
	switch( cmd_no)
		{
		case D_ANIMATE: animatemode = !animatemode;break;		// test!
		case D_HELP:  if( argc ==1) 
							DisplayUsage();
					  else
							debug_help(argc, argv);
					  break;
		case D_REG:   if( argc ==1 ) DisplayRegisters(R); else debug_reg(argc, argv); break;
		case D_STEP:  disasm_is_backscroll=0; return 1;
		case D_GO:    Trap=0xFFFF;Trace=0; disasm_is_backscroll=0; kbFlagGraph = 0; p6key = 0; return 1;
		case D_GOFULL:Trap=0xFFFF;Trace=0; disasm_is_backscroll=0; kbFlagGraph = 0; p6key = 0; return 2;
		case D_DUMP:   debug_dump(argc,argv); break;			// memory dump
		case D_EDIT:   debug_edit(argc,argv); break;
		case D_DISASM: is_debug_rdmem_ram=0; make_disasm( R->PC.W ,S );break;			// disasm
		case D_UR:     is_debug_rdmem_ram=1; make_disasm( R->PC.W ,S );break;           // ur
		case D_OUT:    debug_out( argc ,argv ); break; 
		case D_IN:     debug_in( argc ,argv );  break; 
		case D_BREAK:  debug_break( argc, argv ); break;
		case D_SAVEVRAM: debug_savevram(argc, argv); break;
		case D_LOADVRAM: debug_loadvram(argc, argv); break;
		case D_VRAMS:    debug_vrams(); break;
		//case D_NOWAIT:   debug_nowait(argc, argv); break;
		case D_LOADMEM:  debug_loadmem(argc, argv); break;
		case D_SAVEMEM:  debug_savemem(argc, argv); break;
		case D_RESET:    debug_reset(argc,argv); break;
		case D_PWD:      debug_pwd(argc, argv);  break;
		case D_DIR:		 debug_dir(argc, argv);  break;
		case D_CD:		 debug_cd(argc, argv);  break;
		case D_SET:		 debug_set(argc, argv); break;
		case D_SETBIN:   debug_setbin(argc, argv); break;

		case D_BT:       debug_bt(argc, argv); break;
		case D_PRINT:	 debug_print(argc,argv); break;
		case D_FILL:	 debug_fill(argc,argv); break;
		case D_HOGE:	 break;
		default:         at += sprintf(DebugResult + at, "? Unknown command \n\r");

		}

  /* Continue emulation */
  	DisplayStatus(R);
	DisplayIO();

  return 0;
}






//*************************************************************/
//				Debug Put String    デバッグ用文字列の出力
//*************************************************************/

// ******** write string to Debug screen  *****************
void DebugPutString( int x, int y , unsigned char *str ,int max_x ,char *attr)
{
	unsigned char c;
	int  sx;
	
	setRefreshSurface( debug_surface);
	setwidth(1-1);


	sx =x;
	do {
		c = *str;
		if( c >= ' ' || c==0 )
		    {
		    putOneChar(x,y ,c , *attr); 
		    x++;
			}
		else if( c =='\n')            // caridge return
			{
			for( ;x< sx+max_x ; x++)
		      putOneChar(x,y ,' ', 0x00); 
		   
			y++; 
			x= sx;     // auto indent
			}
		else if( c=='\\')		// esc sequence
			{					// ESC[ fore color ; back color m　　
			 str++;
			 if( *str=='[')	
			 	{
			 	int fore_color=0;
			 	int back_color=0;
			 	int i;
			 	char fcol[10],bcol[10];
				char *p= fcol;				// foreground color

				fcol[0]= bcol[0]=0;
				
			 	for(i=0; i< 3; i++)
			 		{
			 	 	if( *str=='m') 				// end of esc sequence
			 	 		break;
			 	 	else if( *str==';') 		// change to background color
			 	 		{p=bcol; i=0;}
			 	 	else if( isdigit( *str))	// store number 
			 	 		{
			 	 		*p++ = *str++;
						*p =0;
						}
			 		}
			 	 fore_color = atoi( fcol );
			 	 back_color = atoi( bcol );
			 	 if( fore_color == 0) fore_color =16;
			 	}
		  }
		str++;
		attr++;
		}
	while( c != 0);
	
	while( x < MAX_COMMANDCOWS)                 // put space character
	    {
		 putOneChar(x,y ,' ', 0x00); 
         x++;
        }

	setRefreshSurface( surface1);
	setwidth(scale-1);
	//RefreshDebugWindow();
}



void  DebugDo(void)
{

	switch( current_pain)
		{
		case P_COMMAND:	DebugCommandPrompt(); break;
		case P_DISASM:  DebugDisasmPrompt();  break;
		case P_DUMP:    DebugDumpPrompt(); break;
		}
}




//*************************************************************/
//				Debug Disasm Prompt  デバッグの逆アセンブラ　入力待ち
//*************************************************************/
// ********* read key  on Debug screen *****************
void  DebugDisasmPrompt( void)
{
	int keydown, scancode;
	char J=0;
	int ret;
	int keyGFlag;
	int p6keycode;
	int osdkeycode;
	int idx;

	DisplayDisasm();

	ret = read_keybuffer( keybuffer, NULL ,&keydown , &scancode ,&osdkeycode ) ;
	if( ret)
		{
		keyboard_set_stick( osdkeycode , keydown);

	 	if( keydown)
			{
			printf("keydown=%02X (%c)\n",  osdkeycode, osdkeycode);

			switch( osdkeycode)
				{
				case OSDK_F9:			// ブレークポイント設定
					idx = get_breakpoint( B_PC,  disasm_cursor_adr);	// 条件に合うブレークポイントが設定されているか？
					if( idx >=1)		// すでにブレークポイントが設定されているか？
						set_breakpoint( B_PC , disasm_cursor_adr ,idx ,255);	// すでに設定されていたら、反転する
					else				// 新規設定
						{
						idx = get_breakpoint_disable();		// 無効なインデックスを探す
						if( idx >=1)
							set_breakpoint( B_PC , disasm_cursor_adr ,idx ,1);		// 設定する
						}

					DebugPutResult( );
					DebugResult[0]=0;
					at=0;
					{
					 com_xx=0;
					 commandLine[ com_back_log_yy][com_xx++]='I';
					 commandLine[ com_back_log_yy][com_xx++]='P';
					 commandLine[ com_back_log_yy][com_xx++]='6';
					 commandLine[ com_back_log_yy][com_xx++]='>';
					}
					RefreshDebugString();
					break;
				case OSDK_F11:
					strcpy( DebugResult_sub , "IP6>s\n");
				 	inTrace = DEBUG_END;
					break;

				case OSDK_TAB:
					if(( keyboard_get_stick() & STICK0_SHIFT)==0) 
						{
						//dump_is_ram_bak = dump_is_ram; 

						current_pain= P_DUMP;
						DisplayDisasm();
						DisplayMemDump();
						}
					else
						{
						current_pain= P_COMMAND;
						DisplayDisasm();
						DisplayMemDump();
						}
					break;
				case OSDK_UP:
					if( disasm_yy >0) 
						{
						disasm_yy--;
						DisplayDisasm();
						break;
						}				/* 範囲外だと、PAGEUPを実行 */
					else
						{
						if( !disasm_is_backscroll ) disasm_is_backscroll=1;						
										/* 命令長によって、戻るアドレスを決定する */
						disasm_start_adr = disasm_pre_adr;
						//disasm_start_adr --;
						DisplayDisasm();
						}
					break;
				case OSDK_PAGEUP:
					if( !disasm_is_backscroll ) disasm_is_backscroll=1;
												
										/* 命令長によって、戻るアドレスを決定する */
					if( keyboard_get_stick() & STICK0_SHIFT) // shift+PAGEUP
						disasm_start_adr -= 0x1000;
					else
						disasm_start_adr = disasm_pre2_adr;
					//disasm_start_adr --;
					DisplayDisasm();
					break;
					
				case OSDK_DOWN:
					if( disasm_yy < MAX_DISASM_LINES-1) 
						{
						disasm_yy++;
						DisplayDisasm();
						}			/* 範囲外だと、PAGE DOWN を実行*/
					else
						{
						if( !disasm_is_backscroll ) disasm_is_backscroll=1;
						disasm_start_adr = disasm_next_adr;
						DisplayDisasm();
						}
					break;
				case OSDK_PAGEDOWN:
					if( !disasm_is_backscroll ) disasm_is_backscroll=1;
					if( keyboard_get_stick() & STICK0_SHIFT) // shift+PAGEDOWN
						disasm_start_adr += 0x1000;
					else
						disasm_start_adr = disasm_next2_adr;
					DisplayDisasm();
					break;
				}
			keyboard_set_key( osdkeycode , &keyGFlag , &p6keycode);
			}
		}
	chk_backlog();			// backlog チェック
	}


					
//*************************************************************/
//				Debug Dump Prompt  デバッグのダンプリスト　入力待ち
//*************************************************************/
// ********* read key  on Debug screen *****************
void  DebugDumpPrompt( void)
{
	int keydown, scancode;
	char J=0;
	int ret;
	int keyGFlag;
	int p6keycode;
	int osdkeycode;

	static char hex[3];
	static char attr[3]= "\x0a\x0a";
	static int  hex_idx=0;

	//DisplayMemDump();

	ret = read_keybuffer( keybuffer, NULL ,&keydown , &scancode ,&osdkeycode ) ;
	if( ret)
		{
		keyboard_set_stick( osdkeycode , keydown);

	 	if( keydown)
			{
			int key_flag=0;		// 1: 違うキーの処理もする
			printf("keydown=%02X (%c)\n",  osdkeycode, osdkeycode);
			
			// -------------- 16進数　入力 --------------
			if(( osdkeycode >='0' && osdkeycode <='9') || ( osdkeycode >='A' && osdkeycode <='F') || ( osdkeycode >='a' && osdkeycode <='f') 
			 ||( osdkeycode >=OSDK_KP0 && osdkeycode <= OSDK_KP9))
				{
				 int tmp;

				 if( osdkeycode >= OSDK_KP0 && osdkeycode <= OSDK_KP9)  osdkeycode = osdkeycode - OSDK_KP0+'0';
				 if( osdkeycode >= 'a' && osdkeycode <= 'z') osdkeycode -= 0x20;
				 hex[ hex_idx++] = osdkeycode;
				 hex[ hex_idx] =0;
				 
				 DebugPutString( DUMP_TOP_XX+5+ dump_xx*3 , DUMP_TOP_YY+ 1+dump_yy , hex , 3 ,attr );

				 if( hex_idx >=2)		// 16進数入力確定
					{
					 hex[ 2 ] = 0;
					 sscanf( hex ,"%02X",&tmp);

					 word addr = (dump_start_adr + dump_yy * 16 + dump_xx) & 0xffff;
					 if( dump_is_ram == DEFAULT_BANK)
						 M_WRMEM( addr , tmp & 0xff);
					 else if( dump_is_ram == RAM_BANK )
						poke_memory( addr ,tmp & 0xff);
					 else if( dump_is_ram == EXTRAM_BANK)
						poke_ext_memory( addr ,tmp & 0xff);

					 hex_idx=0;
					 if( dump_xx <16-1 )
						{
						 dump_xx++;
						}
					 else 
						{
						 if( dump_yy < MAX_DUMP_LINES-1)
							{ dump_yy++; dump_xx=0;}
						 else
							{ osdkeycode = OSDK_PAGEDOWN; key_flag =1; dump_xx=0; }
						}
					 DisplayMemDump();
					 disasm_is_backscroll =1;
					 DisplayDisasm();
					}
				}

			do {
				switch( osdkeycode)
					{
					case OSDK_END:
						dump_is_ram++;
						if( !use_extram64 && dump_is_ram >1)
							dump_is_ram= 0; 
						else if( dump_is_ram >2)
							dump_is_ram = 0;
						DisplayMemDump();
						break;
					case OSDK_TAB:
						if(( keyboard_get_stick() & STICK0_SHIFT)==0) 
							{
							//dump_is_ram = dump_is_ram_bak;		//ダンプから抜けるときに、元に戻す
							current_pain= P_COMMAND;
							DisplayMemDump();
							}
						else 
							{
							current_pain= P_DISASM;
							DisplayMemDump();
							}
						break;
					case OSDK_UP:
						if( dump_yy >0) 
							dump_yy--;
						else
							{osdkeycode = OSDK_PAGEUP; key_flag =1;}	/* 一番上 */
						DisplayMemDump();
					    hex_idx=0;		// カーソル移動すると、HEX入力をキャンセル
						break;
					case OSDK_DOWN:
						if( dump_yy < MAX_DUMP_LINES-1) 
							dump_yy++;
						else
							{osdkeycode = OSDK_PAGEDOWN; key_flag =1;}	/* 一番下 */
						DisplayMemDump();
					    hex_idx=0;		// カーソル移動すると、HEX入力をキャンセル
						break;
					case OSDK_LEFT:
						if( dump_xx >0) 
							dump_xx--;
						else 
							if( dump_yy >0) 
								{dump_xx=15; dump_yy--;}
							else
								{osdkeycode = OSDK_PAGEUP; key_flag=1; dump_xx=15; }	/* 左上　*/
						DisplayMemDump();
					    hex_idx=0;		// カーソル移動すると、HEX入力をキャンセル
						break;
					case OSDK_RIGHT:
						if( dump_xx <16-1) 
							dump_xx++;
						else if( dump_yy < MAX_DUMP_LINES-1) 
								{dump_xx=0;	dump_yy++;}
							else
								{osdkeycode = OSDK_PAGEDOWN; key_flag=1; dump_xx=0;}	/* 右下 */
						DisplayMemDump();
					    hex_idx=0;		// カーソル移動すると、HEX入力をキャンセル
						break;
					case OSDK_PAGEUP:
						{
						int pow;
						if( key_flag) pow=1; else pow=4;

						if( keyboard_get_stick() & STICK0_SHIFT) pow=16*16;	// shift+PAGEUP
						dump_start_adr-= 16*pow;
						DisplayMemDump();
						key_flag =0;
					    hex_idx=0;		// カーソル移動すると、HEX入力をキャンセル
						break;
						}
					case OSDK_PAGEDOWN:
						{
						int pow;
						if( key_flag) pow=1; else pow=4;

						if( keyboard_get_stick() & STICK0_SHIFT) pow=16*16;	// shift+PAGEDOWN
						dump_start_adr+= 16*pow;
						DisplayMemDump();
						key_flag =0;
					    hex_idx=0;		// カーソル移動すると、HEX入力をキャンセル
						break;
						}
					}
				}
			while(key_flag);
			keyboard_set_key( osdkeycode , &keyGFlag , &p6keycode);
			}
		}
	}


//*************************************************************/
//				Debug Command Prompt   デバッグのコマンドプロンプト入力待ち
//*************************************************************/
// ********* read key  on Debug screen *****************
void  DebugCommandPrompt( void)
{
	int keydown, scancode;
	char J=0;
	int ret;
	int keyGFlag;
	int p6keycode;
	int osdkeycode;

	static int cmd_history_idx = 0;		  /* コマンドヒストリー のインデックス */
	static int current_cmd_history_idx=0; /* コマンドヒストリー の現在インデックス */
	static int start_history_cmd=0;		  /* 1:ヒストリーが開始した　0:通常 */

	if( com_xx==0)
		{
		 commandLine[ com_back_log_yy][com_xx++]='I';
		 commandLine[ com_back_log_yy][com_xx++]='P';
		 commandLine[ com_back_log_yy][com_xx++]='6';
		 commandLine[ com_back_log_yy][com_xx++]='>';
		}
	commandLineAttr[ com_back_log_yy ][com_xx] = 0xf0;		// カーソル反転

	ret = read_keybuffer( keybuffer, NULL ,&keydown , &scancode ,&osdkeycode ) ;
	if( ret)
		{
		keyboard_set_stick( osdkeycode , keydown);

	 	if( keydown)
			{
			printf("keydown=%02X (%c)\n",  osdkeycode, osdkeycode);

			switch( osdkeycode)
				{
				case OSDK_END:
					dump_is_ram++;
					if (!use_extram64 && dump_is_ram > 1)
						dump_is_ram = 0;
					else if (dump_is_ram > 2)
						dump_is_ram = 0;
					DisplayMemDump();
					break;
				case OSDK_UP:		// HISTORY
					{
					int i;
					if( current_cmd_history_idx >0)
						current_cmd_history_idx--;

					commandLineAttr[ com_back_log_yy ][com_xx] = 0x0f;		// カーソル消す
					strcpy( &commandLine[ com_back_log_yy][ 4] ,commandHistory[ current_cmd_history_idx]);
					com_xx = 4+ strlen( commandHistory[ current_cmd_history_idx]);
					}
					break;
				case OSDK_DOWN:		// HISTORY
					{
					int i;
					if( current_cmd_history_idx < cmd_history_idx)
						current_cmd_history_idx++;
					
					commandLineAttr[ com_back_log_yy ][com_xx] = 0x0f;		// カーソル消す
					strcpy( &commandLine[ com_back_log_yy][ 4] ,commandHistory[ current_cmd_history_idx]);
					com_xx = 4+ strlen( commandHistory[ current_cmd_history_idx]);
					}
					break;
				case OSDK_F11:
					commandLine[ com_back_log_yy][com_xx++] = 's'; 
					commandLine[ com_back_log_yy][com_xx++] = 0; 
					
					osdkeycode = 0x0d;
					break;
					
				case OSDK_TAB:
					if(( keyboard_get_stick() & STICK0_SHIFT)==0) 
						current_pain= P_DISASM;
					else
						current_pain= P_DUMP;
					commandLineAttr[ com_back_log_yy ][com_xx] = 0;
					break;
				case OSDK_PAGEUP:
					{
					if( !com_is_backscroll && com_top_yy >0) 
						{com_is_backscroll=1; com_top_yy_bak = com_top_yy;}

					if( com_top_yy >0) com_top_yy -=5;
					if( com_top_yy <0) com_top_yy = 0;
					RefreshDebugString();
					}
					break;
					
				case OSDK_PAGEDOWN:
					{
					if( com_top_yy < com_top_yy_bak && com_is_backscroll ) 
						{
						com_top_yy +=5;
						if( com_top_yy > com_top_yy_bak) 
							{com_top_yy = com_top_yy_bak; com_is_backscroll=0; com_top_yy_bak=-1;}
						} 
					else 
						{ com_is_backscroll = 0; com_top_yy_bak= -1;}

					RefreshDebugString();
					}
					break;
				default:
					if( com_is_backscroll)		/* バックスクロール中に 違うキーを押した場合、バックスクロールを中止して元に戻る*/
						{
						com_top_yy = com_top_yy_bak; com_is_backscroll=0; com_top_yy_bak = -1;
						RefreshDebugString();
						if( osdkeycode == OSDK_RETURN) osdkeycode= 0;
						}
					break;
				}
			keyboard_set_key( osdkeycode , &keyGFlag , &p6keycode);



		
			if( (p6keycode >=' ' && p6keycode < 0x7f )  && com_xx < MAX_COMMANDCOWS)
				{
				commandLine    [ com_back_log_yy][com_xx] = p6keycode; 	// store to command Line
				commandLineAttr[ com_back_log_yy][com_xx] = 0x0f;
				com_xx++;
				commandLine    [ com_back_log_yy][com_xx] = 0x0;
				commandLineAttr[ com_back_log_yy][com_xx] = 0xf0;
				}
			else if( p6keycode==0x8 && com_xx > 4)		// back space
				{
				com_xx--;
				commandLine[ com_back_log_yy][com_xx] = 0;
				}
			else if( p6keycode==0xd)				// return
				{
				int ret;

				if( cmd_history_idx > MAX_CMD_HISTORY-2)		// あふれる前に、前につめる
					{
					 int i;
					 for(i=0; i< MAX_CMD_HISTORY-2; i++) 
						 strcpy( commandHistory[ i] , commandHistory[i+1]);
					 cmd_history_idx = MAX_CMD_HISTORY-2;
					 //*commandHistory[ cmd_history_idx ] = 0;		// 一番下は、必ず、空文字列にする
					}
				strcpy( commandHistory[ cmd_history_idx++ ] ,commandLine[ com_back_log_yy]+4);
				current_cmd_history_idx = cmd_history_idx;


				com_xx=0;
				ret = DebugCommand( &R ,commandLine[ com_back_log_yy]+4 );	// execute debug command 
				if( ret==0 )
					{
						// enter のみ入力したとき は、改行するようにする
					 com_yy++; com_back_log_yy++;		// 命令の次にｙ座標移す。
					 DebugPutResult();
					 
			 		 printf("**** return yy=%d \n",com_yy);
					}
				else if( ret ==1)
					{
				 	inTrace = DEBUG_END;
				 	com_yy++; com_back_log_yy++;		// 命令の次にｙ座標移す。
					}
				else if( ret ==2)
					{
				 	inTrace = DEBUG_END;
				 	com_yy++; com_back_log_yy++;		// 命令の次にｙ座標移す。
					close_debug_dialog();
					return ;
					}
				}
			}

		chk_backlog();			// backlog チェック			
		printf( "scancode =%X  keycode=%X \n",scancode ,J);
		}


	// fullscreen / window 切り替え時に消える対策
    DisplayDisasm( );
	DisplayStatus(&R);
    DisplayMemDump();
	DisplayIO();
	DisplayFunction();

	RefreshDebugString();
}



void chk_backlog(void)
{
		if( com_back_log_yy >MAX_BACK_LOG-50 && !com_is_backscroll)        // overflow backlog buffer
		  {
	   	   int y;
           for(y=0; y< MAX_BACK_LOG -50; y++)
                {
                memcpy( commandLine[ 0+y] , commandLine[ 50+y] , MAX_COMMANDCOWS);
                }
            for(y= MAX_BACK_LOG- 50 ; y< MAX_BACK_LOG ; y++)
                {
                memset( commandLine[ y] ,0 , MAX_COMMANDCOWS);
                }
            com_back_log_yy-= 50;
            com_top_yy -=50;
		  }		 
}			


//*************************************************************/
//			Debug Put Result  デバッグ結果の出力
//*************************************************************/
void DebugPutResult(void)
{
	
	char c;
	int x;
	char *in;
		
	x=0;
	in = DebugResult;
	do {
		c = *in;
		if( c >=' ')
			{
			 commandLine [ com_back_log_yy][x] = c;
			 x++;
			}
		else if( c=='\n')
			{
			com_back_log_yy++; 
			x=0;  com_yy++;        // cursor
			}
		in++;
		}
	while( c !=0);


 	if( com_yy > MAX_COMMANDLINES-2 && !com_is_backscroll) 		// scroll up ? 
		{
 	 	int scroll_yy;
 	 	scroll_yy = com_yy - (MAX_COMMANDLINES-2);        // scroll lines
	 	com_top_yy += scroll_yy;
 	 	com_yy -= scroll_yy;
 	 	printf("Debug scollup  sa=%d  com_back_log_yy %d ,scroll_yy=%d , com_top_yy=%d  , yy=%d \n",com_back_log_yy- com_top_yy , com_back_log_yy ,scroll_yy, com_top_yy, com_yy);
		}	
}

//*************************************************************/
//			Refresh Debug　String　コマンドラインをスクリーンに出力	
//*************************************************************/
void RefreshDebugString(void)
{
    int y;
    
    
    // *************** command line ***************
    for(y=0; y < MAX_COMMANDLINES; y++)
	   DebugPutString( 0, y +COMMANDLINE_TOP_YY , commandLine[ com_top_yy+ y] ,MAX_COMMANDCOWS ,commandLineAttr[ com_top_yy+y]);
	   
   	RefreshDebugWindow();
}

//*************************************************************/
//		open debug dialog   デバッグのダイアログを開く		
//*************************************************************/
void open_debug_dialog(void)
{

	if( !is_open_debug_dialog )		// すでに開いているか確認
		{
		//	int isfullscrn = isFullScreen();
		if( inTrace== DEBUG_END) backup_scale = scale;

	    debug_surface = OSD_CreateSurface(M6WIDTH* DEBUG_WINDOW_RATE ,M6HEIGHT* DEBUG_WINDOW_RATE ,bitpix ,SURFACE_BITMAP);

		resizewindow((float)scale ,DEBUG_WINDOW_RATE  , WINDOW_NOBOARDER);

		ignore_padding_flag =1;		// padding 無視する
		if( isFullScreen() ) {toggleFullScr();toggleFullScr();}	// isfullscrn 1:  back to fullscreen 

		is_open_debug_dialog = 1;	// 開いた状態
		}
}

//*************************************************************/
//		close debug dialog   デバッグのダイアログを閉じる		
//*************************************************************/
void close_debug_dialog(void)
{
	scale = backup_scale;

	resizewindow((float)scale, (float)scale ,0);
	OSD_ReleaseSurface( debug_surface); 
	debug_surface=NULL;

	if( isFullScreen()) {toggleFullScr();toggleFullScr();}	// isfullscrn 1:  back to fullscreen
	ignore_padding_flag =0;

	inTrace=DEBUG_END;
	is_open_debug_dialog = 0;
	clear_keybuffer( keybuffer);
}

#endif /* DEBUG */




/*
	at += sprintf
	(
		DebugResult+at, "AF:%04X HL:%04X DE:%04X BC:%04X PC:%04X SP:%04X IX:%04X IY:%04X\r\n",
		R->AF.W,R->HL.W,R->DE.W,R->BC.W,R->PC.W,R->SP.W,R->IX.W,R->IY.W
	);
	at += sprintf
	( 
		DebugResult+at, "AT PC: [%02X - %-13s]   AT SP: [%04X]   FLAGS: [%s]\r\n\r\n",
		M_RDMEM(R->PC.W),S,M_RDMEM(R->SP.W)+M_RDMEM(R->SP.W+1)*256,T
	);
*/

#if 0
	S[0] = '\0';
	strcpy(S, Command);
	for(J = 0; S[J] >= ' '; J++)
		S[J]=toupper(S[J]);
	S[J]='\0';
    switch (S[0]) {
	case 'H':
	case '?':   DisplayUsage();
				break;
	case 'R':   DisplayRegisters(R);
				break;
	case 'S':   return(1);
	case '=':   if (strlen(S) >= 2) { sscanf(S+1,"%hX",&Trap);Trace=0;return(1); }
				break;
	case '+':   if (strlen(S) >= 2) {
					sscanf(S+1,"%hX",&Trap);
					Trap+=R->PC.W;Trace=0;
					return(1);
				}
				break;
	case 'J':   if (strlen(S) >= 2) { sscanf(S+1,"%hX",&(R->PC.W));Trace=0;return(1); }
				break;
	case 'C':   Trap=0xFFFF;Trace=0;return(1); 
	case 'M':   make_memdump( R,S ,0);
	            break;
	case 'D':   make_disasm( R ,S );
		        break;
	case 'O':   if (strlen(S) >=2) {
					int value;
					sscanf(S+1,"%hX",&value);
					DoOut( 0xf0, value);
				}
	}
#endif

