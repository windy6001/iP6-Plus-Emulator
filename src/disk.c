/** iP6: PC-6000/6600 series emualtor ************************/
/**                                                         **/
/**            インテリジェントディスクユニット             **/
/**                                                         **/
/**                     name is disk.c                      **/
/**                                                         **/
/** by windy                                                **/
/*************************************************************/
/*
インテリジェントタイプです。

エミュレータ処理に付いて

   コマンド／データを読み込んでいき、規定の長さに達したら、各コマンドの実行をします。
   データの受信要求があったら、既に読んであるデータを 渡します。

各コマンドの説明

  write sector は、コマンドの後に、書き込みたいデータを並べます。
  read  sector は、コマンドを発行してから、SEND DATA で データを読みます。
  result statusは、コマンドの実行結果を返します。
  result drive status は、ドライブの状態を返します。
  XMIT         は、高速モード搭載の有無を チェックするのに使用しているようです。
  SetSurfaceModeは、インテリじぇんとタイプで、1DDか、1Dかのチェックをしているみたいです

高速モードについて

  通常モードは、一回のハンドシェイクに、１バイトしか受渡しできないのに対して、
  高速モードは、一回のハンドしぇいくで、２バイトも受渡しできます。
    エミュレータ的には、高速モードでも通常モードでも、ほとんど同じ実装です。
 
参考文献
 http://www2.odn.ne.jp/~haf09260/Pc80/Pc80s31.htm
*/
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef __APPLE__
#include <machine/types.h>
#endif

#include <string.h>
#if HAVE_DIRENT_H
#include <dirent.h>
#endif

#include "types.h"
#include "P6.h"
#include "mem.h"
#include "disk.h"
#include "d88.h"
#include "error.h"
#include "Timer.h"

#include "os.h"

#include "dokodemo.h"

//extern int disk_num;

// ------------ command length -----------------------------------------
static int cmd_length[]={1 ,0 ,5 ,1 ,8 ,2 ,1 ,1 ,5 ,1 ,1 ,1 ,1,1,1,1,  1 ,0,1};

#define PC_SEND_DONE       0x8
#define PC_RECV_READY      0xA
#define PC_RECV_DONE       0xC
#define PC_SEND_REQUEST    0xE

#define FD_SEND_DONE         1
#define FD_RECV_READY        2
#define FD_RECV_DONE         4

#define SET_8255          0x91

static int cur_drive=0;   // current drive

int fd_send;			// 1: PC -> FD Senging..
int fd_recv;            // 1: PC <- FD Recieving..
int fdindex_in=0;       // index of fdbuff input
int fdindex_out=0;      // index of fdbuff output
byte fdbuff_in[256*16];	// fd intenal buffer input
byte fdbuff_out[256*16];// fd intenal buffer output
						// 256*16 : length of 1 Track

void dokodemo_save_disk(void)
{
	dokodemo_putsection("[DISK]");
	DOKODEMO_PUTENTRY_INT( fd_send);
	DOKODEMO_PUTENTRY_INT( fd_recv);
	DOKODEMO_PUTENTRY_INT( fdindex_in);
	DOKODEMO_PUTENTRY_INT( fdindex_out);
	dokodemo_putentry_buffer("fdbuff_in" , fdbuff_in  , 256*16);
	dokodemo_putentry_buffer("fdbuff_out", fdbuff_out , 256*16);
	
}

void dokodemo_load_disk(void)
{
	DOKODEMO_GETENTRY_INT( fd_send);
	DOKODEMO_GETENTRY_INT( fd_recv);
	DOKODEMO_GETENTRY_INT( fdindex_in);
	DOKODEMO_GETENTRY_INT( fdindex_out);
	dokodemo_getentry_buffer("fdbuff_in" , fdbuff_in  , 256*16);
	dokodemo_getentry_buffer("fdbuff_out", fdbuff_out , 256*16);
	
}

void dump(char *buff,int length);

void set_fd_send(int val)
{
 fd_send = val;
}

