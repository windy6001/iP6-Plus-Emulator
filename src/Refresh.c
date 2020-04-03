/** iP6: PC-6000/6600 series emualtor ************************/
/**                                                         **/
/**                         Refresh.c                       **/
/**                                                         **/
/** by Windy                                                **/
/** by ISHIOKA Hiroshi 1998,1999                            **/
/** This code is based on fMSX written by Marat Fayzullin   **/
/** and Adaptions for any X-terminal by Arnold Metselaar    **/
/*************************************************************/

/* get Type for storing X-colors */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif


#include <stdio.h>
#include <string.h>
#include <assert.h>

#ifdef X11
#include <X11/Xlib.h>
#endif

#define  GLOBAL
#include "Video.h"
#include "Refresh.h"


#include "os.h"
#include "types.h"
#include "P6.h"
#include "mem.h"


int Putchar(int x,int y, char chr, char atr);

static OSD_Surface *surface =NULL;

/*OSD_Surface * SDL_GetVideoSurface()
{
	return system_surface;
}
*/

// ****************************************************************************
//         書き込みするサーフェスの設定
// ****************************************************************************
void setRefreshSurface( OSD_Surface *in_surface)
{
    surface = in_surface;
}


// ****************************************************************************
//         書き込みするサーフェスの取得
// ****************************************************************************
OSD_Surface * getRefreshSurface( void)
{
    return surface;
}


// ****************************************************************************
//         サーフェスのクリア
// ****************************************************************************
void ClearScr( void)
{
	if( surface && surface->pixels)
		{
		OSD_LockSurface( surface );

		memset( surface->pixels , 0, surface->h * surface->pitch );

		OSD_UnlockSurface( surface );
		}
}




// ****************************************************************************
//         カラーテーブル
// ****************************************************************************

  int param[16][3] = /* {R,G,B} */
    { {5,5,5},	// 0: 透明
      {4,3,0},	// 1: 橙
      {0,4,3},	// 2: 青緑
      {3,4,0},	// 3: 黄緑
      {3,0,4},	// 4: 青紫
      {4,0,3},	// 5: 赤紫
      {0,3,4},	// 6: 空色
      {3,3,3},	// 7: 灰色
      {5,5,5},	// 8: 黒
      {4,0,0},	// 9: 赤
      {0,4,0},	// 10:緑
      {4,4,0},	// 11:黄
      {0,0,4},	// 12:青
      {4,0,4},	// 13:マゼンダ
      {0,4,4},	// 14:シアン
      {4,4,4} 	// 15:白
      };
  unsigned short trans[] = { 0x0000, 0x3fff, 0x7fff, 0xafff, 0xffff ,0x1400 };

  int Pal11[ 4] = { 15,		// 白
  					 8, 	// 黒
  					10, 	// 緑
                     8 };	// 黒

  int Pal12[ 8] = { 10,		// 緑
  					11,		// 黄
                    12,		// 青
                     9,		// 赤
                    15,		// 白
                    14,		// シアン
                    13,		// マゼンダ
                     1 };	// 橙

  int Pal13[ 8] = { 10,		// 緑
  					11, 	// 黄
                    12, 	// 青
                     9, 	// 赤
                    15, 	// 白
                    14, 	// シアン
                    13, 	// マゼンダ
                     1 };	// 橙

  int Pal14[ 4] = {  8,		// 黒
  					10,		// 緑
                     8,		// 黒
                    15 };	// 白

  int Pal15[ 8] = {  8,		// 黒
  					13,		// マゼンダ
                    11,		// 黄
                    10,		// 緑
                     8,		// 黒
                    13,		// マゼンダ
                    10,		// 緑
                    15 };	// 白

  // ------ N60m/66 BASIC  color table -------
  int Pal53[32] = {  0,		// 透明	     CSS3=0
  					 4,		// 青紫
                     1,		// 橙
                     5, 	// 赤紫
                     2, 	// 青緑
                     6, 	// 空色
                     3, 	// 黄緑
                     7, 	// 灰色
                     8,		// 黒
				    12, 	// 青
                     9,		// 赤
                    13,		// マゼンダ
                    10,		// 緑
                    14,		// シアン
                    11,		// 黄
                    15,		// 白

                    10,		// 緑		CSS3=1
                    11,		// 黄
                    12,		// 青
                     9,		// 赤
                    15,		// 白
                    14,		// シアン
                    13,		// マゼンダ
                     1,		// 橙
                    10,		// 緑
                    11,		// 黄
                    12,		// 青
                     9,		// 赤
                    15,		// 白
                    14,		// シアン
                    13,		// マゼンダ
                     1 };	// 橙

typedef struct  
{ 
  int bp_bitpix; 
  void (* SeqPix22Wide)  (ColTyp,ColTyp);
  void (* SeqPix22Narrow)(ColTyp,ColTyp); 
  void (* SeqPix21Wide)  (ColTyp); 
  void (* SeqPix21Narrow)(ColTyp); 
  void (* SeqPix41Wide)  (ColTyp); 
  void (* SeqPix41Narrow)(ColTyp); 
} bp_struct; 

/* pointers to pixelsetters, used to adapt to different screen depths
   and window widths: */
typedef void (* t_SeqPix22) (ColTyp,ColTyp);
typedef void (* t_SeqPix21) (ColTyp);
typedef void (* t_SeqPix41) (ColTyp);
static t_SeqPix22  SeqPix22;
static t_SeqPix21 SeqPix21;
static t_SeqPix41 SeqPix41;
static bp_struct *funcs;

/* variables used to handle the buffer, should be register static but
   this is hard to do */
static byte *P;
static byte buf;
static int sft;

static int screen_256_192;

// ****************************************************************************
//        　サーフェスの、ラインごとの、先頭アドレスを計算
// ****************************************************************************

/* initialisation of these variables */
static void SetScrVar(int y1,int x1)
{
//  P=XBuf+(long)(Width*y1+x1)*scale*bitpix/8;

  P=(byte*)surface->pixels+(long)(( surface->pitch)*y1 +x1 *surface->format->BytesPerPixel)*scale;
  buf=0;sft=(lsbfirst?0:8);
}

static void debug_SetScrVar(int y1,int x1)
{
//  P=XBuf+(long)(Width*y1+x1)*scale*bitpix/8;

  P=(byte*)surface->pixels+(long)(( surface->pitch)*y1 +x1 *surface->format->BytesPerPixel);
  buf=0;sft=(lsbfirst?0:8);
}


// ****************************************************************************
//         ピクセルの作成ルーチン　（各bppに対応している）
// ****************************************************************************

/* functions to set various numbers of pixels for various values of
   bitpix and lsbfirst. their names are built as follows:
   sp<md>_<bpp>[<bitfirst>]<width>
   <md>  =  22   to set 2 pixels in 2 colors or
            21   to set 2 pixels in 1 color
   <bpp> =  the number of bits per pixel
   <bitfirst> = m for msb first or
                l for lsb first
   <width>    = w for wide screen (set both pixels) or
                n for narrow screen (set only one)
*/

static void sp22_1mw(ColTyp c0,ColTyp c1)
{ buf+=(2*c0.ct_byte[0] +c1.ct_byte[0]) << (sft-=2);
  if (!sft) {*P++=buf; buf=0; sft=8;} }
static void sp22_1mn(ColTyp c0,ColTyp c1)
{ buf+= c0.ct_byte[0] << (--sft);
  if (!sft) {*P++=buf; buf=0; sft=8;} }
