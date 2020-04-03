/** iP6: PC-6000/6600 series emualtor ************************/
/**                                                         **/
/**                           Sound.h                       **/
/** by Windy 2002                                           **/
/** by ISHIOKA Hiroshi 1998-2000                            **/
/** This code is based on fMSX written by Marat Fayzullin   **/
/*************************************************************/


#define USEWAVEOUT
#define USECREATE
#define USERING

//#define VOICE


#ifdef __cplusplus
extern "C" {
#endif
// ****************************************************************************
//             	      DEFINE
// ****************************************************************************
/*
** Output rate (Hz)
*/

#ifndef SOUND_RATE
#define SOUND_RATE 22050	// default sound rate
#endif  //SOUND_RATE

#define FREQ 2052654284		// 
#define CHANNEL         1   // sound channel  1:mono  2:streao
#define SAMPLING_BITS  16	// sampling bit
//#define SOUND_BUFSIZE  (367*2) // 220 // 256 // (512)  /* 1024*2 だと遅延が・・。*//* 以前は、(256)でした。 */

#define MAX_SOUND_BUFSIZE   4096  *2




/*
** Buffer size. SOUND_BUFSIZE should be equal to (or at least less than)
** 2 ^ SOUND_BUFSIZE_BITS
*/
/*
** Number of audio buffers to use. Must be at least 2, but bigger value
** results in better behaviour on loaded systems (but output gets more
** delayed)
*/


/* *************** UNIX ********************* */
#ifdef  UNIX
#define SOUND_BUFSIZE_BITS 8    // 8  8 だと、FreeBSD で音が鳴らない？
#define SOUND_NUM_OF_BUFS 8  // 2
#define SOUND_BUFSIZE     256
#endif



#ifdef  SDL
#undef  SOUND_BUFSIZE
#define SOUND_BUFSIZE   (1024*2)
#endif


#ifdef WIN32
#include "win/WinSound.h"
#endif


/* 変換する */
#define LIMITS( val , min, max)  ((val < min)? min: (val >max)? max : val)


// ****************************************************************************
//             	      PUBLIC
// ****************************************************************************
#include "buffer.h"

/*extern RINGBUFFER *audiobuffer;
extern RINGBUFFER *voicebuffer; */


#define AUDIOBUFFER      0
#define VOICEBUFFER      1
#define MAX_STREAMBUFFER 2
extern RINGBUFFER *streambuffer[ MAX_STREAMBUFFER ];


int InitSound(void);
void TrashSound(void);
void FlushSound(void);
void StopSound(void);
void ResumeSound(void);
void play_sub(short *src,int len);
void SoundOut(byte R,byte V);
extern void fmsound_update(int samples);
void SoundUpdate(void);

    
extern int UseSound;
extern int newUseSound;
extern int sound_rate;
extern int sound_samples;
//extern  int   keyclick;				// use key click sound 1: use

/*
typedef struct {
	char *name;
	void SCALL *OpenProc;
	void SCALL *playProc;
	void SCALL *pauseProc;
	void SCALL *CloseProc;
} SOUND_PLUGIN;
*/
#ifdef __cplusplus
}
#endif

