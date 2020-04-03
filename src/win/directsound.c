/** iP6: PC-6000/6600 series emualtor ************************/
/**                                                         **/
/**                     directsound.c                       **/
/**                                                         **/
/** by windy 2002-2004                                      **/
/*************************************************************/
#if 0			// delete directsound.c  2009/7/28
#ifdef WIN32
#include <windows.h>
#include <dsound.h>
#include "..\P6.h"
#include "..\Sound.h"
#include "..\SThread.h"
#include "..\fm.h"

#define USEVOICE

#include "../sysdep/sysdep_dsp.h"
#include "../sysdep/sysdep_dsp_priv.h"
#include "../sysdep/plugin_manager.h"

LPGUID				 pDSGUID;
LPDIRECTSOUND		 pDirectSound;
DSBUFFERDESC		 dsbdesc;

LPDIRECTSOUNDBUFFER  pPrimaryDirectSoundBuffer;
LPDIRECTSOUNDBUFFER  pSecondaryDirectSoundBuffer[2];

#define AUDIOBUFFER 0


extern SMutex *soundMutex;
extern HWND hwndMain;

extern int sound_rate;
extern int sound_samples;

int soundthread =0;

int create_buffers(void);


#define SECONDARY_BUFFER_SIZE  (4 * sizeof(short) * sound_samples)


int directsound_open(void);
int create_buffers(void);
int set_event(void);
int write_buffer(int SecondaryBufferIdx , unsigned char *wavedata ,int datasize);
void close_directsound(void);
int play_start_directsound(void);
int directsound_ResumeSound(void);
int directsound_StopSound(void);

// ****************************************************************************
//		directsound 音源を開く
// ****************************************************************************
int directsound_open(void)
{
	create_buffers();		// create all buffers
	set_event();			// set event
	play_start_directsound();
	return 1;
}

// ****************************************************************************
//		バッファの作成
// ****************************************************************************
int create_buffers(void)
{
	int    ret=0;
	HRESULT hr;
	WAVEFORMATEX pcmwf;

	PRINTDEBUG( SND_LOG,"[DIRECTSOUND][create_buffers]\n");
	// --------------------------------------------------------------------------------------
	// Create Direct Sound object
	pDSGUID = NULL;
	hr = DirectSoundCreate( pDSGUID , &pDirectSound ,NULL);
	if( FAILED( hr))
	{
		PRINTDEBUG( SND_LOG,"Create Direct Sound FAILED \n");
		goto error;
	}
	
	// Set Cooperative level
	hr = IDirectSound_SetCooperativeLevel(pDirectSound, hwndMain , DSSCL_PRIORITY);
	if( FAILED( hr))
	{
		PRINTDEBUG( SND_LOG,"Set Cooperative level  FAILED \n");
		goto error;
	}

	// --------------------------------------------------------------------------------------
	// Create Primary buffer 
	ZeroMemory( &dsbdesc, sizeof(DSBUFFERDESC));
	dsbdesc.dwSize = sizeof( DSBUFFERDESC);

	dsbdesc.dwFlags = DSBCAPS_CTRLVOLUME | DSBCAPS_PRIMARYBUFFER ; // |DSBCAPS_LOCDEFER;
	dsbdesc.dwBufferBytes = 0;
	dsbdesc.lpwfxFormat = NULL;

	hr =   IDirectSound_CreateSoundBuffer( pDirectSound,&dsbdesc ,&pPrimaryDirectSoundBuffer , (LPUNKNOWN)NULL);
	if(FAILED(hr))
	{
		PRINTDEBUG( SND_LOG,"Create Sound buffer  FAILED \n");
		goto error;
	}

	// --------------------------------------------------------------------------------------
	// Set Primary buffer 's format
	ZeroMemory( &pcmwf, sizeof( WAVEFORMATEX));
	pcmwf.wFormatTag = WAVE_FORMAT_PCM;
	pcmwf.nChannels = 1;
	pcmwf.nSamplesPerSec = sound_rate;
	pcmwf.wBitsPerSample = 16;
	pcmwf.nBlockAlign = pcmwf.wBitsPerSample * pcmwf.nChannels / 8;;
	pcmwf.nAvgBytesPerSec = pcmwf.nSamplesPerSec * pcmwf.nBlockAlign;
	
	IDirectSoundBuffer_SetFormat( pPrimaryDirectSoundBuffer, &pcmwf);



	// Create Secondary buffer 
	ZeroMemory( &dsbdesc, sizeof( DSBUFFERDESC));
	dsbdesc.dwSize = sizeof( DSBUFFERDESC);
	dsbdesc.dwFlags = DSBCAPS_GETCURRENTPOSITION2   // 再生位置の取得をできるようにします
						|DSBCAPS_GLOBALFOCUS        // グローバルフォーカスで作ります。
						|DSBCAPS_LOCDEFER 
						| DSBCAPS_CTRLPOSITIONNOTIFY; // 再生位置通知イベントを有効にします;

	//dsbdesc.dwBufferBytes = 4* pcmwf.nAvgBytesPerSec;	// バッファ長
	dsbdesc.dwBufferBytes = SECONDARY_BUFFER_SIZE;	// バッファ長
	dsbdesc.lpwfxFormat = &pcmwf;


	hr = IDirectSound_CreateSoundBuffer(pDirectSound, &dsbdesc,&pSecondaryDirectSoundBuffer[ AUDIOBUFFER],NULL);
	if( FAILED(hr))
		{
		 PRINTDEBUG(SND_LOG,"can't create secondary buffer \n");
		 goto error;
		}

	hr = IDirectSound_CreateSoundBuffer(pDirectSound, &dsbdesc,&pSecondaryDirectSoundBuffer[ VOICEBUFFER],NULL);
	if( FAILED(hr))
		{
		 PRINTDEBUG(SND_LOG,"can't create secondary buffer \n");
		 goto error;
		}

	ret =1;
	return(ret);

error:
	ret = 0;
return(ret);
}



