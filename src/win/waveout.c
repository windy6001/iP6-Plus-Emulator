/** iP6: PC-6000/6600 series emualtor ************************/
/**                                                         **/
/**                         waveout.c                       **/
/**                                                         **/
/** by windy 2002-2004                                      **/
/*************************************************************/
/*
int  open_waveout(void );  サウンドドライバを開きます。

void close_waveout(void); 	サウンドドライバを閉じます。

void write_waveout(short *src,int len);  音源に書き込みをします。

int SCALL SoundMainLoop( LPVOID param);　サウンドの終了を検知して、音源に書き込みをするタイミングを取ります。

static void SoundUpdate(void);           ストリームをミキシングしながら、ストリームに書き込みをします。

void waveout_ResumeSound(void);　　      一時停止中の音源を再開します。

void waveout_StopSound(void);　          音源を一時停止します。

*/

#ifdef WIN32
#include <Windows.h>
#include <assert.h>
#include "waveout.h"
#include "../Sound.h"
#include "../SThread.h"
#include "../P6.h"
#include "../timer.h"
#include "../types.h"

#include "../sysdep/sysdep_dsp.h"
#include "../sysdep/sysdep_dsp_priv.h"
#include "../sysdep/plugin_manager.h"


// ****************************************************************************
//				Variables
// ****************************************************************************

MMRESULT     mmRes;					// result
HWAVEOUT     hwo;					// sound device handle
WAVEFORMATEX wfx;					// format setting
WAVEHDR      whdr[ WINSOUND_BUFFERS];		// data block
//HANDLE       hThread1;					// handle of thread

static short pcmBuffer[ WINSOUND_BUFFERS][ MAX_SOUND_BUFSIZE];  // waveout buffer
static int sidx;												// waveout buffer's index
static int pre_sidx;
//short soundbuf[MAX_SOUND_BUFSIZE];	// sound buffer
extern int      sound_samples;			// sound samples per 1/60 sec.

int is_sound_playing;

// ****************************************************************************
//				thread Variables
// ****************************************************************************

int SoundMainThreadRun;				// TRUE: thread run
SThread *SoundMainThread;			// thread object
extern SMutex    *soundMutex;		// sound mutex



// ****************************************************************************
//				Private Functions define
// ****************************************************************************
//static void SoundUpdate(void);

#ifdef USECREATE
int SoundMainLoop( LPVOID param);
#else
void _USERENTRY SoundMainLoop( void *param);
#endif



// **************************************************
//		waveout 音源を開く
//   In:
//   Out: TRUE: success  FALSE: failed
// **************************************************
int waveout_open(void)
{
	int i;

	is_sound_playing =FALSE;
	SoundMainThreadRun=1;	/* True: Thread run   False: Thread abort */
	//SoundSubThreadRun=1;	/* True: Thread run   False: Thread abort */

	// ------------ Create message recieve thread --------
#ifdef USECREATE
	SoundMainThread= (SThread*)OSD_CreateThread( SoundMainLoop,0);
	if( !SoundMainThread )
		{
		 printf("FAILED\n");
		 putlasterror();
		 return(0);
		}
#endif


 // -------------- open device --------------------------
 	if(Verbose) printf("OK\nOpening Sound device...");
    wfx.wFormatTag    = WAVE_FORMAT_PCM;
    wfx.nChannels     = CHANNEL;
    wfx.nSamplesPerSec= sound_rate;		// sampling rate freq.
    wfx.wBitsPerSample= SAMPLING_BITS;		// sampling bit
    wfx.nBlockAlign   = wfx.nChannels * wfx.wBitsPerSample / 8;
    wfx.nAvgBytesPerSec=wfx.nSamplesPerSec * wfx.nBlockAlign;
    wfx.cbSize = 0;

	mmRes = waveOutOpen( &hwo,WAVE_MAPPER, &wfx,(DWORD)(LPVOID)SoundMainThread->Id/*dwThreadId1*/
	 ,(DWORD)NULL, CALLBACK_THREAD );
	if( mmRes != MMSYSERR_NOERROR) 
		{
		//messagebox("wave device open error \n","");
	    return(0);
		}

	if(Verbose) puts("OK");

	Sleep(100);

 // -------------- pomping sound buffers  --------------------------
 	for(i=0 ; i< WINSOUND_BUFFERS; i++)
		{
		whdr[i].lpData = (char*)pcmBuffer[ i];
		whdr[i].dwBufferLength = sound_samples *sizeof(short);  // sizeof( soundbuf);
		memset(whdr[i].lpData, 0,sound_samples *sizeof(short)); // sizeof( soundbuf)
		whdr[i].dwBytesRecorded=0;
		whdr[i].dwUser=0;
		whdr[i].dwFlags=0;
		whdr[i].dwLoops=1;
		whdr[i].lpNext =NULL;
		whdr[i].reserved=0;

     	mmRes = waveOutUnprepareHeader( hwo, &whdr[i], sizeof(WAVEHDR));
	 	if( mmRes != MMSYSERR_NOERROR) {
			messagebox("waveOutUnPrepareHeader(): failed!!","");
	 	        return(0);
			}
	 	mmRes = waveOutPrepareHeader(hwo, &whdr[i],sizeof(WAVEHDR));
	 	if( mmRes != MMSYSERR_NOERROR) {
			messagebox("waveOutPrepareHeader(): failed!!\n","");
	 	        return(0);
			}
	 	mmRes = waveOutWrite(        hwo, &whdr[i],sizeof(WAVEHDR));
	 	if( mmRes != MMSYSERR_NOERROR) {
			messagebox("waveOutWrite(): failed!!","");
		        return(0);
			}
		}
	waveout_ResumeSound();
	return(1);
}

