/*
name is conv.c

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

#include "types.h"
#include "conv.h"

// *******************************************************************
//            Shift JIS <----> p6 code conversion table
// *******************************************************************

struct {
	unsigned char sjis[3];
	unsigned char p6[3];
} conv_tbl[] = {
	            {"I",0x21},{"h",0x22},{"”",0x23},{"",0x24},{"“",0x25},{"•",0x26},{"f",0x27},
	{"i",0x28},{"j",0x29},{"–",0x2a},{"{",0x2b},{"C",0x2c},{"|",0x2d},{"D",0x2e},{"^",0x2f},
	{"‚O",0x30},{"‚P",0x31},{"‚Q",0x32},{"‚R",0x33},{"‚S",0x34},{"‚T",0x35},{"‚U",0x36},{"‚V",0x37},
	{"‚W",0x38},{"‚X",0x39},{"F",0x3a},{"G",0x3b},{"ƒ",0x3c},{"",0x3d},{"„",0x3e},{"H",0x3f},

	            {"‚`",0x41},{"‚a",0x42},{"‚b",0x43},{"‚c",0x44},{"‚d",0x45},{"‚e",0x46},{"‚f",0x47},
	{"‚g",0x48},{"‚h",0x49},{"‚i",0x4a},{"‚j",0x4b},{"‚k",0x4c},{"‚l",0x4d},{"‚m",0x4e},{"‚n",0x4f},
	{"‚o",0x50},{"‚p",0x51},{"‚q",0x52},{"‚r",0x53},{"‚s",0x54},{"‚t",0x55},{"‚u",0x56},{"‚v",0x57},
	{"‚w",0x58},{"‚x",0x59},{"‚y",0x5a},{"m",0x5b},{"",0x5c},{"n",0x5d},{"O",0x5e},{"Q",0x5f},


	            {"‚",0x61},{"‚‚",0x62},{"‚ƒ",0x63},{"‚„",0x64},{"‚…",0x65},{"‚†",0x66},{"‚‡",0x67},
	{"‚ˆ",0x68},{"‚‰",0x69},{"‚Š",0x6a},{"‚‹",0x6b},{"‚Œ",0x6c},{"‚",0x6d},{"‚Ž",0x6e},{"‚",0x6f},
	{"‚",0x70},{"‚‘",0x71},{"‚’",0x72},{"‚“",0x73},{"‚”",0x74},{"‚•",0x75},{"‚–",0x76},{"‚—",0x77},
	{"‚˜",0x78},{"‚™",0x79},{"‚š",0x7a},{"o",0x7b},{"b",0x7c},{"p",0x7d},{"`",0x7e},

	{"",  0x80},{"",  0x81},{"",  0x82},{"Ÿ",0x83},{"›",0x84},{"œ",0x85},{"‚ð",0x86},{"‚Ÿ",0x87},
	{"‚¡",0x88},{"‚£",0x89},{"‚¥",0x8a},{"‚§",0x8b},{"‚á",0x8c},{"‚ã",0x8d},{"‚å",0x8e},{"‚Á",0x8f},
	            {"‚ ",0x91},{"‚¢",0x92},{"‚¤",0x93},{"‚¦",0x94},{"‚¨",0x95},{"‚©",0x96},{"‚«",0x97},
	{"‚­",0x98},{"‚¯",0x99},{"‚±",0x9a},{"‚³",0x9b},{"‚µ",0x9c},{"‚·",0x9d},{"‚¹",0x9e},{"‚»",0x9f},
	            {"B",0xa1},{"u",0xa2},{"v",0xa3},{"A",0xa4},{"E",0xa5},{"ƒ’",0xa6},{"ƒ@",0xa7},
	{"ƒB",0xa8},{"ƒD",0xa9},{"ƒF",0xaa},{"ƒH",0xab},{"ƒƒ",0xac},{"ƒ…",0xad},{"ƒ‡",0xae},{"ƒb",0xaf},
	{"[",0xb0},{"ƒA",0xb1},{"ƒC",0xb2},{"ƒE",0xb3},{"ƒG",0xb4},{"ƒI",0xb5},{"ƒJ",0xb6},{"ƒL",0xb7},
	{"ƒN",0xb8},{"ƒP",0xb9},{"ƒR",0xba},{"ƒT",0xbb},{"ƒV",0xbc},{"ƒX",0xbd},{"ƒZ",0xbe},{"ƒ\",0xbf},

	{"ƒ^",0xc0},{"ƒ`",0xc1},{"ƒc",0xc2},{"ƒe",0xc3},{"ƒg",0xc4},{"ƒi",0xc5},{"ƒj",0xc6},{"ƒk",0xc7},
	{"ƒl",0xc8},{"ƒm",0xc9},{"ƒn",0xca},{"ƒq",0xcb},{"ƒt",0xcc},{"ƒw",0xcd},{"ƒz",0xce},{"ƒ}",0xcf},
	{"ƒ~",0xd0},{"ƒ€",0xd1},{"ƒ",0xd2},{"ƒ‚",0xd3},{"ƒ„",0xd4},{"ƒ†",0xd5},{"ƒˆ",0xd6},{"ƒ‰",0xd7},
	{"ƒŠ",0xd8},{"ƒ‹",0xd9},{"ƒŒ",0xda},{"ƒ",0xdb},{"ƒ",0xdc},{"ƒ“",0xdd},

	{"‚½",0xe0},{"‚¿",0xe1},{"‚Â",0xe2},{"‚Ä",0xe3},{"‚Æ",0xe4},{"‚È",0xe5},{"‚É",0xe6},{"‚Ê",0xe7},
	{"‚Ë",0xe8},{"‚Ì",0xe9},{"‚Í",0xea},{"‚Ð",0xeb},{"‚Ó",0xec},{"‚Ö",0xed},{"‚Ù",0xee},{"‚Ü",0xef},
	{"‚Ý",0xf0},{"‚Þ",0xf1},{"‚ß",0xf2},{"‚à",0xf3},{"‚â",0xf4},{"‚ä",0xf5},{"‚æ",0xf6},{"‚ç",0xf7},
	{"‚è",0xf8},{"‚é",0xf9},{"‚ê",0xfa},{"‚ë",0xfb},{"‚í",0xfc},{"‚ñ",0xfd},{"J",0xde},{"K",0xdf},

	// ------------- ‘÷‰¹E”¼‘÷‰¹ -----------------

	{"‚ª",0x96, 0xDE},{"‚¬",0x97, 0xDE},{"‚®",0x98, 0xDE},{"‚°",0x99, 0xDE},{"‚²",0x9a, 0xDE},
	{"‚´",0x9b, 0xDE},{"‚¶",0x9c, 0xDE},{"‚¸",0x9d, 0xDE},{"‚º",0x9e, 0xDE},{"‚¼",0x9f, 0xDE},
	{"‚¾",0xe0, 0xDE},{"‚À",0xe1, 0xDE},{"‚Ã",0xe2, 0xDE},{"‚Å",0xe3, 0xDE},{"‚Ç",0xe4, 0xDE},
	{"‚Î",0xea, 0xDE},{"‚Ñ",0xeb, 0xDE},{"‚Ô",0xec, 0xDE},{"‚×",0xed, 0xDE},{"‚Ú",0xee, 0xDE},
	{"‚Ï",0xea, 0xDF},{"‚Ò",0xeb, 0xDF},{"‚Õ",0xec, 0xDF},{"‚Ø",0xed, 0xDF},{"‚Û",0xee, 0xDF},

	{"ƒK",0xb6, 0xDE},{"ƒM",0xb7, 0xDE},{"ƒO",0xb8, 0xDE},{"ƒQ",0xb9, 0xDE},{"ƒS",0xba, 0xDE},
	{"ƒU",0xbb, 0xDE},{"ƒW",0xbc, 0xDE},{"ƒY",0xbd, 0xDE},{"ƒ[",0xbe, 0xDE},{"ƒ]",0xbf, 0xDE},
	{"ƒ_",0xc0, 0xDE},{"ƒa",0xc1, 0xDE},{"ƒd",0xc2, 0xDE},{"ƒf",0xc3, 0xDE},{"ƒh",0xc4, 0xDE},
	{"ƒo",0xca, 0xDE},{"ƒr",0xcb, 0xDE},{"ƒu",0xcc, 0xDE},{"ƒx",0xcd, 0xDE},{"ƒ{",0xce, 0xDE},
	{"ƒp",0xca, 0xDF},{"ƒs",0xcb, 0xDF},{"ƒv",0xcc, 0xDF},{"ƒy",0xcd, 0xDF},{"ƒ|",0xce, 0xDF},

	{{0xff,0xff},{0xff,0xff}},
};

// *******************************************************************
//	 convert SJIS string --> P6 key code
// *******************************************************************
int convertSjis2p6key(unsigned char *inData ,unsigned char *outData)
{
//	int found = FALSE;
	int i,j;

	*outData =0;
	for(j=0; j< strlen(inData);j++) {	// read inData
		if( isSjis( inData[j+0])) {		// SJIS ?
			i=0;
			do {							// search table
				if( conv_tbl[i].sjis[0]== 0xff) {
					break;
					}
				if( inData[j+0] == conv_tbl[i].sjis[0] && inData[j+1]==conv_tbl[i].sjis[1]) {
					strcat( outData, conv_tbl[i].p6);
					//found = TRUE;
					break;
				}
				i++;
			}while(1);
			j++;
		}
		else {								// hankaku
			unsigned char buff[2];
			buff[0] = inData[j];
			buff[1] = 0;
			strcat( outData, buff);
		}
	}
	return 1;
}


/*
int convertSjis2p6key(unsigned char* inData, unsigned char* outData)
{
	int found = FALSE;
	int i, j;

	*outData = 0;
	for (j = 0; j < strlen(inData); j += 2) {
		i = 0;
		do {
			if (conv_tbl[i].sjis[0] == 0xff) {
				break;
			}
			if (inData[0 + j] == conv_tbl[i].sjis[0] && inData[1 + j] == conv_tbl[i].sjis[1]) {
				strcat(outData, conv_tbl[i].p6);
				found = TRUE;
				break;
			}
			i++;
		} while (1);
	}
	return found;
}
*/

// *******************************************************************
//	 convert P6 key code --> SJIS string
// *******************************************************************
int convertp6key2Sjis(unsigned char* inData, unsigned char* outData)
{
	int found = FALSE;
	int i, j;

	*outData = 0;
	for (j = 0; j < strlen(inData); j += 2) {
		i = 0;
		do {
			if (conv_tbl[i].sjis[0] == 0xff) {
				break;
			}
			if (inData[0 + j] == conv_tbl[i].p6[0]) {
				strcat(outData, conv_tbl[i].sjis);
				found = TRUE;
				break;
			}
			i++;
		} while (1);
	}
	return found;
}


// *******************************************************************
//	 is SJIS ?
// *******************************************************************
int isSjis(unsigned char c) 
{
	int ret= FALSE;
	if (((c >= 0x81) && (c <= 0x9f)) || ((c >= 0xe0) && (c <= 0xfc))) {
		ret= TRUE;
		}
	return ret;
	
}