/** iP6: PC-6000/6600 series emualtor ************************/
/**                                                         **/
/**                           Win32stick.h                  **/
/** by Windy 2002-2004                                      **/
/*************************************************************/
#ifndef _WIN32STICK_H
#define _WIN32STICK_H

#include "../types.h"

int JoysticOpen(void);				//          JoysticOpen: JOYSTICK を初期化
byte JoystickGetState(int joy_no);	//          JoysticGetState: JOYSTICK の状態を取得
int NumJoysticks(void);             //          NumJoysticks: JOYSTICK の数を取得
int JoysticClose(void);				//          JoystickClose: JOYSTICK の後始末

#endif
