/*
Copyright (c) 2020 Windy

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/


/** iP6: PC-6000/6600 series emualtor ************************/
/**          ローマ字変換                                   **/
/**          name is romaji.c                               **/
/**                                                         **/
/**          by Windy                                       **/
/*************************************************************/
#include "keys.h"
#include "P6.h"
#include "schedule.h"
#include "mem.h"
#include "romaji.h"

struct _romaji_tbl {
    int num;
    char romaji[4];
    int  keycode[7];
};

// ****************************************************************************
//          ローマ字変換テーブル
// ****************************************************************************
static struct _romaji_tbl romaji_tbl[] = {

{1,{ 0      ,0       ,0     ,0}, {SHIFT_DOWN , OSDK_Z , SHIFT_UP  ,0}},	// っ
{1,{ 0      ,0       ,0     ,0}, {OSDK_Y , 0 , 0  ,0}},	// ん

{1,{ OSDK_A , 0      , 0    ,0}, {OSDK_3 , 0,0,0}},		// あいうえお
{1,{ OSDK_I , 0      , 0    ,0}, {OSDK_E , 0,0,0}},
{1,{ OSDK_U , 0      , 0    ,0}, {OSDK_4 , 0,0,0}},
{1,{ OSDK_E , 0      , 0    ,0}, {OSDK_5 , 0,0,0}},
{1,{ OSDK_O , 0      , 0    ,0}, {OSDK_6 , 0,0,0}},

{2,{ OSDK_K ,OSDK_A  , 0    ,0}, {OSDK_T , 0,0,0}},		// かきくけこ
{2,{ OSDK_K ,OSDK_I  , 0    ,0}, {OSDK_G , 0,0,0}},
{2,{ OSDK_K ,OSDK_U  , 0    ,0}, {OSDK_H , 0,0,0}},
{2,{ OSDK_K ,OSDK_E  , 0    ,0}, {OSDK_COLON , 0,0,0}},
{2,{ OSDK_K ,OSDK_O  , 0    ,0}, {OSDK_B , 0,0,0}},

{2,{ OSDK_S ,OSDK_A  , 0    ,0}, {OSDK_X , 0,0,0}},		// さしすせそ
{2,{ OSDK_S ,OSDK_I  , 0    ,0}, {OSDK_D , 0,0,0}},
{2,{ OSDK_S ,OSDK_U  , 0    ,0}, {OSDK_R , 0,0,0}},
{2,{ OSDK_S ,OSDK_E  , 0    ,0}, {OSDK_P , 0,0,0}},
{2,{ OSDK_S ,OSDK_O  , 0    ,0}, {OSDK_C , 0,0,0}},

{2,{ OSDK_T ,OSDK_A  , 0    ,0}, {OSDK_Q , 0,0,0}},		// たちつてと
{2,{ OSDK_T ,OSDK_I  , 0    ,0}, {OSDK_A , 0,0,0}},
{2,{ OSDK_C ,OSDK_I  , 0    ,0}, {OSDK_A , 0,0,0}},
{2,{ OSDK_T ,OSDK_U  , 0    ,0}, {OSDK_Z , 0,0,0}},
{2,{ OSDK_T ,OSDK_E  , 0    ,0}, {OSDK_W , 0,0,0}},
{2,{ OSDK_T ,OSDK_O  , 0    ,0}, {OSDK_S , 0,0,0}},

{2,{ OSDK_N ,OSDK_A  , 0    ,0}, {OSDK_U , 0,0,0}},		// なにぬねの
{2,{ OSDK_N ,OSDK_I  , 0    ,0}, {OSDK_I , 0,0,0}},
{2,{ OSDK_N ,OSDK_U  , 0    ,0}, {OSDK_1 , 0,0,0}},
{2,{ OSDK_N ,OSDK_E  , 0    ,0}, {OSDK_COMMA , 0,0,0}},
{2,{ OSDK_N ,OSDK_O  , 0    ,0}, {OSDK_K , 0,0,0}},

{2,{ OSDK_H ,OSDK_A  , 0    ,0}, {OSDK_F , 0,0,0}},		//  はひふへほ
{2,{ OSDK_H ,OSDK_I  , 0    ,0}, {OSDK_V , 0,0,0}},
{2,{ OSDK_H ,OSDK_U  , 0    ,0}, {OSDK_2 , 0,0,0}},
{2,{ OSDK_H ,OSDK_E  , 0    ,0}, {OSDK_UPPER , 0,0,0}},
{2,{ OSDK_H ,OSDK_O  , 0    ,0}, {OSDK_MINUS , 0,0,0}},

{2,{ OSDK_F ,OSDK_A  , 0    ,0}, {OSDK_2 , SHIFT_DOWN ,OSDK_3,SHIFT_UP}},		//ふぁふぃふふぇふぉ
{2,{ OSDK_F ,OSDK_I  , 0    ,0}, {OSDK_2 , SHIFT_DOWN ,OSDK_E,SHIFT_UP}},
{2,{ OSDK_F ,OSDK_U  , 0    ,0}, {OSDK_2 , 0,0,0}},
{2,{ OSDK_F ,OSDK_E  , 0    ,0}, {OSDK_2 , SHIFT_DOWN ,OSDK_5,SHIFT_UP}},
{2,{ OSDK_F ,OSDK_O  , 0    ,0}, {OSDK_2 , SHIFT_DOWN ,OSDK_6,SHIFT_UP}},

{2,{ OSDK_M ,OSDK_A  , 0    ,0}, {OSDK_J , 0,0,0}},				// まみむめも
{2,{ OSDK_M ,OSDK_I  , 0    ,0}, {OSDK_N , 0,0,0}},
{2,{ OSDK_M ,OSDK_U  , 0    ,0}, {OSDK_RIGHTBRACKET , 0,0,0}},
{2,{ OSDK_M ,OSDK_E  , 0    ,0}, {OSDK_SLASH , 0,0,0}},
{2,{ OSDK_M ,OSDK_O  , 0    ,0}, {OSDK_M , 0,0,0}},

{2,{ OSDK_Y ,OSDK_A  , 0    ,0}, {OSDK_7 , 0,0,0}},				// やゆよ
{2,{ OSDK_Y ,OSDK_U  , 0    ,0}, {OSDK_8 , 0,0,0}},
{2,{ OSDK_Y ,OSDK_O  , 0    ,0}, {OSDK_9 , 0,0,0}},

{2,{ OSDK_R ,OSDK_A  , 0    ,0}, {OSDK_O , 0,0,0}},				// らりるれろ
{2,{ OSDK_R ,OSDK_I  , 0    ,0}, {OSDK_L , 0,0,0}},
{2,{ OSDK_R ,OSDK_U  , 0    ,0}, {OSDK_PERIOD , 0,0,0}},
{2,{ OSDK_R ,OSDK_E  , 0    ,0}, {OSDK_SEMICOLON , 0,0,0}},
{2,{ OSDK_R ,OSDK_O  , 0    ,0}, {OSDK_UNDERSCORE , 0,0,0}},


{2,{ OSDK_W ,OSDK_A  , 0    ,0}, {OSDK_0 , 0,0,0}},				// わうぃうぇをん
{2,{ OSDK_W ,OSDK_I  , 0    ,0}, {OSDK_4 , SHIFT_DOWN, OSDK_E, SHIFT_UP}},
{2,{ OSDK_W ,OSDK_E  , 0    ,0}, {OSDK_4 , SHIFT_DOWN, OSDK_5, SHIFT_UP}},

{2,{ OSDK_W ,OSDK_O  , 0    ,0}, {SHIFT_DOWN , OSDK_0 , SHIFT_UP , 0}},
{2,{ OSDK_N ,OSDK_N  , 0    ,0}, {OSDK_Y , 0,0,0}},
{3,{ OSDK_N ,OSDK_N  ,OSDK_N,0}, {OSDK_Y , 0,0,0}},


{3,{ OSDK_K ,OSDK_Y  ,OSDK_A ,0}, {OSDK_G , SHIFT_DOWN , OSDK_7 , SHIFT_UP}},	// きゃきぃきゅきぇきょ
{3,{ OSDK_K ,OSDK_Y  ,OSDK_I ,0}, {OSDK_G , SHIFT_DOWN , OSDK_E , SHIFT_UP}},
{3,{ OSDK_K ,OSDK_Y  ,OSDK_U ,0}, {OSDK_G , SHIFT_DOWN , OSDK_8 , SHIFT_UP}},
{3,{ OSDK_K ,OSDK_Y  ,OSDK_E ,0}, {OSDK_G , SHIFT_DOWN , OSDK_5 , SHIFT_UP}},
{3,{ OSDK_K ,OSDK_Y  ,OSDK_O ,0}, {OSDK_G , SHIFT_DOWN , OSDK_9 , SHIFT_UP}},

{3,{ OSDK_S ,OSDK_Y  ,OSDK_A ,0}, {OSDK_D , SHIFT_DOWN , OSDK_7 , SHIFT_UP}},	// しゃしぃしゅしぇしょ
{3,{ OSDK_S ,OSDK_Y  ,OSDK_I ,0}, {OSDK_D , SHIFT_DOWN , OSDK_E , SHIFT_UP}},
{3,{ OSDK_S ,OSDK_Y  ,OSDK_U ,0}, {OSDK_D , SHIFT_DOWN , OSDK_8 , SHIFT_UP}},
{3,{ OSDK_S ,OSDK_Y  ,OSDK_E ,0}, {OSDK_D , SHIFT_DOWN , OSDK_5 , SHIFT_UP}},
{3,{ OSDK_S ,OSDK_Y  ,OSDK_O ,0}, {OSDK_D , SHIFT_DOWN , OSDK_9 , SHIFT_UP}},

{3,{ OSDK_T ,OSDK_Y  ,OSDK_A ,0}, {OSDK_A , SHIFT_DOWN , OSDK_7 , SHIFT_UP}},	// ちゃちぃちゅちぇちょ
{3,{ OSDK_T ,OSDK_Y  ,OSDK_I ,0}, {OSDK_A , SHIFT_DOWN , OSDK_E , SHIFT_UP}},
{3,{ OSDK_T ,OSDK_Y  ,OSDK_U ,0}, {OSDK_A , SHIFT_DOWN , OSDK_8 , SHIFT_UP}},
{3,{ OSDK_T ,OSDK_Y  ,OSDK_E ,0}, {OSDK_A , SHIFT_DOWN , OSDK_5 , SHIFT_UP}},
{3,{ OSDK_T ,OSDK_Y  ,OSDK_O ,0}, {OSDK_A , SHIFT_DOWN , OSDK_9 , SHIFT_UP}},

{3,{ OSDK_C ,OSDK_Y  ,OSDK_A ,0}, {OSDK_A , SHIFT_DOWN , OSDK_7 , SHIFT_UP}},	// ちゃちぃちゅちぇちょ
{3,{ OSDK_C ,OSDK_Y  ,OSDK_I ,0}, {OSDK_A , SHIFT_DOWN , OSDK_E , SHIFT_UP}},
{3,{ OSDK_C ,OSDK_Y  ,OSDK_U ,0}, {OSDK_A , SHIFT_DOWN , OSDK_8 , SHIFT_UP}},
{3,{ OSDK_C ,OSDK_Y  ,OSDK_E ,0}, {OSDK_A , SHIFT_DOWN , OSDK_5 , SHIFT_UP}},
{3,{ OSDK_C ,OSDK_Y  ,OSDK_O ,0}, {OSDK_A , SHIFT_DOWN , OSDK_9 , SHIFT_UP}},

{3,{ OSDK_N ,OSDK_Y  ,OSDK_A ,0}, {OSDK_I , SHIFT_DOWN , OSDK_7 , SHIFT_UP}},	// にゃにぃにゅにぇにょ
{3,{ OSDK_N ,OSDK_Y  ,OSDK_I ,0}, {OSDK_I , SHIFT_DOWN , OSDK_E , SHIFT_UP}},
{3,{ OSDK_N ,OSDK_Y  ,OSDK_U ,0}, {OSDK_I , SHIFT_DOWN , OSDK_8 , SHIFT_UP}},
{3,{ OSDK_N ,OSDK_Y  ,OSDK_E ,0}, {OSDK_I , SHIFT_DOWN , OSDK_5 , SHIFT_UP}},
{3,{ OSDK_N ,OSDK_Y  ,OSDK_O ,0}, {OSDK_I , SHIFT_DOWN , OSDK_9 , SHIFT_UP}},

{3,{ OSDK_H ,OSDK_Y  ,OSDK_A ,0}, {OSDK_V , SHIFT_DOWN , OSDK_7 , SHIFT_UP}},	// ひゃひぃひゅひぇひょ
{3,{ OSDK_H ,OSDK_Y  ,OSDK_I ,0}, {OSDK_V , SHIFT_DOWN , OSDK_E , SHIFT_UP}},
{3,{ OSDK_H ,OSDK_Y  ,OSDK_U ,0}, {OSDK_V , SHIFT_DOWN , OSDK_8 , SHIFT_UP}},
{3,{ OSDK_H ,OSDK_Y  ,OSDK_E ,0}, {OSDK_V , SHIFT_DOWN , OSDK_5 , SHIFT_UP}},
{3,{ OSDK_H ,OSDK_Y  ,OSDK_O ,0}, {OSDK_V , SHIFT_DOWN , OSDK_9 , SHIFT_UP}},

{3,{ OSDK_M ,OSDK_Y  ,OSDK_A ,0}, {OSDK_N , SHIFT_DOWN , OSDK_7 , SHIFT_UP}},	// みゃみぃみゅみぇみょ
{3,{ OSDK_M ,OSDK_Y  ,OSDK_I ,0}, {OSDK_N , SHIFT_DOWN , OSDK_E , SHIFT_UP}},
{3,{ OSDK_M ,OSDK_Y  ,OSDK_U ,0}, {OSDK_N , SHIFT_DOWN , OSDK_8 , SHIFT_UP}},
{3,{ OSDK_M ,OSDK_Y  ,OSDK_E ,0}, {OSDK_N , SHIFT_DOWN , OSDK_5 , SHIFT_UP}},
{3,{ OSDK_M ,OSDK_Y  ,OSDK_O ,0}, {OSDK_N , SHIFT_DOWN , OSDK_9 , SHIFT_UP}},

{3,{ OSDK_R ,OSDK_Y  ,OSDK_A ,0}, {OSDK_L , SHIFT_DOWN , OSDK_7 , SHIFT_UP}},	// りゃりぃりゅりぇりょ
{3,{ OSDK_R ,OSDK_Y  ,OSDK_I ,0}, {OSDK_L , SHIFT_DOWN , OSDK_E , SHIFT_UP}},
{3,{ OSDK_R ,OSDK_Y  ,OSDK_U ,0}, {OSDK_L , SHIFT_DOWN , OSDK_8 , SHIFT_UP}},
{3,{ OSDK_R ,OSDK_Y  ,OSDK_E ,0}, {OSDK_L , SHIFT_DOWN , OSDK_5 , SHIFT_UP}},
{3,{ OSDK_R ,OSDK_Y  ,OSDK_O ,0}, {OSDK_L , SHIFT_DOWN , OSDK_9 , SHIFT_UP}},

{3,{ OSDK_J ,OSDK_Y  ,OSDK_A ,0}, {OSDK_D , OSDK_AT ,SHIFT_DOWN , OSDK_7 , SHIFT_UP}},	// じゃじぃじゅじぇじょ
{3,{ OSDK_J ,OSDK_Y  ,OSDK_I ,0}, {OSDK_D , OSDK_AT ,SHIFT_DOWN , OSDK_E , SHIFT_UP}},
{3,{ OSDK_J ,OSDK_Y  ,OSDK_U ,0}, {OSDK_D , OSDK_AT ,SHIFT_DOWN , OSDK_8 , SHIFT_UP}},
{3,{ OSDK_J ,OSDK_Y  ,OSDK_E ,0}, {OSDK_D , OSDK_AT ,SHIFT_DOWN , OSDK_5 , SHIFT_UP}},
{3,{ OSDK_J ,OSDK_Y  ,OSDK_O ,0}, {OSDK_D , OSDK_AT ,SHIFT_DOWN , OSDK_9 , SHIFT_UP}},

{2,{ OSDK_J ,OSDK_A  ,0      ,0}, {OSDK_D , OSDK_AT ,SHIFT_DOWN , OSDK_7 , SHIFT_UP}},	// じゃじじゅじぇじょ
{2,{ OSDK_J ,OSDK_I  ,0      ,0}, {OSDK_D , OSDK_AT ,0          , 0      , 0       }},
{2,{ OSDK_J ,OSDK_U  ,0      ,0}, {OSDK_D , OSDK_AT ,SHIFT_DOWN , OSDK_8 , SHIFT_UP}},
{2,{ OSDK_J ,OSDK_E  ,0      ,0}, {OSDK_D , OSDK_AT ,SHIFT_DOWN , OSDK_5 , SHIFT_UP}},
{2,{ OSDK_J ,OSDK_O  ,0      ,0}, {OSDK_D , OSDK_AT ,SHIFT_DOWN , OSDK_9 , SHIFT_UP}},

{2,{ OSDK_G ,OSDK_A  ,0      ,0}, {OSDK_T , OSDK_AT    , 0      , 0       }},	// がぎぐげご
{2,{ OSDK_G ,OSDK_I  ,0      ,0}, {OSDK_G , OSDK_AT    , 0      , 0       }},
{2,{ OSDK_G ,OSDK_U  ,0      ,0}, {OSDK_H , OSDK_AT    , 0      , 0       }},
{2,{ OSDK_G ,OSDK_E  ,0      ,0}, {OSDK_COLON , OSDK_AT, 0      , 0       }},
{2,{ OSDK_G ,OSDK_O  ,0      ,0}, {OSDK_B , OSDK_AT    , 0      , 0       }},


{2,{ OSDK_Z ,OSDK_A  ,0      ,0}, {OSDK_X , OSDK_AT    , 0      , 0       }},	// ざじずぜぞ
{2,{ OSDK_Z ,OSDK_I  ,0      ,0}, {OSDK_D , OSDK_AT    , 0      , 0       }},
{2,{ OSDK_Z ,OSDK_U  ,0      ,0}, {OSDK_R , OSDK_AT    , 0      , 0       }},
{2,{ OSDK_Z ,OSDK_E  ,0      ,0}, {OSDK_P , OSDK_AT    , 0      , 0       }},
{2,{ OSDK_Z ,OSDK_O  ,0      ,0}, {OSDK_C , OSDK_AT    , 0      , 0       }},

{2,{ OSDK_D ,OSDK_A  ,0      ,0}, {OSDK_Q , OSDK_AT    , 0      , 0       }},	// だぢづでど
{2,{ OSDK_D ,OSDK_I  ,0      ,0}, {OSDK_A , OSDK_AT    , 0      , 0       }},
{2,{ OSDK_D ,OSDK_U  ,0      ,0}, {OSDK_Z , OSDK_AT    , 0      , 0       }},
{2,{ OSDK_D ,OSDK_E  ,0      ,0}, {OSDK_W , OSDK_AT    , 0      , 0       }},
{2,{ OSDK_D ,OSDK_O  ,0      ,0}, {OSDK_S , OSDK_AT    , 0      , 0       }},

{2,{ OSDK_B ,OSDK_A  ,0      ,0}, {OSDK_F , OSDK_AT    , 0      , 0       }},	// ばびぶべぼ
{2,{ OSDK_B ,OSDK_I  ,0      ,0}, {OSDK_V , OSDK_AT    , 0      , 0       }},
{2,{ OSDK_B ,OSDK_U  ,0      ,0}, {OSDK_2 , OSDK_AT    , 0      , 0       }},
{2,{ OSDK_B ,OSDK_E  ,0      ,0}, {OSDK_UPPER , OSDK_AT, 0      , 0       }},
{2,{ OSDK_B ,OSDK_O  ,0      ,0}, {OSDK_MINUS , OSDK_AT, 0      , 0       }},

{2,{ OSDK_P ,OSDK_A  ,0      ,0}, {OSDK_F , OSDK_LEFTBRACKET    , 0      , 0       }},	// ぱぴぷぺぽ
{2,{ OSDK_P ,OSDK_I  ,0      ,0}, {OSDK_V , OSDK_LEFTBRACKET    , 0      , 0       }},
{2,{ OSDK_P ,OSDK_U  ,0      ,0}, {OSDK_2 , OSDK_LEFTBRACKET    , 0      , 0       }},
{2,{ OSDK_P ,OSDK_E  ,0      ,0}, {OSDK_UPPER , OSDK_LEFTBRACKET, 0      , 0       }},
{2,{ OSDK_P ,OSDK_O  ,0      ,0}, {OSDK_MINUS , OSDK_LEFTBRACKET, 0      , 0       }},


{3,{ OSDK_J ,OSDK_Y  ,OSDK_A ,0}, {OSDK_X , OSDK_AT    , SHIFT_DOWN, OSDK_7,SHIFT_UP}},	// じゃじぃじゅじぇじょ
{3,{ OSDK_J ,OSDK_Y  ,OSDK_I ,0}, {OSDK_D , OSDK_AT    , SHIFT_DOWN, OSDK_E,SHIFT_UP}},
{3,{ OSDK_J ,OSDK_Y  ,OSDK_U ,0}, {OSDK_R , OSDK_AT    , SHIFT_DOWN, OSDK_8,SHIFT_UP}},
{3,{ OSDK_J ,OSDK_Y  ,OSDK_E ,0}, {OSDK_P , OSDK_AT    , SHIFT_DOWN, OSDK_5,SHIFT_UP}},
{3,{ OSDK_J ,OSDK_Y  ,OSDK_O ,0}, {OSDK_C , OSDK_AT    , SHIFT_DOWN, OSDK_9,SHIFT_UP}},

{3,{ OSDK_Z ,OSDK_Y  ,OSDK_A ,0}, {OSDK_X , OSDK_AT    , SHIFT_DOWN, OSDK_7,SHIFT_UP}},	// じゃじぃじゅじぇじょ
{3,{ OSDK_Z ,OSDK_Y  ,OSDK_I ,0}, {OSDK_D , OSDK_AT    , SHIFT_DOWN, OSDK_E,SHIFT_UP}},
{3,{ OSDK_Z ,OSDK_Y  ,OSDK_U ,0}, {OSDK_R , OSDK_AT    , SHIFT_DOWN, OSDK_8,SHIFT_UP}},
{3,{ OSDK_Z ,OSDK_Y  ,OSDK_E ,0}, {OSDK_P , OSDK_AT    , SHIFT_DOWN, OSDK_5,SHIFT_UP}},
{3,{ OSDK_Z ,OSDK_Y  ,OSDK_O ,0}, {OSDK_C , OSDK_AT    , SHIFT_DOWN, OSDK_9,SHIFT_UP}},

{3,{ OSDK_D ,OSDK_Y  ,OSDK_A  ,0}, {OSDK_A , OSDK_AT    ,SHIFT_DOWN, OSDK_7,SHIFT_UP}},	// ぢゃぢぃぢゅぢぇぢょ
{3,{ OSDK_D ,OSDK_Y  ,OSDK_I  ,0}, {OSDK_A , OSDK_AT    ,SHIFT_DOWN, OSDK_E,SHIFT_UP}},
{3,{ OSDK_D ,OSDK_Y  ,OSDK_U  ,0}, {OSDK_A , OSDK_AT    ,SHIFT_DOWN, OSDK_8,SHIFT_UP}},
{3,{ OSDK_D ,OSDK_Y  ,OSDK_E  ,0}, {OSDK_A , OSDK_AT    ,SHIFT_DOWN, OSDK_5,SHIFT_UP}},
{3,{ OSDK_D ,OSDK_Y  ,OSDK_O  ,0}, {OSDK_A , OSDK_AT    ,SHIFT_DOWN, OSDK_9,SHIFT_UP}},

{3,{ OSDK_B ,OSDK_Y  ,OSDK_A  ,0}, {OSDK_V , OSDK_AT    ,SHIFT_DOWN, OSDK_7,SHIFT_UP}},	// びゃびぃびゅびぇびょ
{3,{ OSDK_B ,OSDK_Y  ,OSDK_I  ,0}, {OSDK_V , OSDK_AT    ,SHIFT_DOWN, OSDK_E,SHIFT_UP}},
{3,{ OSDK_B ,OSDK_Y  ,OSDK_U  ,0}, {OSDK_V , OSDK_AT    ,SHIFT_DOWN, OSDK_8,SHIFT_UP}},
{3,{ OSDK_B ,OSDK_Y  ,OSDK_E  ,0}, {OSDK_V , OSDK_AT    ,SHIFT_DOWN, OSDK_5,SHIFT_UP}},
{3,{ OSDK_B ,OSDK_Y  ,OSDK_O  ,0}, {OSDK_V , OSDK_AT    ,SHIFT_DOWN, OSDK_9,SHIFT_UP}},


{3,{ OSDK_P ,OSDK_Y  ,OSDK_A ,0}, {OSDK_V , OSDK_LEFTBRACKET, SHIFT_DOWN, OSDK_7,SHIFT_UP}},// ぴゃぴぃぴゅぴぇぴょ
{3,{ OSDK_P ,OSDK_Y  ,OSDK_I ,0}, {OSDK_V , OSDK_LEFTBRACKET, SHIFT_DOWN, OSDK_E,SHIFT_UP}},
{3,{ OSDK_P ,OSDK_Y  ,OSDK_U ,0}, {OSDK_V , OSDK_LEFTBRACKET, SHIFT_DOWN, OSDK_8,SHIFT_UP}},
{3,{ OSDK_P ,OSDK_Y  ,OSDK_E ,0}, {OSDK_V , OSDK_LEFTBRACKET, SHIFT_DOWN, OSDK_5,SHIFT_UP}},
{3,{ OSDK_P ,OSDK_Y  ,OSDK_O ,0}, {OSDK_V , OSDK_LEFTBRACKET, SHIFT_DOWN, OSDK_9,SHIFT_UP}},

{3,{ OSDK_G ,OSDK_Y  ,OSDK_A ,0}, {OSDK_G , OSDK_AT, SHIFT_DOWN, OSDK_7,SHIFT_UP}},// ぎゃぎぃぎゅぎぇぎょ
{3,{ OSDK_G ,OSDK_Y  ,OSDK_I ,0}, {OSDK_G , OSDK_AT, SHIFT_DOWN, OSDK_E,SHIFT_UP}},
{3,{ OSDK_G ,OSDK_Y  ,OSDK_U ,0}, {OSDK_G , OSDK_AT, SHIFT_DOWN, OSDK_8,SHIFT_UP}},
{3,{ OSDK_G ,OSDK_Y  ,OSDK_E ,0}, {OSDK_G , OSDK_AT, SHIFT_DOWN, OSDK_5,SHIFT_UP}},
{3,{ OSDK_G ,OSDK_Y  ,OSDK_O ,0}, {OSDK_G , OSDK_AT, SHIFT_DOWN, OSDK_9,SHIFT_UP}},



{2,{ OSDK_L ,OSDK_A  ,0      ,0}, {SHIFT_DOWN , OSDK_3 , SHIFT_UP  ,0}},	// ぁ
{2,{ OSDK_L ,OSDK_I  ,0      ,0}, {SHIFT_DOWN , OSDK_E , SHIFT_UP  ,0}},	// ぃ
{2,{ OSDK_L ,OSDK_U  ,0      ,0}, {SHIFT_DOWN , OSDK_4 , SHIFT_UP  ,0}},	// ぅ
{2,{ OSDK_L ,OSDK_E  ,0      ,0}, {SHIFT_DOWN , OSDK_5 , SHIFT_UP  ,0}},	// ぇ
{2,{ OSDK_L ,OSDK_O  ,0      ,0}, {SHIFT_DOWN , OSDK_6 , SHIFT_UP  ,0}},	// ぉ
{2,{ OSDK_L ,OSDK_L  ,0      ,0}, {SHIFT_DOWN , OSDK_Z , SHIFT_UP  ,0}},	// っ


{ 2,{ OSDK_X ,OSDK_A  ,0      ,0 },{ SHIFT_DOWN , OSDK_3 , SHIFT_UP  ,0 } },	// ぁ
{ 2,{ OSDK_X ,OSDK_I  ,0      ,0 },{ SHIFT_DOWN , OSDK_E , SHIFT_UP  ,0 } },	// ぃ
{ 2,{ OSDK_X ,OSDK_U  ,0      ,0 },{ SHIFT_DOWN , OSDK_4 , SHIFT_UP  ,0 } },	// ぅ
{ 2,{ OSDK_X ,OSDK_E  ,0      ,0 },{ SHIFT_DOWN , OSDK_5 , SHIFT_UP  ,0 } },	// ぇ
{ 2,{ OSDK_X ,OSDK_O  ,0      ,0 },{ SHIFT_DOWN , OSDK_6 , SHIFT_UP  ,0 } },	// ぉ

{3,{ OSDK_L ,OSDK_T  ,OSDK_U ,0}, {SHIFT_DOWN , OSDK_Z , SHIFT_UP  ,0}},	// っ
{3,{ OSDK_L ,OSDK_Y  ,OSDK_A ,0}, {SHIFT_DOWN , OSDK_7 , SHIFT_UP  ,0}},	// ゃ
{3,{ OSDK_L ,OSDK_Y  ,OSDK_U ,0}, {SHIFT_DOWN , OSDK_8 , SHIFT_UP  ,0}},	// ゅ
{3,{ OSDK_L ,OSDK_Y  ,OSDK_O ,0}, {SHIFT_DOWN , OSDK_9 , SHIFT_UP  ,0}},	// ょ

{1,{ OSDK_MINUS       ,0  ,0 ,0}  	, {OSDK_BACKSLASH  , 0  , 0    ,0}},	//  -
{1,{ OSDK_BACKSLASH   ,0  ,0 ,0}  	, {OSDK_BACKSLASH  , 0  , 0    ,0}},	//  -
{1,{ OSDK_Q           ,0  ,0 ,0}    , {0      , 0      , 0         ,0}},	// Q
{1,{ OSDK_SEMICOLON   ,0  ,0 ,0}    , {0      , 0      , 0         ,0}},	// ;
{1,{ OSDK_COLON       ,0  ,0 ,0}    , {0      , 0      , 0         ,0}},	// :
{1,{ OSDK_RIGHTBRACKET,0  ,0 ,0}  	, {0      , 0      , 0         ,0}},	//  ]
{1,{ OSDK_1           ,0  ,0 ,0}  	, {0      , 0      , 0         ,0}},	//  1
{1,{ OSDK_2           ,0  ,0 ,0}  	, {0      , 0      , 0         ,0}},	//  2
{1,{ OSDK_3           ,0  ,0 ,0}  	, {0      , 0      , 0         ,0}},	//  3
{1,{ OSDK_4           ,0  ,0 ,0}  	, {0      , 0      , 0         ,0}},	//  4
{1,{ OSDK_5           ,0  ,0 ,0}  	, {0      , 0      , 0         ,0}},	//  5
{1,{ OSDK_6           ,0  ,0 ,0}  	, {0      , 0      , 0         ,0}},	//  6
{1,{ OSDK_7           ,0  ,0 ,0}  	, {0      , 0      , 0         ,0}},	//  7
{1,{ OSDK_8           ,0  ,0 ,0}  	, {0      , 0      , 0         ,0}},	//  8
{1,{ OSDK_9           ,0  ,0 ,0}  	, {0      , 0      , 0         ,0}},	//  9
{1,{ OSDK_0           ,0  ,0 ,0}  	, {0      , 0      , 0         ,0}},	//  0
{1,{ OSDK_UPPER       ,0  ,0 ,0}  	, {0      , 0      , 0         ,0}},	//  ^

{1,{ OSDK_AT          ,0  ,0 ,0}  	, {OSDK_AT, 0      , 0         ,0}},	//  @ 濁点
{1,{ OSDK_LEFTBRACKET ,0  ,0 ,0}  	, {OSDK_LEFTBRACKET, 0      , 0         ,0}},	//  | 半濁点

{1,{ OSDK_UPPER       ,0  ,0 ,0}  	, {0      , 0      , 0         ,0}},	//  ^

{1,{ OSDK_COMMA       ,0  ,0 ,0}  	, {SHIFT_DOWN , OSDK_COMMA , SHIFT_UP ,0}},	//  ,
{1,{ OSDK_PERIOD      ,0  ,0 ,0}  	, {SHIFT_DOWN , OSDK_PERIOD, SHIFT_UP ,0}},	//  .
{1,{ OSDK_SLASH       ,0  ,0 ,0}  	, {0      , 0      , 0         ,0}},	//  /
{1,{ OSDK_UNDERSCORE  ,0  ,0 ,0}  	, {0      , 0      , 0         ,0}},	//  _

{ 3,{ OSDK_D ,OSDK_D  ,OSDK_A ,0}, {SHIFT_DOWN, OSDK_Z,SHIFT_UP , OSDK_Q,OSDK_AT} },// っだ
{ 3,{ OSDK_D ,OSDK_D  ,OSDK_O ,0}, {SHIFT_DOWN, OSDK_Z,SHIFT_UP , OSDK_S,OSDK_AT} },// っど


{-1,{ -1     ,-1      , -1   },{-1  , 0,0,0}},

};





