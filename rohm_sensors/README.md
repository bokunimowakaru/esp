# ボクにもわかる IoT Express + Bluetooth LE

CQ出版社の IoT Express を使用した IoTセンサのサンプル・スケッチ集です。  
Rohm製 SensorShield EVK を IoT Express へ接続し、スケッチを書き込むとIoTセンサを製作することが出来ます。

    気圧センサ              ROHM BM1383AGLV
    加速度センサ            ROHM KX224
    地磁気センサ            ROHM BM1422AGMV
    照度・近接一体型センサ  ROHM RPR-0521RS
    カラーセンサ            ROHM BH1749NUC

## Bluetooth LE 対応 IoTセンサ

### iot_exp_press_ble

気圧センサBM1383AGLVを使用した Bluetooth LE 対応 IoT センサです。IoT Expressへ書き込んで、動作確認用に使用することが出来ます。受信には後述の ble_logger_sens_scan.py を使用します。

### iot_exp_sensorShield_ble

気圧センサ、加速度センサ、地磁気センサ、照度・近接センサ、カラーセンサを使用した Bluetooth LE 対応 IoT センサです。IoT Expressへ書き込みます。ble_logger_sens_scan.py で受信します。

### iot_exp_sensorShield_ble_rh

スマホ用アプリ ROHM RHRawDataMedal2 対応版です。IoT Expressへ書き込みます。スマホ用アプリとble_logger_sens_scan.py で受信します。

## UDP/IP 対応 IoT センサ

### iot_exp_press_udp

UDP/IP送信に対応した気圧センサです。IoT Expressへ書き込みます。受信には後述の udp_logger.py を使用します。

### iot_exp_sensorShield_udp

UDP/IP送信に対応した気圧センサ、加速度センサ、地磁気センサ、照度・近接センサ、カラーセンサです。 udp_logger.py で受信します。IoT Expressへ書き込み、udp_logger.py で受信します。

## Bluetooth LE ＋ UDP/IP 対応 IoTセンサ

### iot_exp_press_udp_ble

BLEとUDPの両対応版の IoT 気圧センサです。IoT Expressへ書き込み、ble_logger_sens_scan または udp_logger.py で受信します。

### iot_exp_sensorShield_udp_ble

BLEとUDPの両対応版の IoT 気圧センサ、加速度センサ、地磁気センサ、照度・近接センサ、カラーセンサです。IoT Expressへ書き込み、ble_logger_sens_scan または udp_logger.py で受信します。

## Raspberry Pi 対応 受信ソフト（Python）

### ble_logger_sens_scan.py

Rasperry Piで実行すると、各Bluetooth LE対応IoTセンサが送信するセンサ値情報を受信し、表示します。

	pi@raspberrypi:~ $ cd  
	pi@raspberrypi:~ $ git clone http://github.com/bokunimowakaru/esp  
	pi@raspberrypi:~ $ cd ~/esp/rohm_sensors  
	pi@raspberrypi:~/esp/rohm_sensors $ sudo ./ble_logger_sens_scan.py  
	  
	Device xx:xx:xx:xx:xx:xx (public), RSSI=-56 dB  
	  Flags = 06  
	  Complete Local Name = R  
	  Manufacturer = 01004c6cf10093009aff59ff0a0fc40080fee0fcdf521f  
	    ID            = 0x1  
	    SEQ           = 147  
	    Temperature   = 29.03 ℃  
	    Pressure      = 1002.359 hPa  
	    Illuminance   = 200.8 lx  
	    Accelerometer = 0.941 g ( -0.025 -0.041 0.94 g)  
	    Geomagnetic   = 90.9 uT ( 19.6 -38.4 -80.0 uT)  
	    RSSI          = -69 dB  
	  
	Device xx:xx:xx:xx:xx:xx (public), RSSI=-27 dB, Connectable=True  
	  255 Manufacturer = 0100b1e4c90000308147ff0041f1bbbada  
	    1 Flags = 06  
	    9 Complete Local Name = espRohm  
	    isRohmMedal   = Sensor Kit espRohm  
	    ID            = 0x1  
	    SEQ           = 218  
	    Temperature   = 29.25 ℃  
	    Pressure      = 999 hPa  
	    Illuminance   = 167.5 lx  
	    Proximity     = 0 count  
	    Color RGB     = 19 28 50 %  
	    Color IR      = 3 %  
	    Accelerometer = 1.016 g ( -0.016 0.0 1.016 g)  
	    Geomagnetic   = 99.4 uT ( -15 -69 -70 uT)  
	    RSSI          = -27 dB  


### udp_logger.py

Rasperry Piで実行すると、各UDP/IP対応IoTセンサが送信するセンサ値情報を受信し、表示します。

----------------------------------------------------------------------

# 最新版のダウンロード方法

ラズベリー・パイやCygwinから下記のコマンドを入力することで、最新版のサンプル・プログラムをダウンロードすることが出来ます。

	$ cd
	$ git clone http://github.com/bokunimowakaru/esp.git
	$ git clone http://github.com/bokunimowakaru/RaspberryPi.git

上記のgitコマンドでダウンロードした場合、今後は下記のコマンドで差分だけをダウンロードすることが出来ます。

	$ cd ~/esp
	$ git pull
	$ cd ~/RaspberryPi
	$ git pull

差分と同じ場所を変更していた場合、競合が発生します。競合を自分で解決できない場合は、フォルダ名「esp」を他の名前に書き換えて、git clone で新たにダウンロードする方法が簡単です。

# サポートページ  
<https://bokunimo.net/bokunimowakaru/cq/esp/>

# ライセンス

ライセンスについては各ソースリストならびに各フォルダ内のファイルに記載の通りです。  
使用・変更・配布は可能ですが、権利表示を残してください。  
また、提供情報や配布ソフトによって生じたいかなる被害についても，一切，補償いたしません。  

Copyright (c) 2016-2019 Wataru KUNINO  
<https://bokunimo.net/>
