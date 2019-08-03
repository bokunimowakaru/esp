/*******************************************************************************
IoT Press 気圧センサ UDP+BLE
ROHM BM1383AGLV
乾電池などで動作するIoT気圧センサです
※フラッシュを約1.4MB消費しますので、アプリ用に2MB以上を割り当ててください

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
#define PIN_EN 2                            // GPIO 2 にLEDを接続
#define PIN_BUZZER 12                       // GPIO 12 にスピーカを接続
#define SSID "1234ABCD"                     // 無線LANアクセスポイントのSSID
#define PASS "password"                     // パスワード
#define PORT 1024                           // 送信のポート番号
#define SLEEP_P 50*1000000ul                // スリープ時間 50秒(uint32_t)
#define DEVICE "press_1,"                   // デバイス名(5文字+"_"+番号+",")
#define BLE_DEVICE "espRohmPress"           // BLE用デバイス名

#include <Wire.h>
#include "BM1383AGLV.h"

BM1383AGLV bm1383aglv;
IPAddress IP;                               // 本機IPアドレス
IPAddress IP_BC;                            // ブロードキャストIPアドレス

BLEAdvertising *pAdvertising;
boolean wifi_enable = true;

RTC_DATA_ATTR byte SEQ_N = 0;				// 送信番号
int wake;

void setup(){                               // 起動時に一度だけ実行する関数
    int waiting=0;                          // アクセスポイント接続待ち用
    pinMode(PIN_EN,OUTPUT);                 // LEDを出力に
    digitalWrite(PIN_EN,1);                 // LEDをON
    pinMode(PIN_BUZZER,OUTPUT);             // ブザーを接続したポートを出力に
    delay(10);                              // 起動待ち時間
    Serial.begin(115200);                   // 動作確認のためのシリアル出力開始
    Serial.println("IoT Press BM1383AGLV"); // 「IoT Press」をシリアル出力表示
    wake = TimerWakeUp_init();
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
            return;                         // 30回(15秒)を過ぎたら抜ける
        }
    }
    IP = WiFi.localIP();
    IP_BC = (uint32_t)IP | ~(uint32_t)(WiFi.subnetMask());
    Serial.println(IP);                     // 本機のIPアドレスをシリアル出力
    if(wake<3) morseIp0(PIN_BUZZER,50,IP);  // IPアドレス終値をモールス信号出力
}

void loop() {
    WiFiUDP udp;                            // UDP通信用のインスタンスを定義
    float temp,press;                       // センサ用の浮動小数点数型変数

    int rc=bm1383aglv.get_val(&press,&temp);// 気圧と温度を取得して各変数へ代入
    if(rc) sleep();                         // 取得失敗時はsleepへ
    Serial.print("Temperature    =  ");
    Serial.print(temp,2);
    Serial.println(" [degrees Celsius]");
    Serial.print("Pressure        = ");
    Serial.print(press,2);
    Serial.println(" [hPa]");
    // UDP
    if(wifi_enable){
        udp.beginPacket(IP_BC, PORT);       // UDP送信先を設定
        udp.print(DEVICE);                  // デバイス名を送信
        udp.print(round(temp),0);           // 変数tempの値を送信
        udp.print(", ");                    // 「,」カンマを送信
        udp.println(round(press),0);        // 変数pressの値を送信
        udp.endPacket();                    // UDP送信の終了(実際に送信する)
    }
    // BLE Advertizing
    pAdvertising = BLEDevice::getAdvertising();
    setBleAdvData(temp,press);
    pAdvertising->start();                  // Start advertising
    SEQ_N++;
    sleep();
}

void sleep(){
    ledcWriteNote(0,NOTE_D,8);              // 送信中の音
    delay(150);                             // 送信待ち時間
    if(wake<2) for(int i=0; i<20;i++){
        ledcWrite(0, 0);
        delay(490);
        ledcWriteNote(0,NOTE_D,8);
        delay(10);
    }
    ledcWrite(0, 0);
    digitalWrite(PIN_EN,0);                 // LEDをOFF
    pAdvertising->stop();
    esp_deep_sleep(SLEEP_P);                // Deep Sleepモードへ移行
}

void setBleAdvData(float temp, float press){
    long val;
    
    BLEAdvertisementData oAdvertisementData = BLEAdvertisementData();
    BLEAdvertisementData oScanResponseData = BLEAdvertisementData();
    
    std::string strServiceData = "";
    strServiceData += (char)9;              // Len
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
    strServiceData += (char)(SEQ_N);        // 送信番号

    oAdvertisementData.addData(strServiceData);
    oAdvertisementData.setFlags(0x06);      // LE General Discoverable Mode | BR_EDR_NOT_SUPPORTED
    oAdvertisementData.setName(BLE_DEVICE); // oAdvertisementDataは逆順に代入する
    pAdvertising->setAdvertisementData(oAdvertisementData);
    pAdvertising->setScanResponseData(oScanResponseData);

    Serial.print("data            = ");
    int len=strServiceData.size();
    if(len != (int)(strServiceData[0]) + 1 || len < 2) Serial.println("ERROR: BLE length");
    for(int i=2;i<len;i++) Serial.printf("%02x ",(char)(strServiceData[i]));
    Serial.println();
    Serial.print("data length     = 2 + 6 = ");
    Serial.printf("%d (%d)\n",len-2,22-strlen(BLE_DEVICE)-6);
}
