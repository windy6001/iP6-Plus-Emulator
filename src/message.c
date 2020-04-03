/*
Copyright (c) 2020 windy

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
/**                           message.c                     **/
/** by Windy 2003                                           **/
/*************************************************************/
/*
What? 

    １番上のメッセージを渡すと、日本語ロケールのときは、３つ目のメッセージを返却します。
	日本語以外のロケールのときは、２つ目のメッセージを返却します。
    １番目のメッセージと、どれも一致しなければ、引数をそのまま返却します。

	ようするに、日本語ロケール以外だと、世界の共通語の英語に変換します。
*/

#define MSG_EN 1
#define MSG_JP 2


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "message.h"
#include "os.h"



static unsigned char *translate[][3]={
{
	"MSG_ENABLE_CONFIG_REBOOT",
	"To enable the new change after reboot this app.",
	"(*)マークの設定は、iP6 Plusを再起動後に有効になります。",
},
{
	"MSG_1D_FDD",
	"[WARNING] The PC-6601SR/ PC-6001mkIISR cannot use 1D format disk, \n You must convert 1DD format disk.",
	"警告！　SR で、1Dタイプのディスクを使うときは、ファイル変換しないと使用できません。",
},
{
	"MSG_NO_1DD",
	"[ERROR]    PC-6001/6001mkII/6601 cannot use 1DD format disk. This disk ejected.",
	"この機種で 1DDタイプのディスクは使えないので、イジェクトします。",
},
{
	"MSG_RESET_CPU",
	"RESET CPU Ok?",
	"リセットしてもいいですか？",
},
{
	"MSG_EXIT",
	"Exit Ok?",
	"終了してもいいですか？",
},
{
	"MSG_FAILED_RESET",
	"RESET failed!  check ROM files.",
	"リセットに失敗しました。　ROMファイルをチェックしてください。",
},
{
	"MSG_REBOOT_ENABLE_CONFIG",
	"Reboot to enable new setting?",
	"設定を有効にするために、今すぐ再起動しますか？",
},
{
	"MSG_ROM_FOUND_BOOT",
	"ROM file found.  Launching... ",
	"ROM ファイルがみつかりました。起動中です。",
},
{
	"MSG_REWIND_LOAD_TAPE",
	"May the LOAD TAPE rewind?",
	"LOAD用 テープを巻き戻してもいいですか？",
},
{
	"MSG_REWIND_SAVE_TAPE",
	"May the SAVE TAPE rewind?",
	"SAVE用 テープを巻き戻してもいいですか？",
},
{
	"MSG_SEARCH_ROM",
	"ROM file not found.  Do you try search ROM file ? ",
	"ROM ファイルが見つかりません。　ROMファイルを検索してみますか? ",
},
{
	"MSG_SAME_DISK_IMAGE",
	"Do not put the same disk to drive 1 and drive 2",
	"ドライブ1と、ドライブ2 に同じディスクを入れないで下さい",
},
{
	"MSG_ROM_NOT_FOUND",
	"ROM file is not found.",
	"ROM ファイルが見つかりません。",
},
{
	"MSG_NOT_FOUND_COMPATIBLE_ROM",
	"ROM file is not found. Launch with compatible rom ?",
	"ROMファイルが見つかりません。互換ROMで起動しますか？",
},
{
	"MSG_COMPATIBLE_ROM",
	"Launch with compatible rom ?",
	"互換ROMで起動しますか？",
},
{
	"MSG_SET_ROM_PATH",
	"Set ROM file path in Control-> Configure menu or...",
	"Control->Configure メニューでROM ファイルのパスを設定するか、",
},
{
	"MSG_COPY_ROM_FILE",
	"copy ROM file to the following path",
	"下記のパスにROM ファイルをコピーしてください"
},
{NULL,NULL,NULL}
};


//char *mgettext(char *key)
unsigned char *mgettext(unsigned char *key)
{
    int  i;
    int  destIdx;
    unsigned char *p;
	int  lang;

	lang = OSD_getlocale();
	destIdx = MSG_EN;
	if (lang == OSDL_JP)
		{
		destIdx = MSG_JP;
		}

    p= key;
    for(i=0; translate[i][0]!=NULL ; i++)
        {
         if( strncmp( key, translate[i][0],strlen(translate[i][0])) ==0)
            {
             p=  translate[i][destIdx];
             break;
            }
        }
    return( p);
}