// ****************************************************************************
//      テキストファイルから自動入力変換テーブル
// ****************************************************************************

static struct _romaji_tbl  text_tbl[] = {
{ 1,{ 0x21   ,0  ,0 ,0 },{ SHIFT_DOWN , OSDK_1 , SHIFT_UP ,0 }},	//  !
{ 1,{ 0x22   ,0  ,0 ,0 },{ SHIFT_DOWN , OSDK_2 , SHIFT_UP ,0 } },//  "
{ 1,{ 0x23   ,0  ,0 ,0 },{ SHIFT_DOWN , OSDK_3 , SHIFT_UP ,0 } },//  #
{ 1,{ 0x24   ,0  ,0 ,0 },{ SHIFT_DOWN , OSDK_4 , SHIFT_UP ,0 } },//  $
{ 1,{ 0x25   ,0  ,0 ,0 },{ SHIFT_DOWN , OSDK_5 , SHIFT_UP ,0 } },//  %
{ 1,{ 0x26   ,0  ,0 ,0 },{ SHIFT_DOWN , OSDK_6 , SHIFT_UP ,0 } },//  &
{ 1,{ 0x27   ,0  ,0 ,0 },{ SHIFT_DOWN , OSDK_7 , SHIFT_UP ,0 } },//  '
{ 1,{ 0x28   ,0  ,0 ,0 },{ SHIFT_DOWN , OSDK_8 , SHIFT_UP ,0 } },//  (
{ 1,{ 0x29   ,0  ,0 ,0 },{ SHIFT_DOWN , OSDK_9 , SHIFT_UP ,0 } },//  )
{ 1,{ 0x2a   ,0  ,0 ,0 },{ SHIFT_DOWN , OSDK_COLON    , SHIFT_UP ,0 } },//  *
{ 1,{ 0x2b   ,0  ,0 ,0 },{ SHIFT_DOWN , OSDK_SEMICOLON , SHIFT_UP ,0 } },//  +

{ 1,{ 0x2b   ,0  ,0 ,0 },{ SHIFT_DOWN , OSDK_SEMICOLON , SHIFT_UP ,0 } },//  +

{ 2,{ 0x82   ,0xa0 ,0 ,0 },{OSDK_KANA , OSDK_3 , OSDK_KANA } },//  あ
{ 2,{ 0x82   ,0xa2 ,0 ,0 },{OSDK_KANA , OSDK_E , OSDK_KANA } },//  い
{ 2,{ 0x82   ,0xa4 ,0 ,0 },{ OSDK_KANA , OSDK_4 , OSDK_KANA } },//  う
{ 2,{ 0x82   ,0xa6 ,0 ,0 },{ OSDK_KANA , OSDK_5 , OSDK_KANA } },//  え
{ 2,{ 0x82   ,0xa8 ,0 ,0 },{ OSDK_KANA , OSDK_6 , OSDK_KANA } },//  お

{ 2,{ 0x82   ,0x9f ,0 ,0 },{ OSDK_KANA , SHIFT_DOWN, OSDK_3 , SHIFT_UP,OSDK_KANA } },//  ぁ
{ 2,{ 0x82   ,0xa1 ,0 ,0 },{ OSDK_KANA , SHIFT_DOWN, OSDK_E , SHIFT_UP,OSDK_KANA } },//  ぃ
{ 2,{ 0x82   ,0xa3 ,0 ,0 },{ OSDK_KANA , SHIFT_DOWN, OSDK_4 , SHIFT_UP,OSDK_KANA } },//  ぅ
{ 2,{ 0x82   ,0xa5 ,0 ,0 },{ OSDK_KANA , SHIFT_DOWN, OSDK_5 , SHIFT_UP,OSDK_KANA } },//  ぇ
{ 2,{ 0x82   ,0xa7 ,0 ,0 },{ OSDK_KANA , SHIFT_DOWN, OSDK_6 , SHIFT_UP,OSDK_KANA } },//  ぉ


{ 2,{ 0x82   ,0xa9 ,0 ,0 },{ OSDK_KANA , OSDK_T, OSDK_KANA } },    //  か
{ 2,{ 0x82   ,0xab ,0 ,0 },{ OSDK_KANA , OSDK_G , OSDK_KANA } },   //  き
{ 2,{ 0x82   ,0xad ,0 ,0 },{ OSDK_KANA , OSDK_H , OSDK_KANA } },   //  く
{ 2,{ 0x82   ,0xaf ,0 ,0 },{ OSDK_KANA , OSDK_COLON , OSDK_KANA } },//  け
{ 2,{ 0x82   ,0xb1 ,0 ,0 },{ OSDK_KANA , OSDK_B , OSDK_KANA } },   //  こ
{ -1,{ -1} ,{-1}},
};


