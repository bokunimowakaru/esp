/*******************************************************************************
Practice esp32 14 pir 【Wi-Fi 人感センサ子機】

                                           Copyright (c) 2016-2017 Wataru KUNINO
*******************************************************************************/
#include <WiFi.h>                           // ESP32用WiFiライブラリ
#include <WiFiUdp.h>                        // UDP通信を行うライブラリ
#include "esp_sleep.h"                      // ESP32用Deep Sleep ライブラリ
#define PIN_LED 2                           // IO 2にLEDを接続
#define PIN_SW 14                           // IO 14にスイッチを接続
#define PIN_SW_GPIO_NUM GPIO_NUM_14         // IO 14をスリープ解除信号へ設定
#define SSID "1234ABCD"                     // 無線LANアクセスポイントのSSID
#define PASS "password"                     // パスワード
#define SENDTO "192.168.0.255"              // 送信先のIPアドレス
#define PORT 1024                           // 受信ポート番号
#define SLEEP_P 3550ul*1000000ul            // スリープ時間 3550秒(約60分)
#define DEVICE "pir_s_1,"                   // デバイス名(5文字+"_"+番号+",")
boolean pir;                                // 人感センサ値

void setup(){                               // 起動時に一度だけ実行する関数
    pinMode(PIN_LED,OUTPUT);                // LEDを接続したポートを出力に
    pinMode(PIN_SW,INPUT);                  // センサを接続したポートを入力に
    pir=digitalRead(PIN_SW);                // 人感センサの状態を取得
    WiFi.mode(WIFI_STA);                    // 無線LANをSTAモードに設定
    WiFi.begin(SSID,PASS);                  // 無線LANアクセスポイントへ接続
    while(WiFi.status() != WL_CONNECTED){   // 接続に成功するまで待つ
        digitalWrite(PIN_LED,LOW);          // LEDの消灯
        delay(100);                         // 待ち時間
        digitalWrite(PIN_LED,HIGH);         // LEDの点灯
        delay(100);                         // 待ち時間
        if(millis() > 30000) sleep();       // 30000ms(30秒)を過ぎたらスリープ
    }
}

void loop(){                                // 繰り返し実行する関数
    WiFiUDP udp;                            // UDP通信用のインスタンスを定義
    udp.beginPacket(SENDTO, PORT);          // UDP送信先を設定
    udp.print(DEVICE);                      // デバイス名を送信
    udp.print(pir);                         // 起動直後のセンサ状態を送信
    udp.print(", ");                        // 「,」カンマと「␣」を送信
    pir=digitalRead(PIN_SW);                // 人感センサの状態を取得
    udp.println(pir);                       // 現在のセンサの状態を送信
    udp.endPacket();                        // UDP送信の終了(実際に送信する)
    delay(10);                              // 送信待ち時間
    if(pir==LOW)sleep();                    // sleepを実行
    delay(500);                             // 0.5秒の待ち時間
}

void sleep(){
    esp_sleep_enable_ext0_wakeup(PIN_SW_GPIO_NUM,HIGH);    // HIGHを待ち受け
    esp_deep_sleep(SLEEP_P);                // Deep Sleepモードへ移行
}
