/*******************************************************************************
IoT Press 気圧センサ
ROHM BM1383AGLV
乾電池などで動作するIoT気圧センサです

                                          Copyright (c) 2016-2019 Wataru KUNINO
*******************************************************************************/
#include <WiFi.h>                           // ESP32用WiFiライブラリ
#include <WiFiUdp.h>                        // UDP通信を行うライブラリ
#include "esp_sleep.h"                      // ESP32用Deep Sleep ライブラリ
#define PIN_EN 2                            // GPIO 2 にLEDを接続
#define PIN_BUZZER 12                       // GPIO 12 にスピーカを接続
#define SSID "1234ABCD"                     // 無線LANアクセスポイントのSSID
#define PASS "password"                     // パスワード
#define PORT 1024                           // 送信のポート番号
#define SLEEP_P 50*1000000ul                // スリープ時間 50秒(uint32_t)
#define DEVICE "press_1,"                   // デバイス名(5文字+"_"+番号+",")

#include <Wire.h>
#include "BM1383AGLV.h"

BM1383AGLV bm1383aglv;
IPAddress IP;                               // 本機IPアドレス
IPAddress IP_BC;                            // ブロードキャストIPアドレス

void setup(){                               // 起動時に一度だけ実行する関数
    int waiting=0;                          // アクセスポイント接続待ち用
    pinMode(PIN_EN,OUTPUT);                 // センサ用の電源を出力に
    pinMode(PIN_BUZZER,OUTPUT);             // ブザーを接続したポートを出力に
    delay(10);                              // 起動待ち時間
    Serial.begin(115200);                   // 動作確認のためのシリアル出力開始
    Serial.println("IoT Press BM1383AGLV"); // 「IoT Press」をシリアル出力表示
    int wake = TimerWakeUp_init();
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
        if(waiting > 60) sleep();           // 60回(30秒)を過ぎたらスリープ
    }
    IP = WiFi.localIP();
    IP_BC = (uint32_t)IP | ~(uint32_t)(WiFi.subnetMask());
    Serial.println(IP);                     // 本機のIPアドレスをシリアル出力
    if(wake<3) morseIp0(PIN_BUZZER,50,IP);  // IPアドレス終値をモールス信号出力
}

void loop() {
    WiFiUDP udp;                            // UDP通信用のインスタンスを定義
    float temp,press;                       // センサ用の浮動小数点数型変数

    if(!bm1383aglv.get_val(&press,&temp)){  // 気圧と温度を取得して変数へ代入
        udp.beginPacket(IP_BC, PORT);       // UDP送信先を設定
        udp.print(DEVICE);                  // デバイス名を送信
        udp.print(temp,0);                  // 変数tempの値を送信
        Serial.print(temp,2);               // シリアル出力表示
        udp.print(", ");                    // 「,」カンマを送信
        Serial.print(", ");                 // シリアル出力表示
        udp.println(press,0);               // 変数pressの値を送信
        Serial.println(press,2);            // シリアル出力表示
        udp.endPacket();                    // UDP送信の終了(実際に送信する)
    }
    sleep();
}

void sleep(){
    ledcWriteNote(0,NOTE_D,8);              // 送信中の音
    delay(200);                             // 送信待ち時間
    ledcWrite(0, 0);
    esp_deep_sleep(SLEEP_P);                // Deep Sleepモードへ移行
}
