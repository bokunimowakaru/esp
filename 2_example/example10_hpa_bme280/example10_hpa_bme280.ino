/*******************************************************************************
Example 10: (IoTセンサ) Wi-Fi 気圧計 Bosch製 BMP280/BME280 版
Bosch製 BMP280/BME280から取得した気圧と温度を送信するIoTセンサです。

気圧センサLPS25Hの販売終了にともない、Bosch BMP280に対応しました。
BME280にも対応します。湿度測定についてはコメントアウトしてあります。

	BMP280：温度＋気圧センサ
	BME280：温度＋湿度＋気圧センサ

                                          Copyright (c) 2016-2019 Wataru KUNINO
*******************************************************************************/

#include <ESP8266WiFi.h>                    // ESP8266用ライブラリ
#include <WiFiUdp.h>                        // UDP通信を行うライブラリ
#define PIN_EN 13                           // IO 13(5番ピン)をセンサ用の電源に
#define SSID "1234ABCD"                     // 無線LANアクセスポイントのSSID
#define PASS "password"                     // パスワード
#define SENDTO "192.168.0.255"              // 送信先のIPアドレス
#define PORT 1024                           // 送信のポート番号
#define SLEEP_P 29*60*1000000               // スリープ時間 29分(uint32_t)
#define DEVICE "press_1,"                   // デバイス名(5文字+"_"+番号+",")
void sleep();

void setup(){                               // 起動時に一度だけ実行する関数
    int waiting=0;                          // アクセスポイント接続待ち用
    pinMode(PIN_EN,OUTPUT);                 // センサ用の電源を出力に
    digitalWrite(PIN_EN,HIGH);              // センサ用の電源をONに
    bme280_init();                          // 気圧センサの初期化
    Serial.begin(9600);                     // 動作確認のためのシリアル出力開始
    Serial.println("Example 10 hPa");       // 「Example 10」をシリアル出力表示
    WiFi.mode(WIFI_STA);                    // 無線LANをSTAモードに設定
    WiFi.begin(SSID,PASS);                  // 無線LANアクセスポイントへ接続
    while(WiFi.status() != WL_CONNECTED){   // 接続に成功するまで待つ
        delay(100);                         // 待ち時間処理
        waiting++;                          // 待ち時間カウンタを1加算する
        if(waiting%10==0)Serial.print('.'); // 進捗表示
        if(waiting > 300) sleep();          // 300回(30秒)を過ぎたらスリープ
    }
    Serial.println(WiFi.localIP());         // 本機のIPアドレスをシリアル出力
}

void loop(){
    WiFiUDP udp;                            // UDP通信用のインスタンスを定義
    float temp,press;                       // センサ用の浮動小数点数型変数
    
    temp=bme280_getTemp();                  // 温度を取得して変数tempに代入
    press=bme280_getPress();                // 気圧を取得して変数pressに代入
//  float hum=bme280_getHum();              // (湿度を取得)
    bme280_stop();                          // 気圧センサの停止
    if( temp>-100. && press>=0.){           // 適切な値の時
        udp.beginPacket(SENDTO, PORT);      // UDP送信先を設定
        udp.print(DEVICE);                  // デバイス名を送信
        udp.print(temp,0);                  // 変数tempの値を送信
        Serial.print(temp,2);               // シリアル出力表示
        udp.print(", ");                    // 「,」カンマを送信
        Serial.print(", ");                 // シリアル出力表示
        /*
        udp.print(hum,0);                   // 変数tempの値を送信
        Serial.print(hum,1);                // シリアル出力表示
        udp.print(", ");                    // 「,」カンマを送信
        Serial.print(", ");                 // シリアル出力表示
        */
        udp.println(press,0);               // 変数pressの値を送信
        Serial.println(press,2);            // シリアル出力表示
        udp.endPacket();                    // UDP送信の終了(実際に送信する)
    }
    sleep();
}

void sleep(){
    delay(200);                             // 送信待ち時間
    digitalWrite(PIN_EN,LOW);               // センサ用の電源をOFFに
    ESP.deepSleep(SLEEP_P,WAKE_RF_DEFAULT); // スリープモードへ移行する
    while(1){                               // 繰り返し処理
        delay(100);                         // 100msの待ち時間処理
    }                                       // 繰り返し中にスリープへ移行
}
