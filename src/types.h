#ifndef _TYPES_H
#define _TYPES_H

#include <limits.h>

#ifndef PATH_MAX
#define PATH_MAX 256
#endif


#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif


typedef unsigned char byte;
typedef unsigned short word;
typedef signed char offset;

typedef unsigned long  dword;


#ifndef max
#define max(a,b) ((a)>(b)?(a):(b))
#endif
#ifndef min
#define min(a,b) ((a)<(b)?(a):(b))
#endif 


#ifdef __BIG_ENDIAN__ 
#define WORDS_BIGENDIAN
#else
#undef  WORDS_BIGENDIAN
#endif


#endif // _TYPES_H