// ************************************
//      controll (PC -> FD)
// ************************************
//  ATN DAC RFD DAV X  DAC RFD DAV
//
//
void outD3( byte Value)
{
	// portD3=Value;
	if( Value ==SET_8255)	                                // 8255 mode set
		{
		 fd_send=fd_recv= fdindex_in= fdindex_out=0;

		//PRINTDEBUG(DSK_LOG,"disk: set 8255 mode \n");
		}

	if( Value ==PC_SEND_REQUEST+1)   // F     ATN信号ON　コマンド・データ送信要求   // PC karano SEND_REQUEST
		{
		fd_send = 1;
		portD2 |= FD_RECV_READY;     // FD RECV ready on   2
		fdindex_in=0;
		//PRINTDEBUG(DSK_LOG," F) PC_SEND_REQUEST on  fd_send=1 fdindex_in=0\n");
		}
 	if( Value ==PC_SEND_REQUEST)     // E     ATN信号OFF コマンド・データ送信要求	// PC gawa SEND DONE
		{
		fd_send = 1;
		//PRINTDEBUG(DSK_LOG," E) PC_SEND_REQUEST off fd_send=1\n");
		}
	if( Value ==PC_SEND_DONE+1)      // 9     DAV信号ON　コマンド・データ送信完了
		{
		fd_send= 1;
		portD2 |= FD_RECV_DONE;		 // FD RECV done on   4
		//PRINTDEBUG(DSK_LOG," 9) PC_SEND_DONE on    fd_send=1\n");
		}
	if( Value ==PC_SEND_DONE)        // 8     DAV信号OFF　コマンド・データ送信完了
		{
		fd_send= 0;
		fd_send= 1;		// fix me !!  １でないと、コラ娘 が動かない。。こら娘対策。。 
		
		// ----------- command execute ---------------
		portD2 &= ~FD_RECV_DONE;	// FD RECV done off  4
		if( ((fdbuff_in[0]==1)||(fdbuff_in[0]==0x11)) && fdindex_in ==2 ) 	// Write data
			{
			cmd_length[ fdbuff_in[0]]= 5+256*fdbuff_in[1];	// set length
			}
		//PRINTDEBUG(DSK_LOG," 8) PC_SEND_DONE off  fd_send=0 \n");

		if( fdindex_in == cmd_length[ fdbuff_in[0]])
			{
			exec_command();		// execute command
			PRINTDEBUG2(DSK_LOG,"[disk][outD3] command execute cmd no.= %d  fdindex_in=%d\n",fdbuff_in[0] , fdindex_in);
			}
		}


                                                                         // PC gawa Recive READY
	if( Value ==PC_RECV_READY+1)     // B      RFD信号ON　　コマンド・データ受信準備完了
		{
		fd_recv = 1;
		portD2 |= FD_SEND_DONE;		// FD Send done on  1
		//PRINTDEBUG(DSK_LOG," B) PC_RECV_READY on fd_recv=1\n");
		}
	if( Value ==PC_RECV_READY)       // A	　　RFD信号OFF　　コマンド・データ受信準備完了// PC gawa Recieve Ready off
		{
		fd_recv = 1;
		//PRINTDEBUG(DSK_LOG," A) PC_RECV_READY off  fd_recv=1\n");
		}
	if( Value ==PC_RECV_DONE+1)      // D      DAC信号ON　　コマンド・データ受信完了
		{
		fd_recv = 1;
		portD2 &= ~FD_SEND_DONE;		// FD Send done off  1
		//PRINTDEBUG(DSK_LOG," D) PC_RECV_DONE on    fd_recv=1\n");
		}
	if( Value ==PC_RECV_DONE)        // C      DAC信号OFF　　コマンド・データ受信完了
		{
		fd_recv = 0;
		//PRINTDEBUG(DSK_LOG," C) PC_RECV_DONE off   fd_recv=0\n");
		}
 }

// ************************************
//      controll (PC <- FD)
// ************************************
byte inpD2(void)
{
	int ret;
	if(DskStream)
		ret=portD2;
	else
		ret=NORAM;
	return(ret);
}

// ************************************
//       data (PC <- FD)
// ************************************
byte inpD0(void)
{
	byte Value = 0x2e;
	if( fdindex_out > 256*16) { return NORAM;} // out of range ...max Transfer is 16 Sector
	if(fd_recv==1)
		{
		Value=fdbuff_out[ fdindex_out++];
		//   printf("r=%02X",Value);
		}
	return (Value);
}

// ************************************
//      cmd/data (PC -> FD)
// ************************************
void outD1(byte Value)
{
	if( fdindex_in > 256*16) { printf("cmd/data: out of range %d\n",fdindex_in);return;}
	if(fd_send==1 || fdbuff_in[0]==0x11)	// when write sector (fast) command ,too
		{
		fdbuff_in[ fdindex_in++]= Value;
		PRINTDEBUG(DSK_LOG,"fdbuff_in =");
	   	dump( fdbuff_in, fdindex_in); // debug
    	}
	else
		{
	    portD1=Value;	// support for PC-6001Mk2 DISK -  書き込まれた値をそのまま返す。
	   }
}


// ************************************
//      controll output
// ************************************
void disk_out( byte Port, byte Value)
{
 	switch( Port)
    	{
    	case 0xD1: outD1( Value); break;
    	case 0xD3: outD3( Value); break;
    	}
}

// ************************************
//      controll input
// ************************************
byte disk_inp( byte Port)
{
	byte Value;
	switch( Port)
		{
		case 0xD0: Value = inpD0( ); break;
		case 0xD2: Value = inpD2(); break;
		case 0xD1: Value = portD1; break;	// suport for pc-6001MK2 disk
		}
	return( Value);
}


