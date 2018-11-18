# トランジスタ技術 2017年3月号・2016年9月号ESP8266特集 ESP8266モジュール用スケッチ・最新ESP32用スケッチ

## 1_practice

マイコン練習用サンプル・プログラムが含まれているフォルダです。

## 2_example

IoT機器用のサンプル・プログラムが含まれているフォルダです。

### 1-1 Wi-Fi インジケータ

LEDをWi-Fi経由で制御するワイヤレスLチカ実験を行います。

	ESP-WROOM-02用　：example01_led
	ESP32-WROOM-32用：example33_led
	
	a:TELNET版
	b:HTTP版
	d:ソフトウェア無線AP版
	s:IPアドレス固定版

### 1-2 Wi-Fi スイッチャ

スイッチ状態をWi-Fi送信します。後述のWi-Fi LCDなどで受信することが出来ます。

	ESP-WROOM-02用　：example02_sw 
	ESP32-WROOM-32用：example34_sw

### 1-3 Wi-Fi レコーダ

アナログ入力値をWi-Fi送信します。

	ESP-WROOM-02用　：example03_adc
	ESP32-WROOM-32用：example35_adc

### ケチケチ運転術

乾電池などで動作するIoTセンサ用の基本形です。応用しやすいように機能を絞り込みました。

	ESP-WROOM-02用　：example04_le 
	ESP32-WROOM-32用：example36_le
	
	a:Wi-Fi接続タイムアウト機能つき
	c:クラウドサービスAmbientへの送信機能つき
	m:取得値が近いときは送信しない節約機能つき

### 1-4 Wi-Fi LCD

Wi-Fi スイッチャや各種IoTセンサが送信したデータを液晶ディスプレイ（LCD）へ表示する基本形です。後述の応用例「Wi-Fi コンシェルジェ 掲示板担当」の基本動作を理解することが出来るでしょう。

	ESP-WROOM-02用　：example05_lcd
	ESP32-WROOM-32用：example37_lcd
	
	-:UDP版
	a:TCP版（aの無いものが通常のUDP版）

### 2-1 (IoTセンサ) Wi-Fi 照度計

IoTセンサは、システムの中でさまざまな物理量データを集める役割を担います。  
Wi-Fi 照度計は照度値を送信するIoTセンサです。時間帯や照明機器、人影などによって照度が変化するので、室内環境を把握する基本的な機器のひとつです。また、センサを手で遮るなど、簡単にセンサ値を変化させることが出来るので、センサの初期実験に便利です。

	ESP-WROOM-02用　：example06_lum
	ESP32-WROOM-32用：example38_lum

### 2-2 (IoTセンサ) Wi-Fi 温度計

温度値を送信するIoTセンサです。同じ建物内でも、部屋や測定箇所（足元と空調機の近く、窓付近など）によって異なることが多く、複数のIoTセンサによるセンサネットワークを構築するときの代表的なセンサのひとつです。

	ESP-WROOM-02用　：example07_temp 
	ESP32-WROOM-32用：example39_temp

### 2-3 (IoTセンサ) Wi-Fi ドア開閉モニタ

ドアや窓が開いたとき（または閉まったとき）に開閉状態を送信するIoTセンサです。防犯や人の出入りなどを把握します。

	ESP-WROOM-02用　：example08_sw 
	ESP32-WROOM-32用：example40_sw

### 2-4 (IoTセンサ) Wi-Fi 温湿度計

温湿度を送信するIoTセンサです。デジタルのI2Cインタフェースをもつセンサを使用することで、アナログ回路やセンサ値の補正が不要となり、正確な値を手軽に得ることが出来ます。

	ESP-WROOM-02用　：example09_hum_sht31
	ESP32-WROOM-32用：example41_hum_sht31
	
	表示なし:TI製 HDC1000 版
	sht31　 :SENSIRION製 SHT31 版
	si7021　:SILICON LABS製 Si7021 版

### 2-5 (IoTセンサ) Wi-Fi 気圧計

気圧と温度を送信するIoTセンサです。気圧高いと晴れ、低いと天気が崩れる傾向や、海水面からの標高によっても変化します。天候を予想したり、換気扇や窓の開け閉めなどによる室内の圧力変化を検知することも出来るでしょう。

	ESP-WROOM-02用　：example10_hpa
	ESP32-WROOM-32用：example42_hpa
	
	表示なし：STマイクロ製 LPS25H 版
	bme280　：Bosch製 BME280 版

