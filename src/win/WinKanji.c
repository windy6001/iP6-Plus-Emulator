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
/**                                                         **/
/**                           WinKanji.c                    **/
/**                    make kanji rom for WIN32             **/
/** by Windy 2003                                           **/
/*************************************************************/
#ifdef WIN32
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <assert.h>
#include "../types.h"
#include "Win32.h"
#include "../cgrom.h"

extern HWND hwndMain;

#define WIDTH  (16*2)
#define HEIGHT (16*2)

static char  *left_rom;		// left  rom
static char *right_rom;		// right rom


static unsigned char table_extkanji[]={		// Ext Kanji rom
#include "../extkanji.h"
0x00
};



// ****************************************************************************
//          make_extkanjirom: 偽の拡張漢字ROMをメモリー上に作成
// ****************************************************************************
/* In: mem  EXTKANJIROM
  Out: ret  1:successfull  0: failed  */
int make_extkanjirom(byte *mem)
{
#if 1
 OSD_Surface *surface;

 // HDC   hdc;
 int   ret=0;
 char  *str;
 char  *p;
 //int   w;
 int   maxlen;
 int   i;
 int   idx;
 int   dy;
 
 char  tmpstr[3];



 surface = OSD_CreateSurface(WIDTH ,HEIGHT,1 ,SURFACE_BITMAP);	// make surface 1bpp 
 if( surface != NULL)
 	{
	 idx= 0x1210;			// start address   (jis 1 suijun)
	 str   = table_extkanji;
	 maxlen= strlen( str);

	 // フォントを設定するようにした 2017/7/24
	 HFONT* pFont;
	 HFONT font;

	 // フォントを作成する
	 font = CreateFont(
		 15,                   // フォントの高さ(大きさ)
		 0,                    // フォントの幅
		 0,                    // 角度
		 0,                    // 角度
		 FW_DONTCARE,          // 文字の太さ
		 FALSE,                // イタリックならTRUE
		 FALSE,                // 下線ならTRUE
		 FALSE,                // 取り消し線ならTRUE
		 SHIFTJIS_CHARSET,     // フォントの文字セット。
		 OUT_DEFAULT_PRECIS,   // 出力精度の設定。
		 CLIP_DEFAULT_PRECIS,  // クリッピング精度。
		 DRAFT_QUALITY,        // フォントの出力品質。
		 DEFAULT_PITCH,        // フォントのピッチとファミリ
		 "MS UI Gothic" // フォントのタイプフェイス名
	 );
	 pFont = SelectObject(surface->hdcBmp ,&font);        // フォントを設定。
	 SetTextColor(surface->hdcBmp ,RGB(255, 255, 0));     // 色
	 SetBkColor(surface->hdcBmp ,RGB(255, 00, 00));       // 背景色


	 //w   = 2;
	 for(i=0; i< maxlen; i+=2)
		{
#ifdef WIN32
		 SelectObject(surface->hdcBmp, GetStockObject( BLACK_BRUSH));
		 Rectangle(   surface->hdcBmp, 0,0 ,16  ,16);
#endif

		 tmpstr[0] = *(str+i);	 tmpstr[1] = *(str+i+1); tmpstr[2] = 0;
		OSD_textout( surface ,0,0, tmpstr ,16);

		// ------- Convert FONT image ---------
		 p = surface->pixels;
		 for(dy=0 ; dy< 16; dy++)
			{
			 mem[idx * 2]   = *(p + 0);
			 mem[idx * 2+1] = *(p + 1);

			p+= WIDTH/8;
			idx++;
			assert(idx < 0x10000);
		   }


#if 0
	 hdc = GetDC( hwndMain);
	 BitBlt( hdc ,0,0,WIDTH,HEIGHT, surface->hdcBmp,0,0,SRCCOPY);
	 Sleep(10);
	 ReleaseDC( hwndMain ,hdc);
#endif
		}

	 mem[0x2c9 * 2] =  mem[0x2c9 *2+1] = 0x41;		// bios checks character

	SelectObject(surface->hdcBmp ,pFont);                // フォントを元に戻す。
	OSD_ReleaseSurface( surface);
	ret=1;
	}
 else
    {
	 printf("createIBMP failed\n");
	}
 return( ret);
#endif
}
// SelectObject(hb.hdcBmp, hBitmapbak);
//DeleteDC(hb.hdcBmp);



#if 0
// ****************************************************************************
//          save_extkanjirom: 偽の拡張漢字ROMの保存　　（今は未使用）
// ****************************************************************************
int save_extkanjirom(char *file_name)
{
 FILE *fp;

 printf("\nwriting extend kanji rom....");
 fp=fopen( file_name,"wb"); 
 if( fp==NULL) {printf("\n %s open failed \n",file_name);return(0);}
 
 do
    {
	 if(fwrite( left_rom,sizeof( left_rom),1,fp)!=1)
	 	break;
	 if(fwrite(right_rom,sizeof(right_rom),1,fp)!=1)
		break;
	 fclose(fp);
	 printf("done.\n");
	 return(1);
	}
 while(0);
 fclose(fp);
 printf("FAILED.\n");
 return(0);
}
#endif




#endif
