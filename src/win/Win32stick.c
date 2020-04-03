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
//          ïœêî
// ****************************************************************************
static JOYINFOEX joy;
static JOYCAPS   joycaps;

// ****************************************************************************
//          JoysticOpen: JOYSTICK Çèâä˙âª
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
//          JoysticGetState: JOYSTICK ÇÃèÛë‘ÇéÊìæ
//  Out: joystick flags
// ****************************************************************************
byte JoystickGetState(int joy_no)
{
	int stat;
	byte ret;
 	int x,y;
    //int x_center,y_center;
    int x_len, y_len;
    
    ret=0;
	joy.dwSize = sizeof( joy);
	joy.dwFlags= JOY_RETURNALL;
    
    joy_no= (joy_no==0)? JOYSTICKID1: JOYSTICKID2;
	if( (stat = joyGetPosEx(   joy_no, &joy))==JOYERR_NOERROR) {
        joyGetDevCaps( joy_no, &joycaps,sizeof(JOYCAPS));	// get joy stick info
		x= joy.dwXpos;
		y= joy.dwYpos;
        x_len= (joycaps.wXmax- joycaps.wXmin);
        y_len= (joycaps.wYmax- joycaps.wYmin);
        //x_center = (joycaps.wXmin+ joycaps.wXmax)/2;
        //y_center = (joycaps.wYmin+ joycaps.wYmax)/2;

#ifdef DEBUG
        printf("X=%5d  Y=%5d  BUTTON=%4d   ",x,y, joy.dwButtons);
        printf("X_LEN=%5d  Y_LEN=%5d  \n",x_len,y_len);
#endif

		// *********** ç¿ïWïœä∑  ************
		if( x < (x_len*1/10))      ret |=4; 
        else if( x > (x_len*9/10)) ret |=8;

		if( y < (y_len*1/10))      ret |=1;
        else if( y > (y_len*9/10)) ret |=2;

        if( joy.dwButtons & 7) ret|= 0x10;
	}
 else
    {
     //PRINTDEBUG1( "joyGetPosEx: Failed  %d\n",stat);
    }
 return(ret);
}

// ****************************************************************************
//          NumJoysticks: JOYSTICK ÇÃêîÇéÊìæ
// ****************************************************************************
int NumJoysticks(void)
{
 return 1;
}

// ****************************************************************************
//          JoystickClose: JOYSTICK ÇÃå„énññ
// ****************************************************************************
int JoysticClose(void)
{
 return 1;
}

#endif // WIN32
