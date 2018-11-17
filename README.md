# トランジスタ技術 2017年3月号・2016年9月号ESP8266特集 ESP8266モジュール用スケッチ・最新ESP32用スケッチ

## 1_practice

マイコン練習用サンプル・プログラムが含まれているフォルダです。

## 2_example

IoT機器用のサンプル・プログラムが含まれているフォルダです。

### 1-1 Wi-Fiインジケータ

	example01_led
	example33_led

### 1-2 Wi-Fiスイッチャ

	ESP-WROOM-02用　：example02_sw 
	ESP32-WROOM-32用：example34_sw

### 1-3 Wi-Fiレコーダ

	ESP-WROOM-02用　：example03_adc
	ESP32-WROOM-32用：example35_adc

### ケチケチ運転術

	ESP-WROOM-02用　：example04_le 
	ESP32-WROOM-32用：example36_le

### 1-4 Wi-Fi LCD

	ESP-WROOM-02用　：example05_lcd
	ESP32-WROOM-32用：example37_lcd

### 2-1 IoTセンサ Wi-Fi 照度計

	ESP-WROOM-02用　：example06_lum
	ESP32-WROOM-32用：example38_lum

### 2-2 IoTセンサ Wi-Fi 温度計

	ESP-WROOM-02用　：example07_temp 
	ESP32-WROOM-32用：example39_temp

### 2-3 IoTセンサ Wi-Fi ドア開閉モニタ

	ESP-WROOM-02用　：example08_sw 
	ESP32-WROOM-32用：example40_sw

### 2-4 IoTセンサ Wi-Fi 温湿度計

	ESP-WROOM-02用　：example09_hum_sht31
	ESP32-WROOM-32用：example41_hum_sht31

### 2-5 IoTセンサ Wi-Fi 気圧計

	ESP-WROOM-02用　：example10_hpa
	ESP32-WROOM-32用：example42_hpa

### 2-6 IoTセンサ Wi-Fi 人感センサ

	ESP-WROOM-02用　：example11_pir
	ESP32-WROOM-32用：example43_pir

### 2-7 IoTセンサ Wi-Fi 3軸加速度センサ

	ESP-WROOM-02用　：example12_acm
	ESP32-WROOM-32用：example44_acm

### 2-8 IoTセンサ NTP時刻データ転送機

	ESP-WROOM-02用　：example13_ntp
	ESP32-WROOM-32用：example45_ntp

### 2-9 IoTセンサ Wi-Fi リモコン赤外線レシーバ

	ESP-WROOM-02用　：example14_ir_in
	ESP32-WROOM-32用：example46_ir_in

### 2-10 IoTセンサ Wi-Fi カメラ

	ESP-WROOM-02用　：example15_camG
	ESP32-WROOM-32用：example47_camG

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
