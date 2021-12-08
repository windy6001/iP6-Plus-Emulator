/** iP6: PC-6000/6600 series emualtor ************************/
/**                                                         **/
/**                           iP6.c                         **/
/**                                                         **/
/** by Windy  2002-2004                                     **/
/** by ISHIOKA Hiroshi 1998,1999                            **/
/** This code is based on fMSX written by Marat Fayzullin   **/
/*************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include "os.h"
#include "P6.h"
#include "Option.h"
#include "buffer.h"		// for init_keybuffer
#include "Sound.h"

#ifdef X11
#include "unix/Xconf.h"
#endif


#ifdef __IPHONE_OS_VERSION_MIN_REQUIRED
#define main OSD_main
#endif

int main(int argc,char *argv[])
{
	char buff[PATH_MAX];
	OSD_SetModulePath();
	OSD_GetModulePath(buff ,PATH_MAX);
	printf(" Starting iP6 Plus.... \n Current Directory is '%s'\n", buff);
	
	InitVariable();
    //init_keybuffer();	// <--- init key buffer
	OSD_initKeymap();				// init keymap


    ConfigInit();				// <--- config init
    ConfigRead();				// <--- config read 
	if(!chkOption( argc, argv)) return(0);	// <--- option check

#ifdef X11
	Xt_init(&argc,&argv);
#endif
	ConfigWrite();				// <--- config write (一度指定されたオプションを覚える)

	{
	if(!InitMachine()) return(1);
	if (!StartP6())
		{
		CPURunning = 0; 
		}
    InitSound();
	CpuRun();
 	
	TrashMachine();
    TrashP6();
	}
  return(0);
}