static void sp21_1mw(ColTyp c)
{ buf+= (3*c.ct_byte[0]) << (sft-=2);
  if (!sft) {*P++=buf; buf=0; sft=8;} }
static void sp21_1mn(ColTyp c)
{ buf+= c.ct_byte[0] << (--sft);
  if (!sft) {*P++=buf; buf=0; sft=8;} }
static void sp41_1mw(ColTyp c)
{ buf+= (7*c.ct_byte[0]) << (sft-=4);
  if (!sft) {*P++=buf; buf=0; sft=8;} }
/* sp41_1mn = sp21_1mw */

static void sp22_2mw(ColTyp c0,ColTyp c1)
{ buf+=(4*c0.ct_byte[0] +c1.ct_byte[0]) << (sft-=4);
  if (!sft) {*P++=buf; buf=0; sft=8;} }
static void sp22_2mn(ColTyp c0,ColTyp c1)
{ buf+= c0.ct_byte[0] << (sft-=2);
  if (!sft) {*P++=buf; buf=0; sft=8;} }
static void sp21_2mw(ColTyp c)
{ buf+= (5*c.ct_byte[0]) << (sft-=4);
  if (!sft) {*P++=buf; buf=0; sft=8;} }
static void sp21_2mn(ColTyp c)
{ buf+= c.ct_byte[0] << (sft-=2);
  if (!sft) {*P++=buf; buf=0; sft=8;} }
static void sp41_2w(ColTyp c)
{ *P++=85*c.ct_byte[0]; }
/* sp41_2mn = sp21_2mw */

static void sp22_4mw(ColTyp c0,ColTyp c1)
{ *P++=16*c0.ct_byte[0] + c1.ct_byte[0]; }
static void sp22_4mn(ColTyp c0,ColTyp c1)
{ buf+= c0.ct_byte[0] << (sft-=4);
  if (!sft) {*P++=buf; buf=0; sft=8;} }
static void sp21_4w(ColTyp c)
{ *P++= 17*c.ct_byte[0]; }
static void sp21_4mn(ColTyp c)
{ buf+= c.ct_byte[0] << (sft-=4);
  if (!sft) {*P++=buf; buf=0; sft=8;} }
static void sp41_4w(ColTyp c)
{ *P++= 17*c.ct_byte[0]; *P++= 17*c.ct_byte[0]; }
/* sp41_4mn = sp21_4w */

static void sp22_1lw(ColTyp c0,ColTyp c1)
{ buf+=(c0.ct_byte[0] +2* c1.ct_byte[0]) << sft;
  if ((sft+=2)==8) {*P++=buf; buf=0; sft=0;} }
static void sp22_1ln(ColTyp c0,ColTyp c1)
{ buf+= c0.ct_byte[0] << sft;
  if ((++sft)==8) {*P++=buf; buf=0; sft=0;} }
static void sp21_1lw(ColTyp c)
{ buf+= (3*c.ct_byte[0]) << sft;
  if ((sft+=2)==8) {*P++=buf; buf=0; sft=0;} }
static void sp21_1ln(ColTyp c)
{ buf+= c.ct_byte[0] << sft;
  if ((++sft)==8) {*P++=buf; buf=0; sft=0;} }
static void sp41_1lw(ColTyp c)
{ buf+= (7*c.ct_byte[0]) << sft;
  if ((sft+=4)==8) {*P++=buf; buf=0; sft=0;} }
/* sp41_1ln = sp21_1lw */

static void sp22_2lw(ColTyp c0,ColTyp c1)
{ buf+=(c0.ct_byte[0] +4*c1.ct_byte[0]) << sft;
  if ((sft+=4)==8) {*P++=buf; buf=0; sft=0;} }
static void sp22_2ln(ColTyp c0,ColTyp c1)
{ buf+= c0.ct_byte[0] << sft;
  if ((sft+=2)==8) {*P++=buf; buf=0; sft=0;} }
static void sp21_2lw(ColTyp c)
{ buf+= (5*c.ct_byte[0]) << sft;
  if ((sft+=4)==8) {*P++=buf; buf=0; sft=0;} }
static void sp21_2ln(ColTyp c)
{ buf+= c.ct_byte[0] << sft;
  if ((sft+=2)==8) {*P++=buf; buf=0; sft=0;} }
/* sp41_2w */
/* sp41_2ln = sp21_2lw */

static void sp22_4lw(ColTyp c0,ColTyp c1)
{ *P++=c0.ct_byte[0] + 16*c1.ct_byte[0]; }
static void sp22_4ln(ColTyp c0,ColTyp c1)
{ buf+= c0.ct_byte[0] << sft;
  if ((sft+=4)==8) {*P++=buf; buf=0; sft=0;} }
/* sp21_4w */
static void sp21_4ln(ColTyp c)
{ buf+= c.ct_byte[0] << sft;
  if ((sft+=4)==8) {*P++=buf; buf=0; sft=0;} }
/* sp41_4w */
/* sp41_4mn = sp21_4w */

static void sp22_8w(ColTyp c0, ColTyp c1)
{ *P++=c0.ct_byte[0]; *P++=c1.ct_byte[0]; }
static void sp22_8n(ColTyp c0, ColTyp c1)
{ *P++=c0.ct_byte[0]; }
static void sp21_8w(ColTyp c)
{ *P++=c.ct_byte[0]; *P++=c.ct_byte[0]; }
static void sp21_8n(ColTyp c)
{ *P++=c.ct_byte[0]; }
static void sp41_8w(ColTyp c)
{ *(P+3) = *(P+2) = *(P+1) = *P = c.ct_byte[0]; P+=4; }
/* sp41_8n = sp21_8w */

static void sp22_16w(ColTyp c0, ColTyp c1)
{ *P++=c0.ct_byte[0]; *P++=c0.ct_byte[1];
  *P++=c1.ct_byte[0]; *P++=c1.ct_byte[1]; }
static void sp22_16n(ColTyp c0, ColTyp c1)
{ *P++=c0.ct_byte[0]; *P++=c0.ct_byte[1]; }
static void sp21_16w(ColTyp c)
{ *(P+2) = *P = c.ct_byte[0]; P++;
  *(P+2) = *P = c.ct_byte[1]; P+=3; }
static void sp21_16n(ColTyp c)
{ *P++=c.ct_byte[0]; *P++=c.ct_byte[1]; }
static void sp41_16w(ColTyp c)
{ *(P+6) = *(P+4) = *(P+2) = *P = c.ct_byte[0]; P++;
  *(P+6) = *(P+4) = *(P+2) = *P = c.ct_byte[1]; P+=7; }
/* sp41_16n = sp21_16w */

static void sp22_24w(ColTyp c0, ColTyp c1)
{ *P++=c0.ct_byte[0]; *P++=c0.ct_byte[1]; *P++=c0.ct_byte[2];
  *P++=c1.ct_byte[0]; *P++=c1.ct_byte[1]; *P++=c1.ct_byte[2]; }
static void sp22_24n(ColTyp c0, ColTyp c1)
{ *P++=c0.ct_byte[0]; *P++=c0.ct_byte[1]; *P++=c0.ct_byte[2]; }
static void sp21_24w(ColTyp c)
{ *(P+3) = *P = c.ct_byte[0]; P++;
  *(P+3) = *P = c.ct_byte[1]; P++;
  *(P+3) = *P = c.ct_byte[2]; P+=4; }
