/*******************************************************************************
Example 59: 乾電池駆動可能なCO2センサ SGP30 ＋温度・湿度 SHT31 or SHT30 or BME280

CO2センサ  SENSIRION製 SGP30
湿度センサ SENSIRION製 SHT31 or SHT30 or Bosch製 BME280 or BMP280

※ご注意 ESP8266版(AMS CCS811)とは使用するCO2センサが異なります。
※CO2センサの本来の使い方は、12時間以上の通電が必要です。
        （本来の使い方をしたい場合は、 SLEEP_P = 0を設定してください。）

二酸化炭素や有機ガスなどによる室内の空気環境状態を測定する乾電池駆動が可能な
ワイヤレスCO2センサです。  

        ガスセンサ SENSIRION製 SGP30
        環境センサ SENSIRION製 SHT31 / SHT30 / Bosch製 BME280 / BMP280

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

float temp  = 25.;                          // 温度値(℃)保持用変数
float hum   = 50.;                          // 湿度値(％)保持用変数
float press =  0.;                          // 気圧値(Pa)保持用変数
unsigned long start_ms;                     // 起動した時のタイマー値
boolean sht31_en = false;                   // 温湿度センサSHT31の状態
boolean sgp30_en = false;                   // CO2センサSGP30の状態(未使用)
boolean bme280_en = false;                  // 温湿度気圧センサBME280の状態

void print_co2(int co2, int tvoc){
    Serial.printf("CO2 = %d(ppm), TVOC = %d(ppb)\n",co2,tvoc);
}

void print_hum(){
    if(temp >= -45. && temp <= 130.)   Serial.printf("Temp.= %.2f(degC), ",temp);
    if(hum >= 0. && hum <= 100.)       Serial.printf("RHum.= %.1f(%%), ",  hum);
    if(press > 400. && press <= 2000.) Serial.printf("Press.= %.1f(hPa), ",press);
    Serial.printf("AHum.= %.1f(g/m3)\n", sgp30_CalcAbsHumid(temp,hum));
}

void setup(){
    start_ms=millis();                      // 初期化開始時のタイマー値を保存
    pinMode(PIN_CCS_EN_,OUTPUT);            // CCS811を接続したポートを出力に
    digitalWrite(PIN_CCS_EN_,LOW);          // WAKE UP CCS811
    Serial.begin(115200);                   // 動作確認のためのシリアル出力開始
    Serial.println("Example 59 LE");        // 「Example 59」をシリアル出力表示

    Serial.print("initializing BME280, ");  // 温湿度・気圧センサBME280の初期化
    bme280_en = bme280_Setup();             // 温湿度・気圧センサBME280を初期化
    if(bme280_en){                          // BME280が有効な時
        temp=bme280_getTemp();              // 温度を取得して変数tempに代入
        float hum_280 = bme280_getHum();    // 湿度を取得して変数hum_280に代入
        if(hum_280 > 0. ) hum = hum_280;    // hum_280が有効時にhumに代入
        press=bme280_getPress();            // 湿度を取得して変数humに代入
        print_hum();                        // 絶対湿度を表示
    }
    Serial.println("Done, BME280="+String(bme280_en)); 
    Serial.print("initializing SHT31, ");   // 温湿度センサSHT31の初期化
    sht31_en = sht31_Setup();               // 温湿度センサSHT31を初期化
    if(sht31_en){                           // SHT31が有効な時
        temp=sht31_getTemp();               // 温度を取得して変数tempに代入
        hum =sht31_getHum();                // 湿度を取得して変数humに代入
        print_hum();                        // 絶対湿度を表示
    }
    Serial.println("Done, SHT31="+String(sht31_en)); 
    Serial.print("initializing SGP30, ");   // CO2センサSGP30の初期化
    sgp30_en = sgp30_Setup();               // CO2センサSGP30を初期化
    sgp30_setHumid(temp,hum);               // 温度と湿度値をCO2センサへ設定
    print_co2(sgp30_getCo2(),sgp30_getTvoc());  // CO2センサ値を表示
    Serial.println("Done, SGP30="+String(sgp30_en)); 
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

    if(bme280_en){                          // BME280が有効な時
        temp=bme280_getTemp();              // 温度を取得して変数tempに代入
        float hum_280 = bme280_getHum();    // 湿度を取得して変数hum_280に代入
        if(hum_280 > 0. ) hum = hum_280;    // hum_280が有効時にhumに代入
        press=bme280_getPress();            // 湿度を取得して変数humに代入
    }
    if(sht31_en){                           // SHT31が有効な時
        temp=sht31_getTemp();               // 温度を取得して変数tempに代入
        hum =sht31_getHum();                // 湿度を取得して変数humに代入
    }
    sgp30_setHumid(temp,hum);               // 温度と湿度値をCO2センサへ設定

    while(co2<=400 || (abs(co2-prev)+1)*100/co2 > MEAS_TOLER){
        if(millis()-start_ms>MEAS_MS) break;// 最大測定待ち時間超過で終了
        if(tvoc > 0) break;
        Serial.print("Too Low, ");
        print_co2(co2,tvoc);
        delay(1010);                        // CO2測定間隔が1秒なので1.01待ち
        if(co2 > 400) prev=co2;             // 前回値を保存
        co2 = sgp30_getCo2();               // 再測定
    }
    print_hum();                            // 湿度センサ値を表示
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