### 2-6 (IoTセンサ) Wi-Fi 人感センサ

人体などの動きを検出したときに送信するIoTセンサです。在室情報などを把握することが出来ます。

	ESP-WROOM-02用　：example11_pir
	ESP32-WROOM-32用：example43_pir

### 2-7 (IoTセンサ) Wi-Fi 3軸加速度センサ

加速度の変化を検出したときに送信するIoTセンサです。ドアや窓の開閉や傾きの変化などを検出することが出来ます。

	ESP-WROOM-02用　：example12_acm
	ESP32-WROOM-32用：example44_acm

### 2-8 NTP時刻データ転送機

現在時刻をインターネットから取得してWi-Fi送信する機器です。あたかも「時刻センサ」のような動作を行います。センサネットワークではセンサ値と時刻情報を紐づけて取り扱うことが多いので、インターネットから時刻を取得するプロトコルNTPを利用することも多いでしょう。

	ESP-WROOM-02用　：example13_ntp
	ESP32-WROOM-32用：example45_ntp
	
	c:クラウドサービスAmbientへの送信機能つき

### 2-9 (IoTセンサ) Wi-Fi リモコン赤外線レシーバ

赤外線リモコン信号を受信し、受信データをWi-Fi送信します。家電などの操作情報を収集します。収集した情報の集計を行うことで、よく見るテレビのチャンネルやエアコンの使用時間などを把握することも出来るでしょう。

	ESP-WROOM-02用　：example14_ir_in
	ESP32-WROOM-32用：example46_ir_in

### 2-10 (IoTセンサ) Wi-Fi カメラ

定期的にカメラ撮影を行い、撮影後に通知を送信します。通知を受けたサーバはHTTPで写真を取得することが出来ます。HTTP版に加え、FTPで送信するFTP版も収録しました。

	ESP-WROOM-02用　：example15_camG
	ESP32-WROOM-32用：example47_camG
	
	camG:SeeedStudio Grove Serial Camera Kit用
	camL:SparkFun SEN-11610 LynkSprite JPEG Color Camera TTL用
	f:FTP版

### 3-1 Wi-Fi コンシェルジェ 照明担当

各種Wi-Fi コンシェルジェ機器にはWebサーバ機能が装備されています。インターネットブラウザから手動制御を行ったり、Raspberry Piなどのサーバーからの自動制御を行ったりすることが出来ます。  
Wi-Fi コンシェルジェ 照明担当が制御するLEDは、状態や情報を、さりげなくユーザへ通知するときに使われます。天気予報情報に応じて、LEDの点滅方法を変化させたり、トイレに設置した人感センサが検知後の経過時間を通知するといった応用が出来るでしょう。  
なお、AC 100Vの照明機器を制御するには、後述の Wi-Fi コンシェルジェ 電源設備担当を使用します。

	ESP-WROOM-02用　：example16_led
	ESP32-WROOM-32用：example48_led

### 3-2 Wi-Fi コンシェルジェ チャイム担当

ユーザへ気づきを通知するときには、室内のどこにいても認識できるようにチャイム音などが用いられます。来客時やドア開閉、窓開閉といった変化を通知する応用が出来るでしょう。

	ESP-WROOM-02用　：example17_bell 
	ESP32-WROOM-32用：example49_bell

### 3-3 Wi-Fi コンシェルジェ 掲示板担当

液晶ディスプレイ（LCD）を用いることで、より多くの情報をユーザへ通知することができるようになります。

	ESP-WROOM-02用　：example18_lcd
	ESP32-WROOM-32用：example50_lcd

### 3-4 Wi-Fi コンシェルジェ リモコン担当

テレビやエアコン、オーディオ、照明機器など、赤外線リモコンで制御可能な家電機器を制御します。  
赤外線リモコン信号コードはインターネットブラウザから HTTP GET リクエストで取得することが出来ます。ブラウザ上に表示された赤外線リモコン信号を HTTP POST リクエストで送信することで、赤外線リモコン信号を発信することが出来ます。

	ESP-WROOM-02用　：example19_ir_rc
	ESP32-WROOM-32用：example51_ir_rc

