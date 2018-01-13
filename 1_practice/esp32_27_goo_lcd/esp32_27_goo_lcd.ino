/*******************************************************************************
esp32 27 goo lcd

Google カレンダー(予定表) から予定を取得する

クラウド（Google Apps Script）で実行したスクリプトを本機で受け取ります。
予め本フォルダ内のGoogleCalendar.jsを https://script.google.com/ へ転送し、
ウェブアプリケーションを公開しておく必要があります。

                                           Copyright (c) 2017-2018 Wataru KUNINO
********************************************************************************/

#include <WiFi.h>
#include <LiquidCrystal.h>                  // LCDへの表示を行うライブラリ
#include "esp_sleep.h"                      // ESP32用Deep Sleep ライブラリ
#include "HTTPSRedirect.h"                  // リダイレクト接続用ライブラリ

#define CQ_PUB_IOT_EXPRESS                  // CQ出版 IoT Express 用
#define SSID "1234ABCD"                     // 無線LANアクセスポイントのSSID
#define PASS "password"                     // パスワード
#define GScriptId "★ここにGoogle Apps Scriptのトークンを記入してください★"

#define TIMEOUT 7000                        // タイムアウト 7秒
#define SLEEP_P 1*60*1000000                // スリープ時間 1分(uint32_t)
#define HTTPTO "script.google.com"          // HTTPSアクセス先
#define HTRED "script.googleusercontent.com"// HTTPSリダイレクト先
#define PORT 443                            // HTTPSポート番号

LiquidCrystal lcd(17,26,13,14,15,16);       // CQ出版 IoT Express 用 LCD開始
String url = String("/macros/s/") + String(GScriptId) + "/exec";

void setup() {
    unsigned long t=millis();               // 初期化開始時のタイマー値を保存
    lcd.begin(16, 2);                       // 液晶の初期化(16桁×2行)
    lcdisp("Google ｶﾚﾝﾀﾞ LCD");             // タイトルをLCDに表示する
    WiFi.mode(WIFI_STA);                    // 無線LANを【STA】モードに設定
    WiFi.begin(SSID,PASS);                  // 無線LANアクセスポイントへ接続
    lcd.setCursor(0,1);                     // カーソル位置を液晶の左下へ
    while(WiFi.status() != WL_CONNECTED){   // 接続に成功するまで待つ
        if(millis()-t > TIMEOUT) sleep();   // 待ち時間後の処理
        lcd.print(".");                     // 接続進捗を表示
        delay(500);                         // 待ち時間処理
    }
    Serial.println(WiFi.localIP());         // IPアドレスをシリアル出力表示
}

void loop() {
    String data;                            // 受信データ
    String event;                           // イベント名
    char disp[97];                          // 液晶表示データ(3バイト×32文字分)
    int sp,ep;                              // 文字列位置用の変数
    int events_n;                           // イベント数
    int hour,min;                           // 時刻
    
    HTTPSRedirect client(PORT);             // リダイレクト可能なHTTP接続client
    if(!client.connect(HTTPTO,PORT))sleep();// HTTP接続の実行(失敗時はスリープ)
    data=client.getData(url,HTTPTO,HTRED);  // データ受信
    Serial.println(data);                   // 受信データをシリアルへ出力
    sp=data.indexOf("|Length,");            // 受信データから文字列Lengthを検索
    if(sp>=0) sp+=8; else sleep();          // 見つからなければsleepへ
    events_n=data.substring(sp).toInt();    // 予定数(Length値)を保存
    lcd.clear();                            // 画面を消去
    if(events_n>2) events_n=2;              // 2件以上の時は表示上限値の2件に
    for(int i=0; i<events_n; i++){          // 予定回数の繰り返し
        sp=data.indexOf("|",sp+1)+1;        // 次の区切りの文字位置を変数spへ
        hour=data.substring(sp).toInt();    // 文字位置spの予定時間（時）を取得
        sp=data.indexOf(",",sp+1)+1;        // カンマの次の文字位置を変数spへ
        min=data.substring(sp).toInt();     // 文字位置spの予定時間（分）を取得
        sp=data.indexOf(",",sp+1)+1;        // カンマの次の文字位置を変数spへ
        ep=data.indexOf("|",sp+1);          // 次の区切り文字の位置を変数epへ
        event=data.substring(sp,ep);        // sp～epの範囲の文字列を変数eventへ
        memset(disp,0,97);                  // 文字配列変数dispの初期化
        snprintf(disp,7,"%02d:%02d,",hour,min); // dispへ時刻の文字を代入
        event.getBytes((byte*)disp+6,90);   // 時刻の後ろに変数eventの内容を代入
        lcdisp(disp,i);                     // LCDのi+1行目に時刻と予定を表示
    }
    sleep();                                // スリープの実行
}

void sleep(){
    delay(10);                              // 処理完了待ち時間
    esp_deep_sleep(SLEEP_P);                // Deep Sleepモードへ移行
}
