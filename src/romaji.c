/*
name is romaji.c

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

based on https://github.com/windy6001/ROMAJI/
*/


#include "keys.h"
#include "P6.h"
#include "conv.h"
#include "romaji.h"


// ****************************************************************************
//      functions
// ****************************************************************************
static void convertKana2Katakana(char* buff);


// ****************************************************************************
//          ローマ字変換テーブル
// ****************************************************************************
struct _romaji_tbl {
    int num;
    char romaji[4];
    char keycode[7];
};

static struct _romaji_tbl romaji_tbl[] = {

{2,{ OSDK_L ,OSDK_L  ,0      ,0}, {"っ"}},	// っ        一番最初のデータは、「っ」でないといけない
{3,{ OSDK_L ,OSDK_T  ,OSDK_U ,0}, {"っ"}},	// っ

{1,{ OSDK_A , 0      , 0    ,0}, {"あ" }},		// あいうえお
{1,{ OSDK_I , 0      , 0    ,0}, {"い" }},
{1,{ OSDK_U , 0      , 0    ,0}, {"う" }},
{1,{ OSDK_E , 0      , 0    ,0}, {"え" }},
{1,{ OSDK_O , 0      , 0    ,0}, {"お" }},


{2,{ OSDK_L ,OSDK_A  ,0      ,0}, {"ぁ"}},	// ぁ
{2,{ OSDK_L ,OSDK_I  ,0      ,0}, {"ぃ"}},	// ぃ
{2,{ OSDK_L ,OSDK_U  ,0      ,0}, {"ぅ"}},	// ぅ
{2,{ OSDK_L ,OSDK_E  ,0      ,0}, {"ぇ"}},	// ぇ
{2,{ OSDK_L ,OSDK_O  ,0      ,0}, {"ぉ"}},	// ぉ


{ 2,{ OSDK_X ,OSDK_A  ,0      ,0 },{"ぁ"} },	// ぁ
{ 2,{ OSDK_X ,OSDK_I  ,0      ,0 },{"ぃ"} },	// ぃ
{ 2,{ OSDK_X ,OSDK_U  ,0      ,0 },{"ぅ"} },	// ぅ
{ 2,{ OSDK_X ,OSDK_E  ,0      ,0 },{"ぇ"} },	// ぇ
{ 2,{ OSDK_X ,OSDK_O  ,0      ,0 },{"ぉ"} },	// ぉ

{ 3,{ OSDK_W ,OSDK_H  ,OSDK_A ,0 },{"うぁ"} },	// うぁ
{ 3,{ OSDK_W ,OSDK_H  ,OSDK_I ,0 },{"うぃ"} },	// うぃ
{ 2,{ OSDK_W ,OSDK_I  ,0      ,0 },{"うぃ"} },	// うぃ
{ 2,{ OSDK_W ,OSDK_U  ,0      ,0 },{"う"  } },  // う
{ 3,{ OSDK_W ,OSDK_H  ,OSDK_E ,0 },{"うぇ"} },	// うぇ
{ 2,{ OSDK_W ,OSDK_E  ,0      ,0 },{"うぇ"} },	// うぇ
{ 3,{ OSDK_W ,OSDK_H  ,OSDK_O ,0 },{"うぉ"} },	// うぉ


{2,{ OSDK_K ,OSDK_A  , 0    ,0}, {"か"}},		// かきくけこ
{2,{ OSDK_K ,OSDK_I  , 0    ,0}, {"き"}},
{2,{ OSDK_K ,OSDK_U  , 0    ,0}, {"く"}},
{2,{ OSDK_K ,OSDK_E  , 0    ,0}, {"け"}},
{2,{ OSDK_K ,OSDK_O  , 0    ,0}, {"こ"}},

{3,{ OSDK_K ,OSDK_Y  ,OSDK_A ,0}, {"きゃ"}},	// きゃきぃきゅきぇきょ
{3,{ OSDK_K ,OSDK_Y  ,OSDK_I ,0}, {"きぃ"}},
{3,{ OSDK_K ,OSDK_Y  ,OSDK_U ,0}, {"きゅ"}},
{3,{ OSDK_K ,OSDK_Y  ,OSDK_E ,0}, {"きぇ"}},
{3,{ OSDK_K ,OSDK_Y  ,OSDK_O ,0}, {"きょ"}},

{2,{ OSDK_G ,OSDK_A  ,0      ,0}, {"が"}},	// がぎぐげご
{2,{ OSDK_G ,OSDK_I  ,0      ,0}, {"ぎ"}},
{2,{ OSDK_G ,OSDK_U  ,0      ,0}, {"ぐ"}},
{2,{ OSDK_G ,OSDK_E  ,0      ,0}, {"げ"}},
{2,{ OSDK_G ,OSDK_O  ,0      ,0}, {"ご"}},

{3,{ OSDK_G ,OSDK_Y  ,OSDK_A ,0}, {"ぎゃ"}},// ぎゃぎぃぎゅぎぇぎょ
{3,{ OSDK_G ,OSDK_Y  ,OSDK_I ,0}, {"ぎぃ"}},
{3,{ OSDK_G ,OSDK_Y  ,OSDK_U ,0}, {"ぎゅ"}},
{3,{ OSDK_G ,OSDK_Y  ,OSDK_E ,0}, {"ぎぇ"}},
{3,{ OSDK_G ,OSDK_Y  ,OSDK_O ,0}, {"ぎょ"}},

{3,{ OSDK_G ,OSDK_W  ,OSDK_A ,0}, {"ぐぁ"}},// ぐぁぐぃぐぅぐぇぐぉ
{3,{ OSDK_G ,OSDK_W  ,OSDK_I ,0}, {"ぐぃ"}},
{3,{ OSDK_G ,OSDK_W  ,OSDK_U ,0}, {"ぐぅ"}},
{3,{ OSDK_G ,OSDK_W  ,OSDK_E ,0}, {"ぐぇ"}},
{3,{ OSDK_G ,OSDK_W  ,OSDK_O ,0}, {"ぐぉ"}},

{2,{ OSDK_S ,OSDK_A  , 0     ,0}, {"さ"}},		// さしすせそ
{2,{ OSDK_S ,OSDK_I  , 0     ,0}, {"し"}},
{3,{ OSDK_S ,OSDK_H  ,OSDK_I ,0}, {"し"}},
{2,{ OSDK_S ,OSDK_U  , 0     ,0}, {"す"}},
{2,{ OSDK_S ,OSDK_E  , 0     ,0}, {"せ"}},
{2,{ OSDK_S ,OSDK_O  , 0     ,0}, {"そ"}},

{3,{ OSDK_S ,OSDK_Y  ,OSDK_A ,0}, {"しゃ"}},	// しゃしぃしゅしぇしょ
{3,{ OSDK_S ,OSDK_Y  ,OSDK_I ,0}, {"しぃ"}},
{3,{ OSDK_S ,OSDK_Y  ,OSDK_U ,0}, {"しゅ"}},
{3,{ OSDK_S ,OSDK_Y  ,OSDK_E ,0}, {"しぇ"}},
{3,{ OSDK_S ,OSDK_Y  ,OSDK_O ,0}, {"しょ"}},

{3,{ OSDK_S ,OSDK_H  ,OSDK_A ,0}, {"しゃ"}},	// しゃしゅ　しぇしょ
{3,{ OSDK_S ,OSDK_H  ,OSDK_U ,0}, {"しゅ"}},
{3,{ OSDK_S ,OSDK_H  ,OSDK_E ,0}, {"しぇ"}},
{3,{ OSDK_S ,OSDK_H  ,OSDK_O ,0}, {"しょ"}},

{3,{ OSDK_S ,OSDK_W  ,OSDK_A ,0}, {"すぁ"}},	// すぁすぃすぅすぇすぉ
{3,{ OSDK_S ,OSDK_W  ,OSDK_I ,0}, {"すぃ"}},
{3,{ OSDK_S ,OSDK_W  ,OSDK_U ,0}, {"すぅ"}},
{3,{ OSDK_S ,OSDK_W  ,OSDK_E ,0}, {"すぇ"}},
{3,{ OSDK_S ,OSDK_W  ,OSDK_O ,0}, {"すぉ"}},

{2,{ OSDK_Z ,OSDK_A  ,0      ,0}, {"ざ"}},	// ざじずぜぞ
{2,{ OSDK_Z ,OSDK_I  ,0      ,0}, {"じ"}},
{2,{ OSDK_J ,OSDK_I  ,0      ,0}, {"じ"}},
{2,{ OSDK_Z ,OSDK_U  ,0      ,0}, {"ず"}},
{2,{ OSDK_Z ,OSDK_E  ,0      ,0}, {"ぜ"}},
{2,{ OSDK_Z ,OSDK_O  ,0      ,0}, {"ぞ"}},

{3,{ OSDK_Z ,OSDK_Y  ,OSDK_A ,0}, {"じゃ"}},	// じゃじぃじゅじぇじょ
{3,{ OSDK_Z ,OSDK_Y  ,OSDK_I ,0}, {"じぃ"}},
{3,{ OSDK_Z ,OSDK_Y  ,OSDK_U ,0}, {"じゅ"}},
{3,{ OSDK_Z ,OSDK_Y  ,OSDK_E ,0}, {"じぇ"}},
{3,{ OSDK_Z ,OSDK_Y  ,OSDK_O ,0}, {"じょ"}},

{2,{ OSDK_J ,OSDK_A  ,0      ,0}, {"じゃ"}},	// じゃじゅじぇじょ
{2,{ OSDK_J ,OSDK_U  ,0      ,0}, {"じゅ"}},
{2,{ OSDK_J ,OSDK_E  ,0      ,0}, {"じぇ"}},
{2,{ OSDK_J ,OSDK_O  ,0      ,0}, {"じょ"}},

{3,{ OSDK_J ,OSDK_Y  ,OSDK_A ,0}, {"じゃ"} },	// じゃじぃじゅじぇじょ
{3,{ OSDK_J ,OSDK_Y  ,OSDK_I ,0}, {"じぃ"} },
{3,{ OSDK_J ,OSDK_Y  ,OSDK_U ,0}, {"じゅ"} },
{3,{ OSDK_J ,OSDK_Y  ,OSDK_E ,0}, {"じぇ"} },
{3,{ OSDK_J ,OSDK_Y  ,OSDK_O ,0}, {"じょ"}},



{2,{ OSDK_T ,OSDK_A  , 0    ,0},  {"た"} },		// たちつてと
{2,{ OSDK_T ,OSDK_I  , 0    ,0},  {"ち"} },
{3,{ OSDK_C ,OSDK_H  ,OSDK_I,0},  {"ち"} },
{2,{ OSDK_C ,OSDK_I  , 0    ,0},  {"ち"} },
{2,{ OSDK_T ,OSDK_U  , 0    ,0},  {"つ"}},
{3,{ OSDK_T ,OSDK_S  ,OSDK_U,0},  {"つ"} },
{2,{ OSDK_T ,OSDK_E  , 0    ,0},  {"て"} },
{2,{ OSDK_T ,OSDK_O  , 0    ,0},  {"と"}},

{3,{ OSDK_T ,OSDK_Y  ,OSDK_A  ,0}, {"ちゃ"}},	// ちゃちぃちゅちぇちょ
{3,{ OSDK_T ,OSDK_Y  ,OSDK_I  ,0}, {"ちぃ"}},
{3,{ OSDK_T ,OSDK_Y  ,OSDK_U  ,0}, {"ちゅ"}},
{3,{ OSDK_T ,OSDK_Y  ,OSDK_E  ,0}, {"ちぇ"} },
{3,{ OSDK_T ,OSDK_Y  ,OSDK_O  ,0}, {"ちょ"} },

{ 3,{ OSDK_C ,OSDK_Y  ,OSDK_A  ,0}, {"ちゃ"} },	// ちゃちぃちゅちぇちょ
{ 3,{ OSDK_C ,OSDK_Y  ,OSDK_I  ,0}, {"ちぃ"} },
{ 3,{ OSDK_C ,OSDK_Y  ,OSDK_U  ,0}, {"ちゅ"} },
{ 3,{ OSDK_C ,OSDK_Y  ,OSDK_E  ,0}, {"ちぇ"} },
{ 3,{ OSDK_C ,OSDK_Y  ,OSDK_O  ,0}, {"ちょ"}},

{ 3,{ OSDK_T ,OSDK_S  ,OSDK_A  ,0}, {"つぁ"}},	// つぁつぃつぇつぉ
{ 3,{ OSDK_T ,OSDK_S  ,OSDK_I  ,0}, {"つぃ"}},
{ 3,{ OSDK_T ,OSDK_S  ,OSDK_E  ,0}, {"つぇ"}},
{ 3,{ OSDK_T ,OSDK_S  ,OSDK_O  ,0}, {"つぉ"}},

{ 3,{ OSDK_T ,OSDK_H  ,OSDK_A  ,0}, {"てゃ"} },	// てゃてぃてゅてぇてょ
{ 3,{ OSDK_T ,OSDK_H  ,OSDK_I  ,0}, {"てぃ"} },
{ 3,{ OSDK_T ,OSDK_H  ,OSDK_U  ,0}, {"てゅ"} },
{ 3,{ OSDK_T ,OSDK_H  ,OSDK_E  ,0}, {"てぇ"} },
{ 3,{ OSDK_T ,OSDK_H  ,OSDK_O  ,0}, {"てょ"}},

{3,{ OSDK_T ,OSDK_W  ,OSDK_A  ,0}, {"とぁ"}},	// とぁとぃとぅとぇとぉ
{3,{ OSDK_T ,OSDK_W  ,OSDK_I  ,0}, {"とぃ"}},
{3,{ OSDK_T ,OSDK_W  ,OSDK_U  ,0}, {"とぅ"}},
{3,{ OSDK_T ,OSDK_W  ,OSDK_E  ,0}, {"とぇ"}},
{3,{ OSDK_T ,OSDK_W  ,OSDK_O  ,0}, {"とぉ"}},

{2,{ OSDK_D ,OSDK_A  ,0      ,0}, {"だ"}},	// だぢづでど
{2,{ OSDK_D ,OSDK_I  ,0      ,0}, {"ぢ"}},
{2,{ OSDK_D ,OSDK_U  ,0      ,0}, {"づ"}},
{2,{ OSDK_D ,OSDK_E  ,0      ,0}, {"で"}},
{2,{ OSDK_D ,OSDK_O  ,0      ,0}, {"ど"}},

{3,{ OSDK_D ,OSDK_Y  ,OSDK_A  ,0}, {"ぢゃ"}},	// ぢゃぢぃぢゅぢぇぢょ
{3,{ OSDK_D ,OSDK_Y  ,OSDK_I  ,0}, {"ぢぃ"}},
{3,{ OSDK_D ,OSDK_Y  ,OSDK_U  ,0}, {"ぢゅ"}},
{3,{ OSDK_D ,OSDK_Y  ,OSDK_E  ,0}, {"ぢぇ"}},
{3,{ OSDK_D ,OSDK_Y  ,OSDK_O  ,0}, {"ぢょ"}},


