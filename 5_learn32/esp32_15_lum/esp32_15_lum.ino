/*******************************************************************************
Practice esp32 15 lum 【Wi-Fi 照度センサ子機】ディープスリープ版

                                           Copyright (c) 2016-2019 Wataru KUNINO
*******************************************************************************/
#include <WiFi.h>                           // ESP32用WiFiライブラリ
#include <WiFiUdp.h>                        // UDP通信を行うライブラリ
#include "esp_sleep.h"                      // ESP32用Deep Sleep ライブラリ
#define PIN_EN 2                            // IO 2を照度センサの電源に
#define PIN_AIN 33                          // IO 33へ照度センサの出力を接続
#define SSID "1234ABCD"                     // 無線LANアクセスポイントのSSID
#define PASS "password"                     // パスワード
#define SENDTO "192.168.0.255"              // 送信先のIPアドレス
#define PORT 1024                           // 受信ポート番号
    #define SLEEP_P 20ul  *1000000ul        // スリープ時間 20秒(実験用)
//  #define SLEEP_P 3550ul*1000000ul        // スリープ時間 3550秒(約60分)
#define DEVICE "illum_1,"                   // デバイス名(5文字+"_"+番号+",")

void setup(){                               // 起動時に一度だけ実行する関数
    pinMode(PIN_AIN,INPUT);                 // アナログ入力の設定
    pinMode(PIN_EN,OUTPUT);                 // センサ用の電源を出力に
    WiFi.mode(WIFI_STA);                    // 無線LANをSTAモードに設定
    WiFi.begin(SSID,PASS);                  // 無線LANアクセスポイントへ接続
    while(WiFi.status() != WL_CONNECTED){   // 接続に成功するまで待つ
        digitalWrite(PIN_EN,LOW);           // LEDの消灯
        delay(100);                         // 待ち時間
        digitalWrite(PIN_EN,HIGH);          // LEDの点灯・センサ用の電源をONに
        delay(100);                         // 待ち時間
        if(millis()>30000) esp_deep_sleep(SLEEP_P);  // 30秒を過ぎたらスリープ
    }
}

void loop(){                                // 繰り返し実行する関数
    WiFiUDP udp;                            // UDP通信用のインスタンスを定義
    float lux = mvAnalogIn(PIN_AIN);        // 照度値用の変数へAD変換値を代入

    digitalWrite(PIN_EN,LOW);               // センサ用の電源をOFFに
    lux *= 100. / 330.;                     // 照度(lx)へ変換
    udp.beginPacket(SENDTO, PORT);          // UDP送信先を設定
    udp.print(DEVICE);                      // デバイス名を送信
    udp.println(lux,0);                     // 照度値を送信
    udp.endPacket();                        // UDP送信の終了(実際に送信する)
    delay(10);                              // 送信待ち時間
    esp_deep_sleep(SLEEP_P);                // Deep Sleepモードへ移行
}

float mvAnalogIn(uint8_t PIN){
    return mvAnalogIn(PIN, 0.0);            // 動作最小電圧 0.0 ～ 0.1(V)程度
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
    if( in3 < 200 ){
        analogSetPinAttenuation(PIN,ADC_0db);
        in0=analogRead(PIN);
        ad0 = 2.442116e-4 * (float)in0 + offset;
        if( in3 >= 100 ){
            ad3 = ad3 * ((float)in3 - 100.) / 100.
                + ad0 * (200. - (float)in3) / 100.;
        }else{
            ad3 = ad0;
        }
    }
    return ad3 * 1000.;
}