#define NOTIFYEVENTS 2 // バッファに　ｎ数のイベント通知位置を設定します。

LPDIRECTSOUNDNOTIFY lpDsNotify;
HANDLE  hEvent[ NOTIFYEVENTS+1];

// ****************************************************************************
//		通知イベントの設定
// ****************************************************************************
int set_event(void)
{

	HRESULT hr;
	DSBPOSITIONNOTIFY PositionNotify[ NOTIFYEVENTS];
	int i;

	PRINTDEBUG( SND_LOG,"[DIRECTSOUND][set_event]\n");
	//HRESULT QueryInterface(
	//  REFIID iid,
	//  void ** ppvObject
	//);

	// イベントの設定
	/* Query for the interface */
	//result = IDirectSoundBuffer_QueryInterface(mixbuf, &IID_IDirectSoundNotify, (void *)&notify);

	//　IID_* を使う場合、dxguid.lib をリンクすること
	hr = IDirectSoundBuffer_QueryInterface( pSecondaryDirectSoundBuffer[ AUDIOBUFFER ] ,
		&IID_IDirectSoundNotify, 
        (void*)&lpDsNotify);

	if( FAILED(hr))
		{
		PRINTDEBUG( SND_LOG,"[DIRECTSOUND][set_event]  QueryInterface failed \n");
		return 0;
		}

	// イベントとシグナルになる位置を取得
	for( i = 0 ; i < NOTIFYEVENTS ; i++ )
		{
		hEvent[i] = CreateEvent( NULL, FALSE, FALSE, NULL ); // イベント作成
		PositionNotify[i].dwOffset = (dsbdesc.dwBufferBytes / NOTIFYEVENTS) * i; // シグナルになる位置を計算
		PositionNotify[i].hEventNotify = hEvent[i];			// DirectSoundに渡す構造体にイベントハンドルをコピー
		}


	// イベントをセット
	hr = IDirectSoundNotify_SetNotificationPositions( lpDsNotify ,NOTIFYEVENTS,  PositionNotify);
	if( FAILED(hr) )
		{
		PRINTDEBUG( SND_LOG,"[DIRECTSOUND][set_event]  SetNotificationPositions failed \n");
		return 0;
		}
return 1;
}

static DWORD write_c[ 3]={0,0,0};

