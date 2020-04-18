  IRRC32(IRServer32) - ESP32で赤外線学習リモコン
=================================================

|①|②|③|
|---|---|---|
|![](DOC/livingS.png)|![](DOC/studyS.png)|![](DOC/bedroomS.png)|

ESP32-WROOM-32ボードに赤外線リモコン受信モジュールと赤外線LEDを接続し、学習リモコンにします。</br>
Wi-Fiで接続したスマホやPCなどから操作できます。

## 機能
一般的な「NECまたは家製協フォーマット」の赤外線リモコンに対応します。
- NECフォーマットまたは、家製協フォーマットの信号を学習可能（Sonyフォーマットは未対応）
- リモコン画面もカスタマイズ可能

× 状況に応じて送出信号が変化するエアコンなどでは、信号フォーマットを解析して、対応する信号を合成するための処理を(Javascriptで)書込む必要があります。
  また、３フレーム以上の信号形式（ダイキンエアコンなど）には対応できていません。</br>
× 学習できないリモコン信号もあると思われます、全てのリモコンに対応することはできませんことをご了承の上ご試用ください。</br>

全体概略図

![](DOC/system.png)

## 導入

#### ハードウェア(付加回路)
|付加部分の回路図|付加回路の実装|
|---|---|
|![](DOC/scam2.png)|![](DOC/IRRC32S2.jpg)|

#### ソフトウェア(Arduino)

|スケッチ名|機能|備考|
|---|---|---|
|IRRC32.ino|Webサーバー本体|FSWebServerを改変|
|IRdefs.h|各種設定値|SSID、パスワードなど|
|ir\_io.ino|赤外線受光、送信処理||
|sr\_cgi.ino|サーバー補助処理||
|mtqq.ino|mqtt処理||

#### スケッチを書込む前に：
IRServerでは、使用する画像、リモコンデータなどを記録するためにESP32-WROOM-32のフラッシュメモリ上に作られたファイルシステム(FFS)を使用します。

#### ソフトウェアの導入手順
1. このリポジトリからファイルをダウンロード
1. Arduinoのスケッチフォルダにダウンロードしたファイルに含まれているフォルダ「IRRC32」をフォルダごとコピー
1. Arduino IDEでスケッチブック内のスケッチ(IRRC32)を開く
1. 「IRdefs.h」の先頭部分にある下記の2行を環境に合わせて書換える

```IRdefs.h
const char* ssid = "wifi-ssid";
const char* password = "wifi-password";
```
5. スケッチをコンパイルしてESP32-WROOM-32に書込む
1. 「ツール」⇒「ESP32 Sketch Data Upload」でIRRC32/dataフォルダ内のリモコンデータをアップロード
1. シリアルモニターを起動
1. 表示されるESP32-WROOM-32に割振られたipアドレスを確認（例では192.168.0.141を仮定）
1. ブラウザから「192.168.0.141/edit」にアクセスできることを確認</br>

![](DOC/edit.png)

###### editの画面
- editはFSWebServerに組込まれているファイルブラウザです、</br>
  ファイルのアップロードやダウンロード、テキストの編集などができます。
## 動作確認
### IRServerの動作確認（デバッグ用コンソールを使用）
IRServerにはシリアルターミナルを使用して、ログ表示や動作を確認するためのコマンドを実行する機能があります。</br>
本格的な学習機能を使ったリモコンとして使用する前にデバッグコマンドを使用して、プログラムとハードウェアが基本的に機能しているか確認することできます。

ここでは「M5Stack」を使用して、実際に使用したい機器のリモコンを操作できるかを「簡単な受信／送信回路」を接続して実験する手順を示します。