static void sp21_24n(ColTyp c)
{ *P++=c.ct_byte[0]; *P++=c.ct_byte[1]; *P++=c.ct_byte[2]; }
static void sp41_24w(ColTyp c)
{ *(P+9) = *(P+6) = *(P+3) = *P = c.ct_byte[0]; P++;
  *(P+9) = *(P+6) = *(P+3) = *P = c.ct_byte[1]; P++;
  *(P+9) = *(P+6) = *(P+3) = *P = c.ct_byte[2]; P+=10; }
/* sp41_24n = sp21_24w */

static void sp22_32w(ColTyp c0, ColTyp c1)
{ *(XID*)P=c0.ct_xid; P+=4; *(XID*)P=c1.ct_xid; P+=4; }
static void sp22_32n(ColTyp c0, ColTyp c1)
{ *(XID*)P=c0.ct_xid; P+=4;}
static void sp21_32w(ColTyp c)
{ *(XID*)P=c.ct_xid; P+=4; *(XID*)P=c.ct_xid; P+=4; }
static void sp21_32n(ColTyp c)
{ *(XID*)P=c.ct_xid; P+=4;}
static void sp41_32w(ColTyp c)
{ *(XID*)P=c.ct_xid; P+=4; *(XID*)P=c.ct_xid; P+=4;
  *(XID*)P=c.ct_xid; P+=4; *(XID*)P=c.ct_xid; P+=4; }
/* sp41_32n = sp21_32w */

/** Pixel setting routines for any number of bits per pixel **/
bp_struct PixSetters[10]= 
{
/* for msb first: */
  { 1, sp22_1mw, sp22_1mn, sp21_1mw, sp21_1mn, sp41_1mw, sp21_1mw},
  { 2, sp22_2mw, sp22_2mn, sp21_2mw, sp21_2mn, sp41_2w, sp21_2mw},
  { 4, sp22_4mw, sp22_4mn, sp21_4w,  sp21_4mn, sp41_4w,  sp21_4w},
/* for lsb first: */ 
  { 0x81, sp22_1lw, sp22_1ln, sp21_1lw, sp21_1ln, sp41_1lw, sp21_1lw},
  { 0x82, sp22_2lw, sp22_2ln, sp21_2lw, sp21_2ln, sp41_2w, sp21_2lw},
  { 0x84, sp22_4lw, sp22_4ln, sp21_4w,  sp21_4ln, sp41_4w,  sp21_4w},
/* Whole bytes, same routines for MSB-first and LSB-first */
  { 8, sp22_8w,  sp22_8n,  sp21_8w,  sp21_8n,  sp41_8w,  sp21_8w},
  {16, sp22_16w, sp22_16n, sp21_16w, sp21_16n, sp41_16w, sp21_16w},
  {24, sp22_24w, sp22_24n, sp21_24w, sp21_24n, sp41_24w, sp21_24w},
  {32, sp22_32w, sp22_32n, sp21_32w, sp21_32n, sp41_32w, sp21_32w},
};



// ****************************************************************************
//         インターレースなし　（前のラインを、コピーする）
// ****************************************************************************
/* duplicates scanlines, used if interlacing is off  */
static void NoIntLac( byte Y)
{
   byte *P;
   word linlen;
  
//  linlen=Width*bitpix/8;
  linlen=surface->pitch;
//  P=XBuf+(long)scale*Y*linlen;
  P=(byte*)surface->pixels+(long)scale*Y*linlen;
  memcpy(P+linlen,P,linlen);
}

// ****************************************************************************
//         
// ****************************************************************************

void choosefuncs(int lsbfirst, int bitpix)
{
  int i;

  if (lsbfirst && (bitpix<8)) bitpix|=0x80;
  for (i=0 ; (PixSetters[i].bp_bitpix!=bitpix)&&(i<10) ; i++)
    ;
  funcs=&(PixSetters[i]);
}

// ****************************************************************************
//         
// ****************************************************************************
void setwidth(int wide)
{
  SeqPix22=wide ? funcs->SeqPix22Wide : funcs->SeqPix22Narrow;
  SeqPix21=wide ? funcs->SeqPix21Wide : funcs->SeqPix21Narrow;
  SeqPix41=wide ? funcs->SeqPix41Wide : funcs->SeqPix41Narrow;
}


// ****************************************************************************
//         
// ****************************************************************************
/** Screen Mode Handlers [N60/N66][SCREEN MODE] **************/
void (*SCR[2+2][4])() =
{
  { RefreshScr10, RefreshScr10, RefreshScr10, RefreshScr10 },
  { RefreshScr51, RefreshScr51, RefreshScr53, RefreshScr54 },
  					/* ************ add 2002/2  Windy ******** */
  { RefreshScr61, RefreshScr62, RefreshScr62, RefreshScr63 },
  { RefreshScr61, RefreshScr62, RefreshScr62, RefreshScr63 },
};


#define NOINTLACM1(Y)	NoIntLac((M5HEIGHT-M1HEIGHT)/2+Y)
#define NOINTLACM5(Y)	NoIntLac(Y)
#define SETSCRVARM1(Y)	SetScrVar((M5HEIGHT-M1HEIGHT)/2+Y,(M5WIDTH-M1WIDTH)/2)
#define SETSCRVARM5(Y)	SetScrVar(Y,0)



// ****************************************************************************
//        ピクセルデータの取得 
// ****************************************************************************
//ColTyp getpixel(unsigned char *image, int x, int y,int bitpix)
ColTyp getpixel(OSD_Surface *surface, int x, int y)
{
 ColTyp c;
// int i;
// int bytes;
 byte *P;
 
// bytes = bitpix/8;
// P=image+(long)(y * Width + x )* bytes;
 int bitpix = surface->format->BitsPerPixel;
 
 P = (unsigned char *)surface->pixels + (y * surface->pitch +x * bitpix/8 );
 
 switch( bitpix)
 	{
     case 8:  c.ct_byte[0] = *P;
              break;
     case 16: c.ct_byte[0] = *(P+1);
              c.ct_byte[1] = *P;
              break;
     case 24: c.ct_byte[0] = *(P+2);
     	      c.ct_byte[1] = *(P+1);
     	      c.ct_byte[2] = *P;
              break;
     case 32: c.ct_xid     = *((XID*)P);
              break;
    }
 return( c );
}

/* bug fixed: (XID*) *P は間違いです。by Windy  2004/4/29 */


/** RefreshScr ***********************************************/
/** Draw window functions for each basic/screen mode        **/
/*************************************************************/


// ****************************************************************************
//         N60 モードの描画ルーチン     （ここから、アトリビュートによって分岐する）
// ****************************************************************************
/** RefreshScr10: N60-BASIC select function ******************/
void RefreshScr10()
{
/*  if ((*VRAM&0x80) == 0x00)
     RefreshScr11();
    else */

	int i;
	int isGraphics =0;	// 1:graphics mode 0: text mode

	// attribute 複数チェックする　ひとつでも、MSB=1のやつがあれば、グラフィックスモードとする
	for(i=0; i<2; i++)
		if ((*(VRAM+i)&0x80)==0x80) {
			isGraphics =1;
			screen_256_192 = *(VRAM+i) & 0x10;
			break;
			}

	if( !isGraphics )
		RefreshScr11();
	else
    switch (*(VRAM+i)&0x1C) {
    case 0x00: case 0x10: /*  64x 64 color / 128x 64 */
      RefreshScr13a(); break;
    case 0x08: /* 128x 64 color */
      RefreshScr13b(); break;
    case 0x18: /* 128x 96 */
      RefreshScr13c(); break;
    case 0x04: /* 128x 96 color */
      RefreshScr13d(); break;
    case 0x14: /* 128x192 */
      RefreshScr13e(); break;
    default: /* 128x192 color / 256x192 */
      RefreshScr13(); break;
    }
}

