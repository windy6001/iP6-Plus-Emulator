// ******************************************************
// 			  buffer.c  
// by Windy 2002-2003
// *******************************************************
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "buffer.h"




// ****************************************************************************
//          ringbuffer_Open: リングバッファの初期化
//  IN: size= ring buffer の数
// ****************************************************************************
RINGBUFFER* ringbuffer_Open(int size)
{
	RINGBUFFER *buff;
	
	buff = (RINGBUFFER*) malloc( sizeof(RINGBUFFER));	/* RINGBUFFER 構造体を作成 */
	if( buff ==NULL) {return NULL;}


    buff->read_idx=0;
	buff->write_idx=0;
    buff->length= size;
	buff->in_length = size*50; 
    buff->datas =0;
  
    buff->buffer = (short*)malloc( buff->in_length *sizeof(short) );  /* RING BUFFER のバッファを作成 */
    if( buff->buffer !=NULL)
        {
        memset( buff->buffer,0, buff->in_length *sizeof(short) );
        }
    else
        buff = NULL;
    return( buff);
  //printf("init_soundbuff() \n");
}

// ****************************************************************************
//          ringbuffer_Close: リングバッファの後始末
// ****************************************************************************
void ringbuffer_Close(RINGBUFFER *buff)
{
    if( buff !=NULL)
        {
        free( buff->buffer );
		free( buff);
		buff = NULL;
        }
}


// ****************************************************************************
//           ringbuffer_Get: リングバッファの読み込み（1データ）
//  IN:   buffer:バッファ
//    :   read data
//  OUT: 1 = success
//       0 = failed
// ****************************************************************************
int ringbuffer_Get( RINGBUFFER *buff ,short *dat)
{
	int ret;
    if( buff->datas >0)
      	{
   		 *dat = buff->buffer[ buff->read_idx];
 
  		 if( ++buff->read_idx > buff->in_length -1)
   			{
   			 buff->read_idx=0;
   			}
 		 buff->datas--;
		 ret=1;
		}
	else
		{
		ret =0;
		*dat =0;
		//printf("ringbuffer_Get: data empty... \n");
		}
	return ret;
}


// ****************************************************************************
//          ringbuffer_Put: リングバッファへの書き込み (1データ）
//  IN: :  buffer:バッファ
//         dat:データ
//  OUT: 1 = success
//       0 = failed
// ****************************************************************************
int ringbuffer_Put(RINGBUFFER *buff ,short dat)
{
	int ret=0;
	
    //if( buff ==NULL ) {printf("not initialize ring buffer"); return 0;}
	//if( buff->buffer ==NULL) {printf("not initialize ring buffer"); return 0;}

    if( buff->datas < buff->in_length -1)
      	{
   	     buff->buffer[ buff->write_idx] = dat;
   	     if( ++buff->write_idx > buff->in_length-1)
   			{
   			 buff->write_idx=0;
   			}
   		 buff->datas++;
		 ret =1;
       	}
	return(ret);
}


// ****************************************************************************
//           ringbuffer_DataNum: リングバッファの中にあるデータ数
//  IN:   buffer:バッファ
//  OUT:  buffer number
// ****************************************************************************
int ringbuffer_DataNum( RINGBUFFER *buff )
{
//	 return( ((buff->datas < buff->length) ? buff->datas : buff->length) );
	return( buff->datas);
}


// ****************************************************************************
//           ringbuffer_FreeNum: リングバッファの残りサンプル数
//  IN:   buffer:バッファ
// 
//  OUT:  buffer number
// ****************************************************************************
int ringbuffer_FreeNum( RINGBUFFER *buff )
{
    int ret;
    ret = buff->length - buff->datas;
   if( ret <0) 
       { printf("[buffer.c][ringbuffer_FreeNum] ret=%d  buff->length=%d ,buff->datas=%d \n",ret , buff->length , buff->datas); ret=0;}
	return ret;
}



