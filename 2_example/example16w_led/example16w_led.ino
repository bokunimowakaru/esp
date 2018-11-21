/*******************************************************************************
Example 16: Wi-Fi コンシェルジェ 照明担当 (キャンドルLED制御)
HTTPによるWebサーバ機能搭載 Wi-FiコンシェルジェがキャンドルLEDを制御します。
[ESP8266WebServer対応版]
                                          Copyright (c) 2016-2019 Wataru KUNINO
*******************************************************************************/

#include <ESP8266WiFi.h>                    // Wi-Fi機能を利用するために必要
#include <ESP8266WebServer.h>
extern "C" {
#include "user_interface.h"                 // ESP8266用の拡張IFライブラリ
}
#define PIN_LED 13                          // IO 13(5番ピン)にLEDを接続する
#define SSID "1234ABCD"                     // 無線LANアクセスポイントのSSID
#define PASS "password"                     // パスワード

ESP8266WebServer server(80);                // Webサーバ(ポート80=HTTP)定義
int led=0;                                  // 現在のLEDの輝度(0は消灯)
int target=0;                               // LED設定値(0は消灯)

void handleRoot(){
    char html[2048];                        // Web表示用コンテンツ格納用変数
    Serial.println("Connected");            // 接続表示
    
    if(server.hasArg("L")){                 // 引数Lが含まれていた時
        String s=server.arg("L");           // 引数Lの値を取得し文字変数sへ代入
        target=s.toInt();                   // 文字変数sから数値を取得しtargetへ
    }
    getHtml(html,target);                   // HTMLコンテンツを取得
    server.send(200, "text/html", html);    // HTMLコンテンツを送信
    Serial.println("Disconnected");         // 切断表示
    
    if(target==0) led=ledCtrl(led,0,4);     // ゆっくりと消灯
    if(target==1) led=ledCtrl(led,1023,4);  // ゆっくりと点灯
    if(target<=0 && target>=-10) led=ledCtrl(led,-100*target,4);// 輝度変更
}

void setup(){                               // 起動時に一度だけ実行する関数
    pinMode(PIN_LED,OUTPUT);                // LEDを接続したポートを出力に
    Serial.begin(9600);                     // 動作確認のためのシリアル出力開始
    Serial.println("Example 16 LED HTTP");  // 「Example 16」をシリアル出力表示
    wifi_set_sleep_type(LIGHT_SLEEP_T);     // 省電力モードに設定する
    WiFi.mode(WIFI_STA);                    // 無線LANをSTAモードに設定
    WiFi.begin(SSID,PASS);                  // 無線LANアクセスポイントへ接続
    morse(PIN_LED,50,"HELLO");              // 「HELLO」をモールス信号出力
    while(WiFi.status() != WL_CONNECTED){   // 接続に成功するまで待つ
        Serial.print('.');                  // 進捗表示
        digitalWrite(PIN_LED,!digitalRead(PIN_LED));    // LEDの点滅
        delay(500);                         // 待ち時間処理
    }
    server.on("/", handleRoot);             // HTTP接続時のコールバック先を設定
    server.begin();                         // Web サーバを起動する
    Serial.println("\nStarted");            // 起動したことをシリアル出力表示
    Serial.println(WiFi.localIP());         // 本機のIPアドレスをシリアル出力
    morseIp0(PIN_LED,50,WiFi.localIP());    // IPアドレス終値をモールス信号出力
}

void loop(){                                // 繰り返し実行する関数
    server.handleClient();                  // クライアントからWebサーバ呼び出し
    if(target>1 && target<=10){             // 1よりも大きく10以下のとき
        led=ledCtrl(led,23+random(0,target*100),20);
    }                                       // LEDの輝度を乱数値23～1023に設定
}
