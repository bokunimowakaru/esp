/*******************************************************************************
Example 59: 乾電池駆動可能なCO2センサ SDP30 ＋温度・湿度 SHT31 or SHT30

CO2センサ SENSIRION SDP30
湿度センサ SENSIRION SHT31 or SHT30

※ご注意 ESP8266版(AMS CCS811 + BME280)とは使用するセンサが異なります。
※CO2センサの本来の使い方は、12時間以上の通電が必要です。
        （本来の使い方をしたい場合は、 SLEEP_P = 0を設定してください。）

二酸化炭素や有機ガスなどによる室内の空気環境状態を測定する
        ガスセンサSENSIRION製 SDP30 と

温度・湿度を測定する
        環境センサ SENSIRION製 SHT31 を使った、

乾電池駆動が可能なワイヤレスCO2センサです。  

制約事項：

　測定精度と起動後の測定継続時間は、トレードオフの関係になります。
　本来のセンサの測定精度を得ることは出来ません（12時間以上の連続動作が必要）。

PIN_I2C_SDA = 21
PIN_I2C_SCL = 22

                                           Copyright (c) 2017-2021 Wataru KUNINO
*******************************************************************************/

#include <WiFi.h>                           // ESP32用WiFiライブラリ
#include <WiFiUdp.h>                        // UDP通信を行うライブラリ
#include "esp_sleep.h"                      // ESP32用Deep Sleep ライブラリ
#define PIN_CCS_EN_ 2                       // IO 2(7番ピン)にCCS811のWAKを接続
#define SSID "1234ABCD"                     // 無線LANアクセスポイントのSSID
#define PASS "password"                     // パスワード
#define SENDTO "192.168.0.255"              // 送信先のIPアドレス
#define PORT 1024                           // 送信のポート番号
#define SLEEP_P 9*60*1000000ul              // スリープ時間 9分(uint32_t)
#define DEVICE "e_co2_1,"                   // デバイス名(5文字+"_"+番号+",")
#define WAIT_MS 10000                       // (最大)起動待ち時間
#define MEAS_MS 20000                       // (最大)測定待ち時間 15秒以上
#define MEAS_TOLER 5                        // 測定値の目標収束誤差（％）

float temp;                                 // 温度値(℃)保持用変数
float hum;                                  // 湿度値(％)保持用変数
float press = 0.; // 未使用                 // 気圧値(Pa)保持用変数
unsigned long start_ms;                     // 起動した時のタイマー値

void print_co2(int co2, int tvoc){
    Serial.printf("CO2 = %d(ppm), TVOC = %d(ppb)\n",co2,tvoc);
}

void print_hum(float temp, float hum){
    Serial.printf("Temp.= %.2f(degC), RHum.= %.1f(%%), AHum.= %.1f(g/m3)\n",
        temp,
        hum,
        sgp30_CalcAbsHumid(temp,hum)
    );
}

void setup(){
    start_ms=millis();                      // 初期化開始時のタイマー値を保存
    pinMode(PIN_CCS_EN_,OUTPUT);            // CCS811を接続したポートを出力に
    digitalWrite(PIN_CCS_EN_,LOW);          // WAKE UP CCS811
    Serial.begin(115200);                   // 動作確認のためのシリアル出力開始
    Serial.println("Example 59 LE");        // 「Example 59」をシリアル出力表示
    Serial.print("initializing SHT31.");
    sht31_Setup();                          // 温湿度・気圧センサを初期化
    temp=sht31_getTemp();                   // 温度を取得して変数tempに代入
    hum =sht31_getHum();                    // 湿度を取得して変数humに代入
    print_hum(temp,hum);                    // 湿度センサ値を表示
    Serial.println(" Done."); 

    Serial.print("initializing SDP30. ");   // CO2センサの初期化
    sgp30_Setup();
    sgp30_setHumid(temp,hum);               // 温度と湿度値をCO2センサへ設定
    print_co2(sgp30_getCo2(),sgp30_getTvoc());  // CO2センサ値を表示
    Serial.println(" Done.");

    Serial.print("Wi-Fi");                  // Wi-Fi接続(約2.2秒)
    WiFi.mode(WIFI_STA);                    // 無線LANをSTAモードに設定
    WiFi.begin(SSID,PASS);                  // 無線LANアクセスポイントへ接続
    while(WiFi.status() != WL_CONNECTED){   // 接続に成功するまで待つ
        delay(200);                         // 待ち時間処理
        Serial.print('.');                          // 進捗表示
        if(millis()-start_ms > WAIT_MS) sleep();    // 待ち時間後スリープ
    }
    start_ms=millis();                      // 測定開始時のタイマー値を保存
    Serial.println(WiFi.localIP());         // 本機のIPアドレスをシリアル出力
}

void loop(){
    WiFiUDP udp;                            // UDP通信用のインスタンスを定義
    
    int co2 = sgp30_getCo2();               // CO2センサ値を取得
    int tvoc= sgp30_getTvoc();              // TVOCセンサ値を取得
    int prev= 99999;                        // 前回の値を保持する変数
    temp=sht31_getTemp();                   // 温度を取得して変数tempに代入
    hum =sht31_getHum();                    // 湿度を取得して変数humに代入
    sgp30_setHumid(temp,hum);               // 温度と湿度値をCO2センサへ設定

    while(co2<=400 || (abs(co2-prev)+1)*100/co2 > MEAS_TOLER){
        if(millis()-start_ms>MEAS_MS) break;// 最大測定待ち時間超過で終了
        if(tvoc > 0) break;
        Serial.print("Too Low ");
        print_co2(co2,tvoc);
        delay(1010);                        // CO2測定間隔が1秒なので1.01待ち
        if(co2 > 400) prev=co2;             // 前回値を保存
        co2 = sgp30_getCo2();               // 再測定
    }
    print_hum(temp,hum);                    // 湿度センサ値を表示
    print_co2(co2,tvoc);                    // CO2センサ値を表示
    udp.beginPacket(SENDTO, PORT);          // UDP送信先を設定
    udp.print(DEVICE);                      // デバイス名を送信
    udp.print(temp,2);                      // 変数tempの値を送信
    Serial.print(temp,2);                   // シリアル出力表示
    udp.print(", ");                        // 「,」カンマを送信
    Serial.print(", ");                     // シリアル出力表示
    udp.print(hum,2);                       // 変数humの値を送信
    Serial.print(hum,2);                    // シリアル出力表示
    udp.print(", ");                        // 「,」カンマを送信
    Serial.print(", ");                     // シリアル出力表示
    udp.print(press,2);                     // 変数press/100の値を送信
    Serial.print(press,2);                  // シリアル出力表示
    udp.print(", ");                        // 「,」カンマを送信
    Serial.print(", ");                     // シリアル出力表示
    udp.println(co2);                       // 変数co2の値を送信
    Serial.println(co2);                    // シリアル出力表示
    udp.endPacket();                        // UDP送信の終了(実際に送信する)
    delay(200);                             // 送信待ち時間
    sleep();                                // スリープへ
}

void sleep(){
    if(SLEEP_P==0){
        delay(19800);                       // 20秒間隔
        return;
    }
    digitalWrite(PIN_CCS_EN_,HIGH);         // センサ用の電源をOFFに
    delay(200);                             // 送信待ち時間
    esp_deep_sleep(SLEEP_P);                // Deep Sleepモードへ移行
}
