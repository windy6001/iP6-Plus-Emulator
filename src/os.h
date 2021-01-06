/** iP6: PC-6000/6600 series emualtor ************************/
/**                                                         **/
/**                           os.h                          **/
/**                                                         **/
/** by Windy                                                **/
/*************************************************************/
#ifndef __OS_H_INCLUDED__
#define __OS_H_INCLUDED__

#include "types.h"
//#include "Video.h"

#ifdef WIN32
#include "win/Win32.h"
#endif


#ifdef UNIX
#include "unix/Unix.h"
#endif

#ifdef __APPLE__
#endif

int OSD_ChkEvent(void);
int OSD_RunApplicationEventLoop(void);
void OSD_initKeymap(void);
int OSD_transkey( int scancode);
int OSD_GetModulePath(char *path , int max);
void OSD_SetModulePath( char *path );
int OSD_MessageBox( const char *mes, const char *cap, int type );
int OSD_GetModulePath(char *path , int max);
int setMenuTitle(int type);
void OSD_InstallTimerEvent(void);
byte * OSD_GetXtab(void);
void OSD_SetWindowTitle(char *name);
void OSD_Delay(int s);

int OSD_getlocale(void);

#define	OSDM_OK				0x000
#define	OSDM_OKCANCEL		0x001
#define	OSDM_YESNO			0x002
#define	OSDM_YESNOCANCEL	0x003

#define	OSDM_ICONERROR		0x010
#define	OSDM_ICONQUESTION	0x020
#define	OSDM_ICONWARNING	0x030
#define	OSDM_ICONINFO		0x040



#define	OSDR_OK				0x00
#define	OSDR_CANCEL			0x01
#define	OSDR_YES			0x02
#define	OSDR_NO				0x03


// LANGUAGE
#define  OSDL_JP		1		// JAPANESE
#define  OSDL_EN		2		// ENGLISH
#define  OSDL_OT		100		// OTHER

enum FileMode { FM_Load, FM_Save};

#endif
