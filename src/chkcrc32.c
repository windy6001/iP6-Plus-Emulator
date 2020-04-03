/** iP6: PC-6000/6600 series emualtor ************************/
/**                                                         **/
/**                        chkcrc32.c                       **/
/**                                                         **/
/*************************************************************/
// 参考: http://www.tnksoft.com/reading/zipfile/nonarc.php

static unsigned int table[256];

// 計算の元となるテーブルをあらかじめ作成
void InitCRC32(void)
{
	unsigned int poly = 0xEDB88320, u, i, j;

	for(i = 0; i < 256; i++){
		u = i;

		for(j = 0; j < 8; j++){
			if(u & 0x1){
				u = (u >> 1) ^ poly;
			}else{
				u >>= 1;
			}
		}

		table[i] = u;
	}
}

// crc32_startでは、前のバッファブロックのCRC32値をここに指定する。
// バッファが単一であるのなら、個々に引数は指定しなくてよい(0xFFFFFFFFが初期値)。
unsigned int GetCRC32(unsigned char *buffer, unsigned int bufferlen, unsigned int crc32_start)
{
	unsigned int i;
	unsigned int result = crc32_start;
	for( i = 0; i < bufferlen; i++){
		result = (result >> 8) ^ table[buffer[i] ^ (result & 0xFF)];
	}
	return ~result;
}