// ****************************************************************************
//       RefreshScr11: N60 BASIC screen 1,2  
// ****************************************************************************
void RefreshScr11()
{
   byte X,Y,K;
   ColTyp FC,BC;
   byte *S,*T1,*T2;
  byte *G;
  
  G = CGROM;		/* CGROM */ 
  T1 = VRAM;		/* attribute data */
  T2 = VRAM+0x0200;	/* ascii/semi-graphic data */
  for(Y=0; Y<M1HEIGHT; Y++) {
    SETSCRVARM1(Y);	/* Drawing area */
    for(X=0; X<32; X++, T1++, T2++) {
      /* get CGROM address and color */
      if (*T1&0x40) {	/* if semi-graphic */
		if (*T1&0x20) {		/* semi-graphic 6 */
		  S = G+((*T2&0x3f)<<4)+0x1000;
		  FC = BPal12[(*T1&0x02)<<1|(*T2)>>6]; BC = BPal[8];
		} else {		/* semi-graphic 4 */
		  S = G+((*T2&0x0f)<<4)+0x2000;
		  FC = BPal12[(*T2&0x70)>>4]; BC = BPal[8];
		}
      } else {		/* if normal character */
		S = G+((*T2)<<4); 
		FC = BPal11[(*T1&0x03)]; BC = BPal11[(*T1&0x03)^0x01];
	      }
	      K=*(S+Y%12);
	      SeqPix21(K&0x80? FC:BC); SeqPix21(K&0x40? FC:BC);
	      SeqPix21(K&0x20? FC:BC); SeqPix21(K&0x10? FC:BC);
	      SeqPix21(K&0x08? FC:BC); SeqPix21(K&0x04? FC:BC);
	      SeqPix21(K&0x02? FC:BC); SeqPix21(K&0x01? FC:BC);
	    }
    if ((scale==2) && !IntLac) NOINTLACM1(Y);
    if (Y%12!=11) { T1-=32; T2-=32; }
  }
  if(EndOfFrame) PutImage();
}

// ****************************************************************************
//         RefreshScr13: N60-BASIC screen 3,4
// ****************************************************************************
void RefreshScr13()
{
   byte X,Y;
   byte *T1,*T2;
  byte attr;

  T1 = VRAM;		/* attribute data */
  T2 = VRAM+0x0200;	/* graphic data */
  for (Y=0; Y<M1HEIGHT; Y++) {
	SETSCRVARM1(Y);	/* Drawing area */
    for (X=0; X<32; X++,T1++,T2++) {
//		if (*T1&0x10 ) { /* 256x192 (SCREEN 4) */
		if (screen_256_192 ) { 
			if (scr4col) {
				attr = (*T1&0x02)<<1;
				SeqPix41(BPal15[attr|(*T2&0xC0)>>6]);
				SeqPix41(BPal15[attr|(*T2&0x30)>>4]);
				SeqPix41(BPal15[attr|(*T2&0x0C)>>2]);
				SeqPix41(BPal15[attr|(*T2&0x03)   ]);
			} else {
				attr = *T1&0x02;
				SeqPix21(BPal14[attr|(*T2&0x80)>>7]);
				SeqPix21(BPal14[attr|(*T2&0x40)>>6]);
				SeqPix21(BPal14[attr|(*T2&0x20)>>5]);
				SeqPix21(BPal14[attr|(*T2&0x10)>>4]);
				SeqPix21(BPal14[attr|(*T2&0x08)>>3]);
				SeqPix21(BPal14[attr|(*T2&0x04)>>2]);
				SeqPix21(BPal14[attr|(*T2&0x02)>>1]);
				SeqPix21(BPal14[attr|(*T2&0x01)   ]);
			}
      } else { /* 128x192 color (SCREEN 3) */
		attr = (*T1&0x02)<<1;
		SeqPix41(BPal13[attr|(*T2&0xC0)>>6]);
		SeqPix41(BPal13[attr|(*T2&0x30)>>4]);
		SeqPix41(BPal13[attr|(*T2&0x0C)>>2]);
		SeqPix41(BPal13[attr|(*T2&0x03)   ]);
      }
    }
    if (T1 == VRAM+0x200) T1=VRAM;
    if ((scale==2) && !IntLac) NOINTLACM1(Y);
  }
  if(EndOfFrame) PutImage();
}




// ****************************************************************************
//         RefreshScr13a: N60-BASIC screen 3,4
// ****************************************************************************
void RefreshScr13a() /*  64x 64 color / 128x 64 */
{
   byte X,Y;
   byte *T1,*T2;
  byte attr;
  ColTyp L;

  T1 = VRAM;		/* attribute data */
  T2 = VRAM+0x0200;	/* graphic data */
  for (Y=0; Y<M1HEIGHT; Y++) {
    SETSCRVARM1(Y);	/* Drawing area */
    for (X=0; X<16; X++,T1++,T2++) {
      if (*T1&0x10) { /* 128x 64 */
		if (scr4col) {
		  attr = (*T1&0x02)<<1;
		  SeqPix41(L=BPal15[attr|(*T2&0xC0)>>6]);
		  SeqPix41(L);
		  SeqPix41(L=BPal15[attr|(*T2&0x30)>>4]);
		  SeqPix41(L);
		  SeqPix41(L=BPal15[attr|(*T2&0x0C)>>2]);
		  SeqPix41(L);
		  SeqPix41(L=BPal15[attr|(*T2&0x03)   ]);
		  SeqPix41(L);
		} else { /*  64x 64 color */
		  attr = *T1&0x02;
		  SeqPix41(BPal14[attr|(*T2&0x80)>>7]);
		  SeqPix41(BPal14[attr|(*T2&0x40)>>6]);
		  SeqPix41(BPal14[attr|(*T2&0x20)>>5]);
		  SeqPix41(BPal14[attr|(*T2&0x10)>>4]);
		  SeqPix41(BPal14[attr|(*T2&0x08)>>3]);
		  SeqPix41(BPal14[attr|(*T2&0x04)>>2]);
		  SeqPix41(BPal14[attr|(*T2&0x02)>>1]);
		  SeqPix41(BPal14[attr|(*T2&0x01)   ]);
		}
      } else { /*  64x 64 color */
		attr = (*T1&0x02)<<1;
		SeqPix41(L=BPal13[attr|(*T2&0xC0)>>6]);
		SeqPix41(L);
		SeqPix41(L=BPal13[attr|(*T2&0x30)>>4]);
		SeqPix41(L);
		SeqPix41(L=BPal13[attr|(*T2&0x0C)>>2]);
		SeqPix41(L);
		SeqPix41(L=BPal13[attr|(*T2&0x03)   ]);
		SeqPix41(L);
      }
    }
    if (Y%3 != 2) { T1-=16; T2-=16; }
    else if (T1 == VRAM+0x200) T1=VRAM;
    if ((scale==2) && !IntLac) NOINTLACM1(Y);
  }
  if(EndOfFrame) PutImage();
}

