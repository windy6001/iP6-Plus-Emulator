/** iP6: PC-6000/6600 series emualtor ************************/
/**                                                         **/
/**                         voice.c                         **/
/**                                                         **/
/** translate from PC6001V's C++ to C  by windy             **/
/**                                                         **/
/** Original program:   PC6001V                             **/
/** Original author:    Yumitaro.                           **/
/** Original URL:       http://www.papicom.net/p6v/         **/
/*************************************************************/


#define VOICE_DEBUG

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "voice.h"
#include "Pd7752.h"
#ifdef X11
#include "OpenAL.h"
#endif

#ifdef UNIX
#include <unistd.h>
#endif

#include "Sound.h"
#include "SThread.h"
#include "Timer.h"
#include "buffer.h"
#include "P6.h"
#include "types.h"
#include "schedule.h"
#include "wav.h"

#include "Z80.h"



// ****************************************************************************
//		thread Variables
// ****************************************************************************

//pthread_t voiceThread;
//static pthread_mutex_t voiceMutex;
static SMutex    *voiceMutex;		// voice mutex
static SThread   *voiceThread;		// voice thread object
//static SEvent    *voiceEvent;

extern SMutex    *soundMutex;		// sound mutex


static int ThreadLoopStop = 1;
static int VoiceCancel = 0;
int VoiceFlag = 0;					// Voice処理中1になるフラグ

int voiceHz;

// ****************************************************************************
//		Variables
// ****************************************************************************

// I/Oポート
static byte portE0;
static byte portE2;
static byte portE3;
	
static int VStat;					// ステータス
	
// 内部句用...
int IVLen;					// サンプル数
int *IVBuf=NULL;			// データバッファ
int IVPos;					// 読込みポインタ

// パラメータバッファ(1フレームのパラメータ数は7個)
static byte ParaBuf[7];             // パラメータバッファ
static byte Pnum;					// パラメータ数
static int Fnum;					// 繰り返しフレーム数
static int PReady;					// パラメータセット完了

static D7752_SAMPLE *Fbuf = NULL;	// フレームバッファポインタ(10kHz 1フレーム)

void FreeVoice( void );

	/*unsigned char voicebuf[samples];
	for(i=0; i<samples; i++) {
		int dat = (double)Fbuf[i*GetFrameSize()/samples]/10.0;
		if (dat > 127) dat = 127;
		if (dat < -128) dat = -128;
		voicebuf[i] = dat+128;
	}*/
	//coreaudio_dsp_write(dsp, voicebuf, samples) ;


// ****************************************************************************
//	サンプリングレートとビット幅を変換して ストリームに書き込む
// ****************************************************************************
static void UpConvert(void)
{
	int i;
	//short *voicebuf;
	int framesize = GetFrameSize();

	// 10kHz -> 実際に再生するサンプリングレートに変換した時のサンプル数
	int samples = GetFrameSize() * sound_rate / 10000;

	PRINTDEBUG2(VOI_LOG ,"[voice][UpConvert] WRITTING VOICE DATA    framesize=%d -> samples=%d \n",GetFrameSize(), samples);

	if( UseSound)
		{
		OSD_MutexLock( soundMutex);
		for(i=0; i< samples ; i++)
			{
			int dat = Fbuf[ i * framesize/samples]*4;		// * 4 は 16bit<-14bit のため
			ringbuffer_Put( streambuffer[VOICEBUFFER] ,  dat*2);	// 出力レベルが低いのでとりあえず2倍
			}
		OSD_MutexUnlock( soundMutex);
		PRINTDEBUG1(VOI_LOG,"[voice][Upconvert] VOICE DATA=%d \n",ringbuffer_DataNum( streambuffer[VOICEBUFFER]));
		}


	//testPutData(voicebuf ,samples  * sizeof(short));
	//printf("samples == %d \n",samples);
	//OSD_MutexLock( voiceMutex);
	//ringbuffer_Write(audiobuffer ,voicebuf ,samples * sizeof(short));
	//OSD_MutexUnlock( voiceMutex);
	
	//free( voicebuf);
}