### ソースコードの変更
IRdefs.hのポート設定を変更します。</br>
（下記の例では、ifdefで囲って変更しています、その他にWi-Fiのssidとpasswordも変更しています）
```
#ifdef ARDUINO_M5Stack_Core_ESP32
  #define PIN_ir_in    16   // IR Receiver
  #define PIN_ir_out   17   // IR Send 
  #define PIN_ir_LED    2   // Status LED
  #define A0           36   // Dummy
#else
  #define PIN_ir_in    12   // IR Receiver
  #define PIN_ir_VCC   13   // for IR receiver 3.3V
  #define PIN_ir_out   14   // IR Send 
  #define PIN_ir_LED   16   // Status LED
#endif
```

|簡単な送信／受信回路図|
|---|
|![](DOC/bread_IR1.png)|

|ブレッドボード配置|ブレッドボード写真|
|---|---|
|![](DOC/bread_IR2.png)|![](DOC/bread_IR3.png)|

#### リモコン信号の受信
IRServerの設定変更後、コンパイルしてターゲットにプログラムを書込みます。</br>
Arduino IDEのシリアルターミナルを起動して、デバッグコマンドを使って「リモコン信号の受信」が正しく行われることを確認します。

起動時の表示メッセージ
```
ets Jun  8 2016 00:22:57

rst:0x1 (POWERON_RESET),boot:0x17 (SPI_FAST_FLASH_BOOT)
configsip: 0, SPIWP:0xee
clk_drv:0x00,q_drv:0x00,d_drv:0x00,cs0_drv:0x00,hd_drv:0x00,wp_drv:0x00
mode:DIO, clock div:1
load:0x3fff0018,len:4
load:0x3fff001c,len:1216
ho 0 tail 12 room 4
load:0x40078000,len:9720
ho 0 tail 12 room 4
load:0x40080400,len:6352
entry 0x400806b8

FS File: /ac.htm, size: 1.14KB
FS File: /ac1.htm, size: 493B
FS File: /ac2.htm, size: 498B
FS File: /bedroom.htm, size: 748B
FS File: /cl.htm, size: 1.00KB
FS File: /cl11.htm, size: 479B
FS File: /cl12.htm, size: 466B

...中略

FS File: /tv.htm, size: 1.62KB
FS File: /tv1.htm, size: 473B
FS File: /tv2.htm, size: 465B
FS File: /tv3.htm, size: 448B
FS File: /IRdata.txt, size: 128.00KB

Connecting to Lisa
.
Connected! IP address: 192.168.0.34
handleFileRead: /index.htm
<!DOCTYPE html>
<html>
<head></head>
<body>IRServer32 Running ..</body>
</html>
room = 
HTTP server started

1>

```
リモコンコードをテーブルに読取ります。</br>

読取りコマンド：nR[Enter]</br>
"n"はテーブル番号を指定します。</br>
テーブル番号を省略した場合は、現在のテーブル番号を使用します。</br>
（現在のテーブル番号は、コマンドプロンプトに「n>」の形式で表示されます）

コマンド「1R[Enter]」で赤外線リモコン信号受信待ちになります、
赤外線リモコンセンサーに向けて読取りたいリモコンボタンを押してください。
```
1>1R


NEC [  67] : 9337+5120
 518+ 767  516+ 768  515+2021  519+ 768  515+ 768  515+ 768  516+ 768  516+2022 - 84
 518+2021  518+ 768  516+ 768  515+ 768  515+2022  518+ 768  516+2022  518+ 767 - 51
 516+ 767  613+1924  518+ 768  613+1924  518+2022  518+ 768  612+ 671  541+1996 - 9A
 619+1921  622+ 663  540+1997  543+ 742  541+ 742  541+1998  542+1997  543+ 742 - 65
 540+20051 !
```
例としてシーリングライトの「ON」ボタンを押しました。</br>
「NEC」フォーマットで 84h, 51h, 9Ah, 65h の32ビットのコードを読み取っています。</br>
読取ったコードは指定のテーブル(1)に保存されます。</br>
</br>
反応のない場合は、プログラム内の赤外線リモコンセンサー接続先のポート番号（PIN_ir_in）に誤りがあるか、ハードの接続に誤りがある可能性があります。</br>
設定や接続を確認してください。</br>

