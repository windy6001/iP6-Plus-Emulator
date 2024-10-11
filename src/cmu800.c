/*
name is cmu800.c

*/

#include "types.h"
#include "P6.h"

/*
	CMU-800 のI/O ポート

　　多機種展開されていた関係上、ジャンパー切り替えで、I/Oポートの開始を設定できたようです。
　　P6では、10h から始まります。
　　他機種で使う場合は、適時、足したり引いたりしてください。


　　8253(1)
10h  チャンネル１　（メロディ）の音階設定
11h  チャンネル２　（ベース）の音階設定
12h  チャンネル３　（コード１）の音階設定
13h　8253(1)の設定

　　8253(2)
14h  チャンネル４　（コード２）の音階設定
15h  チャンネル５　（コード３）の音階設定
16h  チャンネル６　（コード４）の音階設定
17h　8253(2)の設定

	8255
18h  CV-OUT, TUNE-GATE ,KEY ON/OFF の設定
19h  内蔵リズムの発声
1Ah	 チャンネルセレクト、設定完了
1Bh  8255の設定


	チャンネル１の鳴らし方　の大きな流れ

1. 18H: KEY ON にする 
2. 1AH: LSBを1にする 　　(b3-b1を０にする（チャンネル１）)
3. 1AH: LSBを0にして設定反映モードにする 　　(b3-b1を０にする（チャンネル１）)
4. 10H: チャンネル１の周波数を出力する(下位）
5. 10H: チャンネル１の周波数を出力する(上位）
6. 1AH: LSBを1にして反映モード終了 　　(b3-b1を０にする（チャンネル１）)

7. 音の長さにより、必要な回数、1-6まで繰り返し実行する

18H 最終的に、KEY OFFする


説明

	音の高さは設定できるが、音の長さは設定できないのが不思議だったが、
	どうやら、小刻みに千切りされた音をつなげていくことで、音の長さを出すらしい
	音の長さが長いと、短い時と比べて、多くの、KEY ON の設定と、周波数の設定が行われる


KEY ON/OFFの設定

	18H: KEY ONのときは、MSBを0にする　OFFのときは1にする
	次の、周波数の設定と、セットで設定が行われるようだ。

周波数の設定

	チャンネルによって、つかうI/Oポートが異なっている。
	チャンネル1:10H チャンネル2:11H  チャンネル3:12H  チャンネル4:14H  チャンネル5:15H  チャンネル6:16H
	cmu800_freq[オクターブ][音名]に入っているデータを下位、上位の順番に出力します。
	演奏プログラム側から、鳴ってないチャンネルは、周波数データとして、0001Hが渡される。（意味なしデータ？）

設定の反映

	1AHのLSBを1→0にすることで、設定を反映させるモードになるらしい
	1AHのb3-b1が、チャンネル番号-1 です。終わったら、LSBを、1に戻します。

チャンネル１から６まで周波数を設定したら、リズムの発音制御をするらしい


テンポの速さ

	1AH のb7-b4 が入力になっていて、テンポに合わせて、0000か1111に自動的に変化します
	おそらく、CMU-800 のテンポつまみを回すことで、設定できると思われます。
　　演奏プログラム側でこれをみていて、ウエイトを入れるようだ


バグなど

	PSG音源なので、３和音までしか演奏できない

　　内蔵PSG音源でならしているのを、外部音源にすべき

　　CMU-800の音階は、10オクターブの途中まで出るらしいが、PSG音源は８オクターブぐらいしか出ないので音が出ない音階がある


　　CMU-800の16進数の周波数の値から一対一で、音階に変換しているので、１でも違うと、音が鳴らなくなる。
	付属のBIOS ROMによる、打ち込みソフトなら問題がないが。自作の演奏ソフトなどで、違う値を設定されると鳴らない

	他のエミュレーターではどうやっているのだろう？
*/

byte port18 = 0x00;				//I/O [18]  CMU-800
byte port19 = 0x00;				//I/O [19]  CMU-800
byte port1A = 0x00;				//I/O [1A]  CMU-800
int  cmu800_isKeyOn;			//設定中のキーの状態　TRUE:key on FALSE:key off
//int  cmu800_freq;				//設定中の周波数データ
static int tempo;				//テンポの今の値

