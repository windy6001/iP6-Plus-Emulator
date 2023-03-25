/** iP6: PC-6000/6600 series emualtor ************************/
/**                                                         **/
/**                          mem.c                          **/
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

#include <sys/types.h>
#include <sys/stat.h>
#include "chkcrc32.h"


#if HAVE_DIRENT_H
#include <dirent.h>
#else
#ifdef WIN32
#include "win/dirent_msvc.h"
#define  S_ISDIR(m) ((m) & S_IFDIR)
#endif
#endif

#include "os.h"


#include "buffer.h"
#include "types.h"
#include "os.h"
#include "P6.h"
#include "cgrom.h"
#include "fdc.h"
#include "disk.h"
#include "d88.h"
#include "Video.h"
#include "message.h"
#include "mem.h"
#include "Option.h"

#include "schedule.h"
#include "dokodemo.h"
#include "font6x10.h"
#include "chkcrc32.h"

//--------- PC-6001 ---------------
#include "compatible_rom/basicrom60.h"
#include "compatible_rom/cgrom60.h"


//--------- PC-6601 ---------------
#include "compatible_rom/basicrom66.h"
#include "compatible_rom/cgrom6066.h"
#include "compatible_rom/cgrom6666.h"
#include "compatible_rom/voicerom66.h"
#include "compatible_rom/kanjirom66.h"


int load_extroms(void);
int get_filesize( char * path);


int Romfile_found = FALSE;		// TRUE: ROMが見つかった。(互換ROM含む)  FALSE: 見つからなかった
int Use_CompatibleROM = FALSE;	// TRUE: 互換ROMを使用する  FALSE: 使用しない   (TRUE だと存在する機種のみ選択可能にする)
int new_Use_CompatibleROM = FALSE;
	// ***************** MEMORY ************************

byte *BASICROM = NULL;			// BASICROM 
byte *VOICEROM = NULL;			// VOICEROM
byte *KANJIROM = NULL;			// KANJIROM
byte *CurKANJIROM = NULL;		// CURRENT KANJIROM

/*
byte *ROM2 = NULL;
*/
byte *SYSROM2 = NULL;			// SYSROM2   (old style)
/* 99.06.02.*/

byte *SYSTEMROM1 = NULL;        // SYSTEMROM1
byte *SYSTEMROM2 = NULL;		// SYSTEMROM2
byte *CGROM6 = NULL;            // CGROM for N66SR-BASIC   add 2002/2/20
/* CGROM6 = CGROM1+ CGROM5  */


byte *CGROM = NULL;				// CURRENT CGROM
byte *CGROM1 = NULL;			// N60 CGROM
byte *CGROM5 = NULL;			// N66 CGROM
byte *EXTROM = NULL;			// CURRENT EXTEND ROM
byte *EXTROM1 = NULL;			// EXTEND ROM 1
byte *EXTROM2 = NULL;			// EXTEND ROM 2

byte *EXTKANJIROM= NULL;		// EXT KANJIROM


byte *RdMem[8];					// READ  MEMORY MAPPING ADDRESS  0000-1FFF *8 
byte *WrMem[8];					// WRITE MEMORY MAPPING ADDRESS  0000-1FFF *8
byte *VRAM;						// VRAM ADDRESS (VRAM -> CRTC -> DISPLAY)
byte *VRAM_BITMAP;				// BITMAP VRAM ADDRESS for SR ( VRAM -> CRTC -> CPU )
byte *EXT_VRAM;					// EXTENED VRAM (Super Graphics)

byte *EmptyRAM;
byte *RAM;
byte *EXTRAM64     = NULL;		// EXT RAM 64KB  for SR


byte *DEBUG_CGROM;				// DEBUG CGROM;

byte EnWrite[4];				// MEMORY MAPPING WRITE ENABLE [N60/N66]



int  megarom_flag = 0;          // 1: extrom is megarom   
int  megarom_bank_no = 0;		// megarom bank no.

int  extkanjirom=0;				// EXT KANJIROM 1:enable  0:disable
int  new_extkanjirom=0;

int  extkanjirom_type = EXTKANJI_NON;	// EXKANJI_NON: ROMなし  EXTKANJI_FILE: ROMあり      EXTKANJI_MAKE: 生成した 

int  use_extram64     =0;				// EXT RAM   64KB   1:enable  0:disable
int  new_use_extram64 =0;

int  use_extram32     =0;				// EXT RAM   32KB   for PC-6001  1:enable 0:disable
int  new_use_extram32 =0;

int  use_extvram      =0;               // EXT VRAM  32KB
int  new_use_extvram  =0;


void init_extrom_memorymap(void);





unsigned int VRAMHead[2][4] = {
  { 0xc000, 0xe000, 0x8000, 0xa000 },
  { 0x8000, 0xc000, 0x0000, 0x4000 },
};


/* memory block  for debug */

char read_b[8][10];
								// BASIC0 , BASIC1, BASIC2 BASIC3
								// INTRAM0 , INTRAM1 , INTRAM2 ..3,4,5,6,7
								// EXTRAM0 , EXTRAM1 , EXTRAM2 ..3,4,5,6,7
								// SYS1_0  
char write_b[8][10];


char *basic_b[] ={"BASIC0","BASIC1","BASIC2","BASIC3"};
char *intram_b[]={"INRAM0","INRAM1","INRAM2","INRAM3","INRAM4","INRAM5","INRAM6","INRAM7"};
char *extram_b[]={"EXRAM0","EXRAM1","EXRAM2","EXRAM3","EXRAM4","EXRAM5","EXRAM6","EXRAM7"};
char *cur_kanji_b[]={"",""};
char *_kanji_b[]={"KANJI0","KANJI1","KANJI2","KANJI3"};	// left left right right ...
char *voice_b[]={"VOICE0","VOICE1"};

char *extrom1_b=" extrom1 ";
char *extrom2_b=" extrom2 ";
char *_extrom_b[]={"EXTROM1","EXTROM2"};

char empty_b[10]=" EMPTY ";
char *cgrom_b=" CGROM ";



char sys1_b[] ="SYS1  [%X]";
char sys2_b[] ="SYS2  [%X]";
char sr_intram_b[]="INRAM [%X]";
char sr_extram_b[]="EXRAM [%X]";

//char sysrom1_b[8][10];
//char sysrom2_b[8][10];



	// ************** TAPE / DISK / ROM  PATH  ******************
char PrnName[PATH_MAX] = "";    /* Printer redirect. file */
FILE *PrnStream  = NULL;

char CasName[2][PATH_MAX] = {"",""};  /* Tape image file      0:load  1:save  */
FILE *CasStream[2]  = {NULL,NULL};

fpos_t CasSize[2];		/* tape size */


char DskName[2][PATH_MAX] = {"",""};    /* Disk image file      */
FILE *DskStream[2]  = {NULL,NULL};

char CasPath[2][PATH_MAX] = {"",""};    /* Tape image path */
char DskPath[2][PATH_MAX] = {"",""};    /* Disk image path */

char RomPath[PATH_MAX] = "rom/";		/* Rom  image path */
char RomPath_bak[PATH_MAX];				/* ROM image path backup */

char ExtRomPath[ PATH_MAX] = "";		/* Ext ROM image path */

char Ext1Name[PATH_MAX] = "";    /* Extension ROM 1 file  (4000h-5fffh)*/
char Ext2Name[PATH_MAX] = "";    /* Extension ROM 2 file  (6000h-7fffh)*/

char ImgPath[PATH_MAX] = "";    /* snapshot image path */
char MemPath[PATH_MAX] = "";    /* memory   image path */


	// ******** keybuffer ***************
KEYBUFFER *keybuffer;
KEYBUFFER *auto_keybuffer;


// ******** SR VRAM right start address **************
word sr_vram_right_addr[M6HEIGHT+2];


#define MAX_COMMENT_ADDR 0x10000

char *disasm_comment_basic1[MAX_COMMENT_ADDR];	/* BASIC 1  */
char *disasm_comment_basic2[MAX_COMMENT_ADDR];	/* BASIC 2  (SR BASIC 用）*/
char  *disasm_comment_voice[MAX_COMMENT_ADDR];		/* VOICE  */


char *sym_basic60[]={
#include "sym_basic60.h"
""
};

char *sym_basic62[]={
#include "sym_basic62.h"
""
};

char *sym_basic66[]={
#include "sym_basic66.h"
""
};



// ****************************************************************************************
#if 0
enum {BAS_60, BAS_62 , SYS_1, BAS_66 , VOI_62 , VOI_64, VOI_66 , SYS_2 };


struct _disasm_comment_dat {
	char file_basic1[17];
	char file_basic2[17];
	char file_voice[17];
} disasm_comment_dat[] = {
	{"BASICROM.60.sym"},
	{"BASICROM.62.sym", ""               ,"VOICEROM.62.sym"},
	{"SYSTEMROM11.sym", "SYSTEMROM12.sym"},
	{"BASICROM.66.sym", ""               , "VOICEROM.66.sym"},
	{"SYSTEMROM11.sym", "SYSTEMROM12.sym"},
};
#endif

void init_disasm_comment(void)
{
	int i;
	for(i=0;i<MAX_COMMENT_ADDR;i++)
		{
		 disasm_comment_basic1[i] = malloc( 21);
		 strcpy( disasm_comment_basic1[i] , ".");
		}
	for(i=0;i<MAX_COMMENT_ADDR;i++)
		{
		 disasm_comment_basic2[i] = malloc( 21);
		 strcpy( disasm_comment_basic2[i] , ".");
		}
	for(i=0;i<MAX_COMMENT_ADDR;i++)
		{
		 disasm_comment_voice[i] = malloc( 21);
		 strcpy( disasm_comment_voice[i] , ".");
		}
}

void free_disasm_comment(void)
{
	int i;
	for(i=0;i<MAX_COMMENT_ADDR;i++)
		{
		 free( disasm_comment_basic1[i] );
		}
	for(i=0;i<MAX_COMMENT_ADDR;i++)
		{
		 free( disasm_comment_basic2[i] );
		}
	for(i=0;i<MAX_COMMENT_ADDR;i++)
		{
		 free( disasm_comment_voice[i] );
		}

}


int load_disasm_comment_from_array(char **out_dat , char *in_dat[])
{
	char path[ PATH_MAX];
	char buff[256];
	char *p;
	char *q;
	int i;
	int idx;

	i=0;
	while( *in_dat[i])
		{
		strcpy(buff, in_dat[i]);
		 p = strchr(buff , ' ');
		 if( p != NULL)
			{
			*p = 0;
			sscanf(buff,"%X",&idx);
			if( idx >=0 && idx <= 0x7fff)
				{
				 q = strchr(p+1 , '\x0a');	// \x0a　除外
				 if( q != NULL)
					 *q = 0;
				 my_strncpy( out_dat[ idx] , (p+1) ,20);
				}
			}
		 i++;
		}
}



int load_disasm_comment_from_file(char **out_dat , char *filename)
{
	FILE *fp;
	char path[ PATH_MAX];
	char buff[256];
	char *p;
	char *q;
	int i;
	int idx;

	sprintf( path ,"%s%s", RomPath, filename);
	fp= fopen(path ,"r");
	if( fp !=NULL)
		{
		i=0;
		while( !feof(fp))
			{
			 fgets(buff, 255,fp);

			 if( strlen(buff) >0) 
				{
				 p = strchr(buff , ' ');
				 if( p != NULL)
					{
					*p = 0;
					sscanf(buff,"%X",&idx);
					if( idx >=0 && idx <= 0x7fff)
						{
						 q = strchr(p+1 , '\x0a');	// \x0a　除外
						 if( q != NULL)
							 *q = 0;

						my_strncpy( out_dat[ idx] , (p+1) ,20);
						
						}
					}
				}
			 i++;
			}
		fclose(fp);
		}
}


load_disasm_comment()
{
	switch( P6Version)
		{
		case PC60:	    load_disasm_comment_from_array( disasm_comment_basic1, 	sym_basic60); break;
		case PC60M2:	load_disasm_comment_from_array( disasm_comment_basic1, 	sym_basic62); break;
		case PC66:	    load_disasm_comment_from_array( disasm_comment_basic1, 	sym_basic66); break;
		}
//	load_disasm_comment_from_file( disasm_comment_basic1 , disasm_comment_dat[ P6Version].file_basic1);
//	load_disasm_comment_from_file( disasm_comment_basic2 , disasm_comment_dat[ P6Version].file_basic2);
//	load_disasm_comment_from_file( disasm_comment_voice  , disasm_comment_dat[ P6Version].file_voice);
}



char * read_disasm_comment(word adr)
{
	char *p = "";

	if( P6Version == PC60)				// PC-6001
		p = disasm_comment_basic1[ adr];
	else if(( P6Version == PC60M2SR || P6Version == PC66SR) && port60[0] == 0xf8)	// SR mode 6
		{
		int bank = adr / 0x2000;
		if( port60[bank] > 0xf0)			// BASICROM
			p = disasm_comment_basic2[ adr];
		else if( port60[ bank] > 0xe0)		// VOICEROM
			p = disasm_comment_voice[ adr];
		}
	else																// mkII / 6601 / SR mode 1-5
		{
		int bank = adr / 0x4000;
		if(( bank ==0 && (portF0 & 0x0f) ==0x01) || (bank==1 && (portF0 & 0xf0)==0x10) || (bank==2 && (portF1 & 0x0F)==0x01) || (bank==3 && (portF1 & 0xf0)==0x10)) 
			p = disasm_comment_basic1[ adr];
		}
	return p;
}