// ****************************************************************************
//         RefreshScr13b: N60-BASIC screen 3,4
// ****************************************************************************
void RefreshScr13b() /* 128x 64 color */
{
   byte X,Y;
   byte *T1,*T2;
  byte attr;

  T1 = VRAM;		/* attribute data */
  T2 = VRAM+0x0200;	/* graphic data */
  for (Y=0; Y<M1HEIGHT; Y++) {
    SETSCRVARM1(Y);	/* Drawing area */
    for (X=0; X<32; X++,T1++,T2++) {
      attr = (*T1&0x02)<<1;
      SeqPix41(BPal13[attr|(*T2&0xC0)>>6]);
      SeqPix41(BPal13[attr|(*T2&0x30)>>4]);
      SeqPix41(BPal13[attr|(*T2&0x0C)>>2]);
      SeqPix41(BPal13[attr|(*T2&0x03)   ]);
    }
    if (Y%3 != 2) { T1-=32; T2-=32; }
    else if (T1 == VRAM+0x200) T1=VRAM;
    if ((scale==2) && !IntLac) NOINTLACM1(Y);
  }
  if(EndOfFrame) PutImage();
}

// ****************************************************************************
//         RefreshScr13c: N60-BASIC screen 3,4
// ****************************************************************************
void RefreshScr13c() /* 128x 96 */
{
   byte X,Y;
   byte *T1,*T2;
  byte attr;
  ColTyp L;

  T1 = VRAM;		/* attribute data */
  T2 = VRAM+0x0200;	/* graphic data */
  for (Y=0; Y<M1HEIGHT; Y++) {
    SETSCRVARM1(Y);	/* Drawing area */
    for (X=0; X<16; X++,T1++,T2++) {
      if (scr4col) {
		attr = (*T1&0x02)<<1;
		SeqPix41(L=BPal15[attr|(*T2&0xC0)>>6]);
		SeqPix41(L);
		SeqPix41(L=BPal15[attr|(*T2&0x30)>>4]);
		SeqPix41(L);
		SeqPix41(L=BPal15[attr|(*T2&0x0C)>>2]);
		SeqPix41(L);
		SeqPix41(L=BPal15[attr|(*T2&0x03)   ]);
		SeqPix41(L);
      } else {
		attr = *T1&0x02;
		SeqPix41(BPal14[attr|(*T2&0x80)>>7]);
		SeqPix41(BPal14[attr|(*T2&0x40)>>6]);
		SeqPix41(BPal14[attr|(*T2&0x20)>>5]);
		SeqPix41(BPal14[attr|(*T2&0x10)>>4]);
		SeqPix41(BPal14[attr|(*T2&0x08)>>3]);
		SeqPix41(BPal14[attr|(*T2&0x04)>>2]);
		SeqPix41(BPal14[attr|(*T2&0x02)>>1]);
		SeqPix41(BPal14[attr|(*T2&0x01)   ]);
      }
    }
    if (!(Y&1)) { T1-=16; T2-=16; }
    else if (T1 == VRAM+0x200) T1=VRAM;
    if ((scale==2) && !IntLac) NOINTLACM1(Y);
  }
  if(EndOfFrame) PutImage();
}

// ****************************************************************************
//         RefreshScr13d: N60-BASIC screen 3,4
// ****************************************************************************
void RefreshScr13d() /* 128x 96 color */
{
   byte X,Y;
   byte *T1,*T2;
  byte attr;

  T1 = VRAM;		/* attribute data */
  T2 = VRAM+0x0200;	/* graphic data */
  for (Y=0; Y<M1HEIGHT; Y++) {
    SETSCRVARM1(Y);	/* Drawing area */
    for (X=0; X<32; X++,T1++,T2++) {
      attr = (*T1&0x02)<<1;
      SeqPix41(BPal13[attr|(*T2&0xC0)>>6]);
      SeqPix41(BPal13[attr|(*T2&0x30)>>4]);
      SeqPix41(BPal13[attr|(*T2&0x0C)>>2]);
      SeqPix41(BPal13[attr|(*T2&0x03)   ]);
    }
    if (!(Y&1)) { T1-=32; T2-=32; }
    else if (T1 == VRAM+0x200) T1=VRAM;
    if ((scale==2) && !IntLac) NOINTLACM1(Y);
  }
  if(EndOfFrame) PutImage();
}

// ****************************************************************************
//         RefreshScr13e: N60-BASIC screen 3,4
// ****************************************************************************
void RefreshScr13e() /* 128x192 */
{
   byte X,Y;
   byte *T1,*T2;
  byte attr;
  ColTyp L;

  T1 = VRAM;		/* attribute data */
  T2 = VRAM+0x0200;	/* graphic data */
  for (Y=0; Y<M1HEIGHT; Y++) {
    SETSCRVARM1(Y);	/* Drawing area */
    for (X=0; X<16; X++,T1++,T2++) {
      if (scr4col) {
		attr = (*T1&0x02)<<1;
		SeqPix41(L=BPal15[attr|(*T2&0xC0)>>6]);
		SeqPix41(L);
		SeqPix41(L=BPal15[attr|(*T2&0x30)>>4]);
		SeqPix41(L);
		SeqPix41(L=BPal15[attr|(*T2&0x0C)>>2]);
		SeqPix41(L);
		SeqPix41(L=BPal15[attr|(*T2&0x03)   ]);
		SeqPix41(L);
      } else {
		attr = *T1&0x02;
		SeqPix41(BPal14[attr|(*T2&0x80)>>7]);
		SeqPix41(BPal14[attr|(*T2&0x40)>>6]);
		SeqPix41(BPal14[attr|(*T2&0x20)>>5]);
		SeqPix41(BPal14[attr|(*T2&0x10)>>4]);
		SeqPix41(BPal14[attr|(*T2&0x08)>>3]);
		SeqPix41(BPal14[attr|(*T2&0x04)>>2]);
		SeqPix41(BPal14[attr|(*T2&0x02)>>1]);
		SeqPix41(BPal14[attr|(*T2&0x01)   ]);
      }
    }
    if (T1 == VRAM+0x200) T1=VRAM;
    if ((scale==2) && !IntLac) NOINTLACM1(Y);
  }
  if(EndOfFrame) PutImage();
}

// ****************************************************************************
//          RefreshScr51: N60m/66-BASIC screen 1,2
// ****************************************************************************
void RefreshScr51()
{
   byte X,Y,K;
   ColTyp FC,BC;
   byte *S,*T1,*T2;
  byte *G;

  G = CGROM;		/* CGROM */ 
  T1 = VRAM;		/* attribute data */
  T2 = VRAM+0x0400;	/* ascii/semi-graphic data */
  
  for(Y=0; Y<M5HEIGHT; Y++) {
    SETSCRVARM5(Y);	/* Drawing area */
    for(X=0; X<40; X++, T1++, T2++ ) {
             /* get CGROM address and color */
             S = G+(*T2<<4)+(*T1&0x80?0x1000:0);	/* semi-graphics */
             FC = BPal[(*T1)&0x0F]; BC = BPal[(((*T1)&0x70)>>4)|CSS2];
             K=*(S+Y%10);
             SeqPix21(K&0x80? FC:BC); SeqPix21(K&0x40? FC:BC);
             SeqPix21(K&0x20? FC:BC); SeqPix21(K&0x10? FC:BC);
             SeqPix21(K&0x08? FC:BC); SeqPix21(K&0x04? FC:BC);
             SeqPix21(K&0x02? FC:BC); SeqPix21(K&0x01? FC:BC);
    }
    if ((scale==2) && !IntLac) NOINTLACM5(Y);
    if (Y%10!=9) { T1-=40; T2-=40; }
  }
  if(EndOfFrame) PutImage();
  refreshAll_flag= 0;
}