続けて、同じシーリングライトの「OFF」ボタンをテーブル(2)に読取ります。</br>
上記と同様にコマンド「2R[Enter]」入力し、リモコンの「OFF」ボタンを押しました。
```
1>2R


NEC [  67] : 9642+5194
 566+ 719  564+ 720  565+1973  565+ 721  562+ 721  563+ 721  514+ 768  516+2022 - 84
 561+1979  564+ 720  562+ 722  515+ 769  563+1974  565+ 721  561+1976  562+ 724 - 51
 560+ 723  562+1974  563+ 723  515+2022  564+ 722  557+1980  564+ 721  515+ 769 - 2A
 559+1979  559+ 726  561+1977  517+ 769  560+1977  562+ 724  557+1980  517+2023 - D5
 560+20051 !
```
これでシーリングライトの「ON」がテーブル(1)、「OFF」ボタンがテーブル(2)に読み取れました。</br>

次に、これらを送信して送信動作の確認を行います。</br>

送信コマンド：W[Enter]</br>
送信コマンドは、現在のデータバッファーを送出します。</br>
現在のデータバッファーではなく、特定テーブルのデータを送信するためには、データバッファに必要なテーブルデータを取出す必要があります。</br>

テーブルデータの取出しコマンド：nD[Enter]</br>
"n"はテーブル番号です。</br>

下記の例では、テーブル(1)のデータを取出し、送信しています。
```
2>1DW
1 : NEC [4,4] 84, 51, 9A, 65
[  68] Send
```
これでシーリングライトがオンになれば、正しく動作していることが確認できたことになります。</br>
オンにならない場合は、赤外線LEDの向きを対象機器に向けて何回か試してみてください。</br>
それでも反応のない場合は、プログラム内の赤外線LED接続先のポート番号（PIN_out_in）に誤りがあるか、ハードの接続に誤りがある可能性があります。</br>
設定や接続を確認してください。</br>

=>「簡単な受信／送信回路」のトランジスタを使って一個の赤外線LEDを発光させた場合では、赤外線信号が弱いです。</br>
　推奨のFETで４個の赤外線LEDを使用した場合は、十分な強さで発光します。

オンが動作したら、オフコマンドも試してみてください。</br>
```
1>2DW
2 : NEC [4,4] 84, 51, 2A, D5
[  68] Send
```
以上で基本的なリモコン信号の受信、送信機能の動作確認ができたことになります。</br>
以降では、「推奨の受信回路、送信回路」を接続してカスタマイズに進みます。</br>

## 学習
- リモコンデータ学習</br>
最初のリリースには、シーリングライト、テレビ、エアコンのリモコン画面のサンプルが含まれています。
シーリングライトやテレビであれば、サンプル画面を選んで、現用のリモコン信号を読取らせる（学習）ことで使用するリモコン信号を送出できるようになります。
エアコンでは送出されるリモコン信号が状況によって変わるために読取ったリモコン信号をそのまま送出しても期待した動作にはなりません、これらについては、別途カスタマイズ編で説明します。

- シーリングライトを例に説明します。

1.  使用する画面を選ぶ（cl1、cl2、cl3、cl5）</br>
1. 「ローカル」にある「clxx.htm」ファイルをテキストエディタで編集する</br>
  下記のファイルの```#img = 'cl5'```がリモコン画面に使用する画像ファイルの指定です。</br>
  この指定では```/S/cl5.jpg```を使用することになります。</br>
  （これらファイルは、"Arduinoスケッチフォルダ\IRRC32\data\"フォルダ下にあります）

```cl51.htm
#//
#// cl51.htm
#//
#Sroot      = '/S/' <= 画像関連ファイルを保存しているフォルダ
#Droot      = '/D/' <= リモコンデータを保存しているフォルダ
#title      = '照明学習用'
#name       = 'cl'
#pre_URL    = ''
#now_URL    = '/ir.cgi?rc=cl51'
#nextURL    = ''
#setup      = 1
#<RCmain.htm

#map_start  = 250
// -----------------------------------------------
#title      = '照明'
#name       = 'ODELIC_NRL_350P' <= リモコンデータを保存するファイル名
#data_set   = '1'

#gname      = 'cl'
#img        = 'cl5' <= 使用するリモコン画面の画像ファイル名
#def        = 'STD'

#<RCdefs.htm

#<RCtail.htm
```