byte gvram_read(register word A);
void gvram_write(register word A,register byte V);
void InitMemmap(void);
void makestrcpyemigraph(void);

byte *LoadROM(byte **mem, char *name, size_t size);
int LoadEXTROM(byte *mem, char *name, size_t size);

static int chk_gvram(register word A,int flag);

void initmemory_withcoldstart(byte *mem);
void put_notfoundromfile(void);




void dokodemo_save_mem(void)
{
	int tmp0, tmp1;
	DOKODEMO_PUTENTRY_INT(extkanjirom);				// EXT KANJIROM 1:enable  0:disable
	DOKODEMO_PUTENTRY_INT(new_extkanjirom);
	
	DOKODEMO_PUTENTRY_INT(use_extram64);				// EXT RAM   64KB   1:enable  0:disable
	DOKODEMO_PUTENTRY_INT(new_use_extram64);

	DOKODEMO_PUTENTRY_INT(use_extram32);				// EXT RAM   32KB   1:enable  0:disable
	DOKODEMO_PUTENTRY_INT(new_use_extram32);

	dokodemo_putentry_buffer("INTRAM" , (char*)RAM   , 0x10000);
	dokodemo_putentry_buffer("EXTRAM" , (char*)EXTRAM64, 0x10000);
	dokodemo_putentry_buffer("EnWrite", (char*)EnWrite, 4);

	DOKODEMO_PUTENTRY_INT( megarom_flag );          // 1: extrom is megarom   
	DOKODEMO_PUTENTRY_INT( megarom_bank_no );         // megarom bank no.   

	dokodemo_putentry( "CasPath[0]",CasPath[0]);
	dokodemo_putentry( "CasName[0]",CasName[0]);
	if( CasStream[0] ) tmp0 = ftell( CasStream[0]); else tmp0=0; 
	dokodemo_putentry_int( "CasTell[0]",tmp0 );

	dokodemo_putentry( "CasPath[1]",CasPath[1]);
	dokodemo_putentry( "CasName[1]",CasName[1]);
	if( CasStream[1] ) tmp1 = ftell( CasStream[1]); else tmp1=0; 
	dokodemo_putentry_int( "CasTell[1]",tmp1 );

	dokodemo_putentry( "DskPath[0]",DskPath[0]);
	dokodemo_putentry( "DskName[0]",DskName[0]);

	dokodemo_putentry( "DskPath[1]",DskPath[1]);
	dokodemo_putentry( "DskName[1]",DskName[1]);

	dokodemo_putentry( "PrnName[0]",PrnName);

	dokodemo_putentry( "ExtRomPath", ExtRomPath );		/* Ext ROM image path */
	dokodemo_putentry( "Ext1Name", Ext1Name );    /* Extension ROM 1 file  (4000h-5fffh)*/
	dokodemo_putentry( "Ext2Name", Ext2Name );    /* Extension ROM 2 file  (6000h-7fffh)*/

	DOKODEMO_PUTENTRY_INT( Use_CompatibleROM);	// Use_CompatibleROM
}


void dokodemo_load_mem(void)
{
	int tmp0 ,tmp1;

	DOKODEMO_GETENTRY_INT(extkanjirom);				// EXT KANJIROM 1:enable  0:disable
	DOKODEMO_GETENTRY_INT(new_extkanjirom);
	
	DOKODEMO_GETENTRY_INT(use_extram64);				// EXT RAM   64KB   1:enable  0:disable
	DOKODEMO_GETENTRY_INT(new_use_extram64);

	DOKODEMO_GETENTRY_INT(use_extram32);				// EXT RAM   16KB   1:enable  0:disable
	DOKODEMO_GETENTRY_INT(new_use_extram32);

	dokodemo_getentry_buffer("INTRAM" , (char*)RAM     , 0x10000);
	dokodemo_getentry_buffer("EXTRAM" , (char*)EXTRAM64, 0x10000);  // TODO: fix me!
	dokodemo_getentry_buffer("EnWrite", (char*)EnWrite, 4);
	
	DOKODEMO_GETENTRY_INT( megarom_flag );          // 1: extrom is megarom   

	if( sr_mode)      /* CGROM 2003/11/9 */
  		CGROM=CGROM6;
    else
        CGROM=CGROM1;


	// -------- テープ、ディスク、プリンター　など ---------
	dokodemo_getentry( "CasPath[0]",CasPath[0]);
	dokodemo_getentry( "CasName[0]",CasName[0]);

	dokodemo_getentry( "CasPath[1]",CasPath[1]);
	dokodemo_getentry( "CasName[1]",CasName[1]);

	dokodemo_getentry( "DskPath[0]",DskPath[0]);
	dokodemo_getentry( "DskName[0]",DskName[0]);

	dokodemo_getentry( "DskPath[1]",DskPath[1]);
	dokodemo_getentry( "DskName[1]",DskName[1]);

	dokodemo_getentry( "PrnName[0]",PrnName);

	OpenFile1(FILE_LOAD_TAPE);
	OpenFile1(FILE_SAVE_TAPE);
	OpenFile1(FILE_DISK1);
	OpenFile1(FILE_DISK2);	
	OpenFile1(FILE_PRNT);

	if( CasStream[0]) {
		dokodemo_getentry_int( "CasTell[0]",&tmp0);
		fseek( CasStream[0] , tmp0 , SEEK_SET);
	}
	if( CasStream[1]) {
		dokodemo_getentry_int( "CasTell[1]",&tmp1);
		fseek( CasStream[1] , tmp1 , SEEK_SET);
	}

	// ---------- 拡張ROM　-------------------
	dokodemo_getentry( "ExtRomPath", ExtRomPath );		/* Ext ROM image path */
	dokodemo_getentry( "Ext1Name", Ext1Name );    /* Extension ROM 1 file  (4000h-5fffh)*/
	dokodemo_getentry( "Ext2Name", Ext2Name );    /* Extension ROM 2 file  (6000h-7fffh)*/

	if( *Ext1Name || *Ext2Name)
		{
		load_extroms();				// load roms
		init_extrom_memorymap();	// init ext rom memorymap 

		DOKODEMO_GETENTRY_INT( megarom_bank_no );       // megarom bank no.   
		memorymap_mode1_extmegarom( 0 , megarom_bank_no); // select megarom bank
		}
	DOKODEMO_GETENTRY_INT( Use_CompatibleROM);		// Use_CompatibleROM
}


// ****************************************************************************
//          initmemory_withcoldstart  電源ON直後の、RAMの初期状態を、ある程度再現
// ****************************************************************************
void initmemory_withcoldstart(byte *mem)
{
int i,j;
 if(P6Version == PC60M2 || P6Version == PC66)
	{
	 byte *p = mem;
	 byte *atype="\x00\xFF";
	 byte *btype="\xFF\x00";
	 byte *current_type = atype;

	 for(i=0; i<512; i++)
		{
		 for(j=0; j<64; j++)	// 128 byte ごとに、並べる
			{
			 memcpy( p , current_type , 2);	//2パターン書き込み
			 p+=2;
			}
		 if( current_type == atype)
			 current_type = btype;
		 else
			 current_type = atype;
		}
	}
 else if( P6Version == PC66SR)
	{
	 byte *p = mem;
	 for(i=0; i< 128; i++)		// 256 byte ごとに、並べる
		{
		 memset( p, 0x00 ,0x100); p+= 256;
		 memset( p, 0xFF ,0x100); p+= 256;
		}
	//memset(mem + 0x7d00 , 0, 0x300);	// 201-204 ライン隠す
	//memset(mem + 0x1900 , 0, 0x80);		// 201-204 ライン隠す
	}
 else if( P6Version == PC60M2SR)
	{
	 byte *p = mem;
	 byte *atype="\x00\x00\xFF\xFF";
	 byte *btype="\xFF\xFF\x00\x00";
	 byte *current_type = btype;

	 p+= 0x200;		// start addr
	 for(i=0; i<63; i++)
		{
		 for(j=0; j<256; j++)	// 1024 byte ごとに、並べる
			{
			 memcpy( p , current_type , 4);		// 2パターン書き込み
			 p+=4;
			}
		 if( current_type == atype)
			 current_type = btype;
		 else
			 current_type = atype;
		}
	}

 else if( P6Version == PC60)
	{
	 byte *p = mem;

	p+=0x8000; 
	for(i=0; i<256; i++)
		{
		 memset( p , 0xFF , 64);
		 p+=64;
		 memset( p , 0x00 , 64);
		 p+=64;
		}
	}
}

// ****************************************************************************
//          peek_memory: RAMからデータを読み出す。（常にRAMから読み出す。）
//	In:  A: address
//  Out: data
// ****************************************************************************
byte peek_memory(register word A)
{
	 return *(RAM+A);
}

// ****************************************************************************
//          peek_ext_memory: RAMからデータを読み出す。（常にEXTRAMから読み出す。）
//	In:  A: address
//  Out: data
// ****************************************************************************
byte peek_ext_memory(register word A)
{
	return *(EXTRAM64 + A);
}


// ****************************************************************************
//          poke_memory: RAMにデータを書き込む。（常にRAMに書き込む。）
//	In:  A: address
//  Out: data
// ****************************************************************************
void poke_memory(register word A ,byte V)
{
	 *(RAM+A) = V;
}

// ****************************************************************************
//          poke_ext_memory: EXTRAMにデータを書き込む。（常にEXTRAMに書き込む。）
//	In:  A: address
//  Out: data
// ****************************************************************************
void poke_ext_memory(register word A, byte V)
{
	*(EXTRAM64 + A) = V;
}
// ****************************************************************************
//   		M_RDMEM: This function is called when a read from RAM occurs.
//  In:  A: address
//  Out: data
// ****************************************************************************
byte M_RDMEM(register word A)
{
			/* Graphics Vram Read (SR basic) add 2002/2 */
	if(sr_mode && chk_gvram(A,0))
    	{
    	 return(gvram_read(A));
     	}
	else
			/* normal memory read ..*/
    	{
      	return(RdMem[A>>13][A&0x1FFF]);
     	}
}

// ****************************************************************************
//			M_WRMEM: This function is called when a write to RAM occurs. It 
//						checks for write protection and slot selectors.
//  In: A: address
//      V: data
// ****************************************************************************
void M_WRMEM(register word A,register byte V)
{

			/* Graphics Vram Write (SR basic) add 2002/2 */
   if(sr_mode && chk_gvram(A ,8)) 
		{
        gvram_write(A,V);
   		}
   else {			/* normal memory write ..*/
      	if(EnWrite[A>>14]) 
      		WrMem[A>>13][A&0x1FFF]=V;
#ifdef DEBUG
      else printf("M_WRMEM:%4X\n",A);
#endif
   		}
}


// ****************************************************************************
//    chk_gvram:  Vram Window check  (バンク切替えの窓かどうかを判別。)    
//                                                              
//   In: A: Address   flag: 0 or 8                              
//   Out: True:  Vram Window       False: normal access         
//                                      add 2002/2            
// ****************************************************************************
static int chk_gvram(register word A,int flag)
{
	int ret;
	ret=0;

/* printf("port%X[%X] \n",(A>>13)+flag,port60[(A>>13)+flag]); */
	if(  port60[ (A>>13)+flag ]==0x00 && (bitmap ))	// VRAM の先頭かつ、CRTが BITMAP mode 
     	      ret=1;         /* TRUE: vram window */
return(ret);
}


// ****************************************************************************
//    gvram_read:   Vram Read           SR BASIC      add 2002/2             
//                                                            
//    バンク切替えでなく、アクセスするべきVRAMを計算しています。 
//    他にも、実際にバンク切替えする方法もあるかも知れません。   
// ****************************************************************************
byte gvram_read(register word A)
{
	 byte* adr;
	 byte  ret;
	 int x,y,z,w,off;

//if( A >319) {  PRINTDEBUG("gvram_read:out of range:  x is %d **************\n",A);}
	 x = A & 0x1fff;
	 y = portCF*16+portCE;		      /* y座標 */
	 if( y >=204) y-=204;		      /* Y座標 204 以上だと 204 引く add 2003/10/22 */
	 w = (x <256) ? 256: 64;          /* width:0..255 なら256 / 256..319なら 64にする*/
	 off=(x <256) ? 0x1a00: 0x0000;   /* offset: Vram offset address */
	 x = (x <256) ? x: x-256;	      /* x:256..319 なら 256を引く　 */
	 z = ((y & 1 )==1) ? 2: 0;        /* z:Y座標が奇数なら、2を足す  */

	 if( off == 0x1a00)
		 adr = (VRAM_BITMAP+ (off+ (y>>1)*w + (x&0xffc)+z));  /* 左半分 */
	 else
		 adr = (VRAM_BITMAP+ (off+ sr_vram_right_addr[y] + (x & 0xffc)+z)); /* 右半分 */

/* printf("read) x=%d y=%d w=%d off=%04X add=%d\n",x,y,w,off,(off+(y>>1)*w+x+z));*/
	 switch(x & 3) {		/* Word Access をする */
	    case 0: ret=  *(adr);      break;
	    case 1: ret=  *(adr)>>4;   break;
	    case 2: ret=  *(adr+1);    break;
	    case 3: ret=  *(adr+1)>>4; break;
	   }
 	return(ret );
}


