/*******************************************************************************
IoT SensorShield EVK UDP+BLE

    気圧センサ              ROHM BM1383AGLV
    加速度センサ            ROHM KX224
    地磁気センサ            ROHM BM1422AGMV
    照度・近接一体型センサ  ROHM RPR-0521RS
    カラーセンサ            ROHM BH1749NUC

乾電池などで動作するIoTセンサです

※フラッシュを約1.4MB消費しますので、アプリ用に2MB程度を割り当ててください

                                          Copyright (c) 2016-2019 Wataru KUNINO
*******************************************************************************/
#include <WiFi.h>                           // ESP32用WiFiライブラリ
#include <WiFiUdp.h>                        // UDP通信を行うライブラリ
#include <BLEDevice.h>
#include <BLEServer.h>
#include "esp_sleep.h"                      // ESP32用Deep Sleep ライブラリ
#include "pitches.h"
#define PIN_EN 2                            // GPIO 2 にLEDを接続
#define PIN_BUZZER 12                       // GPIO 12 にスピーカを接続
#define SSID "1234ABCD"                     // 無線LANアクセスポイントのSSID
#define PASS "password"                     // パスワード
#define SENDTO "255.255.255.255"            // 送信先のIPアドレス
#define PORT 1024                           // 送信のポート番号
#define SLEEP_P 50*1000000ul                // スリープ時間 50秒(uint32_t)
#define DEVICE "rohme_1,"                   // デバイス名(5文字+"_"+番号+",")
#define BLE_DEVICE "esp"                    // BLE用デバイス名

#include <Wire.h>
#include "BM1383AGLV.h"
#include "KX224.h"
#include "BM1422AGMV.h"
#include "RPR-0521RS.h"
#include "BH1749NUC.h"

BM1383AGLV  bm1383aglv;
KX224       kx224(0x1E);
BM1422AGMV  bm1422agmv(0x0E);
RPR0521RS   rpr0521rs;
BH1749NUC   bh1749nuc(0x39);

BLEAdvertising *pAdvertising;
boolean wifi_enable = true;

#define VAL_N 8                             // 送信データ項目数
#define DAT_N 32                            // 送信バイト数

RTC_DATA_ATTR byte SEQ_N = 0;				// 送信番号

void setup(){                               // 起動時に一度だけ実行する関数
    int waiting=0;                          // アクセスポイント接続待ち用
    pinMode(PIN_EN,OUTPUT);                 // センサ用の電源を出力に
    delay(10);                              // 起動待ち時間
    Serial.begin(115200);                   // 動作確認のためのシリアル出力開始
    Serial.println("IoT SensorShield EVK"); // 「IoT SensorShield EVK」を表示
    BLEDevice::init(BLE_DEVICE);            // Create the BLE Device
    Wire.begin();
    bm1383aglv.init();                      // 気圧センサの初期化
    kx224.init();                           // 加速度センサの初期化
    bm1422agmv.init();                      // 地磁気センサの初期化
    rpr0521rs.init();                       // 照度・近接一体型センサの初期化
    bh1749nuc.init();                       // カラーセンサの初期化
    WiFi.mode(WIFI_STA);                    // 無線LANをSTAモードに設定
    WiFi.begin(SSID,PASS);                  // 無線LANアクセスポイントへ接続
    chimeBellsSetup(PIN_BUZZER);            // ブザー/LED用するPWM制御部の初期化
    while(WiFi.status() != WL_CONNECTED){   // 接続に成功するまで待つ
        Serial.print('.');                  // 進捗表示
        ledcWriteNote(0,NOTE_B,7);          // 接続中の音
        delay(50);
        ledcWrite(0, 0);
        delay(450);                         // 待ち時間処理
        waiting++;                          // 待ち時間カウンタを1加算する
        if(waiting > 30){
            wifi_enable = false;
            return;                          // 30回(15秒)を過ぎたら抜ける
        }
    }
    Serial.println(WiFi.localIP());         // 本機のIPアドレスをシリアル出力
    morseIp0(PIN_BUZZER,50,WiFi.localIP()); // IPアドレス終値をモールス信号出力
}

int d_append(byte *array,int i, byte d){
    array[i]=d;
    return i+1;
}

int d_append_int16(byte *array,int i, int16_t d){
    array[i] = (byte)(d & 0xFF); 
    array[i+1] = (byte)(d >> 8);
    return i+2;
}