// ****************************************************************************
//	　発声を停止する
// ****************************************************************************
static void AbortVoice(void)
{
	Event_Del( EVENT_VOICE);

	// スレッドのループ停止
	ThreadLoopStop = 1;
	// 残りのパラメータはキャンセル
	Pnum = Fnum = 0;
	PReady = FALSE;
	// フレームバッファ開放
	if (Fbuf) {
		free(Fbuf);
		Fbuf = NULL;
	}
	VStat &= ~D7752E_BSY;
}





int VoiceMain(void* arg)
{
 return 0;	
}


int updateVoice(int num)
{
	int ret=0;
	while( num-- )
		{
		//SndDev::Buf->Put( ( IVBuf[IVPos++] * SndDev::Volume ) / 100 );
		ringbuffer_Put( streambuffer[VOICEBUFFER] , ( IVBuf[IVPos++] * 100 ) / 100);
		}

	if( IVPos >= IVLen ){	// 最後まで発生したら発声終了
		AbortVoice();
	}
}


extern reg R;

// ****************************************************************************
//			音声合成　別スレッドで、パラメータを受け取り、波形を生成する。
// ****************************************************************************
int VoiceMainLoop(void* arg)
{
	int cnt =0;
	//UPeriod_bak = UPeriod =30;

	PRINTDEBUG(VOI_LOG, "[voice][VoiceMainLoop] THREAD START \n");
	//OSD_InitTimer();
	while (1) {
		//int handle, stat;
		//OSD_OpenEvent( voiceEvent, "voiceEvent");
		//stat = WaitForSingleObject( voiceEvent->handle ,30);

		if (VoiceCancel) {  // 発声時にリセットされたら。。。
			PRINTDEBUG(VOI_LOG , "[voice][VoiceMainLoop] VOICE RESET \n");
			VoiceCancel = 0;
			AbortVoice();
			VoiceFlag = 0;
			//pthread_exit(NULL);	
			PRINTDEBUG(VOI_LOG, "[voice][VoiceMainLoop] THREAD END \n");
			break;
		} else if (!ThreadLoopStop) {
			//pthread_mutex_lock(&voiceMutex);
			OSD_MutexLock( voiceMutex);

			if(VStat & D7752E_EXT) {	// 外部句発声処理
				if(PReady) {	// パラメータセット完了してる?
					PRINTDEBUG( VOI_LOG ,"[voice][VoiceMainLoop] EXT VOICE. PARAMETER is done.\n");
					// フレーム数=0ならば終了処理
					if(!(ParaBuf[0]>>3)) {
						PRINTDEBUG( VOI_LOG, "[voice][VoiceMainLoop] if frame ==0 then end\n");
						
						AbortVoice();
						//pthread_mutex_unlock(&voiceMutex);
						OSD_MutexUnlock( voiceMutex);
						VoiceFlag = 0;
						//pthread_exit(NULL);
						break;		// exit thread
					} else {
						// 1フレーム分のサンプル生成
						Synth(ParaBuf, Fbuf);
						// サンプリングレートを変換してバッファに書込む
						UpConvert();
						// 次フレームのパラメータを受け付ける
						PReady = FALSE;
						ThreadLoopStop = 1;
						VStat |= D7752E_REQ;


					}
				} else {
					PRINTDEBUG(VOI_LOG , "[voice][VoiceMainLoop] VOICE STOP\n");
					AbortVoice();		// 発声停止
					VStat = D7752E_ERR;
				}
			}else{						// 内部句発声処理
				// 1フレーム分のサンプルをバッファに書込む
				int num = min( IVLen - IVPos, GetFrameSize() * sound_rate / 10000 );
				PRINTDEBUG( VOI_LOG, "[voice][VoiceMainLoop] INTERNAL VOICE " );
				PRINTDEBUG3( VOI_LOG, "num=%d %d/%d\n", num, IVPos ,IVLen);
				OSD_MutexLock( soundMutex);
				while( num-- )
					{
					//SndDev::Buf->Put( ( IVBuf[IVPos++] * SndDev::Volume ) / 100 );
					ringbuffer_Put( streambuffer[VOICEBUFFER] , ( IVBuf[IVPos++] * 100 ) / 100);
					}
				OSD_MutexUnlock( soundMutex);

				if( IVPos >= IVLen ){	// 最後まで発生したら発声終了
					AbortVoice();
				}
			}
			//pthread_mutex_unlock(&voiceMutex);			
			OSD_MutexUnlock( voiceMutex);
		}
		usleep(300);	// 元々、Unixだけの関数。マイクロ秒スリープする。Windowsでは、代替の関数を作成。 win32.c 
	}
	PRINTDEBUG(VOI_LOG, "[voice][VoiceMainLoop] THREAD END \n");
	//OSD_TrashTimer();

	//UPeriod_bak = UPeriod =2;
	return 0;
}