// ****************************************************************************
//           ringbuffer_Read: リングバッファの読み込み
//  IN: buffer:バッファ
//      size:  バイト数
//  Out: non zero: read data numbers     0 :no data
// ****************************************************************************
int ringbuffer_Read(RINGBUFFER *buff ,short *buffer,int size)
{
 int ret;
 int datas;
 ret=0;

// assert( ringbuffer.datas >=0 && ringbuffer.datas < ringbuffer.length-1);
    if( buff ==NULL ) {printf("not initialize ring buffer"); return 0;}
	if( buff->buffer ==NULL) {printf("not initialize ring buffer"); return 0;}

	//printf("[ringbuffer_Read]  buff->datas=%d  size=%d \n",buff->datas ,size);
	 
	//datas = (int) (size / sizeof(short));
	datas=size;

#if 0						/* 1つづつ読み込む */
    if( buff->datas >0)
      	{
         int i;
         for(i=0;i< datas  ;i++)
         	{
			 //printf("R: %d   Datas: %d \n",buff->read_idx , buff->datas);
			 
    		 *(buffer+i) = buff->buffer[ buff->read_idx];
    		 if( ++buff->read_idx > buff->in_length)
    			{
    			 buff->read_idx=0;
    			}
    		 buff->datas--;
			 if( buff->datas<0) { buff->datas=0; break;}
             ret++;
            }
		}
	 else
		 printf("[ringbuffer_Read] no data for reading %d\n",buff->datas);

#else							/* 一気に読み込む */
	if( datas <= buff->datas)
		{
			/* 一度にコピーできる場合 */
		if( buff->read_idx + datas < buff->in_length)
			{
			 memcpy( buffer , &buff->buffer[ buff->read_idx] , datas * sizeof(short));
			}
		else	/* 二つに分かれている場合 */
			{
			 int size1,size2;				/* size1 right data    size2 left data*/
			 size1 = buff->in_length - buff->read_idx+1;		// don't forget +1   Windy 2009/10/4
			 size2 = datas        - size1 +1;	

			 memcpy( buffer        , buff->buffer + buff->read_idx  , size1*sizeof(short));
			 memcpy( buffer+size1  , buff->buffer                   , size2*sizeof(short));
			}
		buff->datas    -= datas;
		buff->read_idx += datas;
		if( buff->datas < 0 )               buff->datas = 0;
		if( buff->read_idx > buff->length)  buff->read_idx -= buff->in_length; 
	    }
#endif
	 printf("[ringbuffer_Read] read data=%d   zan datas==%d\n",ret ,buff->datas);
     
 return(ret);
}


// ****************************************************************************
//          ringbuffer_Write: リングバッファへの書き込み
//  IN: buffer:バッファ
//      size:  バイト数
//  OUT: 1 = success
//       0 = failed
// ****************************************************************************
int ringbuffer_Write(RINGBUFFER *buff ,short *buffer,int size)
{
	int ret=0;
	int datas;
	
    if( buff ==NULL ) {printf("not initialize ring buffer"); return 0;}
	if( buff->buffer ==NULL) {printf("not initialize ring buffer"); return 0;}

	//datas = (short) size / sizeof(short);
	datas=size;


#if 0			/* 1つづつ書き込む */
    if( buff->datas +datas < buff->in_length)
      	{
    	 unsigned int i;
         for(i=0; i< datas/sizeof(short); i++)
         	{
    	     buff->buffer[ buff->write_idx] = *(buffer+i);
    	     if( ++buff->write_idx > buff->in_length)
    			{
    			 buff->write_idx=0;
    			}
    		buff->datas++;
          	}
		 ret =1;
    	}
#else			/* 一気に書き込む */

	if( buff->datas + datas < buff->in_length ) 	// バッファが一杯？
		{
		if( buff->write_idx < buff->read_idx &&
		    buff->write_idx +datas > buff->read_idx )   // WがRを追い越す場合、何もしない
					printf("****** buffer.c   W  R ******************* \n");

							/* 一度に書き込める場合 */
	 	if( buff->write_idx +datas <  buff->in_length)
			{
			memcpy( buff->buffer+ buff->write_idx , buffer , datas*sizeof(short));
			}
		else			    /* 二つに分かれている場合 */
			{
			int size1,size2;				/* size1 right data    size2 left data*/
			 size1 = buff->in_length - buff->write_idx+1;		// don't forget +1
			 size2 = datas        - size1+1;							// don't forget +1

			 memcpy( buff->buffer+buff->write_idx  , buffer         , size1*sizeof(short));
			 memcpy( buff->buffer                  , buffer+size1   , size2*sizeof(short));
			}
		 
		buff->datas     += datas;
		buff->write_idx += datas;
		if( buff->datas < 0 )                buff->datas = 0;
		if( buff->write_idx > buff->in_length)  buff->write_idx -= buff->in_length; 
		ret=1;
		}
#endif
	//printf("[ringbuffer_Write]  written data=%d  zan data=%d \n", ret , buff->datas);
	return(ret);
}