// ****************************************************************************
//		テキストファイルから、自動入力用変換へ
// ****************************************************************************
int convert_text(char *buff)
{
	int num;
	int found = 0;
	int i=0;
	while (1)
	{
		int k = 0;
		num = text_tbl[i].num;
		if (num == -1) break;

		if ((num == 1 && text_tbl[i].romaji[0] == buff[0]) ||
			(num == 2 && text_tbl[i].romaji[0] == buff[0] && text_tbl[i].romaji[1] == buff[1]))
		{
			do
			{
				if (text_tbl[i].keycode[k] == SHIFT_DOWN)
					write_keybuffer(auto_keybuffer, 0, KEYDOWN, 0, OSDK_SHIFT);	// key down
				else if (text_tbl[i].keycode[k] == SHIFT_UP)
					write_keybuffer(auto_keybuffer, 0, KEYUP, 0, OSDK_SHIFT);	// key up
				else
					write_keybuffer(auto_keybuffer, 0, KEYDOWN, 0, text_tbl[i].keycode[k]);	// key down
				k++;
			} while (text_tbl[i].keycode[k]);
			found = 1;
			break;
		}
		i++;
	}

	if (!found)		// 見つからなかったら、そのまま書き込む
	{
		write_keybuffer(auto_keybuffer, 0, KEYDOWN, 0, *buff);	// key down
		write_keybuffer(auto_keybuffer, 0, KEYUP, 0, *buff);	// key up
		num = 1;
	}

	return num;
}



