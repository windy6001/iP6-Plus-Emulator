/** iP6: PC-6000/6600 series emualtor ************************/
/**                                                         **/
/**                           error.c                       **/
/** by Windy 2002-2003                                      **/
/*************************************************************/
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include "error.h"
#include "os.h"



int debug_level=1;
void outputdebugstring(char *buff);


// ****************************************************************************
//          debug_printf: デバッグプリント
// ****************************************************************************
int debug_printf( char *fmt ,...)
{
	int  ret;
	static char buff[5000];
	va_list ap;

	if( debug_level==0) return(0);

	va_start( ap ,fmt);
	ret = vsprintf( buff  , fmt , ap);
	va_end( ap);

   	fprintf(stdout,"%s",buff);
	outputdebugstring( buff);

	return(ret);
}

