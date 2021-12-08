// ******************************************************
// 			  buffer.h  
//            by Windy
// *******************************************************
#ifndef _BUFFER_H
#define _BUFFER_H

#include "types.h"

//#ifdef WIN32
//#include <windows.h>
//#else
//#define DWORD int
//#endif


// ****************************************************************************
//          ringbuffer: 音源用　リングバッファ
// ****************************************************************************
//  short  buffer[SOUND_BUFSIZE*101];
typedef struct _ringbuff
{
  int   read_idx;		// 読み込みインデックス
  int   write_idx;		// 書き込みインデックス
  int   length;			// 外から見たバッファの長さ
  int   datas;			// 格納されているデータ数
  int   in_length;		// 内部的なバッファの長さ
  short *buffer;		// バッファ
} RINGBUFFER;


RINGBUFFER *ringbuffer_Open(int size);
int  ringbuffer_Read( RINGBUFFER *buf , short *buffer,int size);
int  ringbuffer_Write(RINGBUFFER *buf , short *buffer,int size);
void ringbuffer_Close(RINGBUFFER *buf);

int ringbuffer_Get( RINGBUFFER *buff ,short *dat);
int ringbuffer_Put(RINGBUFFER *buff ,short dat);
int ringbuffer_DataNum( RINGBUFFER *buff );
int ringbuffer_FreeNum( RINGBUFFER *buff );


// ****************************************************************************
//          ringbuffer: キー入力用　リングバッファ
// ****************************************************************************

typedef struct keybuffer_tag {
  int   read_idx;
  int   write_idx;
  int   length;

  char  chr   [1024];

  int   keydown[1024];		// 1: keydown  0: keyup
  int   scancode[1024];		// scan code
  int   osdkeycode[1024];	// osdkeycode
} KEYBUFFER;

KEYBUFFER*  init_keybuffer(void);
void clear_keybuffer(KEYBUFFER* _keybuffer);
void write_keybuffer( KEYBUFFER* keybuffer,char chr ,int keydown , int scancode  ,int osdkeycode );
int  read_keybuffer ( KEYBUFFER* keybuffer,char *chr ,int *keydown ,int *scancode ,int *osdkeycode );
int sense_keybuffer( KEYBUFFER *keybuffer ,char *chr ,int *keydown ,int *scancode ,int *osdkeycode);

void close_keybuffer( KEYBUFFER *_keybuffer);


#endif
