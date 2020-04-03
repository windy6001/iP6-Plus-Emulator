/** iP6: PC-6000/6600 series emualtor ************************/
/**                                                         **/
/**                         WinSound.h                      **/
/**                                                         **/
/** modified by windy 2002-2004                             **/
/** This code is based on fMSX written by Marat Fayzullin   **/
/*************************************************************/
#include "../types.h"

int  win32sound_Open(void);
void win32sound_Close(void);
void win32sound_SoundOut(byte R,byte V);

#ifdef WIN32
#define WINSOUND_BUFFERS 6 // 4 // 8 // 6		/* 8 */
#endif