// ****************************************************************************
//    gvram_write:  Vram Write            SR BASIC       add 2002/2  Windy   
//                                                            
//   バンク切替えでなく、アクセスするべきVRAMを計算しています。           
//   他にも、実際にバンク切替えする方法もあるかも知れません。             
// ****************************************************************************
void gvram_write(register word A,register byte V)
{
 byte* adr;
 int x,y,z,w,off;

 if(( A & 0x1fff) >319) { PRINTDEBUG1(MEM_LOG,"gvram_write:out of range:  x is %d **************\n",A); return;}
	x = A & 0x1fff;
	y = portCF*16+portCE;           /* y座標 */
	if( y >=204) y-=204;			/* Y座標 204 以上だと 204 引く add 2003/10/22 */
	w = (x <256) ? 256: 64;         /* width:0..255 なら256 / 256..319なら 64にする*/
	off=(x <256) ? 0x1a00: 0x0000;  /* offset: Vram offset address */
	x = (x <256) ? x: x-256;	    /* x:256..319 なら 256を引く　 */
	z = ((y & 1 )==1) ? 2: 0;       /* z:Y座標が奇数なら、2を足す  */
	V&= 0x0f;

	if( off == 0x1a00)
		adr = VRAM_BITMAP+(off+ (y>>1)*(w) + (x&0xffc)+z);		/* 左半分 */
	 else
		 adr = (VRAM_BITMAP+ (off+ sr_vram_right_addr[y] + (x & 0xffc)+z)); /* 右半分 */

	/* printf("write) x=%d y=%d w=%d off=%04X ,\n",x,y,w,off,(off+(y>>1)*w+x+z)); */
	 switch(x & 3) {
	    case 0: *(adr)=(*(adr)  &0xf0)  |V;    break;
	    case 1: *(adr)=(*(adr)  &0x0f)  |V<<4; break;
	    case 2: *(adr+1)=(*(adr+1)&0xf0)|V;    break;
	    case 3: *(adr+1)=(*(adr+1)&0x0f)|V<<4; break;
	   }
}


/*
   前から不思議だったのですが、
   SRのY座標は、0?203までなのに、何故か、portCE/CF で、9ビット（0?511）も指定が
   出来るようになっています。不思議ですねぇ?。

   でも、実際には、204以上指定したら、巻物のように、上に戻ります。
   つまり、204以上だと、指定値より 204引いた座標が、本当のY座標になります。

   もしかすろと、Y方向512ドットまで表示可能だった名残なのかも？
   そういえば、X方向のVRAMは、256ドット＋64ドットという風に、分かれていたので、
   もしかすると、もしかするかも？(^^;

   ちなみに、Port CFは  出力してあっても、意味が無いようです。
   どうやら、未使用みたいですね。(汗)
   								                        2003/10/22  Windy
*/



// ****************************************************************************
//  ROM files                                                
// ****************************************************************************
byte **ROMList[6+3] = { &BASICROM, &CGROM1, &CGROM5, &KANJIROM, &VOICEROM,
			 &SYSROM2, &SYSTEMROM1, &SYSTEMROM2 ,&CGROM6};
	/* add 2002/2    added SYSTEMROM1 , SYSTEMROM2  by Windy*/

char *ROMName[5][6+3] = {
    {"BASICROM.60","CGROM60.60",""           ,""           ,""           ,""},
    {"BASICROM.62","CGROM60.62","CGROM60m.62","KANJIROM.62","VOICEROM.62",""},
    {"           ","          ","           ","           ","           ",
     "           ","SYSTEMROM1.64","SYSTEMROM2.64","CGROM68.64"},
    {"BASICROM.66","CGROM60.66","CGROM66.66" ,"KANJIROM.66","VOICEROM.66",""},
    {"           ","          ","           ","           ","           ",
     "           ","SYSTEMROM1.68","SYSTEMROM2.68","CGROM68.68"},
  };
    /* 99.06.02. */
    /* add 2002/2       SYSTEMROM1 , SYSTEMROM2  :-) */

  int sizeList[5][6+3] = {
    { 0x4000, 0x2400,      0,      0,      0,      0 ,0,0},
    { 0x8000, 0x2400, 0x2000, 0x8000, 0x4000,      0 ,0,0},
    { 0x0000, 0     , 0     , 0x0000, 0x0000, 0x0000,0x10000,0x10000 ,0x5000},
    { 0x8000, 0x2400, 0x2000, 0x8000, 0x4000,      0 ,0,0},
    { 0x0000, 0     , 0     , 0x0000, 0x0000, 0x0000,0x10000,0x10000 ,0x5000},
  };
    /* 99.06.02. */
    /* add 2002/2  by Windy */

    /* 2002/5/25   SR なら BASICROM ,KANJIROM,SYSROM2, VOICEROMを読まない */
    /* 2003/8/8    SR のCGROMは、16kb単体対応に変更 */

  char **compati_ROM[5][6 + 3] = {
	{&c_BASICROM60n ,&c_CGROM6060n,NULL           ,NULL           ,NULL           ,NULL},	// PC-6001
	{NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL },													// mkII
	{NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL },													// mkIISR
	{&c_BASICROM66n,&c_CGROM6066n, &c_CGROM6666n , &c_KANJIROM66n, &c_VOICEROM66n,NULL,NULL,NULL,NULL},	// 6601
  };


// ****************************************************************************
//         existROM(): exist ROM file ?                             
// ****************************************************************************
int existROM(int version)
{
	int ret;
	printf("\n Checking ROM files ...");

	ret= existROMsub(version, "");		// search current directory
	if( ret==0)
	     ret = existROMsub(version, RomPath);	// search rom path

	 if( Verbose) 
	    if( ret) 
	    	 printf("OK\n");
		else
	         printf("FAILED\n");
	 return(ret);
}


// ****************************************************************************
//    existROMsub(): exist ROM file ?                          
// ****************************************************************************
int existROMsub(int version, char *dir)
{
	char path[PATH_MAX];
	FILE *fp;
	int i;
	int ret=1;
 
	 for(i=0; i<9;i++)
	 	{
		 if( sizeList[version][i])
		   {
		   sprintf(path,"%s%s",dir, ROMName[version][i]);
		   fp= fopen( path ,"rb"); 
		   if(fp!=NULL) 
	       		fclose(fp);
	       else
    	 		ret=0;
		   }
		}

	return(ret);
}



// ****************************************************************************
//     InitMemmap(): initialize  memory map                   
// ****************************************************************************
void InitMemmap(void)
{
	int J;
	if(Verbose) printf("Initializing memory mappers...");


	if( P6Version == PC60M2SR  || P6Version== PC66SR  ){       /* SR     add 2002/2 by Windy*/
		for(J=0;J<4;J++) {RdMem[J]=SYSTEMROM1+0x2000*J+0x8000;WrMem[J]=RAM+0x2000*J;};
	    for(J=4;J<8;J++) {RdMem[J]=RAM+0x2000*J;              WrMem[J]=RAM+0x2000*J;};
	    EnWrite[0]=EnWrite[1]=0; EnWrite[2]=EnWrite[3]=1;

	    for(J=0;J<4;J++) {sprintf( read_b[J] ,sys1_b ,J*2+8);    sprintf( write_b[J],sr_intram_b , J*2);}
	    for(J=4;J<8;J++) {sprintf( read_b[J] ,sr_intram_b ,J*2); sprintf( write_b[J],sr_intram_b , J*2);}
    	}
    else if( P6Version == PC60M2  || P6Version == PC66 ) {  /* mk2/66  add 2009/3/22 by Windy */
  	    for(J=0;J<4;J++) {RdMem[J]=BASICROM+0x2000*J;         WrMem[J]=RAM+0x2000*J;};
	    for(J=4;J<8;J++) {RdMem[J]=RAM+0x2000*J;              WrMem[J]=RAM+0x2000*J;};
	    EnWrite[0]=EnWrite[1]=0; EnWrite[2]=EnWrite[3]=1;

		for(J=0;J<4;J++) {strcpy( read_b[J], basic_b[J]);     strcpy( write_b[J] ,intram_b[J]);}
		for(J=4;J<8;J++) {strcpy( read_b[J], intram_b[J]);    strcpy( write_b[J] ,intram_b[J]);}
    	}
    else if( P6Version == PC60  ) {                   /* 6001  add 2009/3/22 by Windy */
		if( !use_extram32)
			{										  // RAM 16KB  0xc000-0xffff
			for(J=0;J<2;J++) { RdMem[J]=BASICROM+0x2000*J;          WrMem[J]=EmptyRAM; }
			for(J=2;J<6;J++) { RdMem[J]=EmptyRAM;                  WrMem[J]=EmptyRAM; }
			for(J=6;J<8;J++) { RdMem[J]=RAM+0x2000*J;              WrMem[J]=RAM+0x2000*J; };
			EnWrite[0]=EnWrite[1]=EnWrite[2]=0;   EnWrite[3]=1;

			for (J=0;J<2;J++) { strcpy(read_b[J], basic_b[J]);     strcpy(write_b[J], empty_b); }
			for (J=2;J<6;J++) { strcpy(read_b[J], empty_b);        strcpy(write_b[J], empty_b); }
			for (J=6;J<8;J++) { strcpy(read_b[J], intram_b[J]);    strcpy(write_b[J], intram_b[J]); }
			}
		else
			{										  // RAM 32KB  0x8000-0xffff
			for(J=0;J<2;J++) {RdMem[J]=BASICROM+0x2000*J;         WrMem[J]=EmptyRAM;}
			for(J=2;J<4;J++) {RdMem[J]=EmptyRAM;                  WrMem[J]=EmptyRAM; }
			for(J=4;J<8;J++) {RdMem[J]=RAM+0x2000*J;              WrMem[J]=RAM+0x2000*J;};
			EnWrite[0]=EnWrite[1]=0;     EnWrite[2]=EnWrite[3]=1;

			for (J=0; J < 2; J++) { strcpy(read_b[J], basic_b[J]);     strcpy(write_b[J], empty_b); }
			for (J=2; J < 4; J++) { strcpy(read_b[J], empty_b);        strcpy(write_b[J], empty_b); }
			for (J=4; J < 8; J++) { strcpy(read_b[J], intram_b[J]);    strcpy(write_b[J], intram_b[J]); }
			}

        }
    
	init_extrom_memorymap();	// init ext rom memorymap 

    /* --- init 0x8000 - 0xffff --- */
/*	for(J=4;J<8;J++) {RdMem[J]=RAM+0x2000*J;WrMem[J]=RAM+0x2000*J;};
	EnWrite[0]=EnWrite[1]=0; EnWrite[2]=EnWrite[3]=1;
*/	

	VRAM=RAM;
	VRAM_BITMAP=RAM;		// BITMAP VRAM for SR-BASIC
  
	if( sr_mode)      /* CGROM 2003/11/9 */
  		CGROM=CGROM6;
    else
        CGROM=CGROM1;

	/* PC-6001だと、portF0 は、そもそも存在しない。   2003/7/20*/
  	//if( P6Version ==0) portF0= 0x71;
}


void init_extrom_memorymap(void)
{
	int J;
	// ************* set memory map for MEGAROM *********************
	if( megarom_flag ==1 && EXTROM1 != EmptyRAM )
		{
		 if( P6Version == PC60  || P6Version == PC60M2 || P6Version == PC66 )
		 	{
	    	 for(J=3;J<6;J++) {RdMem[J]=RAM+0x2000*J;              WrMem[J]=RAM+0x2000*J;};
	    	 EnWrite[0]=0; EnWrite[1]= EnWrite[2]= EnWrite[3]=1;
	    	 
	    	 EXTROM2 = RAM + 0x2000*3;		// EXTROM2 は、拡張RAMになる　（FIX ME: 今は、とりあえず、内蔵RAMにつなげている）2009/7/24

			 extrom2_b = extram_b[0];
	    	 printf("Setting memory map for MEGAROM...OK\n");
		 	}
		}

	if( EXTROM1 != EmptyRAM && EXTROM2 != EmptyRAM)
		{
		RdMem[2] = EXTROM1; RdMem[3] = EXTROM2;			strcpy( read_b[2] ,extrom1_b);   strcpy(read_b[3], extrom2_b);
		}
}


