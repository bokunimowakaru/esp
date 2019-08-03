/*******************************************************************************
IoT Temp 内蔵温度センサ BLE
乾電池などで動作するIoT温度センサです
ESP32内蔵の温度センサを使用するので、精度は良くありません。

                                          Copyright (c) 2016-2019 Wataru KUNINO
*******************************************************************************/
/* 
   Based on Neil Kolban example for IDF: https://github.com/nkolban/esp32-snippets/blob/master/cpp_utils/tests/BLE%20Tests/SampleScan.cpp
   Ported to Arduino ESP32 by pcbreflux
*/

//#include <WiFi.h>                         // ESP32用WiFiライブラリ
#include <BLEDevice.h>
#include <BLEServer.h>
#include "esp_sleep.h"                      // ESP32用Deep Sleep ライブラリ
#define PIN_EN 2                            // GPIO 2 にLEDを接続
#define PIN_BUZZER 12                       // GPIO 12 にスピーカを接続
#define SLEEP_P 5*1000000ul                 // スリープ時間 5秒(uint32_t)
#define BLE_DEVICE "espRohmTemp"            // BLE用デバイス名

BLEAdvertising *pAdvertising;
boolean wifi_enable = true;

RTC_DATA_ATTR byte SEQ_N = 0;               // 送信番号
RTC_DATA_ATTR int8_t TEMP_ADJ=0;            // 温度補正値
int wake;

void setup(){                               // 起動時に一度だけ実行する関数
//  WiFi.mode(WIFI_OFF);                    // 無線LANをOFFに設定
    pinMode(PIN_EN,OUTPUT);                 // LEDを出力に
    digitalWrite(PIN_EN,1);                 // LEDをON
    pinMode(PIN_BUZZER,OUTPUT);             // ブザーを接続したポートを出力に
    chimeBellsSetup(PIN_BUZZER);            // ブザー/LED用するPWM制御部の初期化
    Serial.begin(115200);                   // 動作確認のためのシリアル出力開始
    Serial.println("IoT Temperature");      // 「IoT Temperature」をシリアル出力
    wake = TimerWakeUp_init();
    BLEDevice::init(BLE_DEVICE);            // Create the BLE Device
    ledcWriteNote(0,NOTE_D,8);              // 送信中の音
}

void loop() {
    float temp;                             // センサ用の浮動小数点数型変数

    temp = temperatureRead() + (float)TEMP_ADJ - 35.;
    Serial.print("temperature= ");
    Serial.println(temp);
    Serial.print("Temperature    =  ");
    Serial.print(temp,2);
    Serial.println(" [degrees Celsius]");

    // BLE Advertizing
    pAdvertising = BLEDevice::getAdvertising();
    setBleAdvData(temp);
    pAdvertising->start();                  // Start advertising
    SEQ_N++;
    sleep();
}

void sleep(){
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

void setBleAdvData(float temp){
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
    strServiceData += (char)'\0';
    strServiceData += (char)'\0';
    strServiceData += (char)'\0';
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
