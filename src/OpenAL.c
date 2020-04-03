/** iP6: PC-6000/6600 series emualtor ************************/
/**                                                         **/
/**                        OpenAL.c                         **/
/**                                                         **/
/**         OpenAL Driver                                   **/
/** by Windy                                                **/
/*************************************************************/
#define _USE_MATH_DEFINES
#include <stdio.h>

#ifdef __APPLE__
#include <OpenAL/al.h>
#include <OpenAL/alc.h>
#else
#include <AL/al.h>
#include <AL/alc.h>
#endif

//#include <Math.h>
#include <stdlib.h>
#include <unistd.h>

#include "SThread.h"
#include "Sound.h"

#include "types.h"
#include "sysdep/sysdep_dsp.h"
#include "sysdep/sysdep_dsp_priv.h"
#include "sysdep/plugin_manager.h"


// ****************************************************************************
//				thread Variables
// ****************************************************************************

int SoundMainThreadRun;				// TRUE: thread run
SThread *SoundMainThread;			// thread object
extern SMutex    *soundMutex;		// sound mutex

int SoundMainLoop( void *param);


ALCdevice  *device;
ALCcontext *context;

#define FREQ 44100

int is_sound_playing;



extern int sound_samples;


const ALCint attr_list[] = {
    ALC_FREQUENCY , FREQ,
    ALC_SYNC , 1,
    0
};

#define QUEUE_BUFFER_NUM 6
ALuint buffer_id[ QUEUE_BUFFER_NUM];

ALuint status;
ALuint source_id;


ALshort *pcm_data;
int error;

// **************************************************
//		OpenAL 音源を開く
//   In:
//   Out: TRUE: success  FALSE: failed
// **************************************************
int openal_open(void)
{
    printf("Opening OpenAL....");

    // device openAL
    device = alcOpenDevice( NULL);
    if( device == NULL) { printf("FAILED \n");}
    
    // create context
    context = alcCreateContext( device , attr_list);
    if( context == NULL) { printf("FAILED \n");}
    
    
    // enable
    alcMakeContextCurrent( context);
    
    // make buffer
    alGenBuffers( QUEUE_BUFFER_NUM*sizeof(ALuint), &buffer_id[0]);
    
    int i;
    for(i=0 ; i< QUEUE_BUFFER_NUM; i++)
    {
        printf("buffer_id[%d]= %X04 \n",i,buffer_id[i]);
    }

    printf("create buffer size=%d\n",sound_samples);
    pcm_data = malloc( sound_samples*sizeof(short));
    for(size_t i=0; i< QUEUE_BUFFER_NUM; i++)
    {

        
        // サンプルデータをバッファにコピー
        alBufferData( buffer_id[i],
                     AL_FORMAT_MONO16,
                     &pcm_data[0],
                     sound_samples,
                     FREQ);
        if((error=alGetError())!=AL_NO_ERROR){ printf("error=%04X ",error); exit(0);}
    }
    alGenSources( 1, &source_id);
    if((error=alGetError())!=AL_NO_ERROR){ printf("error=%04X ",error); exit(0);}

    //バッファをキューイング
    alSourceQueueBuffers( source_id, QUEUE_BUFFER_NUM,&buffer_id[0]);
    if((error=alGetError())!=AL_NO_ERROR){ printf("error=%04X ",error); exit(0);}
    
    //再生を開始
    alSourcePlay( source_id);
    if((error=alGetError())!=AL_NO_ERROR){ printf("alSourcePlay error=%04X source_id=%X",error ,source_id); exit(0);}
    
    SoundMainThreadRun = 1;
    SoundMainThread= (SThread*)OSD_CreateThread( SoundMainLoop,0);
    if( !SoundMainThread )
    {
        printf("FAILED\n");
        return(0);
    }

    printf("source_id =%X\n", source_id);
    return(1);
}


// **************************************************
//	   OpenAL 音源を閉じる
//    In:
//    Out:
// **************************************************
void openal_dsp_destroy(void)
{
    alcMakeContextCurrent( NULL);
    alcDestroyContext( context);
    alcCloseDevice( device);
    
    printf("Closing OpenAL...");
}




// **************************************************
//	   ストリームに　書き込む
// **************************************************
void openal_dsp_write(struct sysdep_dsp_struct *dsp, short *src,int len)
{
    //ALuint buffer_id;
    //alGenBuffers( 1, &buffer_id);
    //printf("openal_dsp_write len=%d\n",len);
    
        //処理を終えたキューをデキュー
        ALuint buffer;
        alSourceUnqueueBuffers( source_id,1,&buffer);
        if((error=alGetError())!=AL_NO_ERROR){ printf("alSourceUnqueueBuffers  error=%04X buffer=%X\n",error ,buffer); exit(0);}

        if( !buffer) { alGenBuffers( 1, &buffer); }
        
        //バッファにデータの書き込み
        alBufferData( buffer , AL_FORMAT_MONO16, src ,len ,FREQ);
        //alBufferData( buffer , AL_FORMAT_MONO16, &pcm_data[0] , sizeof(pcm_data) ,FREQ);
        if((error=alGetError())!=AL_NO_ERROR){ printf("alBufferData error=%04X buffer=%X\n",error ,buffer); exit(0);}
        
        //再キュー
        alSourceQueueBuffers( source_id, 1,&buffer);
        if((error=alGetError())!=AL_NO_ERROR){ printf("alSourceQueueBuffers error=%04X buffer=%X\n",error ,buffer); exit(0);}

}

// **************************************************
//		サウンドメインループ
// **************************************************
int SoundMainLoop( void* param)
{
    
    
    while( SoundMainThreadRun)
    {
        int processed;
        processed=0;
        
        alGetSourcei( source_id , AL_BUFFERS_PROCESSED , &processed);
        if((error=alGetError())!=AL_NO_ERROR){ printf("alGetSourcei error=%04X source_id=%X processed=%X",error ,source_id ,processed); exit(0);}
        
        if( processed )
        {
           // printf("SoundMainLoop(): processed==%d\n", processed);
            SoundUpdate();		// write next sound
        }
        usleep(16);
    }   	// while()
    return(0);
}




// **************************************************
//		一時停止した音源を　再開する
// **************************************************
void openal_ResumeSound(void)
{
    alSourcePlay( source_id);
	is_sound_playing =TRUE;
}

// **************************************************
//		音源を一時停止する
// **************************************************
void openal_StopSound(void)
{
    alSourcePause( source_id);
	is_sound_playing =FALSE;
}

// ****************************************************************************
//      create
// ****************************************************************************
struct sysdep_dsp_struct * openal_dsp_create(const void *flags)
{
	const struct sysdep_dsp_create_params *params = flags;
	struct sysdep_dsp_struct *dsp = NULL;
	struct coreaudio_private *priv = NULL;

	dsp= malloc( sizeof( struct sysdep_dsp_struct));
	if( dsp == NULL)
		{
		 printf("malloc failed ¥n");
		 return NULL;
		}
	if( !openal_open())
		{
		 printf("OpenAL open failed ¥n");
		 return NULL;
		}

	dsp->write = openal_dsp_write;
	dsp->destroy = openal_dsp_destroy;
	dsp->resume =  openal_ResumeSound;
	dsp->stop   =  openal_StopSound;
	return dsp;
}

/*
 * The public variables structure
 */
const struct plugin_struct sysdep_dsp_openal = {
	"openal",
	"sysdep_dsp",
	"OpenAL plugin",
	NULL, 				/* no options */
	NULL,				/* no init */
	NULL,				/* no exit */
	openal_dsp_create,
	3,				/* high priority */
};




