/** iP6: PC-6000/6600 series emualtor ************************/
/**                                                         **/
/**                           Unix.h                        **/
/**                                                         **/
/** Modified by Windy 2002-2003                             **/
/** This code is written by ISHIOKA Hiroshi 1998-2000       **/
/** This code is based on fMSX written by Marat Fayzullin   **/
/*************************************************************/

#ifdef X11
#include <X11/Xlib.h>
#endif

//#include "../Refresh.h"
//#include "../Video.h"

void SLEEP(int s);

void choosefuncs(int lsbfirst, int bitpix);
void setwidth(int wide);
void PutImage(void);

void ClearScr(void);
int resize(int w,int h);		// resize window 2002/4/29

//ColTyp OSD_Getcolor(int R ,int G ,int B ,int H);
byte * GetXtab(void);


int unix_soundOpen(void );
void unix_soundClose(void);
int OSD_SelectMachine(void);

void OSD_SetModulePath(void);
void OSD_OpenFiler(char *path);