// ****************************************************************************
//         RefreshScr53: N60m/66-BASIC screen 3
// ****************************************************************************
void RefreshScr53()
{
   byte X,Y;
   byte *T1,*T2;
  
  T1 = VRAM;		/* attribute data */
  T2 = VRAM+0x2000;	/* graphic data */
  for(Y=0; Y<M5HEIGHT; Y++) {
    SETSCRVARM5(Y);	/* Drawing area */
    for(X=0; X<40; X++) {
      SeqPix41(BPal53[CSS3|((*T1)&0xC0)>>6|((*T2)&0xC0)>>4]);
      SeqPix41(BPal53[CSS3|((*T1)&0x30)>>4|((*T2)&0x30)>>2]);
      SeqPix41(BPal53[CSS3|((*T1)&0x0C)>>2|((*T2)&0x0C)   ]);
      SeqPix41(BPal53[CSS3|((*T1)&0x03)   |((*T2)&0x03)<<2]);
      T1++; T2++;
    }
    if ((scale==2) && !IntLac) NOINTLACM5(Y);
  }
  if(EndOfFrame) PutImage();
}

// ****************************************************************************
//         RefreshScr54: N60m/66-BASIC screen 4 
// ****************************************************************************
void RefreshScr54()
{
   byte X,Y;
   byte *T1,*T2;
  byte cssor;

  T1 = VRAM;		/* attribute data */
  T2 = VRAM+0x2000;	/* graphic data */
  /* CSS OR */
  cssor = CSS3|CSS2|CSS1;
  for(Y=0; Y<M5HEIGHT; Y++) {
    SETSCRVARM5(Y);	/* Drawing area */
    for(X=0; X<40; X++) {
      SeqPix21(BPal53[cssor|((*T1)&0x80)>>7|((*T2)&0x80)>>6]);
      SeqPix21(BPal53[cssor|((*T1)&0x40)>>6|((*T2)&0x40)>>5]);
      SeqPix21(BPal53[cssor|((*T1)&0x20)>>5|((*T2)&0x20)>>4]);
      SeqPix21(BPal53[cssor|((*T1)&0x10)>>4|((*T2)&0x10)>>3]);
      SeqPix21(BPal53[cssor|((*T1)&0x08)>>3|((*T2)&0x08)>>2]);
      SeqPix21(BPal53[cssor|((*T1)&0x04)>>2|((*T2)&0x04)>>1]);
      SeqPix21(BPal53[cssor|((*T1)&0x02)>>1|((*T2)&0x02)   ]);
      SeqPix21(BPal53[cssor|((*T1)&0x01)   |((*T2)&0x01)<<1]);
      T1++;T2++;
    }
    if ((scale==2) && !IntLac) NOINTLACM5(Y);
  }
  if(EndOfFrame) PutImage();
}



/*  -------------------------------------------------
おぼえがき

★ SRのSCREEN 1は、モード5のルーチンとほとんど同じですが、以下の点が違います。
      1.文字データと属性データの並び方が違う
      2.WIDTH で、文字の大きさや、縦横の大きさが変わる
      3.パレットで色が変わる。

★ SCREEN1 でパレットを適用する時は、以下の注意が必要です。
      1.VRAMの値を、カラーコード-1に変換 (textpalet1)
      2.パレットを適用。                (palet)
      3.カラーコード-1を、VRAMの値に変換 (textpalet2)
    SCREEN 2,3 はそのまま適用しても大丈夫みたいです。

　以前のパレットルーチンは、1ドットずつ　いちいち変換していましたが、
　それだと　あまりにも遅いので、あらかじめ　変換しておくようにしました。


 ★ 640x200 ドットに対応するためには、
      1.scale==2で起動すると、640x400ドットになるので、
      2.横640ドットにしたいところで、setwidth(0)とする。
      3.終ったら、すぐに setwidth(1) する。
      
   以前は、scale==1 のとき、左半分しか表示されない様にしていましたが、
   自動的に、scale==2に　変わるようにしました。
                                                                                                    Windy
*/


// ****************************************************************************
//         RefreshScr61: N66-SR BASIC screen 1
// ****************************************************************************
/* Modified by windy from RefreshScR51() */
/* support: WIDTH 40,20 40,25 80,20 80,25  and PALET */
void RefreshScr61()
{
   byte X,Y,K;
   ColTyp FC,BC;
   byte *S,*T1,*T2;
  byte *G;
  int   high;
  int   addr;
  int   semi_addr;


  if( cols==80) setwidth(0);	// 640x200 pixels

//  assert( CGROM==CGROM6);

  G = CGROM;		/* CGROM */
  T1 = VRAM+1;		/* attribute data */
  T2 = VRAM;	        	/* ascii/semi-graphic data */
  high= (rows==20)? 10:8;       /* charactor's high   SR BASIC 2002/2/23 */
  addr= (rows==20)? 0x2000: 0x1000; /* CGROM address  SR BASIC 2002/2/23 */
  semi_addr= (rows==20)? 0x3000: 0x4000; /* semi address 2003/7/8 */

  for(Y=0; Y<M5HEIGHT; Y++) {
    SETSCRVARM5(Y);	/* Drawing area */
    for(X=0; X< cols; X++, T1+=2, T2+=2 ) {
            S= G+(*T2<<4)+(*T1&0x80?semi_addr:addr); /*for CGROM6 SR semi graph 2002/2/23*//* 2003/7/8 */
            FC = BPal[ (*T1)&0x0F];					   /* fast Palet (2002/9/27) */
            BC = BPal[ (((*T1)&0x70)>>4) |CSS2];
	
            K=*(S+Y%high);			/* character high 2002/2/23 */
            SeqPix21(K&0x80? FC:BC); SeqPix21(K&0x40? FC:BC);
            SeqPix21(K&0x20? FC:BC); SeqPix21(K&0x10? FC:BC);
            SeqPix21(K&0x08? FC:BC); SeqPix21(K&0x04? FC:BC);
            SeqPix21(K&0x02? FC:BC); SeqPix21(K&0x01? FC:BC);
    }
    if ((scale==2) && !IntLac) NOINTLACM5(Y);
    if (Y % high!=high-1) { T1-=cols*2; T2-=cols*2;  } /* character high  2002/2/23 */
  }
  setwidth(scale-1);			// 320x200 pixels
  if(EndOfFrame) PutImage();
}

/* -------------------------------------
N66-SR BASICのグラフィックVRAMの格納方法

グラフィック画面を、左右に分けて格納されています。SCREEN 2のばあい、
左が、(0,0)-(255,199) 右が (256,0)-(319,199)の領域になります。
左の開始アドレスは、1A00h  右は、0000hです。

ピクセルデータの格納方法は独特です。ワード単位で格納されています。
偶数と奇数ラインが、交互に現れています。
このコードで、右端まで行ってから、一旦戻っているのは、そのためです。
あとSCREEN 3 でも、１ワードの中身が違うだけで、ほとんど同じ感じです。

bugs:

このコードでは、左半分と右半分を同じ並び方として扱っていますが、
実機では、右半分の並び方は違いますので、厳密的には、間違っているようです。
                                                                             Windy
*/