// ************************************
//      exec command
// ************************************
//    00:INZ
//    01:WRITE              N,DD,TT,SS,Data *N
//    02:READ               N,DD,TT,SS
//    03:SEND DATA          (Data *N)
//    04:COPY               N,(src)DD,TT,SS  (dest),DD,TT,SS
//    05:FORMAT DD
//    06:SEND RESULT STATUS  
//    07:SEND DRIVE  STATUS
// 0b 11:XMIT               ADDR, BYTECOUNT  (09Hと同じ　FDD->PCにメモリー転送）
// 0c 12:RCV                ADDR, BYTECOUNT
// 0e 14:LOAD               N,DD,TT,SS, ADDR
// 0f 15:SAVE               N,DD,TT,SS, ADDR
//
// DRIVE数 07H で、帰ってくる
// READ  02H で読み込んだ後、03H で、PC側に転送する
// WRITE 01H で、書き込むデータも一緒に渡す
enum {C_INZ ,C_WRITE ,C_READ , C_SENDDATA ,C_COPY ,C_FORMAT ,C_SENDRESULTSTATUS ,C_SENDDRIVESTATUS ,
C_DUMMY8  ,C_DUMMY9 ,C_DUMMYA , C_XMIT    , C_RCV , DUMMYD ,  C_LOAD            , C_SAVE , 
C_DUMMY10 , C_FASTWRITE , C_FASTREAD};

void exec_command(void)
 {
	int nn,dd,tt,ss;
	static int Result;	// 直前のコマンドの成否
	// byte *p=NULL;

	PutDiskAccessLamp(1);

	PRINTDEBUG(DSK_LOG, "[DISK]:");
	switch ( fdbuff_in[0])
		{
		case C_INZ: 
					PRINTDEBUG(DSK_LOG, "Initialize    \n");
					fdbuff_out[0]=0x10;break;

		case C_FASTWRITE/*0x11*/:  
					PRINTDEBUG(DSK_LOG, "(fast mode)");	// fast mode Write Sector
		case C_WRITE /*1*/:
					PRINTDEBUG(DSK_LOG, "Write Sector  \n");
					nn=fdbuff_in[1]; dd=fdbuff_in[2]; tt=fdbuff_in[3]; ss=fdbuff_in[4];
					if( !getDiskProtect( 0))		// FIX ME
						{
						write_d88sector(nn,dd,tt,ss-1,fdbuff_in+5);
						Result=0x40;
						}
					else
						{
						Result=0x01;		// not writeable
						}
                	break;

		case C_READ /*2*/:
			        PRINTDEBUG(DSK_LOG, "Read  Sector \n");
//            		fdbuff_out[0]=0x40;
					Result=0x40;
					break;

		case C_FASTREAD /*0x12*/:  PRINTDEBUG(DSK_LOG, "(fast mode)");	// fast mode  Send Data
		case C_SENDDATA /*3*/:
					nn=fdbuff_in[1]; dd=fdbuff_in[2]; tt=fdbuff_in[3]; ss=fdbuff_in[4];
					PRINTDEBUG4(DSK_LOG, "Send  Data   sectors=%d drive=%d track=%d sector=%d \n",nn,dd,tt,ss);
					memcpy( fdbuff_out, read_d88sector( nn,dd ,tt ,ss-1), 256*nn);
					Result=0x40;
					break;

		case C_SENDRESULTSTATUS /*6*/:
				    PRINTDEBUG(DSK_LOG, "Result Status \n");
//                  fdbuff_out[0]=0x40;
					fdbuff_out[0]= Result;
                	break;

		case C_SENDDRIVESTATUS /*7*/:
				    PRINTDEBUG(DSK_LOG, "Drive  Status \n");
					switch (disk_num) 		// drive numbers   0: 00h  1: 10h  2: 30h
						{
						case 0: fdbuff_out[0]= 0x00; break;
						case 1: fdbuff_out[0]= 0x10; break;
						case 2: fdbuff_out[0]= 0x30; break;
						}
				    break;

				// if 1DD disk then set 00h  ( 高速モード   fast mode)
				// if 1D  disk then set FFh  ( 非高速モード normal mode)
				// 本当は、高速モードと ディスクタイプは関係ないのですが、強引に実装しました。
			case C_XMIT /*0xB*/:
					PRINTDEBUG(DSK_LOG, "XMit   \n");
					fdbuff_out[0]=(is1DD( cur_drive))?  0 : 0xff ;break;
				// 両面/片面設定
				// b1  ドライブ１  1:両面　0:片面
				// b0  ドライブ０  1:画面  0:片面
		case 0x17:  PRINTDEBUG(DSK_LOG, "SetSurfaceMode"); break;
		default:    printf("Not implimented %d",fdbuff_in[0]);break;
		}
	fdindex_out=0;

	//OSD_Delay(10);
	PutDiskAccessLamp(0);
   //  dump( fdbuff_in , cmd_length[ fdbuff_in[0]]);
 }




// **********************************************
//     dump    for debug
// **********************************************
void dump(char *buff,int length)
 {
#if DSK_LOG
  int i;
  printf("%d\t ",length);
  for(i=0; i< length; i++)
      {
       printf("%02x ",*(buff+i) & 0xff);
      }
  printf("\n");
#endif
   }