            // DHA...
            // DWA...

{2,{ OSDK_N ,OSDK_A  , 0    ,0}, {"な"}},		// なにぬねの
{2,{ OSDK_N ,OSDK_I  , 0    ,0}, {"に"}},
{2,{ OSDK_N ,OSDK_U  , 0    ,0}, {"ぬ"}},
{2,{ OSDK_N ,OSDK_E  , 0    ,0}, {"ね"}},
{2,{ OSDK_N ,OSDK_O  , 0    ,0}, {"の"}},

{3,{ OSDK_N ,OSDK_Y  ,OSDK_A ,0}, {"にゃ"}},	// にゃにぃにゅにぇにょ
{3,{ OSDK_N ,OSDK_Y  ,OSDK_I ,0}, {"にぃ"}},
{3,{ OSDK_N ,OSDK_Y  ,OSDK_U ,0}, {"にゅ"}},
{3,{ OSDK_N ,OSDK_Y  ,OSDK_E ,0}, {"にぇ"}},
{3,{ OSDK_N ,OSDK_Y  ,OSDK_O ,0}, {"にょ"}},

{2,{ OSDK_H ,OSDK_A  , 0    ,0}, {"は"}},		//  はひふへほ
{2,{ OSDK_H ,OSDK_I  , 0    ,0}, {"ひ"}},
{2,{ OSDK_H ,OSDK_U  , 0    ,0}, {"ふ"}},
{2,{ OSDK_H ,OSDK_E  , 0    ,0}, {"へ"}},
{2,{ OSDK_H ,OSDK_O  , 0    ,0}, {"ほ"}},

{3,{ OSDK_H ,OSDK_Y  ,OSDK_A ,0}, {"ひゃ"}},	// ひゃひぃひゅひぇひょ
{3,{ OSDK_H ,OSDK_Y  ,OSDK_I ,0}, {"ひぃ"}},
{3,{ OSDK_H ,OSDK_Y  ,OSDK_U ,0}, {"ひゅ"}},
{3,{ OSDK_H ,OSDK_Y  ,OSDK_E ,0}, {"ひぇ"}},
{3,{ OSDK_H ,OSDK_Y  ,OSDK_O ,0}, {"ひょ"}},

{3,{ OSDK_B ,OSDK_Y  ,OSDK_A  ,0}, {"びゃ"}},	// びゃびぃびゅびぇびょ
{3,{ OSDK_B ,OSDK_Y  ,OSDK_I  ,0}, {"びぃ"}},
{3,{ OSDK_B ,OSDK_Y  ,OSDK_U  ,0}, {"びゅ"}},
{3,{ OSDK_B ,OSDK_Y  ,OSDK_E  ,0}, {"びぇ"}},
{3,{ OSDK_B ,OSDK_Y  ,OSDK_O  ,0}, {"びょ"}},

{3,{ OSDK_P ,OSDK_Y  ,OSDK_A ,0}, {"ぴゃ"}},    // ぴゃぴぃぴゅぴぇぴょ
{3,{ OSDK_P ,OSDK_Y  ,OSDK_I ,0}, {"ぴぃ"}},
{3,{ OSDK_P ,OSDK_Y  ,OSDK_U ,0}, {"ぴゅ"}},
{3,{ OSDK_P ,OSDK_Y  ,OSDK_E ,0}, {"ぴぇ"}},
{3,{ OSDK_P ,OSDK_Y  ,OSDK_O ,0}, {"ぴょ"}},

