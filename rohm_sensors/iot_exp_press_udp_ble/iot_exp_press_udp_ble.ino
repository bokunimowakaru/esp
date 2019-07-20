/*******************************************************************************
IoT Press 気圧センサ UDP+BLE
ROHM BM1383AGLV
乾電池などで動作するIoT気圧センサです
※フラッシュを約1.4MB消費しますので、アプリ用に2MB程度を割り当ててください

                                          Copyright (c) 2016-2019 Wataru KUNINO
*******************************************************************************/
/* 
   Based on Neil Kolban example for IDF: https://github.com/nkolban/esp32-snippets/blob/master/cpp_utils/tests/BLE%20Tests/SampleScan.cpp
   Ported to Arduino ESP32 by pcbreflux
*/
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
#define DEVICE "press_1,"                   // デバイス名(5文字+"_"+番号+",")
#define BLE_DEVICE "esp_press_1"            // BLE用デバイス名

#include <Wire.h>
#include "BM1383AGLV.h"

BM1383AGLV bm1383aglv;

BLEAdvertising *pAdvertising;
boolean wifi_enable = true;

void setup(){                               // 起動時に一度だけ実行する関数
    int waiting=0;                          // アクセスポイント接続待ち用
    pinMode(PIN_EN,OUTPUT);                 // センサ用の電源を出力に
    pinMode(PIN_BUZZER,OUTPUT);             // ブザーを接続したポートを出力に
    delay(10);                              // 起動待ち時間
    Serial.begin(115200);                   // 動作確認のためのシリアル出力開始
    Serial.println("IoT Press BM1383AGLV"); // 「IoT Press」をシリアル出力表示
    BLEDevice::init(BLE_DEVICE);            // Create the BLE Device
    Wire.begin();
    bm1383aglv.init();                      // 気圧センサの初期化
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

void setBleAdvData(float temp, float press){
    long val;
    
    BLEAdvertisementData oAdvertisementData = BLEAdvertisementData();
    BLEAdvertisementData oScanResponseData = BLEAdvertisementData();
    oAdvertisementData.setName(BLE_DEVICE);
    oAdvertisementData.setFlags(0x06);      // LE General Discoverable Mode | BR_EDR_NOT_SUPPORTED
    
    std::string strServiceData = "";
    strServiceData += (char)8;              // Len
    strServiceData += (char)0xFF;           // Manufacturer specific data
    strServiceData += (char)0x01;           // Company Identifier(2 Octet)
    strServiceData += (char)0x00;
    val = (long)((temp + 45.) * 374.5);
    strServiceData += (char)(val & 0xFF);   // 温度 下位バイト
    strServiceData += (char)(val >> 8);     // 温度 上位バイト
    val = (long)(press * 2048);
    strServiceData += (char)(val & 0xFF);   // 気圧 下位1バイト目
    strServiceData += (char)(val >> 8);     // 気圧 下位2バイト目
    strServiceData += (char)(val >> 16);    // 気圧 最上位バイト

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

void loop() {
    WiFiUDP udp;                            // UDP通信用のインスタンスを定義
    float temp,press;                       // センサ用の浮動小数点数型変数

    if(!bm1383aglv.get_val(&press,&temp)){  // 気圧と温度を取得して変数へ代入
        // UDP
        if(wifi_enable){
            udp.beginPacket(SENDTO, PORT);  // UDP送信先を設定
            udp.print(DEVICE);              // デバイス名を送信
            udp.print(temp,0);              // 変数tempの値を送信
            Serial.print(temp,2);           // シリアル出力表示
            udp.print(", ");                // 「,」カンマを送信
            Serial.print(", ");             // シリアル出力表示
            udp.println(press,0);           // 変数pressの値を送信
            Serial.println(press,2);        // シリアル出力表示
            udp.endPacket();                // UDP送信の終了(実際に送信する)
        }
        // BLE Advertizing
        pAdvertising = BLEDevice::getAdvertising();
        setBleAdvData(temp,press);
        pAdvertising->start();              // Start advertising
    }
    sleep();
}

void sleep(){
    delay(200);                             // 送信待ち時間
    pAdvertising->stop();
    esp_deep_sleep(SLEEP_P);                // Deep Sleepモードへ移行
}
