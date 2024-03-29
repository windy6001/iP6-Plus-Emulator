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

#ifndef UNIX

struct {
	unsigned char sjis[3];
	unsigned char p6[3];
} conv_tbl[] = {
	            {"�I",0x21},{"�h",0x22},{"��",0x23},{"��",0x24},{"��",0x25},{"��",0x26},{"�f",0x27},
	{"�i",0x28},{"�j",0x29},{"��",0x2a},{"�{",0x2b},{"�C",0x2c},{"�|",0x2d},{"�D",0x2e},{"�^",0x2f},
	{"�O",0x30},{"�P",0x31},{"�Q",0x32},{"�R",0x33},{"�S",0x34},{"�T",0x35},{"�U",0x36},{"�V",0x37},
	{"�W",0x38},{"�X",0x39},{"�F",0x3a},{"�G",0x3b},{"��",0x3c},{"��",0x3d},{"��",0x3e},{"�H",0x3f},

	            {"�`",0x41},{"�a",0x42},{"�b",0x43},{"�c",0x44},{"�d",0x45},{"�e",0x46},{"�f",0x47},
	{"�g",0x48},{"�h",0x49},{"�i",0x4a},{"�j",0x4b},{"�k",0x4c},{"�l",0x4d},{"�m",0x4e},{"�n",0x4f},
	{"�o",0x50},{"�p",0x51},{"�q",0x52},{"�r",0x53},{"�s",0x54},{"�t",0x55},{"�u",0x56},{"�v",0x57},
	{"�w",0x58},{"�x",0x59},{"�y",0x5a},{"�m",0x5b},{"��",0x5c},{"�n",0x5d},{"�O",0x5e},{"�Q",0x5f},


	            {"��",0x61},{"��",0x62},{"��",0x63},{"��",0x64},{"��",0x65},{"��",0x66},{"��",0x67},
	{"��",0x68},{"��",0x69},{"��",0x6a},{"��",0x6b},{"��",0x6c},{"��",0x6d},{"��",0x6e},{"��",0x6f},
	{"��",0x70},{"��",0x71},{"��",0x72},{"��",0x73},{"��",0x74},{"��",0x75},{"��",0x76},{"��",0x77},
	{"��",0x78},{"��",0x79},{"��",0x7a},{"�o",0x7b},{"�b",0x7c},{"�p",0x7d},{"�`",0x7e},

	{"",  0x80},{"",  0x81},{"",  0x82},{"��",0x83},{"��",0x84},{"��",0x85},{"��",0x86},{"��",0x87},
	{"��",0x88},{"��",0x89},{"��",0x8a},{"��",0x8b},{"��",0x8c},{"��",0x8d},{"��",0x8e},{"��",0x8f},
	            {"��",0x91},{"��",0x92},{"��",0x93},{"��",0x94},{"��",0x95},{"��",0x96},{"��",0x97},
	{"��",0x98},{"��",0x99},{"��",0x9a},{"��",0x9b},{"��",0x9c},{"��",0x9d},{"��",0x9e},{"��",0x9f},
	            {"�B",0xa1},{"�u",0xa2},{"�v",0xa3},{"�A",0xa4},{"�E",0xa5},{"��",0xa6},{"�@",0xa7},
	{"�B",0xa8},{"�D",0xa9},{"�F",0xaa},{"�H",0xab},{"��",0xac},{"��",0xad},{"��",0xae},{"�b",0xaf},
	{"�[",0xb0},{"�A",0xb1},{"�C",0xb2},{"�E",0xb3},{"�G",0xb4},{"�I",0xb5},{"�J",0xb6},{"�L",0xb7},
	{"�N",0xb8},{"�P",0xb9},{"�R",0xba},{"�T",0xbb},{"�V",0xbc},{"�X",0xbd},{"�Z",0xbe},{"�\",0xbf},

	{"�^",0xc0},{"�`",0xc1},{"�c",0xc2},{"�e",0xc3},{"�g",0xc4},{"�i",0xc5},{"�j",0xc6},{"�k",0xc7},
	{"�l",0xc8},{"�m",0xc9},{"�n",0xca},{"�q",0xcb},{"�t",0xcc},{"�w",0xcd},{"�z",0xce},{"�}",0xcf},
	{"�~",0xd0},{"��",0xd1},{"��",0xd2},{"��",0xd3},{"��",0xd4},{"��",0xd5},{"��",0xd6},{"��",0xd7},
	{"��",0xd8},{"��",0xd9},{"��",0xda},{"��",0xdb},{"��",0xdc},{"��",0xdd},

	{"��",0xe0},{"��",0xe1},{"��",0xe2},{"��",0xe3},{"��",0xe4},{"��",0xe5},{"��",0xe6},{"��",0xe7},
	{"��",0xe8},{"��",0xe9},{"��",0xea},{"��",0xeb},{"��",0xec},{"��",0xed},{"��",0xee},{"��",0xef},
	{"��",0xf0},{"��",0xf1},{"��",0xf2},{"��",0xf3},{"��",0xf4},{"��",0xf5},{"��",0xf6},{"��",0xf7},
	{"��",0xf8},{"��",0xf9},{"��",0xfa},{"��",0xfb},{"��",0xfc},{"��",0xfd},{"�J",0xde},{"�K",0xdf},

	// ------------- �����E������ -----------------

	{"��",0x96, 0xDE},{"��",0x97, 0xDE},{"��",0x98, 0xDE},{"��",0x99, 0xDE},{"��",0x9a, 0xDE},
	{"��",0x9b, 0xDE},{"��",0x9c, 0xDE},{"��",0x9d, 0xDE},{"��",0x9e, 0xDE},{"��",0x9f, 0xDE},
	{"��",0xe0, 0xDE},{"��",0xe1, 0xDE},{"��",0xe2, 0xDE},{"��",0xe3, 0xDE},{"��",0xe4, 0xDE},
	{"��",0xea, 0xDE},{"��",0xeb, 0xDE},{"��",0xec, 0xDE},{"��",0xed, 0xDE},{"��",0xee, 0xDE},
	{"��",0xea, 0xDF},{"��",0xeb, 0xDF},{"��",0xec, 0xDF},{"��",0xed, 0xDF},{"��",0xee, 0xDF},

	{"�K",0xb6, 0xDE},{"�M",0xb7, 0xDE},{"�O",0xb8, 0xDE},{"�Q",0xb9, 0xDE},{"�S",0xba, 0xDE},
	{"�U",0xbb, 0xDE},{"�W",0xbc, 0xDE},{"�Y",0xbd, 0xDE},{"�[",0xbe, 0xDE},{"�]",0xbf, 0xDE},
	{"�_",0xc0, 0xDE},{"�a",0xc1, 0xDE},{"�d",0xc2, 0xDE},{"�f",0xc3, 0xDE},{"�h",0xc4, 0xDE},
	{"�o",0xca, 0xDE},{"�r",0xcb, 0xDE},{"�u",0xcc, 0xDE},{"�x",0xcd, 0xDE},{"�{",0xce, 0xDE},
	{"�p",0xca, 0xDF},{"�s",0xcb, 0xDF},{"�v",0xcc, 0xDF},{"�y",0xcd, 0xDF},{"�|",0xce, 0xDF},

	{{0xff,0xff},{0xff,0xff}},
};
#endif


// *******************************************************************
//	 convert SJIS string --> P6 key code
// *******************************************************************
int convertSjis2p6key(unsigned char *inData ,unsigned char *outData)
{
#ifndef UNIX
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
#endif
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
#ifndef UNIX
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
#else
	return 0;
#endif
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