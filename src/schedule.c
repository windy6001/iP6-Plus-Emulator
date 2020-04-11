/** iP6: PC-6000/6600 series emualtor ************************/
/**                                                         **/
/**                         schedule.c                      **/
/**                                                         **/
/** translate from PC6001V's C++ to C  by windy             **/
/**                                                         **/
/** Original program:   PC6001V                             **/
/** Original author:    Yumitaro.                           **/
/** Original URL:       http://www.papicom.net/p6v/         **/
/*************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include "schedule.h"
#include "types.h"
#include "P6.h"



evinfo   ev[ MAX_EVENT];
int      SaveCloclk;	// 次のイベント発生用
int      WRClockTmp;	// 実行速度計算用
int		 NextEvent;

void Event_init(void)
{
	int i;
	// 全てのイベントを削除しておく
	for(i=0; i< MAX_EVENT; i++)
		Event_Del( i);
	SaveCloclk =0;	// 次のイベント発生用
	WRClockTmp =0;	// 実行速度計算用
	NextEvent  =0;
}

// ****************************************************************************
// イベント追加
//
// 引数:	dev		イベントデバイスオブジェクトポインタ
//			id		イベントID
//			hz		イベント発生周波数
//			flag	イベントスタイル指定
//				bit0  : 繰り返し指示 0:ワンタイム 1:ループ
//				bit2,3,4: 周期指定単位 000:Hz 001:us 010:ms 100:CPUステート数
// 返値:	BOOL	TRUE:成功 FALSE:失敗
// ****************************************************************************
int Event_Add( int id, double hz ,int flag, int (SCALL *CallbackProc)( int id, void*) )
{
	double master_clock= (( (double)(2000000.0*0.58) *86)/100);
//	if(sr_mode)
//		master_clock *= 1.18 ; // 0.9; // 1.0; // 1.27; // 1.15; // 1.2; // 1.3;		// sr_mode ?
//	else
//	    master_clock *= 1.0; // 1.05;// 0.85; // 0.65;  // 0.68; // 0.77;  // 0.85;  


	switch( flag&(EV_US|EV_MS|EV_STATE) )
		{
		case EV_MS:		// msで指示
			ev[id].Active = TRUE;
			ev[id].nps    = (double)1000 / hz;
			ev[id].Clock  = (int)((double)master_clock / ev[id].nps);
			break;
		default:	// Hz で指定
			ev[id].Active = TRUE;
			ev[id].nps   = hz;
			ev[id].Clock = (int)((double)(master_clock / hz));
			break;
		}

				// CALLBACK 関数の設定
	ev[id].CallbackProc = CallbackProc;

				// 周期の設定
	if( flag & EV_LOOP )	// ループイベント
		{
		ev[id].Period = ev[id].Clock;
		if( ev[id].Period < 1 ) ev[id].Period = 1;
	
		}
	else				// ワンタイムイベント
		ev[id].Period = 0;

	// 次のイベントまでのクロック数更新
	if( NextEvent < 0 ) 
		NextEvent = ev[id].Clock;
	else
		NextEvent = min( NextEvent, ev[id].Clock );
			
	return 1;
}

// ****************************************************************************
// 指定イベントをリセットする
//
// 引数:	dev		イベントデバイスオブジェクトポインタ
//			id		イベントID
// 返値:	なし
// ****************************************************************************
void Event_Reset( int id )
{
	if( ev[id].Active ){
		// 溜め込んだクロック分を考慮
		ev[id].Clock = ev[id].Period + SaveCloclk;
	}
}

// ****************************************************************************
// イベント削除
//
// 引数:	dev		イベントデバイスオブジェクトポインタ
//			id		イベントID
// 返値:	BOOL	TRUE:成功 FALSE:失敗
// ****************************************************************************
void Event_Del(int id)
{
	ev[id].Active = 0;
	ev[id].CallbackProc = NULL;
}

// ****************************************************************************
// イベント更新
//
// 引数:	clk		進めるクロック数
// 返値:	なし
// ****************************************************************************
int Event_Update(int clk)
{
	int i;
	int cnt;

	// クロックを溜め込む
	SaveCloclk += clk;	// 次のイベント発生用
	WRClockTmp += clk;	// 実行速度計算用
	
	// 次のイベント発生クロックに達していなかったら戻る
	// ただし clk=0 の場合は更新を行う
	if( NextEvent > SaveCloclk && clk )
		return 0;
	NextEvent = -1;
	

	do{
		cnt = 0;
		for( i=0; i<MAX_EVENT; i++ )
			{
			// 有効なイベント?
			if( ev[i].Active )
				{
				ev[i].Clock -= SaveCloclk;
				// 更新間隔が長い場合は複数回発生する可能性あり
				// とりあえず全てこなすまで繰り返すってことでいいのか?
				if( ev[i].Clock <= 0 ){
					// イベントコールバックを実行
					if( ev[i].CallbackProc !=NULL)
						ev[i].CallbackProc(ev[i].id,0);
					
					if( ev[i].Period > 0 ){	// ループイベント
						ev[i].Clock += ev[i].Period;
						if( ev[i].Clock <= 0 ) 
							cnt++;	// 次のイベントも発生していたらカウント
					}else{					// ワンタイムイベント
						Event_Del( i );
						break;
					}
				}
				// 次のイベントまでのクロック数更新
				if( NextEvent < 0 ) 
					NextEvent = ev[i].Clock;
				else
					NextEvent = min( NextEvent, ev[i].Clock );
			}
		}
		SaveCloclk = 0;
	}while( cnt > 0 );
	return 0;
}


// ****************************************************************************
// イベントの進行率を求める
//   百分率を10倍して返す(100%が1000)
//   0以下,100以上になる場合は0-100に丸める
//
// 引数:	dev		イベントデバイスオブジェクトポインタ
//			id		イベントID
// 返値:	double	イベント進行率(1.0が100%)
// ****************************************************************************
double Event_Scale( int id )
{
	// イベントが存在し1周期のクロック数が設定されている?
	if( ev[id].Active == TRUE && ev[id].Period > 0 )
		{
		// 溜め込んだクロックを考慮
		double sc = (double)( (double)( ev[id].Period - ev[id].Clock + SaveCloclk ) / (double)ev[id].Period );
		return min( max( 0.0, sc ), 1.0 );
		}
	else
		return 0;
}

// ****************************************************************************
// イベントが登録されているか？
// 引数:	id		イベントID
// 返値:	1: 登録されている　　0:未登録
// ****************************************************************************

int Event_isActive(int id)
{
	return( ev[id].Active );
}

/*Event_Update(int clk)
{
	int i;
	for(i=0; i< MAX_EVENT ;i++)
		{
		if( ev[id].Enable)
			{
			ev[id].Clock -= clk;
			if( ev[id].Clock <0)
				{
				ev[id].CallbackProc();
				ev[id].Clock += ev.Period;
				}
			}
		}
}
*/


