/*******************************************************************************
Example 12: (IoTセンサ) Wi-Fi 3軸加速度センサ ADXL345

加速度の変化を検出したときに送信するIoTセンサです。
ドアや窓の開閉や傾きの変化などを検出することが出来ます。

・初回起動時は重力加速度の測定結果を送信します。
・その後は、重力加速度を減算した値(相対加速値)を送信します。

                                          Copyright (c) 2016-2019 Wataru KUNINO
*******************************************************************************/

#include <ESP8266WiFi.h>                    // ESP8266用ライブラリ
#include <WiFiUdp.h>                        // UDP通信を行うライブラリ
#define PIN_LED 13                          // IO 13(5番ピン)にLEDを接続する
#define SSID "1234ABCD"                     // 無線LANアクセスポイントのSSID
#define PASS "password"                     // パスワード
#define SENDTO "192.168.0.255"              // 送信先のIPアドレス
#define PORT 1024                           // 送信のポート番号
#define SLEEP_P 29*60*1000000ul             // スリープ時間 29分(uint32_t)
#define DEVICE "accem_1,"                   // デバイス名(5文字+"_"+番号+",")
void sleep();

float acm[3];                               // センサ用の浮動小数点数型変数

void setup(){                               // 起動時に一度だけ実行する関数
    int waiting=0;                          // アクセスポイント接続待ち用
    int start,i;
    
    pinMode(PIN_LED,OUTPUT);                // LEDを接続したポートを出力に
    start=adxlSetup(0);                     // 加速度センサの初期化と結果取得
    for(i=0;i<3;i++) acm[i]=getAcm(i);      // 3軸の加速度を取得し変数acmへ代入
    Serial.begin(9600);                     // 動作確認のためのシリアル出力開始
    Serial.println("Example 12 acm");       // 「Example 12」をシリアル出力表示
    switch(start){                          // 初期化時の結果に応じた表示を実行
        case 0:  Serial.println("Accem Started");        break;
        case 1:  Serial.println("Accem Initialized");    break;
        default: Serial.println("Accem ERROR"); sleep(); break;
    }
    WiFi.mode(WIFI_STA);                    // 無線LANをSTAモードに設定
    WiFi.begin(SSID,PASS);                  // 無線LANアクセスポイントへ接続
    while(WiFi.status() != WL_CONNECTED){   // 接続に成功するまで待つ
        delay(100);                         // 待ち時間処理
        waiting++;                          // 待ち時間カウンタを1加算する
        digitalWrite(PIN_LED,waiting%2);    // LEDの点滅
        if(waiting%10==0)Serial.print('.'); // 進捗表示
        if(waiting > 300) sleep();          // 300回(30秒)を過ぎたらスリープ
    }
    Serial.println(WiFi.localIP());         // 本機のIPアドレスをシリアル出力
}

void loop(){
    WiFiUDP udp;                            // UDP通信用のインスタンスを定義
    int i;
    
    udp.beginPacket(SENDTO, PORT);          // UDP送信先を設定
    udp.print(DEVICE);                      // デバイス名を送信
    for(i=0;i<3;i++){                       // X,Y,Zの計3軸分の繰り返し処理
        udp.print(acm[i],0);                // 起動時の加速度値を送信
        Serial.print(acm[i],1);             // シリアル出力表示
        udp.print(",");                     // 「,」カンマを送信
        Serial.print(",");                  // シリアル出力表示
    }
    udp.print(" ");                         // スペースを送信
    Serial.print(" ");                      // シリアル出力表示
    for(i=0;i<3;i++){                       // X,Y,Zの計3軸分の繰り返し処理
        udp.print(getAcm(i),0);             // 現在の加速度値を送信
        Serial.print(getAcm(i),1);          // シリアル出力表示
        if(i<2){
            udp.print(",");                 // 「,」カンマを送信
            Serial.print(",");              // シリアル出力表示
        }else{
            udp.println();                  // 改行を送信
            Serial.println();               // シリアル出力表示
        }
    }
    udp.endPacket();                        // UDP送信の終了(実際に送信する)
    adxlINT();                              // 加速度センサの割込みを有効にする
    sleep();
}

void sleep(){
    digitalWrite(PIN_LED,LOW);              // LEDの消灯/ESP仕様変更に伴い修正
    delay(200);                             // 送信待ち時間
    ESP.deepSleep(SLEEP_P,WAKE_RF_DEFAULT); // スリープモードへ移行する
    while(1){                               // 繰り返し処理
        delay(100);                         // 100msの待ち時間処理
    }                                       // 繰り返し中にスリープへ移行
}