// CMU-800 の周波数 [オクターブ][音階]
word cmu800_freqTable[][12] ={
	{0x9741,0x8EBE, 0x86BB, 0x7F2E, 0x780B, 0x714E, 0x6AED, 0x64EC, 0x5F41, 0x59E8, 0x54D9, 0x5015},
	{0x4B95,0x4754, 0x4353, 0x3F8D, 0x3BFC, 0x389E, 0x356E, 0x326E, 0x2F99, 0x2CED, 0x2A66, 0x2805},
	{0x25C5,0x23A5, 0x21A5, 0x1FC3, 0x1DFB, 0x1C4C, 0x1AB4, 0x1934, 0x17CA, 0x1674, 0x1531, 0x1401},
	{0x12E1,0x11D1, 0x10D1, 0x0FE1, 0x0EFD, 0x0E25, 0x0D59, 0x0C99, 0x0BE4, 0x0B39, 0x0A98, 0x0A00},
	{0x0970,0x08E8, 0x0868, 0x07F0, 0x077E, 0x0712, 0x06AC, 0x064C, 0x05F2, 0x059C, 0x054C, 0x0500},
	{0x04B8,0x0474, 0x0434, 0x03F8, 0x03BF, 0x0389, 0x0356, 0x0326, 0x02F9, 0x02CE, 0x02A6, 0x0280},
	{0x025C,0x023A, 0x021A, 0x01FC, 0x01DF, 0x01C4, 0x01AB, 0x0193, 0x017C, 0x0167, 0x0153, 0x0140},
	{0x012E,0x011D, 0x010D, 0x00FE, 0x00F0, 0x00E2, 0x00D5, 0x00C9, 0x00BE, 0x00B3, 0x00A9, 0x00A0},
	{0x0097,0x008E, 0x0086, 0x007F, 0x0078, 0x0071, 0x006A, 0x0064, 0x005F, 0x0059, 0x0054, 0x0050},
	{0x004B,0x0047, 0x0043, 0x003F, 0x003C, },
};

// PSG音源の周波数
word psg_freq[10][12] = {
	{0xee8, 0x0e12, 0x0d48 ,0x0c88 ,0x0bd4 ,0x0b2a ,0x0a8a ,0x09f2 ,0x964 , 0x8dc , 0x085e , 0x07e6}, 
};

// 音名
char *toneName[] = {"C","C#","D","D#","E","F","F#","G","G#","A","A#","B"};

// ******************************************************
//		CMU800_init
//		初期化
// ******************************************************

void CMU800_init(void)
{
	// PSG音源の周波数は、１オクターブ目だけセットされている。
	// オクターブがあがるごとに、半分になるように、初期設定

	for (int octave = 0; octave < 7; octave++) {
		for (int tone = 0; tone < 12; tone++) {
			psg_freq[octave+1][tone] = psg_freq[octave][tone]/2;
		}
	}
}

