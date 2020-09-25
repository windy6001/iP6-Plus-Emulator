/** iP6: PC-6000/6600 series emualtor ************************/
/**                                                         **/
/**                     fm.cpp                              **/
/**               fmgen interface                           **/
/** by Windy 2002                                           **/
/*************************************************************/
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#define  NDEBUG
#include <stdio.h>
#include <assert.h>
#include <memory.h>
#include "Sound.h"
#include "fmgen/headers.h"
#include "fmgen/opna.h"
#include "fm.h"
#include "error.h"

#include "P6.h"
#include "dokodemo.h"

FM::OPN opn;

//int fm_vol;			// fm volume
//int psg_vol;			// psg volume



// ****************************************************************************
//	YM-2203 init
// ****************************************************************************
int ym2203_Open(void)
{
// if( !opn.Init( 3578545, sound_rate,1)) {
 if( !opn.Init( 3993600, sound_rate,1)) {
 	printf("opn initialize error \n");
 	return(0);
 	}

 ym2203_Reset();
 //ym2203_SetVolumePSG( 14);
 //ym2203_SetVolumeFM( 14);

 return(1);
}


// ****************************************************************************
//			YM-2203 reset
// ****************************************************************************
void ym2203_Reset(void)
{
 opn.Reset();
}


// ****************************************************************************
//			ym2203 set Volume PSG
// ****************************************************************************
void ym2203_SetVolumePSG(int v)
{
 int vol;
 if( v < 0 ) v =0;
 if( v >20)  v =20;

 vol = v * SCALE_VOL;
 vol = vol + BASE_VOL;
 //psg_vol = vol;

 opn.SetVolumePSG( vol );
}


// ****************************************************************************
//			ym2203 get Volume PSG
// ****************************************************************************
/*
int  ym2203_GetVolumePSG(void)
{
 return (psg_vol- BASE_VOL)/ SCALE_VOL;
}
*/

// ****************************************************************************
//			ym2203 set Volume FM
// ****************************************************************************
void ym2203_SetVolumeFM(int vol)
{
 if( vol < 0 ) vol=0;
 if( vol >20 ) vol=20;

 vol = vol * SCALE_VOL;
 vol = vol + BASE_VOL;
 //fm_vol = vol;

 opn.SetVolumeFM( vol );
}


// ****************************************************************************
//			ym2203 get Volume FM
// ****************************************************************************
/*int  ym2203_GetVolumeFM(void)
{
 return (fm_vol- BASE_VOL)/ SCALE_VOL;
}
*/

// ****************************************************************************
//	YM-2203 setreg
//    In: r = register   v= value
// ****************************************************************************
void ym2203_setreg(int r,int v)
{
 //printf("r=%2X ,v=%2X \n",r,v);
 opn.SetReg( r,v);
}

// ****************************************************************************
//	YM-2203 get register
//   In:  register
//   Out: register's value
// ****************************************************************************
int ym2203_getreg(int r)
{
 int v;
 v= opn.GetReg( r);
 return( v);
}

extern "C" {
int testPutData(char *src,int length);
}

// ****************************************************************************
//	YM-2203 makewave
// In: dest: buffer
//     size: バイト数 
// ****************************************************************************
void ym2203_makewave( short *dest ,int size)
{
 FM::Sample *samplebuff;

 if( size <= 0) {PRINTDEBUG1(SND_LOG,"[fm.cpp][ym2203_makewave] size=%d \n",size);return ;}  // size <0  : do nothing

 size*=2;		// stereo からmonoral に変換するので、2倍のデータを作る
 
 samplebuff = (FM::Sample*) malloc( size *sizeof(FM::Sample));
 if( samplebuff != NULL)
	{
	 // int i;
	  memset(  samplebuff , 0, size*sizeof(FM::Sample));	// clear memory
	  opn.Mix( samplebuff , size / sizeof( FM::Sample));	// mixing sound 
	 // opn.Count( 500000);	
	  downcast16( dest , samplebuff , size/sizeof(short));	// 32 bit -> 16bit 
	  free( samplebuff);
	  
	  PRINTDEBUG1(SND_LOG,"[fm.cpp][ym2203_makewave] make sample \t  samples=%d \n",size/sizeof(short)/2);
	  //if( SND_LOG) {for(i=0; i< size/ sizeof(short); i++) printf("%04X ",(*(dest+i) &0xffff)); printf("\n");}
	 //if( SND_LOG) 	testPutData((char*)dest, size* sizeof(short));

	}
 else
	{		// memory allocate failed !!
	printf("[fm.cpp][ym2203_makewave] malloc failed !! \n");
	return ;
	}
}

// ****************************************************************************
//	fmgen down cast  32bit --> 16bit
// ****************************************************************************
void downcast16( short *dest , int *src ,int length)
{
 short *p;
 int i;
 
 if( CHANNEL==1) length/=2;		 // channel =1 , left sound only 2003/1/2
 
 p=dest;
 for(i=0 ; i< length  ; i++)
 	{
 	 *p= ((short)LIMITS( *src , -32768 , 32767));

#if (CHANNEL==2)
	 src++;
#else
	 src+=2;
#endif
	//	 src+= (CHANNEL==2)? 1:2;	// channel =1 , left sound only 2003/1/2
 	 p++;

 	}
}


// ****************************************************************************
//          ym2203_Count : fmgen カウント処理
// ****************************************************************************
int ym2203_Count(int us)
{
 return( opn.Count( us));
}

// ****************************************************************************
//          ym2203_GetNextEvent : fmgen 次にタイマーが発生するまでの時間を求める
// ****************************************************************************
int ym2203_GetNextEvent(void)
{
 return( opn.GetNextEvent());
}





// ****************************************************************************
//          ym2203_ReadStatus: YM2203 のステータスを読む
//
// Out: YM2203 のステータス　(TimerA とTimerBのオーバーフローフラグ・BUSYフラグも。)
// ****************************************************************************
#define FM_BUSY_COUNT 3
int ym2203_ReadStatus(void)
{
 static int cnt= FM_BUSY_COUNT; 	/* BUSY flag counter  */
 int ret;
 ret= opn.ReadStatus();


 cnt--;			/* 擬似的に、BUSYフラグを作ってみる。add 2003/10/26 */
 if( cnt >0)
 	ret |= 0x80;	/* BUSY */
 else
 	{
    cnt  = FM_BUSY_COUNT; 
 	ret &= ~0x80;	/* READY */
    }
 return( ret);
}