// ****************************************************************************
// autokeyに登録
// ****************************************************************************
int register_autokey(char *buff)
{
	int  cnt;
	char *p;

	if (!Event_isActive(EVENT_KEYIN))
	{
		p = buff;
		for (int i = 0; i < strlen(buff); i++)
		{
			cnt = convert_text(p);
			p += cnt;
		}
		if (!Event_Add(EVENT_KEYIN, 200, EV_LOOP | EV_MS, autokeyin_func))  return 0;

	}
}


// autokey 定期的に呼び出される
void autokeyin_func(void)
{
    int keydown;
    int scancode;
    int osdkeycode;
    if( read_keybuffer( auto_keybuffer ,0 ,&keydown , &scancode ,&osdkeycode) )
        {
		if (osdkeycode == OSDK_KANA)
			DoOut(0x90, 4);		// かなキー切り替え
		else
			write_keybuffer( keybuffer, 0, keydown , scancode , osdkeycode);
        }
    else
        {
         Event_Del( EVENT_KEYIN);		// 最後まで行ったら、スケジュールイベント破棄
        }
}



// not 0 :boin    0:not boin
int isBoin( int osdkeycode)
{
    return (osdkeycode==OSDK_A || osdkeycode==OSDK_I || osdkeycode==OSDK_U || osdkeycode==OSDK_E || osdkeycode==OSDK_O );
}