// ****************************************************************************
//		Voice スレッドをキャンセルする
// ****************************************************************************
void CancelVoiceThread(void)
{
	if (VoiceFlag) {
		VoiceCancel = 1;
	}
}


// ****************************************************************************
//		内部句 wave ファイル読み込み
// ****************************************************************************
int  LoadVoice( char *filename )
{
	int i;
	struct _spec ws;
	int len;		// 元データサイズ
	char *buf;	// 元データバッファ
	char filepath[PATH_MAX];
	char path[PATH_MAX];

	buf=NULL;

	path[0]=0;
#ifdef WIN32
	if( !IsDebuggerPresent())
		 strcpy( path, "..\\..\\wav\\");	// on normal
	else
		 strcpy( path ,"..\\wav\\");		// on debugger
#endif

	PRINTDEBUG1( VOI_LOG, "[VOICE][LoadVoice]  [%s] -> ", filename );

	// WAVファイルを読込む
	sprintf( filepath, "%s%s", "wav\\", filename );
	PRINTDEBUG1( VOI_LOG, "%s ->", filepath );
	
	//if( !SDL_LoadWAV( filepath, &ws, &buf, &len ) ){
	if( !loadWav( filepath , &ws, &buf , &len) ) {			// まず、wavディレクトリから読む 
		sprintf( filepath, "%s%s", path, filename );
		PRINTDEBUG1( VOI_LOG, "%s ->", filepath );
	
		if( !loadWav( filepath , &ws, &buf , &len) ) {		// 指定ディレクトリから読む
			PRINTDEBUG( VOI_LOG, "Error!\n" );
			return FALSE;
		}
	}
	
	// 22050Hz以上,16bit,1ch でなかったらダメ
	if( ws.freq < 22050 || ws.samplebit != 16 || ws.channels != 1 ){
		PRINTDEBUG3( VOI_LOG, "F:%d S:%d C:%d -> Error!\n", ws.freq, ws.samplebit, ws.channels );
		//SDL_FreeWAV( buf );
		freeWav( buf );
		return FALSE;
	}
	PRINTDEBUG( VOI_LOG, "OK\n" );
	
	FreeVoice();

	// 変換後のサイズを計算してバッファを確保
	// 発声速度4の時,1フレームのサンプル数は160
	IVLen = (int)( (double)sound_rate * (double)(len/2) / (double)ws.freq
					* (double)GetFrameSize() / (double)160 );
	
	//PRINTD2( VOI_LOG, "Len:%d/%d ->", IVLen, len );
	

	//IVBuf = new int[IVLen];
	IVBuf = malloc( IVLen *sizeof(int));
	if( !IVBuf ){
		//SDL_FreeWAV( buf );
		freeWav( buf );
		IVLen = 0;
		return FALSE;
	}
	
	// 変換
	{
	short *sbuf = (short *)buf;
	for( i=0; i<IVLen; i++ ){
		IVBuf[i] = sbuf[(int)(( (double)i * (double)(len/2) ) / (double)IVLen)];
		}
	
	// WAV開放
	//SDL_FreeWAV( buf );
	freeWav( buf );

	// 読込みポインタ初期化
	IVPos = 0;
	}

	return TRUE;
}


// ****************************************************************************
//		内部句 wave ファイル解放
// ****************************************************************************
void FreeVoice( void )
{
	if( IVBuf ){
		//delete [] IVBuf;
		free( IVBuf);
		IVBuf = NULL;
		IVLen = 0;
	}
}