3. 学習した赤外線データを保存するファイル名を書換える（例では"ABC\_CL100"）

```Name
Org #name       = 'ODELIC_NRL_350P'
New #name       = 'ABC_CL100' <= 実際のファイル名は"/D/clABC_100.1.c"となります
```
4. 書換えたテキストを名前を付けて保存（例では、"clnew.htm"）
5. ブラウザで「192.168.0.141/edit」にアクセスして、このファイル（"clnew.htm"）をESP32-WROOM-32のFFSにアップロード（「ESP32 Sketch Data Upload」でまるごとアップロードしても結構です）
6. ブラウザで「192.168.0.141/ir.cgi?rc=/clnew」にアクセス</br>

 ロード時のログ

```Serial
Connected! IP address: 192.168.0.141
Open http://com16.local/edit to see the file browser
HTTP server started

1>handleFileRead: /living.htm
handleFileRead: /edit.htm
handleFileRead: /index.htm
handleFileList: /
handleFileRead: /favicon.ico
handleFileRead: /cl11.htm
handleFileRead: /cl12.htm
handleFileRead: /cl21.htm
handleFileRead: /cl51.htm
ip:192.168.0.141,mask:255.255.255.0,gw:192.168.0.1
ip:192.168.0.141,mask:255.255.255.0,gw:192.168.0.1
ip:192.168.0.141,mask:255.255.255.0,gw:192.168.0.1
handleFileUpload Name: /clnew.htm
handleFileUpload Size: 458
handleFileList: /
handleFileRead: /clnew.htm
ip:192.168.0.141,mask:255.255.255.0,gw:192.168.0.1
handleFileRead: /clnew.htm
#include: /RCmain.htm:
#include: /RCmain.js:
#include: /S/title.js:
#include: /S/title.jpg:jpeg
#include: /RCdefs.htm:
#include: /S/cl5.js:
#include: /S/cl5.txt:
#include: /D/xxSTD.js:
! sr_error - /D/xxSTD.js:5 - File not found "/D/clABC_CL100.1.c" <= ファイルはまだ作成されていない
#include: /S/cl5.jpg:jpeg
#include: /RCtail.htm:
#include: /S/dummy.jpg:jpeg
```

7. 下図の(1)の「編集モード」ボタンをクリックして編集モードに入る</br>

![](DOC/clnew1.png)

8. (2)の「ECO」ボタンに対応する赤外線信号を読取り（学習）</br>
  現用リモコンの「ECO」に対応するボタンを押す用意をしてから(3)の「Read」ボタンをクリック</br>
  val0、val1 入力フィールドが灰色の状態になり、読取り状態になったことがわかります、</br>
  この状態で現用リモコンの「ECO」に対応するボタンを押します</br>
  ＊長押しないようにしてください

  「ECO」ボタンに対応する信号を正しく読取った状態

![](DOC/clnew2.png)

9. 上記の読取り操作を全て（例では全部で６個）のボタンで行い、</br>
  最後に「編集モード」ボタンをクリックして編集モードを終了する</br>
  学習データの保存ダイアログが表示されるので、「OK」をクリックして保存する（ESP32-WROOM-32上に保存される）

![](DOC/clnew3.png)

10. 保存された信号データは"clnew.htm"で指定した"/D/clABC\_CL100.1.c"に保存されている</br>
  保存された内容はファイルブラウザで確認できる</br>

![](DOC/clnew4.png)