// not 0 :shion    0:not shion
int isShin( int osdkeycode)
{
    return (osdkeycode==OSDK_K || osdkeycode==OSDK_S || osdkeycode==OSDK_T || osdkeycode==OSDK_H || osdkeycode==OSDK_M || osdkeycode==OSDK_Y|| osdkeycode==OSDK_R || osdkeycode==OSDK_W ||
    osdkeycode==OSDK_P || osdkeycode==OSDK_F);
}

// ****************************************************************************
// 		convert_search:
// ****************************************************************************
int convert_search( char *buff , int *line)
{
    int i;
    int found =0;

    *line = 0;

    for(i=0; romaji_tbl[i].num !=-1 ; i++)
        {
            {
            if( strncmp( romaji_tbl[i].romaji, buff ,strlen(buff))==0)
                {
                if( romaji_tbl[i].num == strlen(buff))
                    found= HENKAN_SUCCESS;		// complete match
                else
                    found= HENKAN_DOING;		// part  match
                *line = i;
                break;
                }
            }
        }
    return found;
}




// ****************************************************************************
// 		convert_romaji2kana:
//		ローマ字から、かなに変換する
//
//  OSキーイベントのkeydown のところで、かなモードかつ、ローマ字モードなら、これを呼ぶ。
//
//
//	処理：ローマ字変換できるかチェックして、変換に成功したら、キー入力するように、スケジュールに登録する。
//
//
// Out: 1: ローマ字変換した、キー入力するように、スケジュールに登録した
//     -1: 変換中
//      0: 変換失敗
//      2: 無貧寒
// ****************************************************************************
int convert_romaji2kana( int osdkeycode )
{
    static char buff[4];
    static int  idx=0;
    int   line=0;
    int   found=0;
//	int   tmp1 ,tmp2 ,tmp3;
//	int   i;

    int   saihenkan_flag=0;


    if( osdkeycode < 0x20 || osdkeycode == OSDK_LEFT || osdkeycode == OSDK_RIGHT || osdkeycode== OSDK_UP
       || osdkeycode== OSDK_DOWN || osdkeycode==OSDK_SCROLLOCK || osdkeycode == OSDK_PAGEUP || osdkeycode == OSDK_PAGEDOWN
		|| (osdkeycode >= OSDK_F1 && osdkeycode <= OSDK_F5))
        {
        idx=0;
        memset(buff,0, sizeof(buff));
        return HENKAN_CANCEL;
        }

    if(idx <3  && 0xd <= osdkeycode && osdkeycode <0x7f)
        {
         buff[ idx++ ]= osdkeycode;
         if( buff[0]==buff[1] && isShin( buff[0]) && strlen(buff)==2)	// double shiin
            {
            found =1; line=0;
            idx--;   saihenkan_flag =1;
            buff[0]= buff[1]; buff[1]=buff[2]; buff[2]=buff[3]; buff[3]=0;
            }
         else if( (buff[0]=='n' ||buff[0]=='N') && isShin( buff[1]))	// double shiin
            {
            found =1; line=1;
            idx--;   saihenkan_flag =1;
            buff[0]= buff[1]; buff[1]=buff[2]; buff[2]=buff[3]; buff[3]=0;
            }
         else
             found = convert_search( buff , &line);	// pattern match

         PRINTDEBUG1(KEY_LOG,"[P6][convert_romaji2kana] input buff= '%s' \n ", buff);

        if( !found )			// not match
            {
            PRINTDEBUG1(KEY_LOG,"[P6][convert_romaji2kana] convert searching  '%s'  not found \n", buff);
            memset(buff, 0, sizeof(buff));
            idx=0;
            }
        else if( found ==1)		// complete match
            {
            int j;
            int output_length=0;

            PRINTDEBUG1(KEY_LOG,"[P6][convert_romaji2kana] convert_success '%s' -> ",buff);

                    // キー入力スケジュールにかえること。間隔を開けたコールバックで、write_keybuffer を１つづつ呼ぶようにする。
                    // キー入力スケジュール発生中に、OSのキーイベントがきたら、変換出来る場合は、変換して、
                    // 2つめ以降のキー入力スケジュールをキューに登録する

            for(j=0 ; j< 5 ; j++)
                {
                 int c = romaji_tbl[ line ].keycode[j];
                 switch(c)
                    {
                     case SHIFT_DOWN:
                            write_keybuffer( auto_keybuffer, 0 , KEYDOWN , 0 , OSDK_SHIFT);	// shift key down
                            PRINTDEBUG(KEY_LOG,"[SHIFT DOWN]");
                            break;
                     case SHIFT_UP:
                            write_keybuffer( auto_keybuffer, 0 , KEYUP   , 0 , OSDK_SHIFT);	// shift key down
                            PRINTDEBUG(KEY_LOG,"[SHIFT UP]");
                            break;
                     case 0: j=100;		// exit loop
                            break;
                     default:
                            PRINTDEBUG2(KEY_LOG,"['%c'(%02X)]",c,c);
                            write_keybuffer( auto_keybuffer, 0 , KEYDOWN , 0 , c);	// key down
                            write_keybuffer( auto_keybuffer, 0 , KEYUP   , 0 , c);	// key up
                            break;

                    }
                }



            PRINTDEBUG(KEY_LOG,"\n");

            if( !Event_isActive( EVENT_KEYIN))
                if( !Event_Add( EVENT_KEYIN, 200, EV_LOOP|EV_MS , autokeyin_func) )  return 0;

            if( !saihenkan_flag) {idx =0;	memset( buff , 0,  sizeof( buff));}
            saihenkan_flag=0;
            }
        else	// part match , continue buffaling...
            {
            PRINTDEBUG2(KEY_LOG,"[P6][convert_romaji2kana] convert: part match '%s'  line=%d \n ", buff ,line);
            }
        }

    if( found ==HENKAN_FAILED)	// かな変換キー以外は、そのまま返す。
        {
         idx=0;
         memset( buff, 0,  sizeof( buff));
        }
    return found;
}




#if 0
    // ****** kana mode  converter ********
    if( kanaMode && convert_length==0)
        {
        if( sense_keybuffer( NULL ,&keydown , &scancode ,&osdkeycode) )
            if( keydown )
                {
                int ret;
                convert_length = convert_romaji2kana( osdkeycode);	// convert romaji -> kana
                printf("convert_length %d \n ", convert_length);
                }
        }
    else if( convert_length >0){			// henkan auto key
             if( ++convert_counter < 50 /* 50 60 80 150*/) return; else convert_counter =0;	// thru  n times
             --convert_length;
             usleep(10000*1.2);
            printf("convert_length %d \n", convert_length);
        }
#endif