// ****************************************************************************
//         RefreshScr62  N66-SR BASIC screen 2
// ****************************************************************************
/* Modified by windy  from RefreshScr54() *//* support: PALET */
void RefreshScr62()
{
  byte X,Y;
  byte *T1,*T2 ;

  T1 = VRAM+ 0x1a00;
  T2 = VRAM;

//  for(Y=0; Y<M6HEIGHT; Y++) {
  for(Y=0; Y< lines; Y++) {
    SETSCRVARM5(Y); /* Drawing area */
       for(X=0; X< 256/4; X++) {
             SeqPix21(BPal53[ CSS3|((*T1)&0x0f) ]);
             SeqPix21(BPal53[ CSS3|((*T1)&0xf0)>>4 ]);
             T1++;
             SeqPix21(BPal53[ CSS3|((*T1)&0x0f) ]);
             SeqPix21(BPal53[ CSS3|((*T1)&0xf0)>>4 ]);
             T1+=3;
        }

      for(X=0 ;X<64/4 ; X++) {
             SeqPix21(BPal53[ CSS3|((*T2)&0x0f) ]);
             SeqPix21(BPal53[ CSS3|((*T2)&0xf0)>>4 ]);
             T2++;
             SeqPix21(BPal53[ CSS3|((*T2)&0x0f) ]);
             SeqPix21(BPal53[ CSS3|((*T2)&0xf0)>>4 ]);
             T2+=3;
        }
      if( (Y & 1)==0)
             { T1-=(254);  T2-=(62);   }	/* Y座標  偶数->奇数へ */
      else
             { T1-=2; T2= VRAM+sr_vram_right_addr[Y+2];  }	/* Y座標  奇数->偶数へ */
			/* 右半分(T2)のみ、並び方があっちこっち行ってる。*/
    if ((scale==2) && !IntLac) NOINTLACM5(Y);
  }
  if(EndOfFrame) PutImage();
  refreshAll_flag =0;
}

// ****************************************************************************
//         RefreshScr63  N66-SR BASIC screen 3
// ****************************************************************************
/* Modified by windy  from RefreshScr54() */
void RefreshScr63()
{
  byte X,Y;
  byte *T1,*T2;
  byte cssor;

  if(scale!=2) return;	// scale==1 対策!! Thanks Bernie 2003/8/3


  T1 = VRAM+ 0x1a00;
  T2 = VRAM;
  cssor = CSS3|CSS2|CSS1;

  setwidth( 0);	// add windy  640x200 pixels  add 2002/7/14

//  for(Y=0; Y<M6HEIGHT; Y++) {
  for(Y=0; Y<lines; Y++) {
    SETSCRVARM5(Y); 	/* Drawing area */

       for(X=0; X< 256/4; X++) {
         SeqPix21(BPal53[cssor|((*T1)&0x80)>>7|((*(T1+1))&0x80)>>6]);
         SeqPix21(BPal53[cssor|((*T1)&0x40)>>6|((*(T1+1))&0x40)>>5]);
         SeqPix21(BPal53[cssor|((*T1)&0x20)>>5|((*(T1+1))&0x20)>>4]);
         SeqPix21(BPal53[cssor|((*T1)&0x10)>>4|((*(T1+1))&0x10)>>3]);
         SeqPix21(BPal53[cssor|((*T1)&0x08)>>3|((*(T1+1))&0x08)>>2]);
         SeqPix21(BPal53[cssor|((*T1)&0x04)>>2|((*(T1+1))&0x04)>>1]);
         SeqPix21(BPal53[cssor|((*T1)&0x02)>>1|((*(T1+1))&0x02)   ]);
         SeqPix21(BPal53[cssor|((*T1)&0x01)   |((*(T1+1))&0x01)<<1]);
         T1+=4;
       }

      for(X=0 ;X<64/4 ; X++) {
          SeqPix21(BPal53[cssor|((*T2)&0x80)>>7|((*(T2+1))&0x80)>>6]);
          SeqPix21(BPal53[cssor|((*T2)&0x40)>>6|((*(T2+1))&0x40)>>5]);
          SeqPix21(BPal53[cssor|((*T2)&0x20)>>5|((*(T2+1))&0x20)>>4]);
          SeqPix21(BPal53[cssor|((*T2)&0x10)>>4|((*(T2+1))&0x10)>>3]);
          SeqPix21(BPal53[cssor|((*T2)&0x08)>>3|((*(T2+1))&0x08)>>2]);
          SeqPix21(BPal53[cssor|((*T2)&0x04)>>2|((*(T2+1))&0x04)>>1]);
          SeqPix21(BPal53[cssor|((*T2)&0x02)>>1|((*(T2+1))&0x02)   ]);
          SeqPix21(BPal53[cssor|((*T2)&0x01)   |((*(T2+1))&0x01)<<1]);
         T2+=4;
        }
      if( (Y & 1)==0)
             { T1-=(254);  T2-=(62);}	/* Y座標  偶数->奇数へ */
      else
           //  { T1-=2; T2-=2; }
             { T1-=2; T2= VRAM+sr_vram_right_addr[Y+2];  }	/* Y座標  奇数->偶数へ */
			/* 右半分(T2)のみ、並び方があっちこっち行ってる。*/
    if ((scale==2) && !IntLac) NOINTLACM5(Y);
  }

  setwidth( scale-1);	// add windy  320x200 pixels  2002/7/14
  if(EndOfFrame) PutImage();
}



// ****************************************************************************
//         RefreshScr62  Super Graphics 
// ****************************************************************************
/* Modified by windy  from RefreshScr54() *//* support: PALET */
void RefreshScr72()
{
	byte X, Y;
	byte r, g, b, h;
	byte *T1, *T2 ,*T3 ,*T4;

	T1 = EXT_VRAM;
	T2 = EXT_VRAM + 0x1000;
	T3 = EXT_VRAM + 0x2000;
	T4 = EXT_VRAM + 0x3000;

	//  for(Y=0; Y<M6HEIGHT; Y++) {
	for (Y = 0; Y< lines; Y++) {
		SETSCRVARM5(Y); /* Drawing area */
			for (X = 0; X<40; X++) {
				SeqPix21(BPal53[ ((*T1) & 0x80) >> 7 | ((*T2) & 0x80) >> 6 |((*T3) & 0x80) >> 5 | ((*T4) & 0x80) >> 4]);
				SeqPix21(BPal53[ ((*T1) & 0x40) >> 6 | ((*T2) & 0x40) >> 5 |((*T3) & 0x40) >> 4 | ((*T4) & 0x40) >> 3]);
				SeqPix21(BPal53[ ((*T1) & 0x20) >> 5 | ((*T2) & 0x20) >> 4 |((*T3) & 0x20) >> 3 | ((*T4) & 0x20) >> 2]);
				SeqPix21(BPal53[ ((*T1) & 0x10) >> 4 | ((*T2) & 0x10) >> 3 |((*T3) & 0x10) >> 2 | ((*T4) & 0x10) >> 1]);
				SeqPix21(BPal53[ ((*T1) & 0x08) >> 3 | ((*T2) & 0x08) >> 2 |((*T3) & 0x08) >> 1 | ((*T4) & 0x08)     ]);
				SeqPix21(BPal53[ ((*T1) & 0x04) >> 2 | ((*T2) & 0x04) >> 1 |((*T3) & 0x04)      | ((*T4) & 0x04) << 1]);
				SeqPix21(BPal53[ ((*T1) & 0x02) >> 1 | ((*T2) & 0x02)      |((*T3) & 0x02) << 1 | ((*T4) & 0x02) << 2]);
				SeqPix21(BPal53[ ((*T1) & 0x01)      | ((*T2) & 0x01) << 1 |((*T3) & 0x01) << 2 | ((*T4) & 0x01) << 3]);
				T1++; T2++; T3++; T4++;
			}


		}
		if ((scale == 2) && !IntLac) NOINTLACM5(Y);

	if (EndOfFrame) PutImage();
	refreshAll_flag = 0;
}


