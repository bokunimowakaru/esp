/*******************************************************************************
Practice esp32 16 hum 【Wi-Fi 温湿度センサ子機】ディープスリープ版

                                           Copyright (c) 2016-2019 Wataru KUNINO
*******************************************************************************/
#include <WiFi.h>                           // ESP32用WiFiライブラリ
#include <WiFiUdp.h>                        // UDP通信を行うライブラリ
#include "esp_sleep.h"                      // ESP32用Deep Sleep ライブラリ
#define PIN_EN 2                            // IO 2にセンサ用の電源を接続
#define SSID "1234ABCD"                     // 無線LANアクセスポイントのSSID
#define PASS "password"                     // パスワード
#define SENDTO "192.168.0.255"              // 送信先のIPアドレス
#define PORT 1024                           // 受信ポート番号
    #define SLEEP_P 20ul  *1000000ul        // スリープ時間 20秒(実験用)
//  #define SLEEP_P 3550ul*1000000ul        // スリープ時間 3550秒(約60分)
#define DEVICE "humid_1,"                   // デバイス名(5文字+"_"+番号+",")

void setup() {
    pinMode(PIN_EN,OUTPUT);                 // センサ用の電源を出力に
    WiFi.mode(WIFI_STA);                    // 無線LANをSTAモードに設定
    WiFi.begin(SSID,PASS);                  // 無線LANアクセスポイントへ接続
    digitalWrite(PIN_EN,HIGH);              // センサ用の電源をONに
    shtSetup();                             // センサの初期化処理を実行
    while(WiFi.status() != WL_CONNECTED){   // 接続に成功するまで待つ
        delay(100);                         // 待ち時間
        if(millis()>30000) esp_deep_sleep(SLEEP_P);  // 30秒を過ぎたらスリープ
    }
}

void loop() {
    WiFiUDP udp;                            // UDP通信用のインスタンスを定義
    float temp=getTemp();                   // 温度値を取得し、変数tempへ代入
    float hum =getHum();                    // 湿度値を変数humへ代入
    udp.beginPacket(SENDTO, PORT);          // UDP送信先を設定
    udp.print(DEVICE);                      // デバイス名を送信
    udp.print(temp,2);                      // 変数tempの値をシリアル出力
    udp.print(", ");                        // カンマとスペースをシリアル出力
    udp.println(hum,2);                     // 変数tempの値をシリアル出力
    udp.endPacket();                        // UDP送信の終了(実際に送信する)
    delay(10);                              // 送信待ち時間
    esp_deep_sleep(SLEEP_P);                // Deep Sleepモードへ移行
}