                // FWA....

{2,{ OSDK_F ,OSDK_A  , 0     ,0}, {"ふぁ"}},	//ふぁふぃふふぇふぉ
{2,{ OSDK_F ,OSDK_I  , 0     ,0}, {"ふぃ"}},
{2,{ OSDK_F ,OSDK_U  , 0     ,0}, {"ふ"}},
{2,{ OSDK_F ,OSDK_E  , 0     ,0}, {"ふぇ"}},
{2,{ OSDK_F ,OSDK_O  , 0     ,0}, {"ふぉ"}},


{2,{ OSDK_B ,OSDK_A  ,0      ,0}, {"ば"}},	// ばびぶべぼ
{2,{ OSDK_B ,OSDK_I  ,0      ,0}, {"び"}},
{2,{ OSDK_B ,OSDK_U  ,0      ,0}, {"ぶ"}},
{2,{ OSDK_B ,OSDK_E  ,0      ,0}, {"べ"}},
{2,{ OSDK_B ,OSDK_O  ,0      ,0}, {"ぼ"}},

{2,{ OSDK_P ,OSDK_A  ,0      ,0}, {"ぱ"}},	// ぱぴぷぺぽ
{2,{ OSDK_P ,OSDK_I  ,0      ,0}, {"ぴ"}},
{2,{ OSDK_P ,OSDK_U  ,0      ,0}, {"ぷ"}},
{2,{ OSDK_P ,OSDK_E  ,0      ,0}, {"ぺ"}},
{2,{ OSDK_P ,OSDK_O  ,0      ,0}, {"ぽ"}},


{2,{ OSDK_M ,OSDK_A  ,0      ,0}, {"ま"}},		// まみむめも
{2,{ OSDK_M ,OSDK_I  ,0      ,0}, {"み"}},
{2,{ OSDK_M ,OSDK_U  ,0      ,0}, {"む"}},
{2,{ OSDK_M ,OSDK_E  ,0      ,0}, {"め"}},
{2,{ OSDK_M ,OSDK_O  ,0      ,0}, {"も"}},

{3,{ OSDK_M ,OSDK_Y  ,OSDK_A ,0}, {"みゃ"}},	// みゃみぃみゅみぇみょ
{3,{ OSDK_M ,OSDK_Y  ,OSDK_I ,0}, {"みぃ"}},
{3,{ OSDK_M ,OSDK_Y  ,OSDK_U ,0}, {"みゅ"}},
{3,{ OSDK_M ,OSDK_Y  ,OSDK_E ,0}, {"みぇ"}},
{3,{ OSDK_M ,OSDK_Y  ,OSDK_O ,0}, {"みょ"}},

{2,{ OSDK_Y ,OSDK_A  , 0     ,0}, {"や"}},				// やゆよ
{2,{ OSDK_Y ,OSDK_U  , 0     ,0}, {"ゆ"}},
{2,{ OSDK_Y ,OSDK_O  , 0     ,0}, {"よ"}},

{3,{ OSDK_L ,OSDK_Y  ,OSDK_A ,0}, {"ゃ"}},	// ゃ
{3,{ OSDK_L ,OSDK_Y  ,OSDK_U ,0}, {"ゅ"}},	// ゅ
{3,{ OSDK_L ,OSDK_Y  ,OSDK_O ,0}, {"ょ"}},	// ょ

{2,{ OSDK_R ,OSDK_A  , 0     ,0}, {"ら"}},				// らりるれろ
{2,{ OSDK_R ,OSDK_I  , 0     ,0}, {"り"}},
{2,{ OSDK_R ,OSDK_U  , 0     ,0}, {"る"}},
{2,{ OSDK_R ,OSDK_E  , 0     ,0}, {"れ"}},
{2,{ OSDK_R ,OSDK_O  , 0     ,0}, {"ろ"}},

{3,{ OSDK_R ,OSDK_Y  ,OSDK_A ,0}, {"りゃ"}},	// りゃりぃりゅりぇりょ
{3,{ OSDK_R ,OSDK_Y  ,OSDK_I ,0}, {"りぃ"}},
{3,{ OSDK_R ,OSDK_Y  ,OSDK_U ,0}, {"りゅ"}},
{3,{ OSDK_R ,OSDK_Y  ,OSDK_E ,0}, {"りぇ"}},
{3,{ OSDK_R ,OSDK_Y  ,OSDK_O ,0}, {"りょ"}},

{2,{ OSDK_W ,OSDK_A  , 0    ,0}, {"わ"}},				// わうぃううぇ
{2,{ OSDK_W ,OSDK_I  , 0    ,0}, {"うぃ"}},
{2,{ OSDK_W ,OSDK_U  , 0    ,0}, {"う"}},
{2,{ OSDK_W ,OSDK_E  , 0    ,0}, {"うぇ"}},



{1,{ OSDK_MINUS       ,0  ,0 ,0}  	, {"ー"}},	//  -
{1,{ OSDK_BACKSLASH   ,0  ,0 ,0}  	, {""}},	//  -
//{1,{ OSDK_Q           ,0  ,0 ,0}    , {"Ｑ"}},	// Q
{1,{ OSDK_SEMICOLON   ,0  ,0 ,0}    , {"；"}},	// ;
{1,{ OSDK_COLON       ,0  ,0 ,0}    , {"："}},	// :
{1,{ OSDK_RIGHTBRACKET,0  ,0 ,0}  	, {""}},	//  ]
{1,{ OSDK_1           ,0  ,0 ,0}  	, {"１"}},	//  1
{1,{ OSDK_2           ,0  ,0 ,0}  	, {"２"}},	//  2
{1,{ OSDK_3           ,0  ,0 ,0}  	, {"３"}},	//  3
{1,{ OSDK_4           ,0  ,0 ,0}  	, {"４"}},	//  4
{1,{ OSDK_5           ,0  ,0 ,0}  	, {"５"}},	//  5
{1,{ OSDK_6           ,0  ,0 ,0}  	, {"６"}},	//  6
{1,{ OSDK_7           ,0  ,0 ,0}  	, {"７"}},	//  7
{1,{ OSDK_8           ,0  ,0 ,0}  	, {"８"}},	//  8
{1,{ OSDK_9           ,0  ,0 ,0}  	, {"９"}},	//  9
{1,{ OSDK_0           ,0  ,0 ,0}  	, {"０"}},	//  0
{1,{ OSDK_UPPER       ,0  ,0 ,0}  	, {"＾"}},	//  ^

{1,{ OSDK_AT          ,0  ,0 ,0}  	, {"゛"}},	//  @ 濁点
{1,{ OSDK_LEFTBRACKET ,0  ,0 ,0}  	, {"゜"}},	//  | 半濁点

{1,{ OSDK_UPPER       ,0  ,0 ,0}  	, {"＾"}},	//  ^

{1,{ OSDK_COMMA       ,0  ,0 ,0}  	, {"、"}},	//  ,
{1,{ OSDK_PERIOD      ,0  ,0 ,0}  	, {"。"}},	//  .
{1,{ OSDK_SLASH       ,0  ,0 ,0}  	, {"／"}},	//  /
{1,{ OSDK_UNDERSCORE  ,0  ,0 ,0}  	, {"＿"}},	//  _

