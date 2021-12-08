#include "Refresh.h"

typedef struct bmpfileHeader {
	char filetype[2];
	char filesize[4];
	char reserve[4];
	char imagedataOffset[4];
} _BMPFILEHEADER;

typedef struct bmpInfoHeader {
	char headerSize[4];		// 40
	char width[4];
	char height[4];
	char planes[2];			// 1
	char bitsPerPixel[2];	// ピクセルごとのビット数　0/1/4/8/16/24/32
	char compress[4];		// 圧縮タイプ　0: 圧縮なし
	char imageDataSize[4];	// 
	char suihei[4];
	char suichoku[4];
	char colorIndex[4];		// カラーパレットに格納される色数 8bit= 256
	char InportantIndex[4];

} _BMPINFOHEADER;

_BMPFILEHEADER bh;
_BMPINFOHEADER bi;

void writeData2(char *out , int value)
{
	int low = value & 0xff;
	int high= (int)value /256;

	out[0] = low;
	out[1] = high;
}

void writeData4(char* out , int value)
{
	int tmp0 = value & 0xff;
	int tmp1 = (value >> 8) & 0xff;
	int tmp2 = (value >> 16) & 0xff;
	int tmp3 = (value >> 24) & 0xff;

	out[0] = tmp0;
	out[1] = tmp1;
	out[2] = tmp2;
	out[3] = tmp3;
}


int SaveBmp(char *path , OSD_Surface *surface)
{
	memset(&bh, 0, sizeof(bh));
	bh.filetype[0] = 'B';
	bh.filetype[1] = 'M';

	memset(&bi,0,sizeof(bi));
	writeData4(bi.headerSize, 40);
	writeData4(bi.width , surface->w);
	writeData4(bi.height, surface->h);
	writeData2(bi.planes, 1);
	writeData2(bi.bitsPerPixel, surface->format->BitsPerPixel);
	writeData4(bi.compress, 0);
	writeData4(bi.colorIndex, 1>>surface->format->BitsPerPixel);

}