### 3-5 Wi-Fi コンシェルジェ カメラ担当

カメラのシャッターを制御し、撮影した写真をブラウザで確認したり、Wi-Fi 人感センサで検出した人物を撮影するといった防犯システムへ応用が可能なIoT対応カメラです。撮影した写真データをESPモジュール内のメモリに保持し、撮影写真の一覧表示を行うこともできます。

	ESP-WROOM-02用　：example20_camG 
	ESP32-WROOM-32用：example52_camG
	
	camG:SeeedStudio Grove Serial Camera Kit用
	camL:SparkFun SEN-11610 LynkSprite JPEG Color Camera TTL用

### 3-6 Wi-Fi コンシェルジェ アナウンス担当

音声でユーザへ気づきを通知することが可能なIoT機器です。気づきだけであればチャイム音で通知できますが、「玄関に来客」「寝室のドアが閉まりました」「起床時間です」といった、より具体的な内容を含めて通知することが出来るようになります。外出先から VPN 接続すれば、「もうすぐ帰るよ」と音声メッセージを自宅にいる家族へ伝えることもできるでしょう。

	ESP-WROOM-02用　：example21_talk 
	ESP32-WROOM-32用：example53_talk

### 3-7 Wi-Fi コンシェルジェ マイコン担当

IoTセンサ機器が送信するセンサ値情報を、テレビへ表示することが可能なIoT機器です。テレビへの出力（NTSC）には BASIC マイコンIchigoJam を使用します。BASIC言語 で作成した簡単なプログラムを転送してマイコン上で実行することも可能です。

	ESP-WROOM-02用　：example22_jam
	ESP32-WROOM-32用：example54_jam

### 3-8 Wi-Fi コンシェルジェ コンピュータ担当

Raspberry Piの電源が入っていないときに電源投入の制御を行うことが可能なIoT機器です。外出先から VPN 接続してRaspberry Piの電源を投入することも出来ます。  
また、インターネットブラウザを使って簡単なコマンドをRaspberry Piへ送ったり、応答結果を確認することも出来ます。

	ESP-WROOM-02用　：example23_raspi
	ESP32-WROOM-32用：example55_raspi

### 3-9 Wi-Fi コンシェルジェ 電源設備担当

AC100V 制御が可能なリレー・モジュールを用いることで、ACコンセント用タイマに対応したオーディオ機器や電気スタンドなどの家電機器を制御することが出来ます。たとえ無人で動作させても事故が発生しないように、十分な配慮と安全対策を行ってください。

	ESP-WROOM-02用　：example24_ac
	ESP32-WROOM-32用：example56_ac

### 3-10 Wi-Fi コンシェルジェ 情報担当

各種 Wi－Fi 対応IoT センサ機器から送られてきたセンサ値データを、受信した時刻情報とともに ESPモジュール内に保持します。2Mバイトのフラッシュメモリが実装されているESPモジュールの場合、最大1Mバイトのデータを保持することができます。

	ESP-WROOM-02用　：example25_fs
	ESP32-WROOM-32用：example57_fs

上記に記載していないサンプルについては、各フォルダ内のREADME.mdファイルなどを参照してください。

## 3_misc

本書の関連ファイルです。

## 4_server

各IoT機器をRaspberry Piで制御するためのサンプル・スクリプトです。

## node-red

Node-RED 用のサンプル・フローです。
各IoT機器用のゲートウェイを簡単に製作することができます。
<https://blogs.yahoo.co.jp/bokunimowakaru/56073151.html>

## tools

各IoTセンサのデータを収集することができるudp_logger.shなどの関連ツールです。

# 最新版のダウンロード方法

ラズベリー・パイやCygwinから下記のコマンドを入力することで、最新版のサンプル・プログラムをダウンロードすることが出来ます。

	$ git clone http://github.com/bokunimowakaru/esp.git
	$ git clone http://github.com/bokunimowakaru/RaspberryPi.git

上記のgitコマンドでダウンロードした場合、今後は下記のコマンドで差分だけをダウンロードすることが出来ます。

	$ cd ～/esp
	$ git pull
	$ cd ～/RaspberryPi
	$ git pull

# サポートページ  
<https://bokunimo.net/bokunimowakaru/cq/esp/>

Copyright (c) 2016-2018 Wataru KUNINO  
<https://bokunimo.net/>