// ****************************************************************************
//		音声合成の初期化
// ****************************************************************************
int InitVoice(void)
{
	portE0 = portE2 = portE3 = 0;
	Pnum    = 0;
	Fnum    = 0;
	PReady 	= FALSE;
	Fbuf    = NULL;
	VStat   = D7752E_IDL;

	// ループを停止しておく
	ThreadLoopStop = 1;

	// mutex生成
	//if (pthread_mutex_init(&voiceMutex, NULL) != 0) return 0;
	voiceMutex = OSD_CreateMutex();
	if( voiceMutex ==0) return(0);

	//voiceEvent = OSD_CreateEvent("voiceEvent");


	return 1;
}

// ****************************************************************************
//		音声合成の後処理
// ****************************************************************************
void TrashVoice(void)
{
	//pthread_mutex_destroy(&voiceMutex);
	OSD_CloseMutex( voiceMutex);
	//OSD_CloseEvent( voiceEvent);
	if(Fbuf) free(Fbuf);
	
	//Event_Del( EVENT_VOICE);		// 音声合成イベントを削除


}

// ****************************************************************************
//		モードセット
// ****************************************************************************
static void VSetMode(byte mode)
{
	PRINTDEBUG1(VOI_LOG, "[VOICE][VSetMode]      VOICE START , CLEAR STATUS  mode=%X\n",mode);
	//pthread_mutex_lock(&voiceMutex);
	OSD_MutexLock( voiceMutex);

	// 音声合成開始
	PD7752_Start(mode);
	// ステータスクリア
	VStat = D7752E_IDL;

    //pthread_mutex_unlock(&voiceMutex);
	OSD_MutexUnlock( voiceMutex);
}


// ****************************************************************************
//		コマンドセット
// ****************************************************************************
static void VSetCommand(byte comm)
{
	char filename[ PATH_MAX];

	//pthread_mutex_lock(&voiceMutex);
	OSD_MutexLock( voiceMutex);
	// 発声中なら止める
	AbortVoice();
	switch (comm) {
	case 0x00:		// 内部句選択コマンド ----------
	case 0x01:
	case 0x02:
	case 0x03:
	case 0x04:
		// 内部句再生モード
		PRINTDEBUG1(VOI_LOG,"[voice][VSetCommand] %02X ",comm);
		PRINTDEBUG( VOI_LOG,"INT VOICE CMD \n");
		// スレッド生成 （高速化のため、いちいちスレッドを生成する）
		//pthread_create(&voiceThread, NULL, VoiceMainLoop, NULL);

		PRINTDEBUG( VOI_LOG,"[voice][VSetCommand] Create Thread \n");

		voiceThread = (SThread*) OSD_CreateThread( VoiceMainLoop ,NULL); 
		if( !voiceThread ) {printf("voice thread  create ....FAILED \n"); break;}
		
		OSD_Delay(3);

		
		VoiceFlag = 1;
		// フレームバッファ確保
		Fbuf = malloc(sizeof(D7752_SAMPLE)*GetFrameSize());
		if(!Fbuf) break;

		//内部句　WAVEデータ読み込み

		sprintf( filename ,"f4%d.wav", comm);
		LoadVoice( filename );

		// ステータスを内部句再生モードにセット
		VStat = D7752E_BSY;
		PRINTDEBUG(VOI_LOG, "[voice][VSetCommand] start to recive parameter  \n");
		
		//Event_Add( EVENT_VOICE , (double)10000.0/(double)GetFrameSize() ,EV_LOOP|EV_HZ , VoiceCallback );

		// スレッド再開
		ThreadLoopStop=0;
		break;
	case 0xfe:		// 外部句選択コマンド ----------
		if( sr_mode) VoiceIntFlag = INTFLAG_REQ;
		PRINTDEBUG1(VOI_LOG,"[voice][VSetCommand] %02X ",comm);
		PRINTDEBUG( VOI_LOG,"EXT VOICE CMD \n");
		// スレッド生成 （高速化のため、いちいちスレッドを生成する）
		//pthread_create(&voiceThread, NULL, VoiceMainLoop, NULL);

		PRINTDEBUG( VOI_LOG,"[voice][VSetCommand] Create Thread \n");

		voiceThread = (SThread*) OSD_CreateThread( VoiceMainLoop ,NULL); 
		if( !voiceThread ) {printf("voice thread  create ....FAILED \n"); break;}
		
		OSD_Delay(3);

		
		VoiceFlag = 1;
		// フレームバッファ確保
		Fbuf = malloc(sizeof(D7752_SAMPLE)*GetFrameSize());
		if(!Fbuf) break;
		// ステータスを外部句再生モードにセット，パラメータ受付開始
		VStat = D7752E_BSY | D7752E_EXT | D7752E_REQ;
		PRINTDEBUG(VOI_LOG, "[voice][VSetCommand] start to recive parameter  \n");

		//Event_Add( EVENT_VOICE , (double)10000.0/(double)GetFrameSize() ,EV_LOOP|EV_HZ , VoiceCallback );

		break;
	case 0xff:		// ストップコマンド ----------
		break;
	default:		// 無効コマンド  ----------
		VStat = D7752E_ERR;	// ホント?
		break;
	}
	//pthread_mutex_unlock(&voiceMutex);	
	OSD_MutexUnlock(voiceMutex);
}