// ******************************************************
//			CMU800_DoOut
//		入力: Port: ポート番号
//			  Value:値
// ******************************************************
void CMU800_DoOut(byte Port, byte Value)
{
	switch( Port) {
	// ---------- 周波数の設定 ----------
	// 下位→上位の順番に、連続して出力する
	  case 0x10:
	  case 0x11:
	  case 0x12:
	  case 0x14:
	  case 0x15:
	  case 0x16:

		{
		int channel=0;
		switch (Port) {
			case 0x10: channel = 0; break;
			case 0x11: channel = 1; break;
			case 0x12: channel = 2; break;
			case 0x14: channel = 3; break;
			case 0x15: channel = 4; break;
			case 0x16: channel = 5; break;
		}
		static byte preValue;
		static flag = 0;
		PRINTDEBUG2(DEV_LOG, "OUT &H%02X,&H%02X:  ", Port, Value);
		if (flag == 0) {
			preValue = Value;
			flag++;
			PRINTDEBUG(DEV_LOG, "(下位データ) \n");

		}else {
			flag = 0;
			PRINTDEBUG(DEV_LOG, "(上位データ)\n");
			int octave=0,toneNo=0;
			if( channel >=3) break;		// TO FIX: チャンネル３以上だと何もしない

			if (cmu800_isKeyOn ) {
				int v = Value * 256 + preValue;
				if (cnv(v, &octave, &toneNo)) {
					PRINTDEBUG2(DEV_LOG, "(octave=%d toneName=%s)\n", octave, toneName[toneNo]);
					PSGOut(channel * 2 + 0, psg_freq[octave][toneNo] & 0xff);			// *** TEST CODE ****
					PSGOut(channel * 2 + 1, ((psg_freq[octave][toneNo] & 0xff00) >> 8) & 0xff);	// *** TEST CODE ****
					PSGOut(7, 0x38);
					PSGOut(channel + 8, 14);
				}else {
					PRINTDEBUG(DEV_LOG, "(NO DATA)\n");
					PSGOut(channel + 8, 0);
				}
			}else {
				PSGOut(channel + 8, 0);
			}
		}
		break;

	// -------- CV-OUT TUNE-GATE KEY ON/OFF ---------------
	//     b0-b5: CV-OUT データ　(外部端子に出力される)
	//     b6:    TUNE-GATE
	//     b7:    GATE-DATA 0で発音 1で消音
	  case 0x18:
			port18 = Value;
			cmu800_isKeyOn = (port18 & 0x80) ? FALSE : TRUE;
			PRINTDEBUG2(DEV_LOG, "OUT &H%02X,&h%02X  ", Port, Value);
			PRINTDEBUG3(DEV_LOG, "(CV-OUT:%X TUNE-GATE:%X GATE-DATA:%X)\n",Value & 0x1F,(Value>>6)&1,cmu800_isKeyOn);
			break;

      // ------- リズム音源 --------
	  // 0になったときに発音する　発音後１に戻される
	  case 0x19:
			port19 = Value;					// TO FIX: 何も実装してません
			PRINTDEBUG2(DEV_LOG, "OUT &H%02X,&h%02X  リズム\n", Port, Value);
			break;

	  // -------- チャンネルの設定を反映する ---------------
	  // b0: 1->0 になったときにbit 1-3 のチャンネルの設定を反映させる。その後１に戻さないといけない
	  // b1-3: 設定するチャンネル　0から７
	  // （エミュレータでは、周波数が出力された時に、PSGの周波数を変えてみる）
	  case 0x1A:
			{
			int channel = (Value >>1)& 7;
			//if( channel >2) break;			// TO FIX: とりあえず３チャンネルだけ（内蔵PSG音源で鳴らすので
			port1A = Value;
			PRINTDEBUG2(DEV_LOG, "OUT &H%02X,&H%02X  ", Port, Value);
			PRINTDEBUG1(DEV_LOG, "(Channel= %02X", channel);
			if ((Value & 1) == 0) {
				PRINTDEBUG(DEV_LOG, "反映\n");
			}else {
				PRINTDEBUG(DEV_LOG, "元に戻す\n");
			}
			}
			break;
	  // --------------- 8255 コントロールデータ -----------
	  case 0x1B:
			PRINTDEBUG2(DEV_LOG, "OUT &H%02X,&h%02X (8255 Control word)\n", Port, Value);
			break;
		}
	}
}

// ******************************************************
//       CMU800_DoIn
//      入力: Port  ポート番号
//            Value 値
// ******************************************************
void CMU800_DoIn(byte Port,byte *Value)
{
	switch( Port) {
	// -------  ------
	// 上位4bit 入力：テンポによって1111 or 0000 になる
		case 0x1A:
		{
			static int cnt=0;
			static byte tmp = 0xF0;
			cnt++;
			if(( cnt % tempo)==0) {
				tmp = (tmp == 0xF0) ? 0 : 0xF0;
				*Value = tmp;
				cnt=0;
				}
			break;
		}
	}
}

// ******************************************************
//       CMU800_setTempo
//      入力: _tempo 新しいテンポの値
// ******************************************************
void CMU800_setTempo(int _tempo)
{
	tempo = _tempo;;
}




// ******************************************************
//     CMU-800に渡される周波数データから、オクターブと、音名に変換する
//			入力:Value: 周波数データ
//				 octave:オクターブ
//				 toneNo:トーン番号（ドレミファソラシド）
// ******************************************************
static int cnv(word Value , int *octave , int *toneNo)
{
	int found = FALSE;
	for (int oct = 0; oct < 10; oct++) {
		for (int no = 0; no < 12; no++) {
			if (oct==9 && no==5) goto exit;	// DATAの終端に達したら抜ける
			if (Value == cmu800_freqTable[oct][no]) {
				found =TRUE;
				*octave = oct;
				*toneNo = no;
				goto exit;
			}
		}
	}
exit:
	return found;
}
