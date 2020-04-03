/** iP6: PC-6000/6600 series emualtor ************************/
/**                                                         **/
/**                         dokodemo.h                      **/
/**                                                         **/
/** by Windy 2010                                           **/
/*************************************************************/

#include "types.h"
#define MAX_DOKODEMO_KEY    20
#define MAX_DOKODEMO_VALUE  100
#define MAX_DOKODEMO_LINE 5000
#define DOKODEMO_VERSION "1.0"


#define DOKODEMO_STEP 32

int dokodemo_save(char *path);
int dokodemo_load(char *path);
int dokodemo_getentry( char *key , char *value);
int dokodemo_getentry_int( char *key , int *value);
int dokodemo_getentry_byte( char *key , byte *value);
int dokodemo_getentry_word( char *key , word *value);
int dokodemo_getentry_buffer( char *key , char *value ,int size);


int dokodemo_putentry( char *key , char *value);
int dokodemo_putentry_int( char *key , int value);
int dokodemo_putentry_byte( char *key , byte value);
int dokodemo_putentry_word( char *key , word value);
int dokodemo_putentry_buffer(char *key, char *value ,int size);
int dokodemo_putsection( char *value);

int dokodemo_loadentry( char *key, char *value);


#define DOKODEMO_PUTENTRY_INT(x)    dokodemo_putentry_int( #x,x)
#define DOKODEMO_PUTENTRY_BYTE(x)   dokodemo_putentry_byte( #x,x)
#define DOKODEMO_PUTENTRY_WORD(x)   dokodemo_putentry_word( #x,x)

#define DOKODEMO_GETENTRY_INT(x)    dokodemo_getentry_int( #x,(int*)&x);
#define DOKODEMO_GETENTRY_BYTE(x)   dokodemo_getentry_byte( #x,(byte*)&x);
#define DOKODEMO_GETENTRY_WORD(x)    dokodemo_getentry_word( #x,(word*)&x);
