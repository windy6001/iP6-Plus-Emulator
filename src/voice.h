/** iP6: PC-6000/6600 series emualtor ************************/
/**                                                         **/
/**                         voice.h                         **/
/**                                                         **/
/** translate from PC6001V's C++ to C  by windy             **/
/**                                                         **/
/** Original program:   PC6001V                             **/
/** Original author:    Yumitaro.                           **/
/** Original URL:       http://www.papicom.net/p6v/         **/
/*************************************************************/
#ifndef VOICE_H_INCLUDED
#define VOICE_H_INCLUDED

#include "Z80.h"

// [OUT]
//  E0H : 音声パラメータ転送
//  E1H : ?
//  E2H : モードセット : 0 0 0 0 F S1 S0
//			F     : 分析フレーム周期 0: 10ms/フレーム
//									 1: 20ms/フレーム
//			S1,S0 : 発声速度BIT     00: NORMAL SPEED
//									01: SLOW SPEED
//									10: FAST SPEED
//									11: 禁止

//  E3H : コマンドセット
//		・内部句選択コマンド : 0  0 P5 P4 P3 P2 P1 P0
//			P5-P0 : 句選択ビット(0-63)
//		・ストップコマンド   : 1  1  1  1  1  1  1  1
//		・外部句選択コマンド : 1  1  1  1  1  1  1  0
//

// [IN]
//  E0H : ステータスレジスタ : BSY REQ ~INT/EXT ERR 0 0 0 0
//			BSY      : 音声合成 1:処理中 0:停止中
//			REQ      : 音声パラメータ 1:入力要求 0:禁止
//			~INT/EXT : メッセージデータ 1:外部 0:内部
//			ERR      : エラーフラグ 1:エラー 0:エラーなし
//  E1H : ?
//  E2H : PortE2に書込んだ値?
//  E3H : PortE3に書込んだ値?


// 処理タイミングに関する挙動
// [本来のスペック]
// ・コマンド発行から1フレーム目のパラメータ入力完了まで2ms以内(NORMAL SPEED)
//   【疑問】コマンド発行からREQまでの時間は?
// ・フレーム内で最初のREQから全パラメータ入力まで3/4フレーム時間以内
//   【考察】入力完了から発声までは1/4フレーム時間以内で確実に処理される?
//   【疑問】次のREQまでの時間は?
//
// [実装スペック]
// ・コマンド発行から1フレーム目のパラメータ入力完了まで1フレーム時間以内
// ・1フレーム時間経過時点で発声(1フレーム分のサンプル生成)，同時にREQ=1
// ・発声から次フレームのパラメータ入力完了まで1フレーム時間以内

// translated into C for Cocoa iP6 by Koichi Nishida 2004
// 内部句選択コマンドには未対応

// Voice処理中1になるフラグ
#ifdef __cplusplus
extern "C" {
#endif

	extern int VoiceFlag;

	// 初期化
	int InitVoice(void);
	// 後処理
	void TrashVoice(void);
	// Voice スレッドをキャンセルする
	void CancelVoiceThread(void);

	// I/Oアクセス関数
	void OutE0H(byte data);
	void OutE2H(byte data);
	void OutE3H(byte data);
	byte InE0H(void);
	byte InE2H(void);
	byte InE3H(void);

#ifdef __cplusplus
}
#endif

#endif	// VOICE_H_INCLUDED
