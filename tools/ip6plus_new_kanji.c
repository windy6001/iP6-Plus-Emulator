// *******************************************
// Convert EXTKANJI.ROM to EXKANJI.ROM
//     name is ip6plus_new_kanji.c
//     by Windy
//     Date 2019/9/16
// *******************************************

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define ROM_SIZE 0x20000
#define MAX_PATH 256

enum {FALSE ,TRUE};
char inPath[MAX_PATH]  = "EXTKANJI.ROM";
char outPath[MAX_PATH] = "EXKANJI.ROM";

// *******************************************
// Convert EXTKANJI.ROM to EXKANJI.ROM
// *******************************************
int convKanji(void)
{
 unsigned char *buffer1;
 unsigned char *buffer2;

 // ************* input  **************** 
 FILE *fp = fopen(inPath,"rb");
 if( fp ==NULL) { printf("*** ERROR *** ファイル '%s'はオープンエラーです。 存在するか確認してください。\n", inPath); return(0);}
 
 buffer1 = (unsigned char*)malloc( ROM_SIZE*sizeof(char) );
 if( buffer1 == NULL) {printf("memory allocation error"); fclose(fp); return(0);}
 buffer2 = (unsigned char*)malloc( ROM_SIZE*sizeof(char) );
 if( buffer2 == NULL) {printf("memory allocation error"); fclose(fp); return(0);}

 memset(buffer1, 0xff ,ROM_SIZE*sizeof(char));
 memset(buffer2, 0xff ,ROM_SIZE*sizeof(char));

 if( fread( buffer1 , ROM_SIZE, 1, fp) !=1) { printf("*** ERROR *** ファイル'%s'の読み込みエラーです。\n",inPath); fclose(fp); return(0); }

 
 // ************* convert ***************
 for(int i=0; i< ROM_SIZE/2; i++)
	{
	 buffer2[i*2] = buffer1[i];
	 buffer2[i*2+1] = buffer1[i+ROM_SIZE/2];
	}
 fclose(fp);
 printf("データを読み込みました。\n");


 // ************* output **************** 
 fp =fopen(outPath,"r"); if(fp!=NULL) { printf("*** ERROR *** ファイル'%s'はすでに存在します。書き込みを中止しました。\n",outPath); fclose(fp); return(0);}

 fp =fopen(outPath, "wb"); if(fp==NULL){printf("*** ERROR *** ファイル'%s'のオープンエラーです。",outPath); return(0); }
 printf("データを書き込み中...");
 fwrite( buffer2, ROM_SIZE, 1,fp);
 fclose(fp);
 printf("完了しました\n");

 return 1;
}

int main(int argc, char *argv[])
{
	int ch;
	char str[1024];

	printf("\n\n");
	printf("iP6 Plus の拡張漢字ROMを、新フォーマット（左右、左右...）に変換します。\n");
	printf("ファイル '%s'から読み込んで ファイル '%s'に書き込みます。\n\n",inPath ,outPath);
	printf("準備はいいですか (y/n)?");
	ch= fgetc(stdin);
 	if( ch=='y' || ch=='Y') 
		{
		convKanji();
		}
	else   {
		printf("キャンセルしました。\n");
		}

	system("pause");
}