// ****************************************************************************
//		音声パラメータの転送
// ****************************************************************************
static void VSetData(byte data)
{
	//pthread_mutex_lock(&voiceMutex);
	OSD_MutexLock(voiceMutex);


	PRINTDEBUG1(VOI_LOG,"[voice][VSetData Data]     [%02X] \n",data);
	
	// 再生時のみデータを受け付ける
	if ((VStat & D7752E_BSY)&&(VStat & D7752E_REQ)) {
		if (Fnum == 0 || Pnum) {	// 最初のフレーム?
			// 最初のパラメータだったら繰り返し数を設定
			if(Pnum == 0) {
				Fnum = data>>3;
				if (Fnum == 0) { PReady = TRUE; ThreadLoopStop = 0; }
				PRINTDEBUG(VOI_LOG, "[voice][VSetData] FIRST PARAMETER, SET REPEAT TIME\n");
			}
			// パラメータバッファに保存
			ParaBuf[Pnum++] = data;
			// もし1フレーム分のパラメータが溜まったら発声準備完了
			if(Pnum == 7) {
				VStat &= ~D7752E_REQ;
				Pnum = 0;
				if(Fnum > 0) Fnum--;
				ThreadLoopStop = 0;
				PReady = TRUE;
				PRINTDEBUG(VOI_LOG , "[voice][VSetData] If the parameter for one frame collects, a vocal preparation is completed. *** \n");
			}
		} else {						// 繰り返しフレーム?
			// パラメータバッファに保存
			// PD7752の仕様に合わせる
			int i;
			PRINTDEBUG(VOI_LOG,"[voice][VSetData Data]     繰り返しフレーム? \n");

			for(i=1; i<6; i++) ParaBuf[i] = 0;
			ParaBuf[6] = data;
			VStat &= ~D7752E_REQ;
			Pnum = 0;
			Fnum--;
			ThreadLoopStop = 0;			
			PReady = TRUE;
		}
	}
	//pthread_mutex_unlock(&voiceMutex);
	OSD_MutexUnlock( voiceMutex);


	if( sr_mode) VoiceIntFlag = INTFLAG_REQ;
}

// ****************************************************************************
//		ステータスレジスタを取得
// ****************************************************************************
static int VGetStatus(void)
{
	int ret;
	//PRINTDEBUG( VOI_LOG,"[voice][VGetStatus]\n");
	//pthread_mutex_lock(&voiceMutex);	
	OSD_MutexLock( voiceMutex);
	
	ret = VStat;


	//pthread_mutex_unlock(&voiceMutex);	
	OSD_MutexUnlock( voiceMutex);



	return ret;
}


// ****************************************************************************
//		I/O アクセス関数
// ****************************************************************************

