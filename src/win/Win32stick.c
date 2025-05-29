/** iP6: PC-6000/6600 series emualtor ************************/
/**                                                         **/
/**                           Win32stick.c                  **/
/** by Windy 2003                                           **/
/*************************************************************/

#ifdef WIN32

#include <windows.h>
#include <mmsystem.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Win32.h"
#include "../error.h"

extern HWND hwndMain;

// ****************************************************************************
//          変数
// ****************************************************************************
static JOYINFOEX joy;
static JOYCAPS   joycaps;

// ****************************************************************************
//          JoysticOpen: JOYSTICK を初期化
// ****************************************************************************
int JoysticOpen(void)
{
	int ret=-1;
	//joy.dwSize = sizeof( joy);
	//joy.dwFlags= JOY_RETURNALL;
    
	 if( joySetCapture( hwndMain, JOYSTICKID1,1,0)==JOYERR_NOERROR)
		{
		 joyReleaseCapture( JOYSTICKID1);
		 ret=1;
		}
	else
		{
		 ret=0;
		}
 return(ret);
}

// ****************************************************************************
//          JoysticGetState: JOYSTICK の状態を取得
//  Out: joystick flags
// ****************************************************************************
byte JoystickGetState(int joy_no)
{
    int stat;
    byte ret;
    int x, y;
    //int x_center,y_center;
    int x_len, y_len;

    ret = 0;
    joy.dwSize = sizeof(joy);
    joy.dwFlags = JOY_RETURNALL;

    joy_no = (joy_no == 0) ? JOYSTICKID1 : JOYSTICKID2;
    if ((stat = joyGetPosEx(joy_no, &joy)) == JOYERR_NOERROR) {
        joyGetDevCaps(joy_no, &joycaps, sizeof(JOYCAPS));	// get joy stick info
        
        switch( joy.dwPOV ) {       // 十字キーに対応
            case 0:    ret = 1; break;
            case 4500: ret = 9; break;
            case 9000: ret = 8; break;
            case 13500:ret = 10; break;
            case 18000:ret = 2; break;
            case 22500:ret = 6; break;
            case 27000:ret = 4; break;
            case 31500:ret = 5; break;
        }
        if (joy.dwButtons ) {    // トリガー
            ret |= 0x10;
            }
    }else{
        //PRINTDEBUG1( "joyGetPosEx: Failed  %d\n",stat);
    }
    return(ret);
}



// ****************************************************************************
//          NumJoysticks: JOYSTICK の数を取得
// ****************************************************************************
int NumJoysticks(void)
{
 return 1;
}

// ****************************************************************************
//          JoystickClose: JOYSTICK の後始末
// ****************************************************************************
int JoysticClose(void)
{
 return 1;
}

#endif // WIN32