#if 0
int main(void)
{
	int i,j;
	RINGBUFFER* ringbuffer;
	short inbuffer[512];
	short outbuffer[512];
	short *p;
	
	for(i=0; i<512; i++) inbuffer[i]= i;
	
	ringbuffer = ringbuffer_Open(25);
	ringbuffer_Write( ringbuffer ,inbuffer ,50*sizeof(short));
	p = inbuffer;
	
	for(j=0; j<30; j++)
		{
		 memset( outbuffer , 0, 512);
		 ringbuffer_Write( ringbuffer ,p         ,10);
		 ringbuffer_Read ( ringbuffer ,outbuffer ,10);
		 p+=10;
		 for(i=0; i<10;i++) 
		 	printf("%3d ",outbuffer[i]);
		 printf("\n");
		}
	ringbuffer_Close( ringbuffer);
}
#endif



// ****************************************************************************
//           key buffer (ring buffer)
// ****************************************************************************


KEYBUFFER* init_keybuffer(void)
{
	KEYBUFFER *_keybuffer = malloc( sizeof(KEYBUFFER) );
	
	_keybuffer->read_idx=0;
	_keybuffer->write_idx=0;
	_keybuffer->length=1024-1;
	
	return _keybuffer;
}

void write_keybuffer(KEYBUFFER *_keybuffer , char chr ,int keydown , int scancode ,int osdkeycode)
{
  if( _keybuffer ==NULL) return;

  _keybuffer->chr       [ _keybuffer->write_idx]= chr;
  _keybuffer->keydown   [ _keybuffer->write_idx]= keydown;
  _keybuffer->scancode  [ _keybuffer->write_idx]= scancode;
  _keybuffer->osdkeycode[ _keybuffer->write_idx]= osdkeycode;
  if( ++_keybuffer->write_idx >_keybuffer->length)
	{
	 _keybuffer->write_idx=0;
	}
}


int sense_keybuffer( KEYBUFFER *_keybuffer ,char *chr ,int *keydown ,int *scancode ,int *osdkeycode)
{
  int ret=0;
  if( _keybuffer->read_idx != _keybuffer->write_idx)
  	{
	 if(chr   ) *chr    = _keybuffer->chr   [ _keybuffer->read_idx];

	 if(keydown) *keydown    = _keybuffer->keydown       [ _keybuffer->read_idx];
	 if(scancode) *scancode  = _keybuffer->scancode      [ _keybuffer->read_idx];
	 if(osdkeycode) *osdkeycode = _keybuffer->osdkeycode [ _keybuffer->read_idx];
	 ret=1;
	}
  return ret;
}


int read_keybuffer( KEYBUFFER *_keybuffer ,char *chr ,int *keydown ,int *scancode ,int *osdkeycode)
{
 int ret;
 ret=0;
  if( _keybuffer->read_idx != _keybuffer->write_idx)
  	{
	 sense_keybuffer( _keybuffer , chr , keydown , scancode , osdkeycode);
	 if( ++_keybuffer->read_idx >_keybuffer->length)
		{
		 _keybuffer->read_idx=0;
		}
	 ret=1;
	}
 return(ret);
}


void close_keybuffer( KEYBUFFER *_keybuffer)
{
	free( _keybuffer);
}



// ****************************************************************************
//           tape buffer (ring buffer)
// ****************************************************************************
#if 0
struct _tapebuffer {			// 2002/11/5
	int read_idx;
	int write_idx;
	int length;
	byte buffer[65536];
} tapebuffer;

void init_tapebuffer(void)
{
 tapebuffer.read_idx =0;
 tapebuffer.write_idx=0;
 tapebuffer.length   =sizeof( tapebuffer.buffer)-1;
}

void write_tapebuffer( byte Value)
{
 tapebuffer.buffer[ tapebuffer.write_idx ] = Value;
 if( ++tapebuffer.write_idx > tapebuffer.length)
	{
	 tapebuffer.write_idx=0;
	}
}

int read_tapebuffer(byte *Value)
{
 int ret;
 ret=0;
 if( tapebuffer.read_idx != tapebuffer.write_idx)
	{
	 ret=1;
	 *Value = tapebuffer.buffer[ tapebuffer.read_idx ];
	 if( ++tapebuffer.read_idx > tapebuffer.length)
		{
		 tapebuffer.read_idx=0;
		}
	}
 return( ret);
}
#endif

