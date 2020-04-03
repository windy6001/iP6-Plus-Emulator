/** Z80: portable Z80 emulator *******************************/
/**                                                         **/
/**                         Debug.h                         **/
/**                                                         **/
/** This file contains the built-in debugging routine for   **/
/** the Z80 emulator which is called on each Z80 step when  **/
/** Trap!=0.                                                **/
/**                                                         **/
/** Copyright (C) Marat Fayzullin 1995-1998                 **/
/**     You are not allowed to distribute this software     **/
/**     commercially. Please, notify me, if you make any    **/
/**     changes to this file.                               **/
/*************************************************************/
#include <stdio.h>

void locate(int x,int y);
char *d_fgets( char *buff,int len ,FILE *stream);
void ramdump(char *path);

extern unsigned char DebugResult[10000];
int monitor(reg *R);

void DebugStart(reg *R);
int  DebugCommand(reg *R, const char *Command);

void open_debug_dialog(void);
void close_debug_dialog(void);
int DAsm(char *S, char *M, char *comment, word A);

void  DebugCommandPrompt( void);

extern byte debug_key;		/* debug_key */
extern int   debug_keydown; 

extern int is_open_debug_dialog;  /* 1: open  0:close */


void DebugPutString( int x, int y , unsigned char *str ,int max_x ,char *attr);
void DebugPutResult(void);
void DisplayDisasm(void);
void  DebugDo(void);

#define DEBUG_WINDOW_RATE 3.0   // 2.7
