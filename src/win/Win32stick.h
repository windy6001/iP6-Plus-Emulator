/** iP6: PC-6000/6600 series emualtor ************************/
/**                                                         **/
/**                           Win32stick.h                  **/
/** by Windy 2002-2004                                      **/
/*************************************************************/
#ifndef _WIN32STICK_H
#define _WIN32STICK_H

#include "../types.h"

int JoysticOpen(void);				//          JoysticOpen: JOYSTICK ‚ğ‰Šú‰»
byte JoystickGetState(int joy_no);	//          JoysticGetState: JOYSTICK ‚Ìó‘Ô‚ğæ“¾
int NumJoysticks(void);             //          NumJoysticks: JOYSTICK ‚Ì”‚ğæ“¾
int JoysticClose(void);				//          JoystickClose: JOYSTICK ‚ÌŒãn––

#endif
