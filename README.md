# iP6 Plus    (PC-6000/6600 Emulator) 

Last Update 2020/03/28 <br>
Release 4.9 Beta 3 (候補）<br>

## 1. はじめに

  <p>このプログラムは、NEC PC-6000/6600 シリーズをエミュレートするプログラムです。
  
  <p>isioさんの PC-6000/6600 シリーズのエミュレータである 「 iP6 for Unix/X11」をベースとして、
  SR特有の機能や、ディスク入出力機能等を　新たに追加しました。


## 2. 起動方法

Windows 

<p>バイナリーファイルの中の、iP6.exe をダブルクリックしてください。
ROMファイルが見つからない場合、「実行に必要なROMファイルを検索して、自動設定しますか？」と聞かれるので、はいと答えると、ディスクの中を検索します。ROMファイルが見つかると、「ROMファイルが見つかりました。」と表示されて、OKを押すとエミュレータが起動します。
または、メニューのControl → Configure から、ROMパス名を指定してください。

または、実行ファイルと同じディレクトリか、rom サブディレクトリに、必要なROMファイルをコピーしてください。


### Unix/X11

<pre>
- X11を、起動してください。
- ターミナルから、下記のコマンドで起動できます。

    $ ./iP6 

>　主なオプションについて
>
>    -60 .... PC-6001として起動する
>
>    -62 .... PC-6001mkII として起動する
>
>    -64 .... PC-6001mkIISR として起動する
>
>    -66 .... PC-6601 として起動する
>
>    -68 .... PC-6601SR として起動する


> -tape hoge.p6    hoge.p6 をテープイメージとしてマウントする
>
> -disk hoge.d88  hoge.d88 をディスクイメージとしてマウントする
</pre>

<p>X11版で、 ROMファイルが見つからない時は、終了します。
実行ファイルと同じディレクトリか、rom サブディレクトリに、必要なROMファイルをコピーしてください。

<hr>

## 3. できること

<pre>
オリジナルの iP6 for Unix/X11 に、いくつかのSR特有の機能とディスク入出力機能　などを追加します。

 - PC-6001/PC-6001mk2/PC-6601/PC-6001mk2SR/PC-6601SRの、それぞれの機種として動作可能。
 - PC-Unix ネイティブと Windows ネイディブでの動作をサポート
 - デバッグ機能をサポート
 - テープの高速読み書き機能をサポート。
 - ディスク入出力機能をサポート（2ドライブ対応)
 - SR特有の機能を　サポート
 - FM音源をサポート　（fmgen を使用。）
 - 拡張漢字ROMと、拡張RAM(64kb) をサポート。
 - 戦士のカートリッジ （初代） をサポート
 - 秋川様の互換ROM (PC-6001/PC-6601) をサポート
 - ローマ字入力機能をサポート
</pre>

<hr>

## 4. 使い方

### 4.1 注意してほしいこと

<p>このプログラムを使用されるときは、十分注意してご使用ください。
ROMイメージなど　大切なものに関しては、
**必ずバックアップをとっておいてください。**
　
<p>Rel.4.7 で、ディスクのアクセスランプを右下に作りました。(Windows版)
　<strong>これが点灯中は　ディスクにアクセスに行ってますので、いきなり終了したり、
　リセットしたりしないで下さい。</strong>


<p>  <strong>ディスクやテープにアクセス中は、どこでもセーブしないでください。どこでもロードした場合、正しい動作を期待できない場合があり、最悪の場合、ディスクイメージなどの破損につながります。</strong>

　
　<p>Rel.4.5 以降のバージョンから、d88のファイルが、読み取り専用になっている
　イメージも対応するようになりました。
　ただし、読み込みは出来ますが、書き込みは出来ませんので、注意が必要です。

　
　<p>ゲームでディスクにセーブしたい場合などは、必ず　<strong>読み書き可能な属性にしてからマウントしてください。</strong>
　BASIC上で、save"hoge"や、bsave"hoge"としても、??AT Error となってしまいます。
　なお、d88ファイル内部のヘッダの書込み禁止フラグは見ていません。
　
　

<hr>