int d_append_int24(byte *array,int i, int32_t d){
    array[i] = (byte)(d & 0xFF); 
    array[i+1] = (byte)((d >>8)&0xFF);
    array[i+2] = (byte)((d >>16)&0xFF);
    return i+3;
}

void sensors_log(float *val){
    Serial.print("Temperature    =  ");
    Serial.print(val[0]);
    Serial.println(" [degrees Celsius]");
    Serial.print("Pressure        = ");
    Serial.print(val[1]);
    Serial.println(" [hPa]");
    Serial.write("Accelerometer X = ");
    Serial.print(val[2]);
    Serial.println(" [g]");
    Serial.write("Accelerometer Y = ");
    Serial.print(val[3]);
    Serial.println(" [g]");
    Serial.write("Accelerometer Z = ");
    Serial.print(val[4]);
    Serial.println(" [g]");
    Serial.print("Geomagnetic X   = ");
    Serial.print(val[5], 3);
    Serial.println("[uT]");
    Serial.print("Geomagnetic Y   = ");
    Serial.print(val[6], 3);
    Serial.println("[uT]");
    Serial.print("Geomagnetic Z   = ");
    Serial.print(val[7], 3);
    Serial.println("[uT]");
}

void loop() {
    WiFiUDP udp;                            // UDP通信用のインスタンスを定義
    byte d[DAT_N]; 
    float val[VAL_N];                       // センサ用の浮動小数点数型変数
    long v;
    int l=0;

	bm1383aglv.get_val(&val[1],val);		// 気圧と温度を取得
	v=(long)((val[0] + 45.) * 374.5);       // 温度
	l=d_append_int16(d,l,v);
	v=(long)(val[1] * 2048);
	l=d_append_int24(d,l,v);				// 気圧
	
    l=d_append(d,l,SEQ_N);					// 送信番号
    
	kx224.get_val(&val[2]);					// 加速度を取得
	for(int i=0;i<3;i++){
	    v=(long)(val[2+i] * 4096);
		l=d_append_int16(d,l,v);
	}
    
	bm1422agmv.get_val(&val[5]);			// 地磁気を取得
	for(int i=0;i<3;i++){
	    v=(long)(val[5+i] * 10);
		l=d_append_int16(d,l,v);
	}
    
    sensors_log(val);
    
    if(wifi_enable){
	    udp.beginPacket(SENDTO, PORT);      // UDP送信先を設定
	    udp.print(DEVICE);                  // デバイス名を送信
		for(int i=0; i<VAL_N; i++){
	        udp.print(val[i],0);            // 変数tempの値を送信
	        if(i < VAL_N-1)udp.print(", "); // 「,」カンマを送信
	    }
	    udp.endPacket();                    // UDP送信の終了(実際に送信する)
	}
    // BLE Advertizing
    pAdvertising = BLEDevice::getAdvertising();
    setBleAdvData(d,l);
    pAdvertising->start();              	// Start advertising
    SEQ_N++;
    sleep();
}

void sleep(){
    delay(200);                             // 送信待ち時間
    esp_deep_sleep(SLEEP_P);                // Deep Sleepモードへ移行
}

void setBleAdvData(byte *data, int data_n){
    BLEAdvertisementData oAdvertisementData = BLEAdvertisementData();
    BLEAdvertisementData oScanResponseData = BLEAdvertisementData();
    oAdvertisementData.setName(BLE_DEVICE);
    oAdvertisementData.setFlags(0x06);      // LE General Discoverable Mode | BR_EDR_NOT_SUPPORTED
    
    std::string strServiceData = "";
    strServiceData += (char)(data_n+3);     // Len
    strServiceData += (char)0xFF;           // Manufacturer specific data
    strServiceData += (char)0x01;           // Company Identifier(2 Octet)
    strServiceData += (char)0x00;
    for(int i=0;i<data_n;i++) strServiceData += (char)(data[i]);

    oAdvertisementData.addData(strServiceData);
    pAdvertising->setAdvertisementData(oAdvertisementData);
    pAdvertising->setScanResponseData(oScanResponseData);

    Serial.print("data            = ");
    int len=strServiceData.size();
    if(len != (int)(strServiceData[0]) + 1 || len < 2) Serial.println("ERROR: BLE length");
    for(int i=2;i<len;i++) Serial.printf("%02x ",(char)(strServiceData[i]));
	Serial.println();
    Serial.print("data length     = ");
    Serial.println(len - 2);
}
