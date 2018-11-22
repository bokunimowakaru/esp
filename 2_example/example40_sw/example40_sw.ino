/*******************************************************************************
Example 40=(32+8): ESP32 (IoTセンサ) Wi-Fi ドア開閉モニタ リードスイッチ
ドアや窓が開いたとき（または閉まったとき）に開閉状態を送信するIoTセンサです。

　　　　ESP8266(ESP-WROOM-02)ではENピンを使ってスリープを解除していました。
　　　　一方、ESP32(ESP-WROOM-32)にはGPIO入力にスリープ解除機能があるので、
　　　　リードスイッチをGPIOへ接続するだけで同じ作用を果たすことが出来ます。
　　　　本サンプルでは、GPIO 0へリードスイッチを接続すると、GPIO 0の信号に
　　　　変化があった時に送信します（ドア開閉の両方に対応）。
　　　　開/閉の片方にしたい場合はesp_sleep_enable_ext0_wakeupの第2引数に
　　　　0または1を指定してください。

                                          Copyright (c) 2016-2019 Wataru KUNINO
*******************************************************************************/

#include <WiFi.h>                           // ESP32用WiFiライブラリ
#include <WiFiUdp.h>                        // UDP通信を行うライブラリ
#include "esp_sleep.h"                      // ESP32用Deep Sleep ライブラリ
#define PIN_SW 0                            // GPIO 0(25番ピン)にスイッチを接続
#define PIN_SW_GPIO_NUM GPIO_NUM_0          // GPIO 0をスリープ解除信号へ設定
#define BUTTON_PIN_BITMASK 0x000000001      // 2^(PIN_SW+1) in hex
#define PIN_LED 2                           // GPIO 2(24番ピン)にLEDを接続
#define SSID "1234ABCD"                     // 無線LANアクセスポイントのSSID
#define PASS "password"                     // パスワード
#define SENDTO "192.168.0.255"              // 送信先のIPアドレス
#define PORT 1024                           // 送信のポート番号
#define SLEEP_P 3550*1000000ul              // スリープ時間 3550秒(約60分)
#define DEVICE "rd_sw_1,"                   // デバイス名(5文字+"_"+番号+",")

int reed;                                   // リードスイッチの状態用

void setup(){                               // 起動時に一度だけ実行する関数
    int waiting=0;                          // アクセスポイント接続待ち用
    pinMode(PIN_SW,INPUT_PULLUP);           // スイッチを接続したポートを入力に
    pinMode(PIN_LED,OUTPUT);                // LEDを接続したポートを出力に
    reed=digitalRead(PIN_SW);               // スイッチの状態を取得
    Serial.begin(115200);                   // 動作確認のためのシリアル出力開始
    Serial.println("ESP32 eg.08 REED SW");  // 「Example 08」をシリアル出力表示
    WiFi.mode(WIFI_STA);                    // 無線LANをSTAモードに設定
    WiFi.begin(SSID,PASS);                  // 無線LANアクセスポイントへ接続
    while(WiFi.status() != WL_CONNECTED){   // 接続に成功するまで待つ
        delay(100);                         // 待ち時間処理
        waiting++;                          // 待ち時間カウンタを1加算する
        digitalWrite(PIN_LED,waiting%2);    // LED(EN信号)の点滅
        if(waiting%10==0)Serial.print('.'); // 進捗表示
        if(waiting > 300) sleep();          // 300回(30秒)を過ぎたらスリープ
    }
    Serial.println(WiFi.localIP());         // 本機のIPアドレスをシリアル出力
    Serial.print(reed);                     // 起動直後のスイッチ状態を出力表示
    Serial.print(", ");                     // 「,」カンマと「␣」を出力表示
}

void loop(){
    WiFiUDP udp;                            // UDP通信用のインスタンスを定義
    
    udp.beginPacket(SENDTO, PORT);          // UDP送信先を設定
    udp.print(DEVICE);                      // デバイス名を送信
    udp.print(reed);                        // 起動直後のスイッチ状態を送信
    udp.print(", ");                        // 「,」カンマと「␣」を送信
    reed=digitalRead(PIN_SW);               // スイッチの状態を取得
    udp.println(reed);                      // 現在のスイッチの状態を送信
    Serial.println(reed);                   // シリアル出力表示
    udp.endPacket();                        // UDP送信の終了(実際に送信する)
    sleep();
}

void sleep(){
    delay(200);                             // 送信待ち時間
    esp_sleep_enable_ext0_wakeup(PIN_SW_GPIO_NUM,!reed);  // 1=High,0=Low
                                            // リードスイッチの状態が変化すると
                                            // スリープを解除するように設定
    esp_deep_sleep(SLEEP_P);                // Deep Sleepモードへ移行
}