4.2 必要なROM
　
　<p>実行するには、実機に内蔵されているROMファイルが必要になります。まずは、ROMを取得してください。
　
　<p>実行ファイルと同じディレクトリか、rom というサブディレクトリに置いてください。
　
　<p>
　<table border>
　<caption>(PC-6001 として利用する場合)</caption>
　<tr><th>ファイル名 </th><th>サイズ </th> <th>対応するROM</th></tr>
　<tr><td>BASICROM.60</td><td> 16KB</td> <td>N60-BASICインタプリタROM</td></tr>
　<tr><td>CGROM60.60 </td><td>  4KB</td> <td>N60-BASIC用CGROM        </td></tr>
　</table>
　<br>


　
　<table border>
　<caption>(PC-6001mk2 として利用する場合)</caption>
　<tr><th>ファイル名 </th><th>サイズ  </th> <th>対応するROM    </th></tr>
　<tr><td>BASICROM.62</td><td> 32KB </td> <td>N60m-BASICインタプリタROM</td></tr>
　<tr><td>CGROM60.62 </td><td>  8KB </td> <td>N60-BASIC用CGROM</td></tr>
　<tr><td>CGROM60m.62</td><td>  8KB </td> <td>N60m-BASIC用CGROM</td></tr>
　<tr><td>KANJIROM.62</td><td> 32KB </td> <td>漢字ROM        </td></tr>
　<tr><td>VOICEROM.62</td><td> 16KB </td> <td>音声合成ROM    </td></tr>
　</table>
　<br>
　
　<table border>
　<caption>(PC-6601 として利用する場合)</caption>
　<tr><th>ファイル名 </th><th>サイズ  </th> <th>対応するROM     </th></tr>
　<tr><td>BASICROM.66</td><td> 32KB </td> <td>N66-BASICインタプリタROM </td></tr>
　<tr><td>CGROM60.66 </td><td>  8KB </td> <td>N60-BASIC用CGROM</td></tr>
　<tr><td>CGROM66.66 </td><td>  8KB </td> <td>N66-BASIC用CGROM</td></tr>
　<tr><td>KANJIROM.66</td><td> 32KB </td> <td>漢字ROM         </td></tr>
　<tr><td>VOICEROM.66</td><td> 16KB </td> <td>音声合成ROM     </td></tr>
　</table>
　<br>
　
　<table border>
　<caption>(PC-6001mk2SR として利用する場合)</caption>
　<tr><th>ファイル名    </th><th>サイズ</th><th> 対応するROM  </th></tr>
　<tr><td>SYSTEMROM1.64 </td><td>64KB </td><td> システムROM1 </td></tr>
　<tr><td>SYSTEMROM2.64 </td><td>64KB </td><td> システムROM2 </td></tr>
　<tr><td>CGROM68.64    </td><td>16KB </td><td> CGROM (saver3で取り込んだROM)</td></tr>
　</table>
　<br>
　
　<table border>
　<caption>(PC-6601SR として利用する場合)</caption>
　<tr><th>ファイル名    </th><th>サイズ</th><th> 対応するROM  </th></tr>
　<tr><td>SYSTEMROM1.68 </td><td> 64KB </td><td> システムROM1 </td></tr>
　<tr><td>SYSTEMROM2.68 </td><td> 64KB </td><td> システムROM2 </td></tr>
　<tr><td>CGROM68.68    </td><td> 16KB </td><td> CGROM (saver3で取り込んだROM) </td></tr>
　</table>
　<br>
　
　オプション機能
　<table border>
　<caption>(拡張漢字ROM (PC-6601-01 / PC-6007SR) を、利用する場合)</caption>
　<tr><th>ファイル名    </th><th>サイズ</th><th> 対応するROM  </th></tr>
　<tr><td><del>EXTKANJI.ROM</del> → <span style="color:red;">EXKANJI.ROM</span>  </td><td>128KB </td><td> 拡張漢字ROM 　　　　　　　　　 </td></tr>
　</table>

<br>
### 重要なお知らせ

  <p>Rel 4.5から、4.8 までの拡張漢字ROMのファイルは (EXTKANJI.ROM) でしたが、4.9以降は新ファイル名＆新フォーマットに変更になります。理由としては、他のエミュレータとの齟齬が発覚したためです。

  
  <p>拙作の<a href="http://www.eonet.ne.jp/~windy/pc6001/p6soft.html">ksaver</a> で取り込まれた方は、お手数ですが、下記の通り実行してください。

  <pre>
   1. エミュレータと同時配布の ip6plus_new_kanji.exe を拡張漢字ROMのあるディレクトリーにコピーする
   2. ip6plus_new_kanji.exe を実行する
   3. EXTKANJI.ROM から、EXKANJI.ROM に変換されます。EXKANJI.ROM を使用してください。
  </pre>