// ****************************************************************************
//     TrashP6:  Free memory allocated with StartP6().                   
// ****************************************************************************
void TrashP6(void)
{
 int i;
    CPURunning = 0;

    // fixed double free() EmptyRAM ,add windy 2002/7/7
  if(BASICROM        && BASICROM!=EmptyRAM)  {free(BASICROM); BASICROM=NULL;} else BASICROM=NULL;
  if(CGROM1          && CGROM1  !=EmptyRAM)  {free(CGROM1);   CGROM1=NULL;}   else CGROM1=NULL;
  if(CGROM5          && CGROM5  !=EmptyRAM)  {free(CGROM5);   CGROM5=NULL;}   else CGROM5=NULL;
  if(KANJIROM        && KANJIROM!=EmptyRAM)  {free(KANJIROM); KANJIROM=NULL;} else KANJIROM=NULL;
  if(VOICEROM        && VOICEROM!=EmptyRAM)  {free(VOICEROM); VOICEROM=NULL;} else VOICEROM=NULL;
  if(RAM)                                    {free(RAM); RAM=NULL;}
  if(PrnStream&&(PrnStream!=stdout))         {fclose(PrnStream); PrnStream=NULL;}
  for(i=0;i<2;i++)
  	{
  	if(CasStream[i])                         {fclose(CasStream[i]); CasStream[i]=NULL;}
  	if(DskStream[i])                         {fclose(DskStream[i]); DskStream[i]=NULL;}
    }


   /* add 2002/3/9  for N66SR  */
  if(SYSTEMROM1      && SYSTEMROM1!=EmptyRAM)  {free(SYSTEMROM1); SYSTEMROM1=NULL;} else SYSTEMROM1=NULL;
  if(SYSTEMROM2      && SYSTEMROM2!=EmptyRAM)  {free(SYSTEMROM2); SYSTEMROM2=NULL;} else SYSTEMROM2=NULL;
  if(CGROM6          && CGROM6    !=EmptyRAM)  {free(CGROM6);     CGROM6=NULL;}     else CGROM6 =NULL;

  if(EXTKANJIROM     && EXTKANJIROM!=EmptyRAM) {free(EXTKANJIROM);EXTKANJIROM=NULL;} else EXTKANJIROM=NULL;
  if(EXTROM          && EXTROM  !=EmptyRAM)    {free(EXTROM); EXTROM=EXTROM1=EXTROM2=NULL;} else EXTROM=NULL;

  if(EXTRAM64        && EXTRAM64 !=EmptyRAM)   {free(EXTRAM64);   EXTRAM64=NULL;}  else EXTRAM64=NULL;

  if(EmptyRAM) { free(EmptyRAM); EmptyRAM=NULL;}
  
//  if(OLDVRAM1 !=NULL) free( OLDVRAM1);
//  if(OLDVRAM2 !=NULL) free( OLDVRAM2);
  
  close_keybuffer( keybuffer );
//  close_keybuffer( auto_keybuffer );
}



// ****************************************************************************
//       load_extroms(): load ext roms     
//  Out: 1:success  0:failed              
// ****************************************************************************
int load_extroms(void)
{
  /* **************** Load ExtROMs (4000h-5fffh) (6000h-7fffh)  8KB*2  ********************/
	if(EXTKANJIROM     && EXTKANJIROM!=EmptyRAM) {free(EXTKANJIROM);EXTKANJIROM=NULL;}
	if(EXTROM          && EXTROM  !=EmptyRAM)    {free(EXTROM); EXTROM=EXTROM1=EXTROM2=NULL;}

	if ((*Ext1Name) || (*Ext2Name))
		{
		char tmpPath1[ PATH_MAX], tmpPath2[ PATH_MAX];
		int  size;
		my_strncpy(tmpPath1 , ExtRomPath ,PATH_MAX);
		strncat(   tmpPath1 , Ext1Name ,  PATH_MAX -strlen(ExtRomPath));
		my_strncpy(tmpPath2 , ExtRomPath ,PATH_MAX);
		strncat(   tmpPath2 , Ext2Name ,  PATH_MAX -strlen(ExtRomPath));

		size = get_filesize( tmpPath1);
		if( size >= 0x20000)        // if extrom size 1M over is MEGAROM 2009/3/22
		    {
            megarom_flag = 1;
            }
		else
		    {
		    size = 0x4000;             // 最低 0x4000 は必要なため
		    megarom_flag = 0;
		    }
		
    	if((EXTROM=(byte*)malloc( size ))!=NULL)
			{
    		EXTROM1 = EXTROM; EXTROM2 = EXTROM + 0x2000;
    		if (*Ext1Name)
				if(!LoadEXTROM(EXTROM1,tmpPath1,size))
	      			{ EXTROM1 = EXTROM2 = EmptyRAM; }		// extrom Empty   2003/10/16

			if (*Ext2Name)
				if(!LoadEXTROM(EXTROM2,tmpPath2,0x2000))
	      			{ EXTROM1 = EXTROM2 = EmptyRAM; }
			}
       }
	else 
		{
    	EXTROM1 = EXTROM2 = EmptyRAM;
  		};

  /* ************** Load ExtKANJI ROM ******************/
   if( extkanjirom ) 
		{
		if((EXTKANJIROM=(byte*)malloc(0x20000))!=NULL)
	 		if(!LoadEXTROM(EXTKANJIROM,"EXKANJI.ROM",0x20000))
	    		{
   				if(Verbose) printf("  Creating EXKANJI.ROM...");
		 		if( make_extkanjirom( EXTKANJIROM)) /* make ExtKANJI ROM (win32/Unix)*/
		 			{ 
					 printf("Ok\n\n");
					 extkanjirom_type = EXTKANJI_MAKE;
					}
		 		else
		    		{ 
					PRINTFAILED; free(EXTKANJIROM); EXTKANJIROM = NULL; extkanjirom_type = EXTKANJI_NON;
					} 
				}
			else
				{
				extkanjirom_type = EXTKANJI_ROM;
				}
		}
   else
		{
	   extkanjirom_type = EXTKANJI_NON;
		}
}


// ****************************************************************************
//       getextkanjirom_type 
//  Out: extkanjirom type message               
// ****************************************************************************
char *getextkanjirom_type_msg(void)
{
	static char rom_type_msg[256]="";

	switch (extkanjirom_type)
	{
	case EXTKANJI_ROM:  strcpy(rom_type_msg, "EXKANJIROM_ROM"); break;
	case EXTKANJI_NON:  strcpy(rom_type_msg, "EXKANJIROM_NON"); break;
	case EXTKANJI_MAKE: strcpy(rom_type_msg, "EXKANJIROM_MAKE"); break;
	}
	return rom_type_msg;
}

// ****************************************************************************
//       load_roms(): load roms     
//  Out: 1:success  0:failed              
// ****************************************************************************
int load_roms(void)
{
	int K;
  /* ******* 各種内臓ROMを 読み込む ************************ */
 	if(Verbose) printf("OK\nLoading ROMs:\n");

	for(K=0;K<9;K++)
  		{  /* 99.06.02. */ /* add 2002/2  for SR BASIC */
    	if(sizeList[P6Version][K]) 
			{
			if (!LoadROM(ROMList[K], ROMName[P6Version][K], sizeList[P6Version][K]))
				{
				return(0); 
				}
    		}
		else 
			{
      		//if( !*ROMList[K]) 
				*ROMList[K] = EmptyRAM;		// NULL?  add Windy 2002/7/7
      		/* 99.06.02. */
    		};
  		}

	load_extroms();
	return(1);
}



// ****************************************************************************
//       load_compatible_roms(): load roms     
//  Out: 1:success  0:failed              
// ****************************************************************************
int load_compatible_roms(void)
{
	int K;
	int size;

	/* ******* 各種互換ROMを 読み込む ************************ */
	if (Verbose) printf("OK\nLoading comptible ROMs:\n");

	if (P6Version == PC60 || P6Version == PC66)
	{
		for (K = 0; K < 9; K++)
		{
			if (sizeList[P6Version][K])
			{
				*ROMList[K] = malloc(sizeList[P6Version][K]);
				if (*ROMList[K] != NULL)
				{
					size = sizeList[P6Version][K];
					if (P6Version == PC60 && K == 1) { size = 0x1000; }
					memcpy(*ROMList[K], compati_ROM[P6Version][K], size);
				}
				else
					return 0;
			}
			else
			{
				*ROMList[K] = EmptyRAM;
			};
		}
	}
	else
	{
		for (K = 0; K < 9; K++)
		{
			*ROMList[K] = EmptyRAM;
		}
		return 0;
	}

	load_extroms();
	return 1;
}


// ****************************************************************************
// 		get_filesize  
//
//  In:   path
//  Out:   0: size zero
//        -1: failed   
// ****************************************************************************
int get_filesize( char * path)
{
    FILE *fp;
    int size=0;
    
    fp = fopen( path ,"rb");
    if( fp ==NULL) return -1L;

    fseek( fp, 0L, SEEK_END ); 
    size = ftell( fp ); 

    fclose( fp);
    return size;
}




// ****************************************************************************
// 		Load a ROM data from the file name into mem.           
// Return memory address of a ROM or 0 if failed.
//
//  In: mem : read buffer   name: filename　size: allocate size
// ****************************************************************************
byte *LoadROM(byte **mem, char *name, size_t size)
{
	char path[PATH_MAX];
	//char errmsg[256];
	FILE *F;
  
	if(Verbose) printf("  Opening %s...", name);
	F=fopen( name, "rb");
	if (!F)
  		{					// Read ROM directory
     	sprintf( path,"%s%s",RomPath,name);
     	if(Verbose) printf("FAILED \n  Opening %s...",path);
	 	F=fopen( path, "rb");
		}

	if (F) {
	    if(Verbose) printf("OK\n");
	    if((*mem=(byte*)malloc(size))!=NULL) {
	      if(Verbose) printf("    loading...");
	      /* resize for CGROM60 */
	      if (!strcmp(name,"CGROM60.60")) {
			size=0x1000;
	      } else if (!strncmp(name,"CGROM60.",8)) {
			size=0x2000;
		  }
		  /* resize for CGROM68 */	/* 2003/8/8 */
	      else if (!strncmp(name,"CGROM68.",8)) {
			size=0x4000;
	      }
      if(fread(*mem,1,size,F)!=size) { free(*mem); *mem=NULL; };
	  };
    fclose(F);
  	};

	if(Verbose) printf(*mem? MsgOK:MsgFAILED);
	if( !*mem || !F) {

		return NULL ;		// when error is exit  ,add windy 2002/7/7
		}

	return(*mem);
}
	/*
	↑途中で、サイズを変更しています。何故かというと、
	ROMサイズよりも大きなメモリーを確保させるために行っているようです。
	つまり、ROMサイズも渡すようにすれば、サイズ変更してなくも良くなります。
												2004/1/11	Windy
	*/

// ****************************************************************************
//		 make_internalrom(): make internal rom 
//                   for N66-SR BASIC  by Windy   2002/5/25 
// ****************************************************************************
int make_internalroms(void)
{
  // 内部ROM の作成。 現状のプログラムでは、内部的に必要な、
  // BASICROM.68 ,KANJIROM.68 ,VOICEROM.68 ,SYSROM2.68 ,CGROM1 ,CGROM5 を作成しないといけません。
  if(sr_mode)
    {
    int K;
    struct {
      char *name;	// ROM name
      byte **out;	// output
      byte **in;	// input
      int start;	// start address of input
      int length;	// length of output
      }  internal_rom[]={{"BASICROM",&BASICROM , &SYSTEMROM1, 0x0000,0x8000},
                         {"KANJIROM",&KANJIROM , &SYSTEMROM2 ,0x8000,0x8000},
					  	 {"VOICEROM",&VOICEROM , &SYSTEMROM2 ,0x4000,0x4000},
					  	 {"SYSROM2" ,&SYSROM2  , &SYSTEMROM2 ,0x2000,0x2000},
					  	 {"CGROM1"  ,&CGROM1   , &CGROM6     ,0x0000,0x2400},
					  	 {"CGROM5"  ,&CGROM5   , &CGROM6     ,0x2000,0x2000},};
							/* add 2003/8/8 CGROM6 分割 */
     if(Verbose) printf("Creating  internal ROMs:\n");
     for(K=0 ; K< 6; K++)
        {
         if(Verbose) printf("  Creating %s ....",internal_rom[K].name);
         if( (*internal_rom[K].out = (byte*)malloc( internal_rom[K].length))==NULL) {PRINTFAILED; return(0);}
         memcpy( *internal_rom[K].out, *internal_rom[K].in+ internal_rom[K].start  ,internal_rom[K].length);
         if(Verbose) printf("OK\n");

        }
    }
 return(1);
}