void OutE0H(byte data)
{
	VSetData(data);
}
void OutE2H(byte data)
{
	VSetMode(data);
}
void OutE3H(byte data)
{
	VSetCommand(data);
}
byte InE0H(void)
{
	int stat = VGetStatus();
#if 0
	static int cnt =0;

	if( (stat & 0x80) ==0x80 && (stat & 0x40)==0)
		{
		cnt++;
		printf(" %3d busy ",cnt);
		}
	else
		cnt=0;
	PRINTDEBUG(VOI_LOG, "[voice][VGetStatus]        Status=　 " );
	if( stat & 0x80) PRINTDEBUG(VOI_LOG ,"BSY ") else PRINTDEBUG(VOI_LOG ,"--- ");
	if( stat & 0x40) PRINTDEBUG(VOI_LOG ,"REQ ") else PRINTDEBUG(VOI_LOG ,"--- ");
	if( stat & 0x20) PRINTDEBUG(VOI_LOG ,"EXT ") else PRINTDEBUG(VOI_LOG ,"INT ");
	if( stat & 0x10) PRINTDEBUG(VOI_LOG ,"ERR \n") else PRINTDEBUG(VOI_LOG ,"--- \n");
#endif
#if 0
	if( stat & 0x10) PRINTDEBUG(VOI_LOG ,"ERR \n") else PRINTDEBUG(VOI_LOG ,"--- \n");
#endif


	return stat;
}
byte InE2H(void)
{
	return portE2;
}
byte InE3H(void) {
	return portE3;
}