// **************************************************
//	   waveout 音源を閉じる
//    In:
//    Out:
// **************************************************
void waveout_dsp_destroy(void)
{
	int i;
	for(i=0; i< WINSOUND_BUFFERS; i++)
		    waveOutUnprepareHeader(hwo,&whdr[i],sizeof(WAVEHDR));
	waveOutClose( hwo);

	OSD_DestroyThread( SoundMainThread);	/* Destroy Thread Object */
	//OSD_DestroyThread( SoundSubThread);
	//OSD_DestroyThread( VoiceMainThread);

	SoundMainThreadRun=0;			/* thread aborting...*/
	//SoundSubThreadRun=0;
	//VoiceMainThreadRun=0;

	Sleep(1000);					/* wait several sec. */
	//CloseHandle( hWrite );

}


// **************************************************
//	   ストリームに　書き込む
// **************************************************
void waveout_dsp_write(struct sysdep_dsp_struct *dsp, short *src,int len)
{
	len *= sizeof( short);		/* samples -> bytes */
	//OSD_MutexLock( soundMutex);

	// ---------- unprepare buffer --------------
     mmRes= waveOutUnprepareHeader( hwo, &whdr[sidx], sizeof(WAVEHDR));
	 if( mmRes != MMSYSERR_NOERROR) {
			printf("waveOutUnPrepareHeader(): failed!!\n");
			}


//	whdr[sidx].lpData = pcmBuffer[ sidx];
	whdr[sidx].dwBufferLength = len;
	CopyMemory( whdr[sidx].lpData ,(char*)src, len);	// read data

	whdr[sidx].dwBytesRecorded=0;
	whdr[sidx].dwUser=0;
	whdr[sidx].dwFlags=0;
	whdr[sidx].dwLoops=1;
	whdr[sidx].lpNext =NULL;
	whdr[sidx].reserved=0;


	// ---------- prepare buffer --------------
	 mmRes = waveOutPrepareHeader(hwo, &whdr[sidx],sizeof(WAVEHDR));
	 if( mmRes != MMSYSERR_NOERROR) {
			messagebox("wave device PrepareHeader: failed!!","");
	 	    return;
			}
	// ---------- write sound device --------------
	 mmRes = waveOutWrite(        hwo, &whdr[sidx],sizeof(WAVEHDR));
	 if( mmRes != MMSYSERR_NOERROR) {
			messagebox("wave device Write failed!!","");
		    return;
			}
pre_sidx= sidx;
if( sidx < WINSOUND_BUFFERS-1)
    sidx++;
else
    sidx=0;

	//OSD_MutexUnlock( soundMutex);

}





// **************************************************
//		サウンドの終了を検知して、次のサウンドを書き込むタイミングを取ります。
// **************************************************
#ifdef USECREATE
int SCALL SoundMainLoop( LPVOID param)
#else
void _USERENTRY SoundMainLoop( void *param)
#endif
{
	MSG Msg;
	//HANDLE hRead;
	//int   nomsg;

	//dword TClock;

	sidx=0;		// buffer index 


 // OSD_InitTimer();
 //TClock = OSD_GetTicks();

  while( SoundMainThreadRun)
    {
								/* sound playing! */
		if(GetMessage(&Msg, 0,0,0)) 
			{
			switch( Msg.message)
				{
				case MM_WOM_CLOSE:		// sound end.
				  		return(0); 

				case MM_WOM_DONE:		// sound play done
					/*	while( ringbuffer_FreeNum( streambuffer[ AUDIOBUFFER]) !=0)		// 2010/9/25
							{
							} */
						SoundUpdate();		// write next sound
						break;
				default:/*PRINTDEBUG1(SND_LOG,"unknown message %d\n",Msg.message); OSD_Delay(1);*/ 
						break;
				}
			}
     }   	// while()
 //OSD_TrashTimer();
 return(0);
}




// **************************************************
//		一時停止した音源を　再開する
// **************************************************
void waveout_ResumeSound(void)
{
	is_sound_playing =TRUE;
	waveOutRestart( hwo);
}


// **************************************************
//		音源を一時停止する
// **************************************************
void waveout_StopSound(void)
{
	is_sound_playing =FALSE;
	waveOutPause( hwo);
}


// ****************************************************************************
//     waveout create
// ****************************************************************************
struct sysdep_dsp_struct * waveout_dsp_create(const void *flags)
{
	const struct sysdep_dsp_create_params *params = flags;
	struct sysdep_dsp_struct *dsp = NULL;
	struct coreaudio_private *priv = NULL;

	dsp= malloc( sizeof( struct sysdep_dsp_struct));
	if( dsp == NULL)
		{
		 printf("malloc failed \n");
		 return NULL;
		}
	if( !waveout_open())
		{
		 printf("waveout open failed \n");
		 return NULL;
		}

	dsp->write = waveout_dsp_write;
	dsp->destroy = waveout_dsp_destroy;
	dsp->resume =  waveout_ResumeSound;
	dsp->stop   =  waveout_StopSound;
	return dsp;
}

/*
 * The public variables structure
 */
const struct plugin_struct sysdep_dsp_waveout = {
	"waveout",
	"sysdep_dsp",
	"Windows waveout plugin",
	NULL, 				/* no options */
	NULL,				/* no init */
	NULL,				/* no exit */
	waveout_dsp_create,
	3,				/* high priority */
};



#endif