// ****************************************************************************
//          makestrcpyemigraph: セミグラデータの作成ルーチン
// ****************************************************************************
void makestrcpyemigraph(void)
{
	/* make semi-graphic 6 for 6001 */
	// --- 2000.10.13.
	//if(!strcmp(name,"CGROM60.60")) {
#if 0
	if((!strcmp(name,"CGROM60.60")) || ( !strcmp(name,"CGROM60.64")) ||
     (!strcmp(name,"CGROM60.68"))) {
  	// --- 2000.10.13.
    byte *P = *mem+0x1000;
#endif
    if( CGROM1 !=EmptyRAM) {	/* add !=EmptyRAM  2003/09/15 windy*/
    byte *P = CGROM1+0x1000;
    unsigned int i, j, m1, m2;
    for(i=0; i<64; i++) {
      for(j=0; j<16; j++) {
		switch (j/4) {
          case 0: m1=0x20; m2=0x10; break;
          case 1: m1=0x08; m2=0x04; break;
          case 2: m1=0x02; m2=0x01; break;
          default: m1=m2=0;
		};
		*P++=(i&m1 ? 0xF0: 0) | (i&m2 ? 0x0F: 0);
       };
     };
   };

  /* make semi-graphic 4 for N60-BASIC */
#if 0
	if (!strncmp(name,"CGROM60.",8)) {
	    byte *P = *mem+0x2000;
#endif

	if( CGROM1 !=EmptyRAM) {		/* add !=EmptyRAM  2003/09/15 windy*/
	    byte *P = CGROM1+0x2000;
	    unsigned int i, j, m1, m2;
	    for(i=0; i<16; i++) {
	      for(j=0; j<16; j++) {
			switch (j/6) {
	          case 0: m1=0x08; m2=0x04; break;
	          case 1: m1=0x02; m2=0x01; break;
	          default: m1=m2=0;
			};
			*P++=(i&m1 ? 0xF0: 0) | (i&m2 ? 0x0F: 0);
	      };
	    };
	  };


#if 1
  /* make semi-graphic  for N66SR-BASIC */ /* 2003/8/8 */
	if( CGROM6 !=EmptyRAM) {		/* add !=EmptyRAM  2003/09/15 windy*/
		byte *P = CGROM6+0x4000;	/* CGROM6 の後ろに作成 */
  		unsigned int i, j, m1, m2;
    	for(i=0; i<256; i++) {
    	  for(j=0; j<16; j++) {
			switch (j/2) {
    	      case 0: m1=0x80; m2=0x40; break;
    	      case 1: m1=0x20; m2=0x10; break;
     	      case 2: m1=0x08; m2=0x04; break;
    	      case 3: m1=0x02; m2=0x01; break;
     	     default: m1=m2=0;
			};
			*P++=(i&m1 ? 0xF0: 0) | (i&m2 ? 0x0F: 0);
    	  };
  	  };
  	};
#endif
}

// ****************************************************************************
//    LoadEXTROM:
// Load an extension ROM data from the file name into mem.  
// Return memory address of a ROM or 0 if failed.           
//  In: mem : read buffer   name: filename　size: allocate size
// ****************************************************************************
int LoadEXTROM(byte *mem, char *name, size_t size)
{
	char path[PATH_MAX];
	FILE *F;
	size_t s=0;

	if(Verbose) printf("  Opening %s as EXTENSION ROM...", name);
	F=fopen( name, "rb");
	if (!F)
	    {					// Read ROM directory
	     sprintf( path,"%s%s",RomPath,name);
		 F=fopen( path, "rb");
		}

	if (F) 
    	{
	    if(Verbose) printf("OK\n");
    	if(Verbose) printf("    loading...");
    	s = fread(mem,1,size,F);
    	if(Verbose) printf("(%05x bytes) ", s);
    	fclose(F);
  		};
	if(Verbose) printf((F!=NULL) ? MsgOK : MsgFAILED);
  	return(F!=NULL);
}






// ****************************************************************************
//          memorymap_mode6_readblock: SR(MODE 6) の　読み込みメモリーマップ
//   In:  Port: Port no.   
//        Value: data
// ****************************************************************************
void memorymap_mode6_readblock(byte Port , byte Value)
{
	if( P6Version == PC60M2SR  || P6Version == PC66SR )
		{
		 //static char tmp[20];
		 
         int start_adr;
         start_adr= Value & 0xe;

         port60[Port-0x60]= Value;
         switch( Value & 0xf0) 
		 	{
	    	 case 0xf0: RdMem[(Port& 0xf)]=SYSTEMROM1+(start_adr)*0x1000;break;
	    	 case 0xe0: RdMem[(Port& 0xf)]=SYSTEMROM2+(start_adr)*0x1000;break;
	    	 case 0xd0: RdMem[(Port& 0xf)]=    CGROM6+(start_adr)*0x1000;break;
	    	 case 0xc0: RdMem[(Port& 0xf)]=   EXTROM2; break;
	    	 case 0xb0: RdMem[(Port& 0xf)]=   EXTROM1; break;
	    	 case 0x00: RdMem[(Port& 0xf)]=       RAM+(start_adr)*0x1000;break;
	    	 case 0x20: if(use_extram64)
			 			  RdMem[ Port & 0xf]=  EXTRAM64+((start_adr)*0x1000);
			 			break;
             case 0x30: if(use_extvram)
                          RdMem[ Port & 0xf]=  EXT_VRAM+((start_adr)*0x1000);
                        break;
            }


		 switch( Value & 0xf0)
		 	{
		 	 case 0xf0: sprintf( read_b[ Port& 0xf] , sys1_b , start_adr ); break;
		 	 case 0xe0: sprintf( read_b[ Port& 0xf] , sys2_b , start_adr ); break;
			 case 0xd0: strcpy(  read_b[ Port& 0xf ] , cgrom_b); break;
			 case 0xc0: strcpy(  read_b[ Port& 0xf ] , extrom2_b); break;
			 case 0xb0: strcpy(  read_b[ Port& 0xf ] , extrom1_b); break;
			 case 0x00: sprintf( read_b[ Port& 0xf ] , sr_intram_b , start_adr); break;
			 case 0x20: if(use_extram64)
			 			  {sprintf(  read_b[ Port& 0xf ] , sr_extram_b , start_adr);  }
			 			break;
                    
		 	} 
		}

}
	/* EXTROM1と、EXTROM2 のstart_adr は、意味が無いようなので足さないことにしました。
    　 2003/10/27   Thanks Chocobon!
    */




// ****************************************************************************
//          memorymap_mode6_writeblock: SR (MODE 6) の　書き込みメモリーマップ
//   In:  Port: Port no.   
//        Value: data
// ****************************************************************************
void memorymap_mode6_writeblock(byte Port , byte Value)
{
	if( P6Version == PC60M2SR  || P6Version == PC66SR )
	    {
         port60[Port-0x60]= Value;
		 if((Value & 0xf0)==0x00)
			{
			 int start_adr = Value & 0xe;
			 WrMem[ (Port& 0xf)-8]= RAM+(start_adr*0x1000);
			 EnWrite[ ((Port & 0xe)-8)/2 ]= 1;

			 sprintf( write_b[ Port& 0xf-8 ] , sr_intram_b , start_adr);
			 
			}

		if( use_extram64 )
	       if((Value & 0xf0)==0x20)
				{
				 int start_adr = Value & 0xe;
				 WrMem[ (Port& 0xf)-8]= EXTRAM64+(start_adr*0x1000);

				 sprintf( write_b[ Port& 0xf-8 ] , sr_extram_b , start_adr);

			 //write_b[ Port & 0xf] = 					// FIX ME!!
				}
            
            if( use_extvram )
                if((Value & 0xf0)==0x30)
                {
                    int start_adr = Value & 0xe;
                    WrMem[ (Port& 0xf)-8]= EXT_VRAM+(start_adr*0x1000);
                    
                    
                    //write_b[ Port & 0xf] = 					// FIX ME!!
                }
            
        }

}


#define strcpy strcpy

// ****************************************************************************
//          memorymap_mode6_readblock: MODE 5 の読み込みメモリーマップ０
//   In:  Port: Port no.   
//        Value: data
// ****************************************************************************
void memorymap_mode5_readblock0(byte Port , byte Value)
{
	char *f, *r;

	if( sr_mode || P6Version== PC60 ) return;	/* sr_mode or PC-6001 do nothing! 9/20*/
  												// thanks Mr.Bernie 2003/7/25
	f= read_b[0];	//front
	r= read_b[1];	//rear

	portF0 = Value;
	switch(Value&0x0f)
	{
	case 0x00: RdMem[0]=				RdMem[1]=EmptyRAM;		 strcpy( f ,empty_b);	   strcpy( r ,empty_b); break;
	case 0x01: RdMem[0]=BASICROM;   	RdMem[1]=BASICROM+0x2000;strcpy( f ,basic_b[0]);   strcpy( r ,basic_b[1]); break;
	case 0x02: RdMem[0]=CurKANJIROM;	RdMem[1]=CurKANJIROM+0x2000;strcpy( f ,cur_kanji_b[0]); strcpy( r, cur_kanji_b[1]);break;
	case 0x03: RdMem[0]=RdMem[1]=EXTROM2;						 strcpy( f,extrom2_b);     strcpy( r, extrom2_b); break;	
	case 0x04: RdMem[0]=RdMem[1]=EXTROM1;						 strcpy( f,extrom1_b);     strcpy( r, extrom1_b); break;
    case 0x05: RdMem[0]=CurKANJIROM;	RdMem[1]=BASICROM+0x2000;strcpy( f,cur_kanji_b[0]);strcpy(r ,basic_b[1]); break;
    /*
	case 0x06: RdMem[0]=BASICROM;RdMem[1]=CurKANJIROM+0x2000; break;
	*/
	case 0x06: RdMem[0]=BASICROM;								 strcpy( f ,basic_b[0]);
	           RdMem[1]=(SYSROM2==EmptyRAM ? CurKANJIROM+0x2000 : SYSROM2); 
	           if(SYSROM2==EmptyRAM) strcpy( r ,cur_kanji_b[1]); else strcpy(r , "sysrom2");
	           break;  /* 99.06.02. */
	case 0x07: RdMem[0]=EXTROM1;		RdMem[1]=EXTROM2; 		 strcpy( f,extrom1_b);      strcpy( r ,extrom2_b); break;
	case 0x08: RdMem[0]=EXTROM2;		RdMem[1]=EXTROM1; 		 strcpy( f,extrom2_b);      strcpy( r ,extrom1_b); break;
	case 0x09: RdMem[0]=EXTROM2;		RdMem[1]=BASICROM+0x2000;strcpy( f,extrom2_b);      strcpy( r ,basic_b[1]);break;
	case 0x0a: RdMem[0]=BASICROM;		RdMem[1]=EXTROM2; 		 strcpy( f,basic_b[0]);     strcpy( r ,extrom2_b); break;
	case 0x0b: RdMem[0]=EXTROM1;		RdMem[1]=CurKANJIROM+0x2000;strcpy( f,extrom1_b);   strcpy(r,cur_kanji_b[1]);break;
	case 0x0c: RdMem[0]=CurKANJIROM;	RdMem[1]=EXTROM1;		 strcpy( f,cur_kanji_b[0]); strcpy( r ,extrom1_b); break;
	case 0x0d: RdMem[0]=RAM;			RdMem[1]=RAM+0x2000;	 strcpy( f,intram_b[0]);    strcpy( r ,intram_b[1]); break;
	case 0x0e: if( use_extram64) {			 // 2003/5/4
	   		         RdMem[0]=EXTRAM64;	RdMem[1]=EXTRAM64+0x2000; strcpy( f ,extram_b[0]);    strcpy( r ,extram_b[1]); break;
	   		         }
				else
				    {strcpy( read_b[2], "x extram"); strcpy( read_b[3] , "x extram");}
	case 0x0f: RdMem[0]=				RdMem[1]=EmptyRAM;
				strcpy( f , empty_b);	strcpy( r, empty_b); break;
		};


	f= read_b[2];	//front
	r= read_b[3];	//rear
          
	switch(Value&0xf0) 
		{
		case 0x00: RdMem[2]=RdMem[3]=EmptyRAM; 					      strcpy( f ,empty_b); 	strcpy( r,empty_b); break;
		case 0x10: RdMem[2]=BASICROM+0x4000;RdMem[3]=BASICROM+0x6000; strcpy( f,basic_b[2]);strcpy( r,basic_b[3]);break;
		case 0x20: RdMem[2]=VOICEROM;		RdMem[3]=VOICEROM+0x2000; strcpy( f,voice_b[0]); strcpy( r,voice_b[1]);break;
		case 0x30: RdMem[2]=RdMem[3]=EXTROM2;						  strcpy( f ,extrom2_b); strcpy( r,extrom2_b);break;
		case 0x40: RdMem[2]=				RdMem[3]=EXTROM1;         strcpy( f ,extrom1_b); strcpy(r ,extrom1_b);break;
		case 0x50: RdMem[2]=VOICEROM;		RdMem[3]=BASICROM+0x6000; strcpy( f ,voice_b[0]);strcpy(r,basic_b[3]);break;
		case 0x60: RdMem[2]=BASICROM+0x4000;RdMem[3]=VOICEROM+0x2000; strcpy( f ,basic_b[2]);strcpy(r,voice_b[1]);break;
		case 0x70: RdMem[2]=EXTROM1;		RdMem[3]=EXTROM2; 		  strcpy( f ,extrom1_b); strcpy(r,extrom2_b); break;
		case 0x80: RdMem[2]=EXTROM2;		RdMem[3]=EXTROM1; 		  strcpy( f ,extrom2_b); strcpy(r,extrom1_b); break;
		case 0x90: RdMem[2]=EXTROM2;		RdMem[3]=BASICROM+0x6000; strcpy( f ,extrom2_b); strcpy(r,basic_b[3]);break;
		case 0xa0: RdMem[2]=BASICROM+0x4000;RdMem[3]=EXTROM2;  		  strcpy( f ,basic_b[2]);strcpy(r,extrom2_b); break;
		case 0xb0: RdMem[2]=EXTROM1;		RdMem[3]=VOICEROM+0x2000; strcpy( f ,extrom1_b); strcpy(r,voice_b[1]);break;
		case 0xc0: RdMem[2]=VOICEROM;		RdMem[3]=EXTROM1; 		  strcpy( f ,voice_b[0]);strcpy(r,extrom1_b); break;
		case 0xd0: RdMem[2]=RAM+0x4000;		RdMem[3]=RAM+0x6000; 	  strcpy( f,intram_b[2]);strcpy(r,intram_b[3]);break;
 	/* case 0xe0: RdMem[2]=				RdMem[3]=EmptyRAM; break; */
		case 0xe0: if( use_extram64) 				//2003/5/4
	   				{RdMem[2]=EXTRAM64+0x4000;RdMem[3]=EXTRAM64+0x6000;   strcpy( f, extram_b[2]);strcpy(r,extram_b[3]);} 
	   			   else
			    	{strcpy( f ,"x extram"); strcpy( r ,"x extram");}
			   	   break;
		case 0xf0: RdMem[2]=				RdMem[3]=EmptyRAM; 		  strcpy( f, empty_b); strcpy( r , empty_b);break;
    	};
	if (CGSW93) { RdMem[3] = CGROM;   strcpy( r, cgrom_b);}


}


// ****************************************************************************
//          memorymap_mode5_readblock: MODE 5 の読み込みメモリーマップ１
//   In:  Port: Port no.   
//        Value: data
// ****************************************************************************
void memorymap_mode5_readblock1(byte Port , byte Value)
{
char *f,*r;
f= read_b[4];	//front
r= read_b[5];	//rear

if( sr_mode || P6Version==PC60 ) return;	/* sr_mode or PC-6001 do nothing! 9/20*/
										// (thanks Mr. Bernie 3/7/25)


	portF1 = Value;
	switch(Value&0x0f) 
		{
		case 0x00: RdMem[4]=				RdMem[5]=EmptyRAM; 			strcpy(f,empty_b);       strcpy(r,empty_b); break;
		case 0x01: RdMem[4]=BASICROM;		RdMem[5]=BASICROM+0x2000; 	strcpy(f,basic_b[0]);    strcpy(r,basic_b[1]);break;
		case 0x02: RdMem[4]=CurKANJIROM;	RdMem[5]=CurKANJIROM+0x2000;strcpy(f,cur_kanji_b[0]);strcpy(r,cur_kanji_b[1]);break;
		case 0x03: RdMem[4]=				RdMem[5]=EXTROM2; 			strcpy(f,extrom2_b);     strcpy(r,extrom2_b);break;
		case 0x04: RdMem[4]=				RdMem[5]=EXTROM1; 			strcpy(f,extrom1_b);     strcpy(r,extrom1_b);break;
		case 0x05: RdMem[4]=CurKANJIROM;	RdMem[5]=BASICROM+0x2000;   strcpy(f,cur_kanji_b[0]);strcpy(r,basic_b[1]);break;
		case 0x06: RdMem[4]=BASICROM;		RdMem[5]=CurKANJIROM+0x2000;strcpy(f,basic_b[0]);    strcpy(r,cur_kanji_b[1]); break;
		case 0x07: RdMem[4]=EXTROM1;		RdMem[5]=EXTROM2; 			strcpy(f,extrom1_b);     strcpy(r,extrom2_b); break;
		case 0x08: RdMem[4]=EXTROM2;		RdMem[5]=EXTROM1; 			strcpy(f,extrom2_b);     strcpy(r,extrom1_b);break;
		case 0x09: RdMem[4]=EXTROM2;		RdMem[5]=BASICROM+0x2000;   strcpy(f,extrom2_b);     strcpy(r,basic_b[1]);break;
		case 0x0a: RdMem[4]=BASICROM;		RdMem[5]=EXTROM2;           strcpy(f,basic_b[0]);    strcpy(r,extrom2_b); break;
		case 0x0b: RdMem[4]=EXTROM1;		RdMem[5]=CurKANJIROM+0x2000;strcpy(f,extrom1_b);     strcpy(r,cur_kanji_b[1]);break;
		case 0x0c: RdMem[4]=CurKANJIROM;	RdMem[5]=EXTROM1;           strcpy(f,cur_kanji_b[0]);strcpy(r,extrom1_b); break;
		case 0x0d: RdMem[4]=RAM+0x8000;		RdMem[5]=RAM+0xa000;        strcpy(f,intram_b[4]);   strcpy(r,intram_b[5]);break;
		 /*case 0x0e: RdMem[4]=					RdMem[5]=EmptyRAM; break; */
		case 0x0e: if( use_extram64) // 2003/5/4
	   				{RdMem[4]=EXTRAM64+0x8000;RdMem[5]=EXTRAM64+0xa000;	strcpy(f,extram_b[4]);strcpy(r,extram_b[5]);break;}
				   else
				    {strcpy(f,"x extram"); strcpy(r ,"x extram");}
   		case 0x0f: RdMem[4]=				RdMem[5]=EmptyRAM; 			strcpy(f,empty_b); strcpy(r,empty_b); break;
        };


	f= read_b[6];	//front
	r= read_b[7];	//rear

      switch(Value&0xf0) 
		{
		case 0x00: RdMem[6]=				RdMem[7]=EmptyRAM; 			strcpy(f,empty_b);strcpy(r,empty_b); break;
	 	case 0x10: RdMem[6]=BASICROM+0x4000;RdMem[7]=BASICROM+0x6000;   strcpy(f, basic_b[2]);strcpy(r ,basic_b[3]); break;
	 	case 0x20: RdMem[6]=CurKANJIROM;	RdMem[7]=CurKANJIROM+0x2000;strcpy(f, cur_kanji_b[0]);strcpy(r , cur_kanji_b[1]); break;
		case 0x30: RdMem[6]=				RdMem[7]=EXTROM2;           strcpy(f, extrom2_b); strcpy(r , extrom2_b); break;
		case 0x40: RdMem[6]=				RdMem[7]=EXTROM1; 			strcpy(f, extrom1_b); strcpy(r , extrom1_b); break;
		case 0x50: RdMem[6]=CurKANJIROM;	RdMem[7]=BASICROM+0x6000;   strcpy(f, cur_kanji_b[0]);strcpy(r , basic_b[3]);break;
		case 0x60: RdMem[6]=BASICROM+0x4000;RdMem[7]=CurKANJIROM+0x2000;strcpy(f, basic_b[2]);strcpy(r , cur_kanji_b[1]);break;
		case 0x70: RdMem[6]=EXTROM1;		RdMem[7]=EXTROM2; 			strcpy(f, extrom1_b); strcpy(r , extrom2_b); break;
		case 0x80: RdMem[6]=EXTROM2;		RdMem[7]=EXTROM1; 			strcpy(f, extrom2_b); strcpy(r , extrom1_b); break;
		case 0x90: RdMem[6]=EXTROM2;		RdMem[7]=BASICROM+0x6000;	strcpy(f, extrom2_b); strcpy(r , basic_b[3]);break;
		case 0xa0: RdMem[6]=BASICROM+0x4000;RdMem[7]=EXTROM2; 			strcpy(f, basic_b[2]);strcpy(r , extrom2_b); break;
		case 0xb0: RdMem[6]=EXTROM1;		RdMem[7]=CurKANJIROM+0x2000;strcpy(f, extrom1_b); strcpy(r , cur_kanji_b[1]);  break;
		case 0xc0: RdMem[6]=CurKANJIROM;	RdMem[7]=EXTROM1; 			strcpy(f, cur_kanji_b[0]);strcpy( r , extrom1_b);;break;
		case 0xd0: RdMem[6]=RAM+0xc000;		RdMem[7]=RAM+0xe000;        strcpy(f, intram_b[6]); strcpy(r,intram_b[7]);break;
	 /*case 0xe0: RdMem[6]=					RdMem[7]=EmptyRAM; break; */
		case 0xe0: if( use_extram64) // 2003/5/4
	   				{RdMem[6]=EXTRAM64+0xc000;RdMem[7]=EXTRAM64+0xe000; strcpy(f,extram_b[6]); strcpy(r, extram_b[7]);break;}				  
				   else
				    {strcpy(f, "x extram"); strcpy(r , "x extram");}

		case 0xf0: RdMem[6]=				RdMem[7]=EmptyRAM; 		strcpy(f, empty_b);   strcpy(r ,empty_b); break;
     	};
}

#undef strcpy

// ****************************************************************************
//          memorymap_mode1_extmegarom: MODE 1 の拡張MEGAROM バンク切り替え　メモリーマップ
//   In:  Port: Port no.   
//        Value: data
// ****************************************************************************
void memorymap_mode1_extmegarom( byte Port , byte Value)
{
	Value &= 0xf;
	if(megarom_flag==1 && (P6Version ==PC60 || P6Version == PC60M2  || P6Version ==PC66  ))
		{
		sprintf(read_b[2] , "MEGAROM%02d",Value);	// MEGAROM bank
		
		RdMem[2]=EXTROM1+ 0x2000* Value;			// select bank for MEGAROM
		
		megarom_bank_no = Value;
		//printf(" MEGAROM BANK select  NO. =%d \n", Value);
		}
}





// ****************************************************************************
//          memorymap_mode5_readblock: MODE 5 の書き込みメモリーマップ
//   In:  Port: Port no.   
//        Value: data
// ****************************************************************************

void memorymap_mode5_writeblock(byte Port , byte Value)
{
	if( sr_mode || P6Version== PC60 ) return;	/* sr_mode or PC-6001 do nothing! 9/20*/

    if(Value&0x40) 
		{EnWrite[3]=1;WrMem[6]=RAM+0xc000;WrMem[7]=RAM+0xe000;}
	else
	 	 EnWrite[3]=0;

    if(Value&0x010)
	 	{EnWrite[2]=1;WrMem[4]=RAM+0x8000;WrMem[5]=RAM+0xa000;}
    else
	 	 EnWrite[2]=0;

    if(Value&0x04)
		{EnWrite[1]=1;WrMem[2]=RAM+0x4000;WrMem[3]=RAM+0x6000;}
    else
	 	 EnWrite[1]=0;
    if(Value&0x01)
	 	{EnWrite[0]=1;WrMem[0]=RAM;WrMem[1]=RAM+0x2000;}
    else
		 EnWrite[0]=0;

	if( use_extram64)
		{
		if(Value&0x80){EnWrite[3]=2;WrMem[6]=EXTRAM64+0xc000;WrMem[7]=EXTRAM64+0xe000; strcpy(write_b[6], extram_b[6]);strcpy(write_b[7], extram_b[7]);}
		if(Value&0x20){EnWrite[2]=2;WrMem[4]=EXTRAM64+0x8000;WrMem[5]=EXTRAM64+0xa000; strcpy(write_b[4], extram_b[4]);strcpy(write_b[5], extram_b[5]);}
		if(Value&0x08){EnWrite[1]=2;WrMem[2]=EXTRAM64+0x4000;WrMem[3]=EXTRAM64+0x6000; strcpy(write_b[2], extram_b[2]);strcpy(write_b[3], extram_b[3]);}
		if(Value&0x02){EnWrite[0]=2;WrMem[0]=EXTRAM64+0x0000;WrMem[1]=EXTRAM64+0x2000; strcpy(write_b[0], extram_b[0]);strcpy(write_b[1], extram_b[1]);}
		}
}


// ****************************************************************************
//          memorymap_voice_kanjirom: VOICE ROM / KANJI ROM　メモリーマップ
///   In:  Port: Port no.   
//        Value: data
// ****************************************************************************
			/* ---------------------------------
				B1   B0
				|     ----0: VOICEROM       1:KANJIROM
				----------0: KANJIROM LEFT  1:KANJIROM RIGHT
			--------------------------------*/
void memorymap_voice_kanjirom(byte Port, byte Value)
{
	if(sr_mode) { return;}     /* not SR basic   add 2002/2 */
            							// KANJIROM ON
    if ((Value&0x02)==0x00) 
			{CurKANJIROM=KANJIROM;		 cur_kanji_b[0] = _kanji_b[0]; cur_kanji_b[1] = _kanji_b[1]; }
    else 
			{CurKANJIROM=KANJIROM+0x4000;cur_kanji_b[0] = _kanji_b[2]; cur_kanji_b[1] = _kanji_b[3]; }
			
    if ((Value&0x01)==0x00) 	// VOICEROM ON
    	{
		// --- 2000.10.13.
        if(P6Version== PC66 )
	    	{
	        if(RdMem[0]!=BASICROM       && RdMem[0]!=RAM ) 
	        	{ RdMem[0]=VOICEROM;			strcpy( read_b[0] , voice_b[0]);}
	        if(RdMem[1]!=BASICROM+0x2000&& RdMem[1]!=RAM+0x2000)
	        	{ RdMem[1]=VOICEROM+0x2000;		strcpy( read_b[1] , voice_b[1]);}
			}

		if( P6Version==PC66SR  ) /* add 2002/2/25 for TV yoyaku PC-6601SR*/
	        {  			/* mapping SYSTEMROM2 on mode=5 */
	        if(RdMem[0]!=BASICROM)        {RdMem[0]=SYSTEMROM2;			sprintf( read_b[0] , sys2_b , 0); }
	        if(RdMem[1]!=BASICROM+0x2000) {RdMem[1]=SYSTEMROM2+0x2000;  sprintf( read_b[1] , sys2_b , 1); }
		    }
			     // --- 2000.10.13.
			{
			if(RdMem[2]!=BASICROM+0x4000) {RdMem[2]=VOICEROM;			strcpy( read_b[2] , voice_b[0]);}
			if(RdMem[3]!=BASICROM+0x6000) {RdMem[3]=VOICEROM+0x2000;	strcpy( read_b[3] , voice_b[1]);}
			}
        }
	else 
	    {
	     DoOut(0xF0,portF0);	// curKANJIROM を適用
        };
               /* 99.06.02. */
    /*
    	else {RdMem[0]=CurKANJIROM;RdMem[1]=CurKANJIROM+0x2000;};
    */
	}




/* HOMEディレクトリ以下より、ROM ファイルを検索する */
static int  RomFound = 0;		// found flag
static char separator[]="/";	// path separator
static char ext[10];				// ext (.60 .62 .64 .66 .68)

// ****************************************************************************
//     dirlist :   recursive search file 
//
//    In:  dir:  path
// ****************************************************************************
void dirlist(char *dir)
{
    struct dirent *entry;  struct stat statbuf;  int len;

    DIR *dp = opendir(dir);
//    if (dp == NULL) {printf("FAILED \n"); perror("opendir"); printf("'%s'\n ", dir ); exit(1);}
    if (dp == NULL) {printf("FAILED \n"); perror("opendir"); printf("'%s'\n ", dir ); return;}

    len = strlen(dir);
    while ((entry = readdir(dp)) != NULL && !RomFound)
        if (strcmp(entry->d_name, ".") && strcmp(entry->d_name, "..")) {
            sprintf(dir + len, "%s%s", separator , entry->d_name);
            stat(dir, &statbuf);
            if (S_ISDIR(statbuf.st_mode)){
                //printf("\ndir: %s \n", entry->d_name);
                dirlist(dir);       // recursive call
            } else {
                int n = strlen(entry->d_name);
                if (n > 3 &&  strncmp(entry->d_name + n - 3, ext,3) == 0)
					if(       strncmp(entry->d_name ,"BASICROM",8)==0 || strncmp( entry->d_name,"SYSTEMROM",9 )==0) {
                    // printf("found : '%s' '%s' \n", dir , entry->d_name);
					my_strncpy( RomPath , dir , strlen( dir) - strlen(entry->d_name));
					printf("found : '%s' \n ", RomPath );
					RomFound =1;
					}
            }
        }
    closedir(dp);
}



// ****************************************************************************
//			search_roms :   search rom files
// ****************************************************************************
int search_roms(void)
{
    char dir[ 8192];
    char *p;


	// set ext  .60 .62 .64 .66 .68 
	sprintf( ext,".6%d",P6Version *2);

    p = getenv( "HOME");	// for UNIX
	if( p !=NULL)
    	strcpy( dir , p);
    else
	{
		p = getenv( "HOMEPATH");	// for Windows
		if( p !=NULL)
			strcpy( dir, p);
		else
			dir[0]=0;
	}
	

    printf("\nSearching ROM files... ");
    dirlist( dir);
    
	if( RomFound )
		return 1;
	else
		return 0;
}



	


// ****************************************************************************
//     StartP6:
//
// Allocate memory, load ROM images, initialize mapper, VDP 
// CPU and start the emulation. This function returns 0 in  
// the case of failure.                                     
// ****************************************************************************
int StartP6(void)
{
	int  ret = TRUE;
	int *T, J;
	byte *P;

	unsigned long basic_crc32[] = { 0x54c03109,		// 60	
								 0x950ac401,		// 62
								 0x516b1be3,		// 64
								 0xc0b01772,		// 66 
								 0x516b1be3 };		// 68

	/* patches in BASICROM */
	word BROMPatches[5][32] = {
		//	    { 0x10EA,0x18,0 },		/* 60:to limit 16K	*/
				{ 0 },		/* 60:to limit 16K	*/
				{ 0x78D2,0xD2,0 },	/* 62:fix bug of MON 'Q' Command */ /* 2006/6/11 Windy */
				{ 0 },			/* 64:do nothing        */
				{ 0 },			/* 66:do nothing        */
				{ 0 }			/* 68:do nothing        */
	};

	// { 0x8000+0x5a97,0xc3,0x8000+0x5a98,0x63,0x8000+0x5a99,0x5b,0 }  /* 68:skip menu */

	/* { 0x601C,0x18,0x601D,0x03,0 }, */	/* 66:skip disk check  */
	/* { 0x9025,0xcd,0x9026,0x30,0x9027,0x10,0 },*/  /* 64:screen can change 2002/2 */
	/* { 0x9025,0xcd,0x9026,0x30,0x9027,0x10,0 },*/  /* 68:screen can change 2002/2 */
	/*
	66 では、ディスクを使えるようになりました。
	64と68は、垂直同期の割り込みが 仮実装されました。
																		 Windy
	*/

#ifdef WIN32
	strcpy(separator, "\\");
#endif

	/*** PSG initial register states: ***/
	static byte PSGInit[16] = { 0,0,0,0,0,0,0,0xFD,0,0,0,0,0,0,0xFF,0 };

	/*** STARTUP CODE starts here: ***/

	T = (int *)"\01\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0";


#ifndef WORDS_BIGENDIAN
	if (*T != 1)
	{
		printf("********** This machine is high-endian. **********\n");
		//  printf("Take #define LSB_FIRST out and compile iP6 again.\n");
		printf("Insert #define WORDS_BIGENDIAN and compile iP6 again.\n");
		return(0);
	}
#else
	if (*T == 1)
	{
		printf("********* This machine is low-endian. **********\n");
		//   printf("Insert #define LSB_FIRST and compile iP6 again.\n");
		printf("Take #define WORDS_BIGENDIAN out  and compile iP6 again.\n");
		return(0);
	}
#endif

	saveDateTime();	// 現在時刻を保存する


   /* ----------- SR のVRAMの右半分の各ラインの開始アドレス ---------- */
	{
		int y;
		word addr = 0;
		for (y = 0; y < M6HEIGHT; y += 2)
		{
			sr_vram_right_addr[y] = sr_vram_right_addr[y + 1] = addr;
			switch (y & 0xf)
			{
			case 0:
			case 4:
			case 8:
			case 12:addr += 0x100; break;
			case 2:
			case 6:
			case 10:addr -= 0xc0; break;
			case 14:addr += 0x40; break;
			}
		}
	}

	init_disasm_comment();
	load_disasm_comment();


	CasStream[0] = CasStream[1] = PrnStream = NULL;
	DskStream[0] = DskStream[1] = NULL;		/* add 2002/3/14  by Windy*/

	OpenFile1(FILE_PRNT);
	OpenFile1(FILE_LOAD_TAPE);
	OpenFile1(FILE_SAVE_TAPE);
	OpenFile1(FILE_DISK1);		/* add 2002/3/14  by Windy*/
	OpenFile1(FILE_DISK2);


	sr_mode = (P6Version == PC66SR || P6Version == PC60M2SR) ? 1 : 0;/*  add 2002/3/9  SR_MODE ? */
	disk_type = (P6Version >= PC66) ? 1 : 0;	  /*  add 2002/3/30 disk type ? */

	/* ******* FDC を初期化 ************************ */
	if (disk_type)  fdc_init();		/* add 2002/3/20   disk initialize */

	/* ******* 空メモリー(8KB) を作成 ************************ */
	if (Verbose) printf("Allocating 8kB for empty space...");
	EmptyRAM = (byte*)malloc(0x2000);

	/* ******* メインRAM(64KB) を作成 ************************ */
	/* allocate 64KB RAM */
	if (Verbose) printf("OK\nAllocating 64kB for RAM space...");
	RAM = (byte*)malloc(0x10000);  if (!RAM) return(0);
	memset(RAM, 0, 0x10000);		// clear RAM   add 2002/11/4


  /* ******* 拡張RAM(64KB) を挿入する ************************ add 2003/4/6 */
  /* 拡張RAMは、常に確保しておいて、Use_extram64 で、切り替える */
	{
		if (Verbose) printf("OK\nAllocating 64kB for EXT-RAM space...");
		EXTRAM64 = (byte*)malloc(0x10000);  if (!EXTRAM64) { printf("FAILED\n"); return(0); }
		memset(EXTRAM64, 0, 0x10000);
	}

	/* ******* 拡張VRAM を作成 ************************ */
	if (Verbose) printf("OK\nAllocating 32kB for EXT-VRAM space...");
	EXT_VRAM = (byte*)malloc(0x8000);  if (!EXT_VRAM) return(0);
	memset(EXT_VRAM, 0, 0x8000);

	printf("OK\n");



	DEBUG_CGROM = (byte*)malloc(0x1000);  if (!DEBUG_CGROM) { printf("FAILED\n"); return(0); }
	conv_cgrom(DEBUG_CGROM);

	if (Use_CompatibleROM)		// ====================== Compatible ROM =======================
		{
		if (OSD_MessageBox("MSG_COMPATIBLE_ROM", "", OSDM_YESNO) == OSDR_YES)
			{
			if (P6Version != PC60 && P6Version != PC66)	// 互換ROMのない機種だと、機種選択してもらう
				{
				int stat = OSD_SelectMachine();  // PC-6001/ 6601 どちらかを選択する
				if (stat >= 0)
					{
					newP6Version = P6Version = stat;
					sr_mode = (P6Version == PC66SR || P6Version == PC60M2SR) ? 1 : 0;/* SR_MODE ? */
					}
				}


			if (!load_compatible_roms())
				{
				put_notfoundromfile();
				ret = FALSE;
				}
			}
		else
			{
			put_notfoundromfile();
			ret = FALSE;
			}
		}
	else							// ============== 実機ROM ==========================
		{
		if (!load_roms())           /* load roms */
			{
			if (OSD_MessageBox("MSG_SEARCH_ROM", "", OSDM_YESNO) == OSDR_YES)
				{
				if (search_roms())
					{
					if (load_roms())
						{
						OSD_MessageBox("MSG_ROM_FOUND_BOOT", "", OSDM_OK);
						ConfigWrite();
						OSD_ClearWindow();
						}
					else
						{
						put_notfoundromfile();			// put not found rom to screen
						ret = FALSE;
						}
					}
				else
					{
					if (OSD_MessageBox("MSG_NOT_FOUND_COMPATIBLE_ROM", "", OSDM_YESNO) == OSDR_YES)
						{
						if (P6Version != PC60 && P6Version != PC66)	// 互換ROMのない機種だと、機種選択してもらう
							{
							int stat = OSD_SelectMachine();  // PC-6001/ 6601 どちらかを選択する
							if (stat >= 0)
								{
								newP6Version = P6Version = stat;
								sr_mode = (P6Version == PC66SR || P6Version == PC60M2SR) ? 1 : 0;/* SR_MODE ? */
								}
							}

						if (load_compatible_roms())
							{
							new_Use_CompatibleROM = Use_CompatibleROM = TRUE;
							}
						else
							{
							put_notfoundromfile();
							ret = FALSE;
							}
						}
					}
				}
			else {
				put_notfoundromfile();
				ret = FALSE;
				}
			}
		}
#ifdef X11
	if(ret==FALSE)			/* X11: failed -> exit */
		exit(0);
#endif

	if( ret)
		if( !make_internalroms()) return(0);   /* make internal roms */

   /* 2003/8/8  CGROM6を分割してから、セミグラデータを作成すること。*/
	if(ret) 
		makestrcpyemigraph();		/* make semigraph */
	// CGROM = CGROM1;



	/* ***************  patch BASICROM *****************/
	if( ret)
//	 if((PatchLevel) && (BROMPatches[P6Version][0]) ) 
		{
		int size;
		if( sr_mode) 
			P=SYSTEMROM1;
		else
			P=BASICROM;
		
		InitCRC32();

		size = (!sr_mode ? sizeList[P6Version][0] : 0x8000);
		 if( basic_crc32[ P6Version] == GetCRC32( P, size ,0xFFFFFFFF) )		// CRC32 一致した場合のみ、パッチを当てる
			{
			if(Verbose)  printf( (sr_mode)? "  Patching SYSTEMROM1:  ": "  Patching BASICROM: ");
			for(J=0; BROMPatches[P6Version][J] ;) 
    			{
    			if(Verbose) printf("%04X..",BROMPatches[P6Version][J]);
    			if( sr_mode) 
	   				{                   /* for SR-BASIC add 2002/2 by Windy*/
       				P=SYSTEMROM1+BROMPatches[P6Version][J++];
					}
				else 
	  				{
					P=BASICROM+BROMPatches[P6Version][J++];
	  				}
				P[0]=(byte) BROMPatches[P6Version][J++];
    			};
    		if(Verbose) printf(".OK\n");
			}
		 else
			{
			printf("  BASICROM crc error \n");
			}
		}

	BASICROM[ 0x1a4f ] = 0xc9;			// printer katakana にしない　　暫定パッチ★

  /* ***************  INIT MEMORY MAPPER *****************/
	InitMemmap();

	if (ret)
	{
		initmemory_withcoldstart(RAM);	//電源ON時の　RAMの初期状態をある程度再現する
	}

	if(Verbose) printf("OK\nInitializing PSG, and CPU...");
	memcpy(PSGTMP,PSGInit,sizeof(PSGInit));

	JoyState[0]=JoyState[1]=0xFF;
	PSGReg=0;
	EndOfFrame=1;


	SetValidLine_sr( drawwait);	// re-calc drawwait (再計算)
	SetClock(CPUclock*1000000);

	Event_init();

	ResetZ80();

#if VOI_LOG
  test_clear_voice_data();	/* 音声合成ファイルを空にする */
#endif

  keybuffer      = init_keybuffer();	// <--- init key buffer
  auto_keybuffer = init_keybuffer();

  Romfile_found = ret;
  return( ret);
}

void put_notfoundromfile(void)
{
	char errmsg[256];
	char path[256];
	int max=256;

	OSD_textout( OSD_GetVideoSurface() ,50,100, mgettext("MSG_ROM_NOT_FOUND"),16);

	OSD_textout( OSD_GetVideoSurface() ,50,120, mgettext("MSG_SET_ROM_PATH") ,16);

	OSD_textout( OSD_GetVideoSurface() ,50, 140, mgettext("MSG_COPY_ROM_FILE"), 16);

	OSD_GetModulePath(path, max);
	strcat(path,separator);
	strcat(path,"rom");

	sprintf(errmsg, "ROM PATH: %s", path);
	OSD_textout( OSD_GetVideoSurface(), 10, 170,errmsg , 12);
}




int chk_disk_type(int drive)
{
	int ret =1;
	char *msg1D  = MSG_1D_FDD;
	char *msg1DD = MSG_NO_1DD;

	// ----------- if PC-6001mk2 and PC-6601 and 1DD then error -------------
	if(( P6Version== PC60M2  || P6Version == PC66 )&& is1DD(drive))
		{ 
		//fclose( DskStream[drive]); DskStream[drive]=NULL; *DskName[drive]=0;
		printf( "%s",msg1DD);

		OSD_MessageBox( msg1DD , " floppy disk information",OSDR_OK);
		ret =0;
		}

	/* SRでの、1D format は、1D->1DD コンバートに必要なので、警告だけにしておく */
    // ----------- if PC-6601SR and  1D then error --------------------------
	if(  P6Version== PC66SR  && !is1DD(drive) && !isSYS(drive)      )
		{
		printf( "%s",msg1D );

		OSD_MessageBox( msg1D , " floppy disk information",OSDR_OK);
		ret = 1;
		}
	return ret;
}


int chk_same_disk(int drive ,char *fullpath)
{
	int ret=0;
	char tmp0[PATH_MAX],tmp1[PATH_MAX];
	sprintf(tmp0,"%s%s",DskPath[0],DskName[0]);
	sprintf(tmp1,"%s%s",DskPath[1],DskName[1]);
	if( strcmp( tmp0, tmp1) !=0)		// 二つのディスクが違うか？
		{
		if(Verbose) printf("Using %s as a disk (%d)\n",fullpath ,drive+1);
		ret =1;
		}
	else {
		 OSD_MessageBox( MSG_SAME_DISK_IMAGE, "disk information", OSDM_OK );
		 if(Verbose) printf("%s disk ejected.(because drive1 and drive2 cannot be the same disk)\n",fullpath );

		}
	return ret;
}



int mount_fd(int drive ,char *fullpath)
{
	int ret=0;
	if((DskStream[drive]=fopen( fullpath,"r+b"))!= NULL)
		{
		 if( chk_same_disk( drive ,fullpath))
			{
			read_d88head(drive);
			 if( chk_disk_type( drive))
				{
				setDiskProtect(drive ,0);
				ret=1;
				}
			else {
				 fclose( DskStream[drive]);  DskStream[drive] = NULL; *DskName[drive]=0;	ConfigWrite();

				}
			}
		 else {
			 fclose( DskStream[drive]);   DskStream[drive] = NULL; *DskName[drive]=0;		ConfigWrite();
			}
		}
	else if((DskStream[drive]=fopen( fullpath,"rb"))!= NULL) // write protect
			{
			 if( chk_same_disk( drive ,fullpath))
				{
				read_d88head(drive);
				 if( chk_disk_type( drive))
					{
					setDiskProtect(drive,1);
					ret=1;
					}
				else {
					fclose( DskStream[drive]);  DskStream[drive] = NULL;  *DskName[drive]=0;	ConfigWrite();
					}
				}
 			 else {
				fclose( DskStream[drive]);   DskStream[drive] = NULL; *DskName[drive]=0;	ConfigWrite();
				}
			}
	else 
#if 0 // AUTO_FORMAT
          if( createNewD88( fullpath , (P6Version==2||P6Version==4)? 1:0))
			{
			if((DskStream[drive]=fopen( fullpath,"r+b"))!= NULL)
            	{
		  		if(Verbose) printf("Using %s as a disk (%d) (blank disk)\n",fullpath,drive+1);
				read_d88head();
    	        }
        	else
            	{
			   	*DskName[0]=0;
				}
          	}
          else
#endif // AUTO_FORMAT
			{
			*DskName[0]=0;
			printf("Can't open %s as a disk. \n",DskName[0]);
			}


	return ret;
}


// ****************************************************************************
//		tape counter store/restore
//   テープをイジェクトするときにカウンターを保存しておいて、再度挿入したときに復元する
// ****************************************************************************

#define MAX_TAPECOUNTER 3

struct _tapeCounter {
	char    fullpath[PATH_MAX];
	fpos_t  counter;
} tapeCounter[MAX_TAPECOUNTER];



// ****************************************************************************
//		init tape counter
// ****************************************************************************
void init_tapeCounter(void)
{
#ifdef USE_SAVETAPECOUNTER
	for (int i = 0; i < MAX_TAPECOUNTER; i++)
		{
		 tapeCounter[i].fullpath[0]='\0';
		}
#endif
}

// ****************************************************************************
//		set tape counter
//  In: fpath: fullpath of tape file
//      cnt:   counter of tape file
//  Out: 1: SUCESS  0: FAILED
// ****************************************************************************
int SetTapeCounter(char* fpath, fpos_t cnt)
{
	int found = 0;

#ifdef USE_SAVETAPECOUNTER
	for (int i = 0; i < MAX_TAPECOUNTER; i++)
		{
		if ( tapeCounter[i].fullpath[0] == 0)
			{
			my_strncpy( tapeCounter[i].fullpath ,fpath ,MAX_PATH);
			tapeCounter[i].counter = cnt;
			found =1;
			break;
			}
		}
#endif
	return found;
}

// ****************************************************************************
//		get tape counter
//  In:  fpath: fullpath of tape file
//  Inout: cnt: counter of tape file
//  Out: 1:found     0:not found
// ****************************************************************************
int GetTapeCounter(char* fpath, fpos_t *cnt)
{

	int found =0;

#ifdef USE_SAVETAPECOUNTER
	for(int i=0; i< MAX_TAPECOUNTER ; i++)
		{
		if (strcmp(tapeCounter[i].fullpath, fpath) == 0)
			{
			found =1;
			*cnt = tapeCounter[i].counter;
			tapeCounter[i].fullpath[0]= 0;	// clear counter
			break;
			}
		}
#endif
	return found;
}

// ****************************************************************************
//     Load tape と Save tape は同じか？
//  Out: TRUE: 同じ   FALSE:違う
// ****************************************************************************
int IsSameLoadSaveTape(void)
{
	int ret = FALSE;

#ifdef USE_SAVETAPECOUNTER
	if (strncmp(CasPath[0], CasPath[1], MAX_PATH) == 0)
		{
		if (strncmp(CasName[0], CasName[1], MAX_PATH) == 0)
				{ 
				ret = TRUE;
				}
		}
#endif
	return ret;

}

// ****************************************************************************
//   OpenFIle1:  open file 
// Close(if needed) and Open tape/disk/printer file.
// ****************************************************************************
void OpenFile1(unsigned int num)
{
	int drive = 0;
	char fullpath[ PATH_MAX];		// Path+Name
	switch(num) 
		{
		case FILE_LOAD_TAPE:
	  		sprintf( fullpath,"%s%s",CasPath[0], CasName[0]);	// make fullpath
	   		if(CasStream[0]) {fclose(CasStream[0]); CasStream[0]=NULL;}
    		if(*CasName[0])
    		if((CasStream[0]=fopen(fullpath,"rb")) != NULL)
				{
				 fseek(CasStream[0],0,SEEK_END);
				 fgetpos(CasStream[0], &CasSize[0] );	// tape size

				 fseek(CasStream[0],0,SEEK_SET);
				 if(Verbose) printf("Using %s as a load tape\n",fullpath);
				}
      		else
			    {
				printf("Can't open %s as a load tape\n",CasName[0]);
				}
    		break;

	   case FILE_SAVE_TAPE:
			sprintf( fullpath,"%s%s",CasPath[1], CasName[1]);	// make fullpath
    		if(CasStream[1]) {fclose(CasStream[1]); CasStream[1]=NULL;}
    		if(*CasName[1])
      			if((CasStream[1]=fopen(fullpath,"r+b")) != NULL)
				{
				fclose(CasStream[1]);
		 		if(Verbose) printf("Using %s as a save tape\n",fullpath);
				}
      		else
        		{
         		if((CasStream[1]=fopen(fullpath,"wb")) != NULL)
           			{
					fclose(CasStream[1]);
		    		}
         		else
           			{
            		printf("Can't open %s as a save tape\n",CasName[1]);
            		*CasName[1]=0;
           			}
         		}
			break;

		case FILE_DISK2:
			drive = 1;
		case FILE_DISK1:
			setDiskProtect( drive,0); // set writeable
  	 		sprintf( fullpath,"%s%s",DskPath[drive], DskName[drive]);	// make fullpath
     		if(DskStream[drive]) {fclose( DskStream[drive]); DskStream[drive]=NULL;}
     		/* disk file add 2002/3/14*/

			if(*DskName[drive] && drive+1 <= disk_num)		/* disk file exist */
				{

				if( !mount_fd(drive, fullpath))
					{
	    			}
				}
    		break;

		case FILE_PRNT:
    		if(PrnStream&&(PrnStream!=stdout)) fclose(PrnStream);
    		if(!*PrnName) 
				PrnStream=stdout;
    		else
    	    	{
				if(Verbose) printf("Redirecting printer output to %s...",PrnName);
				//if(!(PrnStream=(FILE*)fopen(PrnName,"wb"))) PrnStream=stdout;
				PrnStream=(FILE*)fopen(PrnName,"wb");
				if(!PrnStream) PrnStream=stdout;

				if(Verbose) printf((PrnStream==stdout)? MsgFAILED:MsgOK);
				}
	    	break;
  		}
	 setMenuTitle(num);
}


// ****************************************************************************
//		OutputSaveTape
//  Save Tape に出力する
//  In: Value : 出力したいデータ
// ****************************************************************************
int OutputSaveTape( byte Value)
{
	if (CasStream[1]) { fclose(CasStream[1]); CasStream[1] = NULL; }
	if (*CasName[1])
		{
		char fullpath[PATH_MAX];
		sprintf(fullpath, "%s%s", CasPath[1], CasName[1]);	// make fullpath
		if ((CasStream[1] = fopen(fullpath, "ab")) != NULL)	// 1バイトごとにopen/close
			{
			fputc(Value, CasStream[1]);
			fclose(CasStream[1]);
			}
		}
}



// ****************************************************************************
//    		ramdump: memory dump
// ****************************************************************************
void ramdump(char *path)
{
  FILE *fp;
  if(!(fp = (FILE*)fopen( path, "wb")))
    { puts("can't open ip6core"); return; }
  if(fwrite(RAM, 1, 0x10000, fp)!=0x10000)
    { puts("can't write ip6core"); return; }
  fclose(fp);
  printf("wrote %s...OK",path);
}



	//TEXTVRAM=RAM;         // TEXT VRAM  for SR-BASIC 
//byte *TEXTVRAM;					// TEXT VRAM ADDRESS (MODE 6のみ) screen 2,2,1対策 add 2003/10/25
