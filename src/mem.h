/** iP6: PC-6000/6600 series emualtor ************************/
/**                                                         **/
/**                          mem.h                          **/
/**                                                         **/
/** modified by Windy 2002-2004                             **/
/** This code is based on ISHIOKA Hiroshi 1998-2000         **/
/** This code is based on fMSX written by Marat Fayzullin   **/
/*************************************************************/

#ifndef _MEM_H
#define _MEM_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <limits.h>
#include "types.h"
#include "buffer.h"
#include "Refresh.h"

extern int Romfile_found;		// TRUE: ROMが見つかった。  FALSE: 見つからなかった
extern int Use_CompatibleROM;	// TRUE: 互換ROMを使用する  FALSE: 使用しない   (TRUE だと存在する機種のみ選択可能にする)
extern int new_Use_CompatibleROM;

extern byte *BASICROM ;			// BASICROM 
extern byte *VOICEROM ;			// VOICEROM
extern byte *KANJIROM ;			// KANJIROM
extern byte *CurKANJIROM ;		// CURRENT KANJIROM

extern byte *SYSROM2 ;			// SYSROM2   (old style)

extern byte *SYSTEMROM1 ;        // SYSTEMROM1
extern byte *SYSTEMROM2 ;		// SYSTEMROM2
extern byte *CGROM6 ;            // CGROM for N66SR-BASIC   add 2002/2/20
/* CGROM6 = CGROM1+ CGROM5  */


extern byte *CGROM ;				// CURRENT CGROM
extern byte *CGROM1 ;			// N60 CGROM
extern byte *CGROM5 ;			// N66 CGROM
extern byte *EXTROM ;			// CURRENT EXTEND ROM
extern byte *EXTROM1 ;			// EXTEND ROM 1
extern byte *EXTROM2 ;			// EXTEND ROM 2

extern byte *EXTKANJIROM;		// EXT KANJIROM
extern byte *EXTRAM64     ;		// EXT RAM 64kb  for SR
extern byte *DEBUG_CGROM;				// DEBUG CGROM;

extern int  extkanjirom;			// EXT KANJIROM 1:enable  0:disable
extern int  new_extkanjirom;

enum { EXTKANJI_NON = 0, EXTKANJI_ROM, EXTKANJI_MAKE };
extern int extkanjirom_type;		// EXKANJI_NON: ROMなし  EXTKANJI_ROM: ROMあり      EXTKANJI_MAKE: 生成した 

extern int  use_extram64    ;		// EXT RAM  64KB     1:enable  0:disable
extern int  new_use_extram64 ;

extern int  use_extram32     ;		// EXT RAM   32KB   for PC-6001  1:enable 0:disable
extern int  new_use_extram32 ;


extern byte *RdMem[8];					// READ  MEMORY MAPPING ADDRESS
extern byte *WrMem[8];					// WRITE MEMORY MAPPING ADDRESS
extern byte *VRAM;						// VRAM ADDRESS (VRAM -> CRTC -> DISPLAY)
extern byte *VRAM_BITMAP;				// BITMAP VRAM ADDRESS for SR ( VRAM -> CRTC -> CPU )
extern byte *EXT_VRAM;					// EXTENED VRAM (Super Graphics)
extern byte *EmptyRAM;
extern byte *RAM;

extern byte EnWrite[4];				// MEMORY MAPPING WRITE ENABLE [N60/N66]
extern unsigned int VRAMHead[2][4];




extern byte CasMode;
extern char PrnName[PATH_MAX];    /* Printer redir. file */

extern char DskName[2][PATH_MAX];    /* Disk image file     */
extern char CasName[2][PATH_MAX];    /* Tape image file     */
extern fpos_t CasSize[2];		/* tape size */

extern char CasPath[2][PATH_MAX];    /* Tape image search path */
extern char DskPath[2][PATH_MAX];    /* Disk image search path */

extern char RomPath[PATH_MAX];    /* Rom  image search path */
extern char RomPath_bak[PATH_MAX];				/* ROM image path backup */

extern char Ext1Name[PATH_MAX];    /* Extension ROM 1 file  (4000h-5fffh)*/
extern char Ext2Name[PATH_MAX];    /* Extension ROM 2 file  (6000h-7fffh)*/

extern char ImgPath[PATH_MAX];    /* snapshot image path */
extern char MemPath[PATH_MAX];    /* memory   image path */
extern char ExtRomPath[ PATH_MAX];		/* Ext ROM image path */

extern FILE *DskStream[2];
extern FILE *CasStream[2];
extern FILE *PrnStream;


extern char read_b[][10];
								// BASIC0 , BASIC1, BASIC2 BASIC3
								// INTRAM0 , INTRAM1 , INTRAM2 ..3,4,5,6,7
								// EXTRAM0 , EXTRAM1 , EXTRAM2 ..3,4,5,6,7
								// SYSROM0 , SYSROM1 ,
extern char write_b[][10];


extern KEYBUFFER *keybuffer;
extern KEYBUFFER *auto_keybuffer;


extern word sr_vram_right_addr[M6HEIGHT+2];	// ******** SR VRAM right start address **************


//extern byte *OLDVRAM1;					// OLD VRAM	(attr data)
//extern byte *OLDVRAM2;					// OLD VRAM (char data)

void memorymap_mode6_readblock(byte Port , byte Value);
void memorymap_mode6_writeblock(byte Port , byte Value);

void memorymap_mode5_readblock0(byte Port , byte Value);
void memorymap_mode5_readblock1(byte Port , byte Value);
void memorymap_mode5_writeblock(byte Port , byte Value);
void memorymap_voice_kanjirom(byte Port, byte Value);
void memorymap_mode1_extmegarom( byte Port , byte Value);

void InitMemmap(void);

void dokodemo_save_mem(void);
void dokodemo_load_mem(void);

byte peek_memory(register word A);
void poke_memory(register word A ,byte V);

char * read_disasm_comment(word adr);


#ifdef __cplusplus
}
#endif

int StartP6(void);
void TrashP6(void);

void init_tapeCounter(void);
int SetTapeCounter(char* fpath, fpos_t cnt);
int GetTapeCounter(char* fpath, fpos_t cnt);


#endif

//extern byte *TEXTVRAM;					// TEXT VRAM ADDRESS (MODE 6 only) fix screen 2,2,1 add 2003/10/25