                                                                // をん
{ 2,{ OSDK_W ,OSDK_O  , 0    ,0}    , {"を"} },
{ 2,{ OSDK_N ,OSDK_N  , 0    ,0}    , {"ん"} },
{ 3,{ OSDK_N ,OSDK_N  ,OSDK_N,0}    , {"ん"} },

{-1,{ -1     ,-1      , -1   },{-1  , 0,0,0}},

};



// ****************************************************************************
// 　母音かどうか？
// 非0  :はい    0:いいえ
// ****************************************************************************
int isBoin( int osdkeycode)
{
    return (osdkeycode==OSDK_A || osdkeycode==OSDK_I || osdkeycode==OSDK_U || osdkeycode==OSDK_E || osdkeycode==OSDK_O );
}

// ****************************************************************************
// 　子音かどうか？
// 非0  :はい    0:いいえ
// ****************************************************************************
int isShin( int osdkeycode)
{
    return (osdkeycode==OSDK_K || osdkeycode==OSDK_S || osdkeycode==OSDK_T || osdkeycode==OSDK_H || osdkeycode==OSDK_M || osdkeycode==OSDK_Y
    || osdkeycode==OSDK_R || osdkeycode==OSDK_W || osdkeycode==OSDK_P || osdkeycode==OSDK_F 
    || osdkeycode==OSDK_G || osdkeycode == OSDK_D || osdkeycode == OSDK_Z || osdkeycode == OSDK_B);
}

