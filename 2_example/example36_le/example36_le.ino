/*******************************************************************************
Example 36(=32+4): ESP32 ケチケチ運転術
乾電池などで動作するIoTセンサ用の基本形です。

                                          Copyright (c) 2016-2019 Wataru KUNINO
*******************************************************************************/
#include <WiFi.h>                           // ESP32用WiFiライブラリ
#include <WiFiUdp.h>                        // UDP通信を行うライブラリ
#include "esp_sleep.h"                      // ESP32用Deep Sleep ライブラリ
#define PIN_EN 2                            // GPIO 2(24番ピン)をセンサの電源に
#define PIN_AIN 34                          // GPIO 34 ADC1_CH6(6番ピン)をADCに
#define SSID "1234ABCD"                     // 無線LANアクセスポイントのSSID
#define PASS "password"                     // パスワード
#define SENDTO "192.168.0.255"              // 送信先のIPアドレス
#define PORT 1024                           // 送信のポート番号
#define SLEEP_P 50*1000000                  // スリープ時間 50秒(uint32_t)
#define DEVICE "adcnv_1,"                   // デバイス名(5文字+"_"+番号+",")

void setup(){                               // 起動時に一度だけ実行する関数
    pinMode(PIN_AIN,INPUT);                 // アナログ入力の設定
    pinMode(PIN_EN,OUTPUT);                 // センサ用の電源を出力に
    Serial.begin(115200);                   // 動作確認のためのシリアル出力開始
    Serial.println("ESP32 eg.04 LE");       // 「ESP32 eg.04」をシリアル出力表示
    WiFi.mode(WIFI_STA);                    // 無線LANをSTAモードに設定
    delay(10);                              // ESP32に必要な待ち時間
    WiFi.begin(SSID,PASS);                  // 無線LANアクセスポイントへ接続
    while(WiFi.status() != WL_CONNECTED){   // 接続に成功するまで待つ
        delay(500);                         // 待ち時間処理
        digitalWrite(PIN_EN,!digitalRead(PIN_EN));      // LEDの点滅
        Serial.print(".");
    }
    Serial.println(WiFi.localIP());         // 本機のIPアドレスをシリアル出力
}

void loop() {
    WiFiUDP udp;                            // UDP通信用のインスタンスを定義
    int adc;                                // 整数型変数adcを定義
    
    digitalWrite(PIN_EN,HIGH);              // センサ用の電源をONに
    delay(5);                               // 起動待ち時間
    adc=(int)mvAnalogIn(PIN_AIN, 0.1);      // AD変換器から値を取得
    digitalWrite(PIN_EN,LOW);               // センサ用の電源をOFFに
    udp.beginPacket(SENDTO, PORT);          // UDP送信先を設定
    udp.print(DEVICE);                      // デバイス名を送信
    udp.println(adc);                       // 変数adcの値を送信
    Serial.println(adc);                    // シリアル出力表示
    udp.endPacket();                        // UDP送信の終了(実際に送信する)
    delay(200);                             // 送信待ち時間
    esp_deep_sleep(SLEEP_P);                // Deep Sleepモードへ移行
}

float mvAnalogIn(uint8_t PIN){
    return mvAnalogIn(PIN, 0.0);            // 動作最小電圧 0.0 ～ 0.1(V)程度
//  return mvAnalogIn(PIN, 1.075584e-1);
}

float mvAnalogIn(uint8_t PIN, float offset){
    int in0,in3;
    float ad0,ad3;
    
    analogSetPinAttenuation(PIN,ADC_11db);
    in3=analogRead(PIN);
    
    if( in3 > 2599 ){
        ad3 = -1.457583e-7 * (float)in3 * (float)in3
            + 1.510116e-3 * (float)in3
            - 0.680858 + offset;
    }else{
        ad3 = 8.378998e-4 * (float)in3 + 8.158714e-2 + offset;
    }
    Serial.print("ADC (ATT=3;11dB) = ");
    Serial.print(ad3,3);
    Serial.print(" [V], ");
    Serial.println(in3);
    if( in3 < 200 ){
        analogSetPinAttenuation(PIN,ADC_0db);
        in0=analogRead(PIN);
        ad0 = 2.442116e-4 * (float)in0 + offset;
        Serial.print("ADC (ATT=0; 0dB) = ");
        Serial.print(ad0,3);
        Serial.print(" [V], "); 
        Serial.println(in0);
        if( in3 >= 100 ){
            ad3 = ad3 * ((float)in3 - 100.) / 100.
                + ad0 * (200. - (float)in3) / 100.;
        }else{
            ad3 = ad0;
        }
    }
    return ad3 * 1000.;
}