// ****************************************************************************
//		セカンダリバッファへの書き込み
// ****************************************************************************
int write_buffer(int SecondaryBufferIdx , unsigned char *wavedata ,int datasize)
{
	HRESULT hr;
	LPVOID pMem1,pMem2;
	DWORD  dwSize1,dwSize2;

	int   idx = SecondaryBufferIdx;

	//DWORD  play_c, write_c;
	//IDirectSoundBuffer_GetCurrentPosition( pDirectSoundBuffer[ SECONDARY_BUFFER] , &play_c, &write_c);
	

//	PRINTDEBUG3( SND_LOG,"[DIRECTSOUND][write_buffer] datasize=%3d  play=%3d  write=%3d ",datasize, play_c ,write_c);
	// lock buffer
	hr = IDirectSoundBuffer_Lock( pSecondaryDirectSoundBuffer[ idx], write_c[idx], datasize,&pMem1,&dwSize1,&pMem2, &dwSize2 ,0); 
																										//DSBLOCK_FROMWRITECURSOR
	if( FAILED(hr))
		{
		IDirectSoundBuffer_Release(  pSecondaryDirectSoundBuffer[ idx]);
		PRINTDEBUG( SND_LOG,"[DIRECTSOUND][write_buffer] Lock failed \n");
		return 0;
		}
	if( hr == DSERR_BUFFERLOST)		// restore buffer
		{
		IDirectSoundBuffer_Restore( pSecondaryDirectSoundBuffer[ idx]);
		hr = IDirectSoundBuffer_Lock( pSecondaryDirectSoundBuffer[ idx],write_c[idx], datasize,&pMem1,&dwSize1,&pMem2, &dwSize2 ,0);
		}

	PRINTDEBUG2(SND_LOG,"dwSize1=%ld  dwSize2=%ld \n" ,dwSize1 , dwSize2);

	// copy data to buffer
	CopyMemory( pMem1, wavedata, dwSize1);
	if( dwSize2 !=0)
		{
		CopyMemory( pMem2,wavedata+ dwSize1, dwSize2);
		}
	
	// unlock buffer
	hr = IDirectSoundBuffer_Unlock( pSecondaryDirectSoundBuffer[ idx] ,pMem1 ,dwSize1 ,pMem2, dwSize2);
	if( FAILED(hr))
		{
		PRINTDEBUG( SND_LOG,"[DIRECTSOUND][write_buffer] Unlock failed \n");
		return 0;
		}
 
write_c[ idx] += datasize;						// next write pointer
 if( SECONDARY_BUFFER_SIZE <= write_c[idx] )
	 write_c[idx] -= SECONDARY_BUFFER_SIZE;
 return 1;
}



// ****************************************************************************
//			directsound 音源を閉じる
// ****************************************************************************
void directsound_dsp_destroy(void)
{
	HRESULT hr;
	soundthread = 0;
	Sleep(200);

	hr= IDirectSoundBuffer_Stop(  pSecondaryDirectSoundBuffer[ AUDIOBUFFER]);
	hr= IDirectSoundBuffer_Stop(  pPrimaryDirectSoundBuffer );
	hr= IDirectSound_Release(lpDsNotify);
	hr= IDirectSoundBuffer_Release( pSecondaryDirectSoundBuffer[ AUDIOBUFFER]);
	hr= IDirectSoundBuffer_Release( pPrimaryDirectSoundBuffer);
	hr= IDirectSound_Release( pDirectSound );

}



static short  soundbuf[ MAX_SOUND_BUFSIZE ];
static short  voicebuf[ MAX_SOUND_BUFSIZE ];

extern int VoiceFlag;

// ****************************************************************************
//				音源に書き込む
// ****************************************************************************
static int SCALL  threadProc1(LPVOID param )
{
	int ret;
#ifdef USEVOICE
	HRESULT hr;
#endif
	//static int voice_playing=0;

	PRINTDEBUG( SND_LOG,"[DIRECTSOUND][threadProc1] start \n");
	directsound_ResumeSound();

	while(soundthread)
		{
		//ret = WaitForSingleObject( hEvent[0], INFINITE);			// wait next event
		ret= WaitForMultipleObjects( NOTIFYEVENTS ,hEvent, FALSE, INFINITE);

		write_buffer( AUDIOBUFFER , (unsigned char*)soundbuf ,sound_samples *sizeof(short)*2);

		

		OSD_MutexLock( soundMutex);
		fmsound_update(-1);		// 残りバッファ分を全部更新する
		ringbuffer_Read( streambuffer[ AUDIOBUFFER], soundbuf, sound_samples*2);
		OSD_MutexUnlock( soundMutex);

		//ym2203_makewave( soundbuf ,sound_samples *sizeof(short)*2);

#ifdef USEVOICE
		write_buffer( VOICEBUFFER , (unsigned char*)voicebuf ,sound_samples *sizeof(short)*2);

		memset(voicebuf, 0,sizeof(voicebuf));
		OSD_MutexLock( soundMutex);
		ringbuffer_Read( streambuffer[ VOICEBUFFER], voicebuf, sound_samples*2);
		OSD_MutexUnlock( soundMutex);

#endif

#if SND_LOG
		//testPutData(soundbuf ,sound_samples* sizeof(short));
#endif
		}
	return 0;
}