　<p>ちなみに、えすびさんの取り込みソフト <a href="http://sbeach.seesaa.net/article/387861522.html">saverkanji.zip</a>で取り込まれる場合は、上記の処理は不要になります。


　<hr>

## ROMの取得方法
　
　<p>基本的には、iP6 のROMを使いまわしできるのですが、SRをお持ちの方は、拙作の <a href="http://www.kisweb.ne.jp/personal/windy/computer/pc6001/p6soft.html">saver3</a> での吸出しをお勧めします。
　<p>(PC-6601SR の場合です。PC-6001mk2SRの方は、拡張子を.64にして下さい）
　
　<pre>
　1. MODE 6で起動して、拙作のsaver3 で次のROMを取得して下さい。
　
　SYSTEMROM1-1 を取得して、ファイル名を仮に SYSROM11にする。
　SYSTEMROM1-2 を取得して、ファイル名を仮に SYSROM12にする。
　SYSTEMROM2-1 を取得して、ファイル名を仮に SYSROM21にする。
　SYSTEMROM2-2 を取得して、ファイル名を仮に SYSROM22にする。
　CGROM　を取得して、　　　ファイル名を CGROM68.68 にする。
　
　2. SYSTEMROM が それぞれ別々に鳴っているので、下記のようにして結合してください。
　
　　　DOSなら、
　　　copy /b  SYSROM11+SYSROM12   SYSTEMROM1.68
　　　copy /b  SYSROM21+SYSROM22   SYSTEMROM2.68
　　　
　　　Unixなら、
　　　cat SYSROM11 SYSROM12  > SYSTEMROM1.68
　　　cat SYSROM21 SYSROM22  > SYSTEMROM2.68
　　　
　　　これでできあがりです。
　　　</pre>
　　　　<p>SR以外の方は、<a href="http://www.retropc.net/isio/">isioさんの saver </a>の使用をお勧めします。
　　　　
　　　　
　　　　<p>テープ経由の場合、最終的にテープ音声から、データに変換する必要があります。 
　　　　<a href="http://webclub.kcom.ne.jp/mb/morikawa/index.html">morikawa さんの　P6DatRec </a>の使用をお勧めします。
　　　　
　　　　<hr>

## ディスクについて
　　　　
　　　　<p>今のところ、下記のような設定になっています。
　　　　
　　　　<table border>
　　　　<tr><th>機種        </th><th>ドライブ</th><th>使用可能なディスク </th></tr>
　　　　<tr><td>PC-6001mk2  </td><td>外付</td><td> 1D                 </td></tr>
　　　　<tr><td>PC-6001mk2SR</td><td>外付</td><td> 1DD/ 1D  自動認識  </td></tr>
　　　　<tr><td>PC-6601     </td><td>内蔵</td><td> 1D                 </td></tr>
　　　　<tr><td>PC-6601SR   </td><td>内蔵</td><td> 1DD　(1Dも一部可能)</td></tr>
　　　　</table>
　　　　
　　　　<p>mk2SRのみ、起動時に、1D/1DDを自動認識します。
　　　　PC-6601SRの場合、基本的に1DDですが、ディスクの先頭に　'SYS'が書かれていると、1Dと認識するようです。ただし、この時の1Dは、読み込みしか出来ません。
　　　　
　　　　<p>サポートしないディスクを指定した場合、ディスクを、自動的にイジェクト
　　　　してしまいます。ディスクなしになってしまうので、注意してください。
　　　　そのときは、メッセージで、お知らせします。
　　　　

<br>
<br>
<!--
### 4.3 Windows 版の起動の仕方

 <pre>
 - 実機からROMファイルを取り込んでください。
 - 実行ファイルと同じディレクトリか、ROMサブディレクトリに、上記のROMを置いてください。
 - iP6.exeを実行させると、まず、デフォルトでPC-6601SRとして起動しようとします。
 - このときに、ROMファイルが見つからない場合は、その旨を表示します。
 - PC-6601SR 以外として動作させたい場合は、Control メニューのConfigureの設定パネルで　ご希望の機種を選択してください。
　</pre>

  <p>設定のうち、(*)マークのついている分については、変更後、再起動するかどうか聞いてきますので今すぐ適用したい場合、再起動してください。
  設定は、ip6kai.iniファイルに書き込まれます。（実行ファイルと同じディレクトリーに　自動的にできます。）

  <p>詳しい使い方については、同梱のHELPファイルを参照してください。(＿)
-->
　　　　


<hr>
## 5. 開発者情報

### 5.1 ライセンス

同梱のLICENSE ファイルを参照ください

5.2 ビルドの仕方

5.2.1  Windows 

    - Windows 10を入れてください
    - Visual Studio を入れてください
    - [libpng](http://www.libpng.org/pub/png/libpng.html) [zlib](http://www.gzip.org/zlib/) のソースリストを落としてくる
    - libpng とzlib のスタティックライブラリ版( libpngd.lib , zlibd.lib ) をビルドする。
    - libpngd.lib と、zlibd.lib を、c:\library\lib にコピーしてください。　（デフォルトの場合）
    - Win_Project/iP6plus.vcxproj を開いて、ビルドしてください。
    
#### 5.2.2 Unix系OSでのコンパイルの仕方


     - OSと、X Window System を入れてください。
     - X11 ライブラリがない場合は、入れてください。
     - libpng と zlib とXaw と OpenAL のライブラリがない場合、入れてください。
     - 適当なディレクトリで、iP6 Plusのソースを展開してください。
     - ./configure   を実行してください
     - make   を実行してください
     - src ディレクトリの下の iP6 が実行ファイルです。


    注意：Mac 用のX11は、<a href="https://www.xquartz.org/"> XQuartz</a>を使ってください。
    　

### 5.3 fmgen の変更点

fmgen.cpp 343行目 void Operator::MakeTable() の中、処理系によっては、優先順位が変わるための対策をしました。

    *p++ = p[-512] / 2;
　　　　↓

    *p = p[-512] / 2;
    p++;
　　　　
file.cpp のWin32 依存部分を #ifdef WIN32 で囲いました。
opna.cpp と、opm.cppの、pow関数の呼び出し、pow(10,db / 40.0) となっているところを、pow(10.0 , db /40.0) というふうに変更しました。
　　　　
<hr>

## 6. 既知の問題
　　　　
- 一部動かないゲームがあります。（ハドソン系はダメです)
- フロッピィドライブは　一部のコマンドしかサポートしていません。
- Unix環境で音が正しく鳴らないかもしれません。
- Unix環境では、設定ダイアログなどが、オリジナルの iP6 0.6.4 のままです。
- SRの音声合成機能は、ちゃんと喋れません。
- 動作がちょっと重いです。


<hr>

## 7. 更新履歴

### Rel 4.9 βの変更点

 - かなのローマ字変換時にスペースキーが押せない問題を修正
 - かなモード時に、Page Upを単体で押すと、常にかな、カナ切り替えになってしまう問題を修正
 - voice.c の中の並び方を変更した
 - ローマ字変換のローマ字を追加した
 - どこでもSAVEで、FDCのI/O DC の値を保存していない問題を修正しました
 - どこでもSAVEで、ノーウエイトの開始アドレスと終了アドレスを保存してない問題を修正しました。
 - ローマ字変換のつづりを修正しました
 - テープにセーブするとき、1バイト出力するごとにファイルをオープン・クローズするようにした


### Rel 4.9 β2の変更点

 - 秋川様のPC-6001/ PC-6601 互換ROMに対応しました。
 - PC-6001 のメモリマップを修正しました。
 - PC-6001 でMODEキーを無効にしました。
 - モニターモードで g / G で再開した時に、ALT押したままにしないようにしました。
 - EXTROM 刺された時のみ さすようにしました。
 - 拡張漢字ROM 作成機能と、ローマ字変換機能を、MIT ライセンスにしました。
 - カタカナ入力モードで、ファンクションキーが、常に SHIFT+ファンクションキー　で入力される問題を修正しました。
 - ローマ字変換機能で、「DDA」「DDO」に対応しました。
 - プログラムの一部を、ノーウエイトで動作させることができる機能を追加しました。( 今回の目玉機能、重い処理をかっとばせるようになります。)　
<br>
　　※デバッグモニターを開いて、nowait [start-addr] [end-addr]　コマンドを使用してください。PCレジスターが、start-addr のアドレスに到達すると、
ノーウエイトモードに移行して、end-addr に到達したら、通常モードに戻ります。<br>


### Rel 4.9 β1の変更点

- 拡張漢字ROM のファイル名と並び方を変更しました。
- About ダイアログの、状態表示を増やしました。（拡張漢字ROMの状態と、現在のロケールは何で認識されているか）
- ウインドウのタイトルバーに機種表示と、fps表示を常に行うようにした

  - 拡張漢字ROM の並び方を変えたので、4.9にあげました。
  - 拡張漢字ROM の状態は、EXKANJIROM_ROM だとROMを認識している、EXKANJIROM_MAKEだとWindows のフォントで自動生成している、EXKANJIROM_NONだとROM なしです
  - ロケールは、LANG_JP だと日本語で、 LANG_EN だと英語です

### Rel 4.8 β5の変更点 

- Windowsで、日本語環境以外の場合、なるべく英語表示するようにした（つもり）。
- Windowsの PAUSEメニューで、一時停止できるようにした

### Rel4.8 β3の変更点

- 漏れていたソースリストを追加した
- Unix/X11 環境でビルド出来なかった問題を修正（したはず） 


### Rel4.8 β2の変更点 

- デフォルト設定をスキャンライン無しにした
- デバッガーで、ダンプメモリーのときに、メモリーの内容を変更しようとすると、落ちていた問題を修正しました。


### Rel4.7以降の変更点 

- [Windows] PC-6601SRで、DATE$と、TIME$への書き込みが出来るようにしました。(起動時にOSの日時を取得して、それ以降、1秒ごとにタイマーを発火させて、1秒ずつ足していきます｡
- ブロック転送命令などの、必要CPUクロックが 0だったのを修正しました。（PC-6601SRの起動メニューの画面表示が速すぎる問題など解消しました）
- [Windows] 最大化できるようにしました
- [Windows] ウインドウの大きさを自由に変更可能に
- 起動時の RAMの初期状態をある程度再現しました。
- どこでも SAVE / LOAD 実装してみた。
- 未定義命令のサポートを追加しました。
- PC6001V のスケジュールを取り込みました。
- 音声合成機能を組み込んでみた。
- PC-6601SRの起動メニューから、テロッパを選ぶと、右下の表示がおかしくなる問題を修正しました。
- デバッグ機能を追加しました。
- デバッグ機能で、逆アセンブラリストをスクロール可能にしました。
- 本来入力できないキー（Shift＋￥）を、入力できないように修正しました。
- 本来入力できるはずのキー（かなの句読点）を入力できるように修正しました。
- ディスクを２ドライブ対応しました。

</pre>



<hr>

## 参考資料

 - PC-Techknow6000Vol.1
 - PC-6001mk2取扱説明書
 - Mr.PCテクニカルコレクション
 - PC-6001/PC-6001MK2  わかるマシン語入門
 - PC-9800 シリーズ テクニカルデータブック HARDWARE  編
 - PC-9821 Undocument   ( http://www.webtech.co.jp/undoc/io_2d.txt )


<hr>

## 免責事項

<p> ソースリストや、実行ファイルは基本的に無保証です。
テストは行っておりますが、使用した結果、何らかの損害があったとしても、当方では 一切感知できませんので、宜しくお願いします。

特に、大切なROMファイルなどは、必ずバックアップしたものを使用してください。



<h2>## 謝辞</h2>

 - iP6 for X11 を作られた 石岡さんに 感謝します。
 - fMSXの作者Marat Fayzullin さんに感謝します。
 - PC6001V の作者 ゆみたろさんに感謝します。
 - iP6win の作者 守谷さんに感謝します。
 - M88のDITTと、fmgen の作者のCISCさんに感謝します。
 - 1DDitt の作者の　bobsaito さんに感謝します。
 - アイコンの作者　天丸さんに感謝します。
 - PC6001VWの作者 Bernieさんに感謝します。
 - PC6001VXの作者 eighttails さんに感謝します。
 - PC-6001とPC-6601の互換ROMの作者 秋川さんに感謝します。
 - バグをたくさん見つけてくれた、チョコぼんさんに、感謝します。
 - そして、このパソコンを作られた　日本電気の方々に感謝します。

<p>石岡さん本当にありがとうございました。(__)

<p>iP6 がなかったら、とてもじゃないけど、一からこれを作るのは、
無理だったと思いますので、感謝しても感謝しきれないです。（＿）




<hr>
<h2>## お問い合わせ先</h2>


<p>なお、プログラム自体に関する　お問い合わせについては、<big><a href="http://www.eonet.ne.jp/~windy/index.html">全て私までお願いします。</a></big>

<p>(isioさんは　このプログラムの開発に直接は関わってませんので、これについてisioさんに問い合わせることは、ご遠慮下さい。）

<P>バグや使用上の問題については、出来れば、対応したいと思っています。
しかし、フリーソフトウエアで有る以上、限界というものも有りますので、よろしくお願いします。




</body>
</html>