// ****************************************************************************
// 		convert_search:ローマ字の綴りをマッチングする (convert_romaji2kana から呼ばれる)
//
//   In:  buff    変換したい文字列
//        line    変換結果(romaji_tbl テーブルのインデックス)
//
//   Out: HENKAN_SUCCESS   変換成功
//        HENKAN_DOING     変換中
//        HENKAN_FAILED    変換失敗
//        HENKAN_SUCCESS_LTU っ変換成功
// ****************************************************************************
int convert_search( char *buff , int *line)
{
    int i;
    int found =HENKAN_FAILED;

    *line = 0;
    
    if( buff[0]== buff[1])				// 子音が重なって入力されたら、っに変換
        if( isShin( buff[0]) && isShin(buff[1]))
            {
             return HENKAN_SUCCESS_LTU;
            }


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
//  OSキーイベントのkeydown のところで、かなモードかつ、ローマ字変換モードなら、これを呼ぶ。
//
//
//	処理：ローマ字変換できるかチェックして、変換に成功したら、キー入力するように、スケジュールに登録する。
//
//
// Out: HENKAN_SUCCESS: ローマ字変換した、キー入力するように、スケジュールに登録した
//      HENKAN_FAILED:  変換失敗
//      HENKAN_DOING  : 変換中
//      HENKAN_CANCEL : 無変換
//      HENKAN_SUCCESS_LTU っ変換成功
//***************************************************************
int convert_romaji2kana( int osdkeycode )
{
    static char buff[4];
    static int  idx=0;
    int   line=0;
    int   found=0;

    int   saihenkan_flag=0;


    if(  osdkeycode < 0x20 || osdkeycode == OSDK_LEFT || osdkeycode == OSDK_RIGHT || osdkeycode== OSDK_UP
       || osdkeycode== OSDK_DOWN || osdkeycode==OSDK_SCROLLOCK || osdkeycode == OSDK_PAGEUP 
        || osdkeycode == OSDK_PAGEDOWN|| (osdkeycode >= OSDK_F1 && osdkeycode <= OSDK_F10) || osdkeycode==OSDK_END 
        || osdkeycode ==OSDK_SHIFT || osdkeycode == OSDK_ALT || osdkeycode == OSDK_SPACE)
        {
        idx=0;
        memset(buff,0, sizeof(buff));
        return HENKAN_CANCEL;
        }

    if(idx <3  && 0xd <= osdkeycode && osdkeycode <0x7f)
        {
         buff[ idx++ ]= osdkeycode;

         found = convert_search( buff , &line);	// convert to romaji ローマ字変換してみる

         PRINTDEBUG1(KEY_LOG,"[P6][convert_romaji2kana] input buff= '%s' \n ", buff);

        if( !found )			// not match
            {
            PRINTDEBUG1(KEY_LOG,"[P6][convert_romaji2kana] convert searching  '%s'  not found \n", buff);
            memset(buff, 0, sizeof(buff));
            idx=0;
            }
        else if( found ==HENKAN_SUCCESS || found == HENKAN_SUCCESS_LTU)		// match
            {
            int j;
            int output_length=0;

            if( found == HENKAN_SUCCESS_LTU)
                line =0;        // 子音がダブルで来たときは、強制的に、「っ」に変換する

            PRINTDEBUG1(KEY_LOG,"[P6][convert_romaji2kana] convert_success '%s' -> ",buff);


           {
            char tmp[10];
            convertSjis2p6key( romaji_tbl[line].keycode, tmp);  // convert shift JIS -> P6 code
            convertKana2Katakana( tmp);
            putAutokeyMessage( tmp);                         // register autokey
           }



            PRINTDEBUG(KEY_LOG,"\n");
            //if( !saihenkan_flag) {idx =0;	memset( buff , 0,  sizeof( buff));}
            //saihenkan_flag=0;
            if (found == HENKAN_SUCCESS_LTU)        // 子音ダブルできたとき（例えば、KKのときは、KK -> K にして、次の母音を待つ
                {
                 buff[0]= buff[1]; buff[1] = buff[2]; buff[2]= buff[3]; buff[3]=0;
                 idx--;
                }
            else 
                {
                memset(buff,0, sizeof(buff));
                idx=0;
                }
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

// ****************************************************************************
//  カタカナが入力される状態だと、ひらがな→カタカナに変換する
//      In: Out: buff バッファ 
// ****************************************************************************
void convertKana2Katakana(unsigned char* buff)
{
    unsigned char *p;
    p= buff;
    for (int i = 0; i< strlen(buff); i++) {
        if (kanaMode && katakana) {
            if ((0x86 <= *p && *p <= 0x9f) || (0xe0 <= *p && *p <= 0xfd)) {   // kana?
                *p ^= 0x20;
            }
        p++;
        }
    }
}