// Voice スレッド
/*
int VoiceMainLoop(void* arg)
{
	int cnt =0;
		UPeriod_bak = UPeriod =60;

	PRINTDEBUG(VOI_LOG, "[voice][VoiceMainLoop] スレッド開始 \n");
	OSD_InitTimer();
	while (1) {
		//int handle, stat;
		//OSD_OpenEvent( voiceEvent, "voiceEvent");
		//stat = WaitForSingleObject( voiceEvent->handle ,30);

		if (VoiceCancel) {  // 発声時にリセットされたら。。。
			PRINTDEBUG(VOI_LOG , "[voice][VoiceMainLoop] 発声時にリセットされた。\n");
			VoiceCancel = 0;
			AbortVoice();
			VoiceFlag = 0;
			//pthread_exit(NULL);	
			PRINTDEBUG(VOI_LOG, "[voice][VoiceMainLoop] スレッド終了 \n");
			break;
		} else if (!ThreadLoopStop) {
			//pthread_mutex_lock(&voiceMutex);
			//OSD_MutexLock( voiceMutex);

			if(VStat & D7752E_EXT) {	// 外部句発声処理
				if(PReady) {	// パラメータセット完了してる?
					PRINTDEBUG( VOI_LOG ,"[voice][VoiceMainLoop] 外部句発声処理,パラメータセット完了。\n");
					// フレーム数=0ならば終了処理
					if(!(ParaBuf[0]>>3)) {
						PRINTDEBUG( VOI_LOG, "[voice][VoiceMainLoop] フレーム数０、終了処理\n");
						
						AbortVoice();
						//pthread_mutex_unlock(&voiceMutex);
						//OSD_MutexUnlock( voiceMutex);
						VoiceFlag = 0;
						//pthread_exit(NULL);
						break;
					} else {
						// 1フレーム分のサンプル生成
						Synth(ParaBuf, Fbuf);
						// サンプリングレートを変換してバッファに書込む
						UpConvert();
						// 次フレームのパラメータを受け付ける
						PReady = FALSE;
						ThreadLoopStop = 1;
						VStat |= D7752E_REQ;
					}
				} else {
					PRINTDEBUG(VOI_LOG , "[voice][VoiceMainLoop] 発声停止\n");
					AbortVoice();		// 発声停止
					VStat = D7752E_ERR;
				}
			}else{						// 内部句発声処理
				// 1フレーム分のサンプルをバッファに書込む
				int num = min( IVLen - IVPos, GetFrameSize() * sound_rate / 10000 );
				PRINTDEBUG( VOI_LOG, "[voice][VoiceMainLoop] 内部句発声処理 " );
				//PRINTD2( VOI_LOG, "%d/%d\n", num, IVPos );
				while( num-- )
					{
					//SndDev::Buf->Put( ( IVBuf[IVPos++] * SndDev::Volume ) / 100 );
					ringbuffer_Put( streambuffer[VOICEBUFFER] , ( IVBuf[IVPos++] * 100 ) / 100);
					}

				if( IVPos >= IVLen ){	// 最後まで発生したら発声終了
					AbortVoice();
				}
			}
			//pthread_mutex_unlock(&voiceMutex);			
			//OSD_MutexUnlock( voiceMutex);
		}
		//if( ++cnt >10) {OSD_Delay(1); cnt=0;}
		OSD_Delay(1);
		//usleep(10);
		//Sleep(5);
	}
	PRINTDEBUG(VOI_LOG, "[voice][VoiceMainLoop] スレッド終了 \n");
	OSD_TrashTimer();

	UPeriod_bak = UPeriod =2;
	return 0;
}
*/
// **************************************************
//	　次の波形データを用意して、ストリームに書き込む
// **************************************************
/*
int SCALL VoiceCallback(void *arg)
{
	int cnt =0;
	PRINTDEBUG( VOI_LOG,"[VOICE][VoiceCallback]\n");

	if( !PReady ) {PRINTDEBUG2(VOI_LOG,"failed Pnum=%d PC=%04X \n" , Pnum ,R.PC.W); return 0;}

	if(VStat & D7752E_EXT) 
		{	// 外部句発声処理
		if(PReady) 
			{	// パラメータセット完了してる?
			PRINTDEBUG( VOI_LOG ,"[voice][VoiceCallback] 外部句発声処理,パラメータセット完了。\n");
			// フレーム数=0ならば終了処理
			if(!(ParaBuf[0]>>3)) 
				{
				PRINTDEBUG( VOI_LOG, "[voice][VoiceCallback] フレーム数０、終了処理\n");
						
				AbortVoice();
				VoiceFlag = 0;
				}
			else {
				// 1フレーム分のサンプル生成
				Synth(ParaBuf, Fbuf);
				// サンプリングレートを変換してバッファに書込む
				UpConvert();
				// 次フレームのパラメータを受け付ける
				PReady = FALSE;
				ThreadLoopStop = 1;
				VStat |= D7752E_REQ;
				}
			} else 
			{
			PRINTDEBUG(VOI_LOG , "[voice][VoiceCallback] 発声停止\n");
			AbortVoice();		// 発声停止
			VStat = D7752E_ERR;
			}
	}else{						// 内部句発声処理
		// 1フレーム分のサンプルをバッファに書込む
		int num = min( IVLen - IVPos, GetFrameSize() * sound_rate / 10000 );
		PRINTDEBUG( VOI_LOG, "[voice][VoiceCallback] 内部句発声処理 " );
		//PRINTD2( VOI_LOG, "%d/%d\n", num, IVPos );
		while( num-- )
			{
			//SndDev::Buf->Put( ( IVBuf[IVPos++] * SndDev::Volume ) / 100 );
			ringbuffer_Put( streambuffer[VOICEBUFFER] , ( IVBuf[IVPos++] * 100 ) / 100);
			}

		if( IVPos >= IVLen ){	// 最後まで発生したら発声終了
					AbortVoice();
			}
		}


	return 0;
}
#if 0 // VOI_LOG
	PRINTDEBUG(VOI_LOG, "[voice][VGetStatus]        Status=　 " );
	if( ret & 0x80) PRINTDEBUG(VOI_LOG ,"BSY ") else PRINTDEBUG(VOI_LOG ,"--- ");
	if( ret & 0x40) PRINTDEBUG(VOI_LOG ,"REQ ") else PRINTDEBUG(VOI_LOG ,"--- ");
	if( ret & 0x20) PRINTDEBUG(VOI_LOG ,"EXT ") else PRINTDEBUG(VOI_LOG ,"INT ");
	if( ret & 0x10) PRINTDEBUG(VOI_LOG ,"ERR \n") else PRINTDEBUG(VOI_LOG ,"--- \n");
#endif
*/