// ****************************************************************************
//		play_start_directsound
// ****************************************************************************
static SThread *SoundThread;

int play_start_directsound(void)
{
	soundthread=1;
	SoundThread= (SThread*)OSD_CreateThread( threadProc1,0);
	if( !SoundThread )
		{
		 PRINTDEBUG( SND_LOG, "[DIRECTSOUND] Create thread FAILED\n");
		 return 0;
		}
	return 1;
}


// ****************************************************************************
//		一時停止した音源を再開する
//      In:
//      Out: TRUE: success  FALSE: failed
// ****************************************************************************
int directsound_ResumeSound(void)
{
	HRESULT hr;	
	write_c[0]=write_c[1]=write_c[2]=0;
	memset( soundbuf,0, sizeof(soundbuf));
	memset( voicebuf,0, sizeof(soundbuf));
	write_buffer( AUDIOBUFFER, (unsigned char*)soundbuf ,sound_samples *sizeof(short)*2);	

	hr = IDirectSoundBuffer_Play( pSecondaryDirectSoundBuffer[ AUDIOBUFFER],0,0,DSBPLAY_LOOPING);
	if( FAILED( hr))
		{
		 PRINTDEBUG( SND_LOG,"[DIRECTSOUND][IDirectSoundBuffer_Play] failed \n");
		 return 0;
		}

	hr = IDirectSoundBuffer_Play( pSecondaryDirectSoundBuffer[ VOICEBUFFER],0,0,DSBPLAY_LOOPING);
	if( FAILED( hr))
		{
		 PRINTDEBUG( SND_LOG,"[DIRECTSOUND][IDirectSoundBuffer_Play] failed \n");
		 return 0;
		}
 return 1;
}

// ****************************************************************************
//		音源を一時停止する
//      In:
//      Out: TRUE: success  FALSE: failed
// ****************************************************************************
int directsound_StopSound(void)
{
	
	HRESULT hr;	
	hr = IDirectSoundBuffer_Stop( pSecondaryDirectSoundBuffer[ AUDIOBUFFER]);
	if( FAILED( hr))
		{
		 PRINTDEBUG( SND_LOG,"[DIRECTSOUND][IDirectSoundBuffer_Stop] failed \n");
		 return 0;
		}
 return 1;
}


// ****************************************************************************
//     directsound create
// ****************************************************************************
struct sysdep_dsp_struct * directsound_dsp_create(const void *flags)
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
	if( !directsound_open())
		{
		 printf("directsound open failed \n");
		 return NULL;
		}

	//dsp->write = waveout_dsp_write;
	dsp->destroy = directsound_dsp_destroy;
	dsp->resume =  directsound_ResumeSound;
	dsp->stop   =  directsound_StopSound;
	return dsp;
}

/*
 * The public variables structure
 */
const struct plugin_struct sysdep_dsp_directsound = {
	"directsound",
	"sysdep_dsp",
	"Windows directsoundt plugin",
	NULL, 				/* no options */
	NULL,				/* no init */
	NULL,				/* no exit */
	directsound_dsp_create,
	3,				/* high priority */
};
#endif


#if 0
			if( !voice_playing) 
				{
				hr = IDirectSoundBuffer_Play( pSecondaryDirectSoundBuffer[ VOICEBUFFER],0,0,DSBPLAY_LOOPING);
				if( FAILED( hr))
					{
					PRINTDEBUG( SND_LOG,"[DIRECTSOUND][IDirectSoundBuffer_Play] failed \n");
					}
				write_buffer( VOICEBUFFER, (unsigned char*)voicebuf ,sound_samples *sizeof(short)*2);
				voice_playing = 1;
				}
			}
		else
			{
			hr = IDirectSoundBuffer_Stop( pSecondaryDirectSoundBuffer[ VOICEBUFFER]);
			voice_playing =0;
			}
#endif

#endif 
