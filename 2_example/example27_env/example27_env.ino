/*******************************************************************************
Example 27: 乾電池駆動が可能なCO2センサ AMS CCS811

※ 温湿度を測定するために BME280が必要

                                                Copyright (c) 2017 Wataru KUNINO
                                                
CCS811の消費電流が4000μAほどと大きかったので、ディープスリープ時にCCS811をOFFし
実測で70μA程度（BME280、ESPモジュール、レギュレータ込）まで下げることが出来た。
これにより乾電池による駆動が可能となる。
（BME280はドライバbme280.ino内のbme280_stop関数により省エネ待機させている。）

ただし、起動後に適切なデータが得られるまでの時間を要する。
ディープスリープの間隔が約3.7分以上の場合に効果がある(起動延長10秒,90mA動作時)。
※ 4分以下の間隔で送信する場合は、CCS811の電源を入れっぱなしにした方が良い。

*******************************************************************************/

#include <ESP8266WiFi.h>                    // ESP8266用ライブラリ
#include <WiFiUdp.h>                        // UDP通信を行うライブラリ
#include <Wire.h>
#define PIN_LED 13                          // IO 13(5番ピン)にLEDを接続する
#define PIN_CCS_EN_ 2                       // IO 2(7番ピン)にCCS811のWAKを接続
#define PIN_CCS_RST 0                       // IO 0(8番ピン)にCCS811のRSTを接続
#define SSID "1234ABCD"                     // 無線LANアクセスポイントのSSID
#define PASS "password"                     // パスワード
#define SENDTO "192.168.0.255"              // 送信先のIPアドレス
#define PORT 1024                           // 送信のポート番号
#define SLEEP_P 59*60*1000000               // スリープ時間 59分(uint32_t)
#define DEVICE "e_co2_1,"                   // デバイス名(5文字+"_"+番号+",")
#define BME280_ADDR 0x76                    // BME280:76(SDO=low), 77(SDO=high)
#define WAIT_MS 10000                       // 起動待ち時間(最大)
#define MEAS_MS 10000                       // 測定待ち時間(最大)
#define MEAS_TOLER 5                        // 測定値の目標収束誤差（％）

float temp;                                 // 温度値(℃)保持用変数
float hum;                                  // 湿度値(％)保持用変数
float press;                                // 気圧値(Pa)保持用変数
unsigned long start_ms;                     // 起動した時のタイマー値

void setup(){
    start_ms=millis();                      // 初期化開始時のタイマー値を保存
    pinMode(PIN_LED,OUTPUT);                // LEDを接続したポートを出力に
    pinMode(PIN_CCS_EN_,OUTPUT);            // CCS811を接続したポートを出力に
    pinMode(PIN_CCS_RST,OUTPUT);            // CCS811を接続したポートを出力に
    digitalWrite(PIN_LED,HIGH);             // LED ON
    digitalWrite(PIN_CCS_EN_,LOW);          // WAKE UP CCS811
    digitalWrite(PIN_CCS_RST,HIGH);         // リセット解除
    Serial.begin(9600);                     // 動作確認のためのシリアル出力開始
    Serial.println("Example 04 LE");        // 「Example 04」をシリアル出力表示
    Serial.print("initializing BME280.");
    bme280_init();                          // 温湿度・気圧センサを初期化
    temp =bme280_getTemp();                 // 温度値を取得
    hum  =bme280_getHum();                  // 湿度値を取得
    press=bme280_getPress();                // 気圧値を取得
    bme280_stop();                          // 温湿度・気圧センサの動作停止
    Serial.println(" Done."); 
    bme280_print(temp,hum,press);           // 取得したセンサ値をシリアル出力

    Serial.print("initializing CCS811. ");  // CO2センサの初期化(約1秒を要する)
    ccs811_setup(temp,hum);        // 温度と湿度値をCO2センサへ設定
    Serial.println(" Done.");

    Serial.print("Wi-Fi");                  // Wi-Fi接続(約2.2秒)
    WiFi.mode(WIFI_STA);                    // 無線LANをSTAモードに設定
    WiFi.begin(SSID,PASS);                  // 無線LANアクセスポイントへ接続
    while(WiFi.status() != WL_CONNECTED){   // 接続に成功するまで待つ
        delay(200);                         // 待ち時間処理
        digitalWrite(PIN_LED,!digitalRead(PIN_LED));// LEDの点滅
        Serial.print('.');                          // 進捗表示
        if(millis()-start_ms > WAIT_MS) sleep();    // 待ち時間後スリープ
    }
    start_ms=millis();                      // 測定開始時のタイマー値を保存
    Serial.println(WiFi.localIP());         // 本機のIPアドレスをシリアル出力
    digitalWrite(PIN_LED,HIGH);
}

void loop(){
    WiFiUDP udp;                            // UDP通信用のインスタンスを定義
    
    int co2 = ccs811_getCO2();              // CO2センサ値を取得
    int prev= 9999;                         // 前回の値を保持する変数
    
    while(co2<=400 || (abs(co2-prev)+1)*100/co2 > MEAS_TOLER){
        if(co2<0){                          // CO2センサのI2Cの通信異常時
            Serial.println("I2C hardware is locked.");
            digitalWrite(PIN_CCS_RST,LOW);  // リセット実行
            delay(1);                       // リセット待ち
            digitalWrite(PIN_CCS_RST,HIGH); // リセット復帰
            if(ccs811_setup(temp,hum)<0) sleep();
        }
        if(millis()-start_ms>MEAS_MS) break;// 最大測定待ち時間超過で終了
        Serial.print("Too Low CO2 = ");
        Serial.print(co2);
        Serial.println(", Remeasuring...");
        delay(1010);                        // CO2測定間隔が1秒なので1.01待ち
        if(co2>400)prev=co2;                // 前回値を保存
        co2 = ccs811_getCO2();              // 再測定
    }
    Serial.print("CO2 =");
    Serial.println(co2);
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
    ccs811_stop();                          // 測定停止
    delay(10);                              // 待ち時間
    digitalWrite(PIN_CCS_EN_,HIGH);         // DISABLE CCS811
    digitalWrite(PIN_CCS_RST,HIGH);         // リセット解除(元々HIGH)
    digitalWrite(PIN_LED,LOW);              // LEDの消灯
    ESP.deepSleep(SLEEP_P,WAKE_RF_DEFAULT); // スリープモードへ移行する
    while(1){                               // 繰り返し処理
        delay(100);                         // 100msの待ち時間処理
    }                                       // 繰り返し中にスリープへ移行
}
