/** iP6: PC-6000/6600 series emualtor ************************/
/**          Windows Direct Input                           **/
/**          name is winInput.cpp                           **/
/**                                                         **/
/**          by Windy                                       **/
/*************************************************************/


/*************************************************************/
/*		Direct Input Variable                                */
/*************************************************************/
#include <stdio.h>
#define DIRECTINPUT_VERSION     0x0800          // DirectInputのバージョン指定
#include <dinput.h>
#include "WinInput.h"

// ライブラリリンク
#pragma comment(lib, "dinput8.lib")
#pragma comment(lib, "dxguid.lib")


// グローバル変数定義
LPDIRECTINPUT8			lpDI = NULL;			// IDirectInput8
LPDIRECTINPUTDEVICE8	lpKeyboard = NULL;		// キーボードデバイス



/************************************************************/
/*    init Direct Input                                     */
/************************************************************/
int init_directinput(HWND hWnd ,HINSTANCE hInstance)
{
	// IDirectInput8の作成
	HRESULT ret = DirectInput8Create(hInstance, DIRECTINPUT_VERSION, IID_IDirectInput8, (LPVOID*)&lpDI, NULL);
	if (FAILED(ret)) {
		// 作成に失敗
		printf("DirectInput8の作成に失敗\n");
		return -1;
	}

	// IDirectInputDevice8の取得
	ret = lpDI->CreateDevice(GUID_SysKeyboard, &lpKeyboard, NULL);
	if (FAILED(ret)) {
		printf("キーボードデバイスの作成に失敗\n");
		lpDI->Release();
		return -1;
	}

	// 入力データ形式のセット
	ret = lpKeyboard->SetDataFormat(&c_dfDIKeyboard);
	if (FAILED(ret)) {
		printf("入力データ形式のセット失敗\n");
		lpKeyboard->Release();
		lpDI->Release();
		return -1;
	}

	// 排他制御のセット
	ret = lpKeyboard->SetCooperativeLevel(hWnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE | DISCL_NOWINKEY);
	if (FAILED(ret)) {
		printf("排他制御のセット失敗\n");
		lpKeyboard->Release();
		lpDI->Release();
		return -1;
	}

	// 動作開始
	lpKeyboard->Acquire();
	return 0;
}


/************************************************************/
/*    get stick   (Direct Input)                            */
/*                                                          */
/************************************************************/
BYTE OSD_GetStickKeyboard(void)
{
	byte ret=0;

	// キーの入力
	BYTE key[256];
	ZeroMemory(key, sizeof(key));
	ret = lpKeyboard->GetDeviceState(sizeof(key), key);
	if (FAILED(ret)) {
		// 失敗なら再開させてもう一度取得
		lpKeyboard->Acquire();
		lpKeyboard->GetDeviceState(sizeof(key), key);
	}

	if (key[DIK_SPACE] & 0x80) {
		ret |= 1 << 7;
	}
	if (key[DIK_LEFT]  & 0x80) {
		ret |= 1 << 5;
	}
	if (key[DIK_RIGHT] & 0x80) {
		ret |= 1 << 4;
	}
	if (key[DIK_DOWN] & 0x80) {
		ret |= 1 << 3;
	}
	if (key[DIK_UP]    & 0x80) {
		ret |= 1 << 2;
	}
	if (key[DIK_END] & 0x80) {
		ret |= 1 << 1;
	}
	if (key[DIK_LSHIFT] & 0x80 || key[DIK_RSHIFT] & 0x80) {
		ret |= 1;
	}
	return ret;
}
