# トランジスタ技術 2017年3月号・2016年9月号ESP8266特集 ESP8266モジュール用スケッチ・最新ESP32用スケッチ

## 1_practice

マイコン練習用サンプル・プログラムが含まれているフォルダです。

## 2_example

IoT機器用のサンプル・プログラムが含まれているフォルダです。

### 1-1 Wi-Fi インジケータ

LEDをWi-Fi経由で制御することが出来ます。

	ESP-WROOM-02用　：example01_led
	ESP32-WROOM-32用：example33_led
	
	a:TELNET版
	b:HTTP版
	d:ソフトウェア無線AP版
	s:IPアドレス固定版

### 1-2 Wi-Fi スイッチャ

スイッチ状態を送信します。

	ESP-WROOM-02用　：example02_sw 
	ESP32-WROOM-32用：example34_sw

### 1-3 Wi-Fi レコーダ

アナログ入力値を送信します。

	ESP-WROOM-02用　：example03_adc
	ESP32-WROOM-32用：example35_adc

### ケチケチ運転術

乾電池などで動作するIoTセンサ用の基本形です。スケッチを把握しやすいように機能を絞り込みました。

	ESP-WROOM-02用　：example04_le 
	ESP32-WROOM-32用：example36_le
	
	a:Wi-Fi接続タイムアウト機能つき
	c:クラウドサービスAmbientへの送信機能つき
	m:取得値が近いときは送信しない節約機能つき

### 1-4 Wi-Fi LCD

Wi-Fi スイッチャや各種IoTセンサが送信したデータを液晶ディスプレイ（LCD）へ表示する基本形です。スケッチを把握しやすいように機能を絞り込みました。

	ESP-WROOM-02用　：example05_lcd
	ESP32-WROOM-32用：example37_lcd
	
	-:UDP版
	a:TCP版（aの無いものが通常のUDP版）

### 2-1 IoTセンサ Wi-Fi 照度計

照度値を送信するIoTセンサです。センサを手で遮るなど、簡単にセンサ値を変化させることが出来るので、センサの初期実験に便利です。

	ESP-WROOM-02用　：example06_lum
	ESP32-WROOM-32用：example38_lum

### 2-2 IoTセンサ Wi-Fi 温度計

温度値を送信するIoTセンサです。センサネットワークで収集するときの代表的なセンサのひとつです。

	ESP-WROOM-02用　：example07_temp 
	ESP32-WROOM-32用：example39_temp

### 2-3 IoTセンサ Wi-Fi ドア開閉モニタ

ドアや窓が開いたとき（または閉まったとき）に開閉状態を送信するIoTセンサです。防犯や家庭内の空気の出入りなどを把握します。

	ESP-WROOM-02用　：example08_sw 
	ESP32-WROOM-32用：example40_sw

### 2-4 IoTセンサ Wi-Fi 温湿度計

温湿度を送信するIoTセンサです。デジタルのI2Cインタフェースをもつセンサを使用することで、アナログ回路やセンサ値の補正が不要となり、正確な値を手軽に得ることが出来ます。

	ESP-WROOM-02用　：example09_hum_sht31
	ESP32-WROOM-32用：example41_hum_sht31
	
	表示なし:TI製 HDC1000 版
	sht31　 :SENSIRION製 SHT31 版
	si7021　:SILICON LABS製 Si7021 版

### 2-5 IoTセンサ Wi-Fi 気圧計

気圧と温度を送信するIoTセンサです。天候状態を予想したり、換気扇による室内の圧力の低下を検出することが出来ます。

	ESP-WROOM-02用　：example10_hpa
	ESP32-WROOM-32用：example42_hpa
	
	表示なし：STマイクロ製 LPS25H 版
	bme280　：Bosch製 BME280 版

### 2-6 IoTセンサ Wi-Fi 人感センサ

人体などの動きを検出したときに送信するIoTセンサです。在室情報を把握することが出来ます。

	ESP-WROOM-02用　：example11_pir
	ESP32-WROOM-32用：example43_pir

### 2-7 IoTセンサ Wi-Fi 3軸加速度センサ

加速度の変化を検出したときに送信するIoTセンサです。ドアや窓の開閉や傾きの変化などを検出することが出来ます。

	ESP-WROOM-02用　：example12_acm
	ESP32-WROOM-32用：example44_acm

### 2-8 NTP時刻データ転送機

インターネットから現在時刻を取得して送信する機器です。時刻センサのような動作を行います。センサネットワークでは時刻を扱うことが多いので、参考になるでしょう。

	ESP-WROOM-02用　：example13_ntp
	ESP32-WROOM-32用：example45_ntp
	
	c:クラウドサービスAmbientへの送信機能つき

### 2-9 IoTセンサ Wi-Fi リモコン赤外線レシーバ

赤外線リモコン信号を受信し、受信データをWi-Fi送信します。家電などの操作情報を検出することが出来ます。

	ESP-WROOM-02用　：example14_ir_in
	ESP32-WROOM-32用：example46_ir_in

### 2-10 IoTセンサ Wi-Fi カメラ

定期的にカメラ撮影を行い、撮影後に通知を送信します。通知を受けたサーバはHTTPで写真を取得することが出来ます。写真をFTPで送信するFTP版も収録しました。

	ESP-WROOM-02用　：example15_camG
	ESP32-WROOM-32用：example47_camG
	
	camG:SeeedStudio Grove Serial Camera Kit用
	camL:SparkFun SEN-11610 LynkSprite JPEG Color Camera TTL用
	f:FTP版

### 3-1 Wi-Fi コンシェルジェ 照明担当

	ESP-WROOM-02用　：example16_led
	ESP32-WROOM-32用：example48_led

### 3-2 Wi-Fi コンシェルジェ チャイム担当

	ESP-WROOM-02用　：example17_bell 
	ESP32-WROOM-32用：example49_bell

### 3-3 Wi-Fi コンシェルジェ 掲示板担当

	ESP-WROOM-02用　：example18_lcd
	ESP32-WROOM-32用：example50_lcd

### 3-4 Wi-Fi コンシェルジェ リモコン担当

	ESP-WROOM-02用　：example19_ir_rc
	ESP32-WROOM-32用：example51_ir_rc

### 3-5 Wi-Fi コンシェルジェ カメラ担当

	ESP-WROOM-02用　：example20_camG 
	ESP32-WROOM-32用：example52_camG

### 3-6 Wi-Fi コンシェルジェ アナウンス担当

	ESP-WROOM-02用　：example21_talk 
	ESP32-WROOM-32用：example53_talk

### 3-7 Wi-Fi コンシェルジェ マイコン担当

	ESP-WROOM-02用　：example22_jam
	ESP32-WROOM-32用：example54_jam

### 3-8 Wi-Fi コンシェルジェ コンピュータ担当

	ESP-WROOM-02用　：example23_raspi
	ESP32-WROOM-32用：example55_raspi

### 3-9 Wi-Fi コンシェルジェ 電源設備担当

	ESP-WROOM-02用　：example24_ac
	ESP32-WROOM-32用：example56_ac

### 3-10 Wi-Fi コンシェルジェ 情報担当

	ESP-WROOM-02用　：example25_fs
	ESP32-WROOM-32用：example57_fs

その他の機器については各フォルダ内のREADME.mdファイルを参照してください。

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