// ****************************************************************************
//         do_palet
// ****************************************************************************
// *************** fast palet ************************************
void do_palet(int dest,int src)
{
 int textpalet2[16]={0,4,1,5,2,6,3,7,8,12,9,13,10,14,11,15}; /*  color code-> VRAM code*/
 	// *************** for RefreshScr 53/54/62/63 ***************************
    if((CSS3 & 0x10) ==0 )	// CSS3 =0
    	{
	    if(dest>=0 && dest<32 && src>=0 && src<32)
	    	{
    	     BPal53[dest]= BPal62[src];
    	    }
    	}
	else					// CSS3 =1
		{
	    if(dest>=0 && dest<32 && src>=0 && src<32)
	       {
			int dest1,dest2;
	        switch( dest+1)
	       	{
			 case 16: dest1 =13; dest2 = 5; break;
			 case 15: dest1 =10; dest2 = 2; break;
			 case 14: dest1 =14; dest2 = 6; break;
			 case 13: dest1 = 1; dest2 = 9; break;
			}
			BPal53[16+dest1-1]= BPal62[src];
			BPal53[16+dest2-1]= BPal62[src];
		   }
    	}
    	
   // ************** for RefreshScr51/61 **************************
	if(dest>=0 && dest<16 && src>=0 && src<16)
        BPal[textpalet2[dest]]= BPal61[ textpalet2[src]];  
}


/* putOneChar */
/* VRAM : Virtual VRAM 
   sx,sy: start xy 
*/


#if 0
// ****************************************************************************
//        　デバッガ用の一文字出力
// ****************************************************************************
#define CHR_WIDTH 8

//extern OSD_Surface * screen;
void xx_putOneChar(int sx,int sy , char chr , char attr)
{
	int X,Y,K;
	ColTyp FC,BC;
 	byte *S;  //,*T1,*T2;
	byte *G;

	G= CGROM;
	X=0;
	sx*= CHR_WIDTH;
	sy*=12;
	for(Y=0; Y <10; Y++)
		{
	    debug_SetScrVar(Y+sy , X+sx );/* Drawing area */
		S = G+(chr <<4)+(chr &0x80?0x1000:0);	/* semi-graphics */
      	FC = BPal61[( attr)&0x0F]; BC = BPal61[(((attr )&0xF0)>>4)];  // BPal だと、パレットの影響受ける BPal61 でないとだめ
      	K=*(S+Y%10);
      	
		SeqPix21(K&0x80? FC:BC); SeqPix21(K&0x40? FC:BC);
		SeqPix21(K&0x20? FC:BC); SeqPix21(K&0x10? FC:BC);
      	SeqPix21(K&0x08? FC:BC); SeqPix21(K&0x04? FC:BC);
      	SeqPix21(K&0x02? FC:BC); SeqPix21(K&0x01? FC:BC);
    	}
}
#endif


// ****************************************************************************
//        　デバッガ用の一文字出力
// ****************************************************************************
#define CHR_WIDTH 6
#define CHR_HEIGHT 10

//extern OSD_Surface * screen;
void putOneChar(int sx, int sy, char chr, char attr)
{
	int X, Y, K;
	ColTyp FC, BC;
	byte *S;  //,*T1,*T2;
	byte *G;

	G = DEBUG_CGROM;
	X = 0;
	sx *= CHR_WIDTH;
	sy *= CHR_HEIGHT+1;
	for (Y = 0; Y < CHR_HEIGHT; Y++)
	{
		debug_SetScrVar(Y + sy, X + sx);/* Drawing area */
		S = G + (chr << 4) + (chr & 0x80 ? 0x1000 : 0);	/* semi-graphics */
		FC = BPal61[(attr) & 0x0F]; BC = BPal61[(((attr) & 0xF0) >> 4)];  // BPal だと、パレットの影響受ける BPal61 でないとだめ
		K = *(S + Y % CHR_HEIGHT);

		SeqPix21(K & 0x20 ? FC : BC); SeqPix21(K & 0x10 ? FC : BC);
		SeqPix21(K & 0x08 ? FC : BC); SeqPix21(K & 0x04 ? FC : BC);
		SeqPix21(K & 0x02 ? FC : BC); SeqPix21(K & 0x01 ? FC : BC);
	}
}


extern OSD_Surface *debug_surface;

// ****************************************************************************
//         RefreshDebugWindow:  デバッグウインドウの内容を、スクリーンに転送する
// ****************************************************************************
void RefreshDebugWindow(void)
{
		int w,h, amari_w;
		float tmp;
		w = screen->w;
		h = screen->h;
		//tmp = (float)h* 1.6; // 1.57;
		//w   = (int)(tmp + 0.5);		/* 四捨五入 */
		amari_w = (screen->w - w )/2;

	OSD_BlitSurface( screen     ,0+amari_w ,0, w      , h ,debug_surface ,0 ,0);  // to screen
	//OSD_BlitSurface( screen, 0, 0, 320, 200,  getRefreshSurface(), 0, 0);		// display emulator screen ??
}

/* RefreshDebug */
/* VRAM : Virtual VRAM 
   sx,sy: start xy 
*/

#if 0
void RefreshDebug(byte * VRAM, int sx ,int sy)
{
   byte X,Y,K;
   ColTyp FC,BC;
   byte *S,*T1,*T2;
  byte *G;

  G = CGROM;		/* CGROM */ 
  T1 = VRAM;		/* attribute data */
  T2 = VRAM+0x0400;	/* ascii/semi-graphic data */
  for(Y=0; Y<M5HEIGHT; Y++) {
    //SETSCRVARM5(Y+sy);	/* Drawing area */
    SetScrVar(Y+sy , X+sx );/* Drawing area */
    for(X=0; X<40; X++, T1++, T2++) {
      /* get CGROM address and color */
      S = G+(*T2<<4)+(*T1&0x80?0x1000:0);	/* semi-graphics */
      FC = BPal[(*T1)&0x0F]; BC = BPal[(((*T1)&0x70)>>4)|CSS2];
      K=*(S+Y%10);
      SeqPix21(K&0x80? FC:BC); SeqPix21(K&0x40? FC:BC);
      SeqPix21(K&0x20? FC:BC); SeqPix21(K&0x10? FC:BC);
      SeqPix21(K&0x08? FC:BC); SeqPix21(K&0x04? FC:BC);
      SeqPix21(K&0x02? FC:BC); SeqPix21(K&0x01? FC:BC);
    }
    if ((scale==2) && !IntLac) NOINTLACM5(Y);
    if (Y%10!=9) { T1-=40; T2-=40; }
  }
  if(EndOfFrame) PutImage();
}
#endif


               /* get CGROM address and color */
              // S = G+(*T2<<4)+(*T1&0x80?0x1000:0);
              // S = G+(*T2<<4)+(*T1&0x80?0x3000:0x2000);   /* for CGROM6 SR BASIC 2002/2/23*/
              //      K=*(S+Y%10);
            /*    if (Y%10!=9) { T1-=40*2; T2-=40*2; } */
							// MODE 6 のTEXT VRAMの先頭アドレスは、TEXTVRAMとする。 2003/10/25