11. "clnew.htm"をリロードし、対象の機器に向けて動作するか確認する</br>
  正しく動作すれば、学習手順は完了です、正しく動作しない場合は・・・(^-^;</br>

  このまま使用しても良いですし、複数のリモコンをまとめた複合リモコンを作成することもできます。</br>

  赤外線信号の読取り、データの保存、editでデータの確認、clnew.htmの再起動までのログ</br>

```Serial
IR exe=Read, btn=256, rep=, v0=, v1=  <= 最後の「OFF」ボタンの読取りコマンド

NEC [  67] : 10229+5092
 613+1928  612+1931  613+1930  612+ 663  612+ 657  613+1930  613+1932  612+1939 - E7
 612+ 654  612+ 659  613+1932  612+1934  613+ 656  612+ 661  613+ 658  612+ 671 - 0C
 613+1921  614+1931  612+ 661  613+1933  613+ 659  612+ 658  613+ 661  619+1931 - 8B
 612+ 655  613+ 658  613+1931  612+ 664  612+1927  613+1932  612+1931  612+ 656 - 74
 634+20050 !

IR exe=Update, btn=, rep=, v0=, v1=/D/clABC_CL100.1.c|250|260|　<= 読取った赤外線データの保存コマンド
/D/clABC_CL100.1.c
handleFileRead: /edit.htm <= editのリロード
handleFileRead: /index.htm
handleFileList: /
handleFileRead: /favicon.ico
handleFileRead: /D/clABC_CL100.1.c <= 保存データの確認
handleFileRead: /clnew.htm <= clnew.htmのリロード
#include: /RCmain.htm:
#include: /RCmain.js:
#include: /S/title.js:
#include: /S/title.jpg:jpeg
#include: /RCdefs.htm:
#include: /S/cl5.js:
#include: /S/cl5.txt:
#include: /D/xxSTD.js:
#include: /D/clABC_CL100.1.c:irdata
/D/clABC_CL100.1.c:irdata:250:10 <= 学習したデータでデータベースファイルを更新
250: :''　　　　　　　　　　　　　　　 （内容は先程の学習で更新されたままなので、書換は無い）
250:-:'',
251: :'000707E70CA25D'
251:-:'000707E70CA25D',
252: :'000707E70C8F70'
252:-:'000707E70C8F70',
253: :'000707E70C9B64'
253:-:'000707E70C9B64',
254: :'000707E70C9F60'
254:-:'000707E70C9F60',
255: :'000707E70C9768'
255:-:'000707E70C9768',
256: :'000707E70C8B74'
256:-:'000707E70C8B74',
257: :''
257:-:'',
258: :''
258:-:'',
259: :''
259:-:''
#include: /S/cl5.jpg:jpeg
#include: /RCtail.htm:
#include: /S/dummy.jpg:jpeg
handleFileRead: /favicon.ico
```

## カスタマイズについて
- [カスタマイズガイド(1) IRServerの仕組み](/DOC/IRRC321.md)
- [カスタマイズガイド(2) リモコン画面のボタンを増やしたい](/DOC/IRRC322.md)
- オリジナルな画像のリモコンを作りたい
=> 「リモコン画面のボタンを増やしたい」の画像ファイルが異なるだけで同様にオリジナルリモコンが作れます
- いくつかのリモコンをまとめた複合リモコンにしたい
=> 学習させた各リモコンファイルを繋げるだけです、living.htm、study.htmやbedroom.htmを参考にしてください
- エアコンのコントロールしたい（信号解析、信号生成方法など）<b><=【後日追加予定】</b>

## 変更／修正／訂正
- 2018/11/24 初版

## 参考情報
#### Raspberry piからIRServerに赤外線信号を送出させる

使用例）python irsend.py 192.168.0.141 126</br>
　192.168.0.141：IRServerのIPアドレス</br>
　126：送信したいリモコンデータのテーブル番号

```irsend.py
# -*- coding: utf-8 -*-
import sys
import urllib

args = sys.argv
url = "http://" + args[1] + "/IR?exe=Send&btn=" + args[2]
response = urllib.urlopen(url).read()
print "[" + response + "]"
```

## 参考
- 赤外線リモコンの通信フォーマット http://elm-chan.org/docs/ir_format.html

(c) goji2100.com
