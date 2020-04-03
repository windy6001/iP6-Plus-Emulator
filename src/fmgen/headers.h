#ifndef WIN_HEADERS_H
#define WIN_HEADERS_H

#ifdef WIN32
#define STRICT
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#define  STDCALL __stdcall
#endif

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <assert.h>
#include "types.h"

#ifdef _MSC_VER
	#undef max
	#define max _MAX
	#undef min
	#define min _MIN
#endif
#ifndef WIN32
#	define HANDLE int
#endif
#ifndef MAX_PATH
#	define MAX_PATH FILENAME_MAX
#endif

#endif	// WIN_HEADERS_H
