/*******************************************************************************
Example 63： ESP32 Google カレンダー(予定表) から予定を取得する IoT情報端末

クラウド（Google Apps Script）で実行したスクリプトを本機で受け取ります。
予め本フォルダ内のGoogleCalendar.jsを https://script.google.com/ へ転送し、
ウェブアプリケーションを公開しておく必要があります。

                                      Copyright (c) 2016-2019 Wataru KUNINO
********************************************************************************

必要なハードウェア
    ・トランジスタ技術 IoT Express (CQ出版社)
    ・LCD Keypad Shield (DF Robot製、SainSmart製、D1 ROBOT製など)
    ・ACアダプタ（マイクロUSB、5V 500mA）
    
ボード設定：
    ・CQ出版社 IoT Express用です（初期値）
        - AE-FT234Xの改造もしくは、コンデンサの容量を増量してください。
            https://blogs.yahoo.co.jp/bokunimowakaru/55924799.html
            http://toragi.cqpub.co.jp/tabid/848/Default.aspx#1
    ・ESPduino 32 や WEMOS D1 32で使用する場合、#define CQ_PUB_IOT_EXPRESSを削除
    
参考ソースコード：
        https://github.com/wilda17/ESP8266-Google-Calendar-Arduino
********************************************************************************/

#include <WiFi.h>
#include <LiquidCrystal.h>                  // LCDへの表示を行うライブラリ
#include "esp_sleep.h"                      // ESP32用Deep Sleep ライブラリ
#include "HTTPSRedirect.h"                  // リダイレクト接続用ライブラリ

#define CQ_PUB_IOT_EXPRESS                  // CQ出版 IoT Express 用
#define SSID "1234ABCD"                     // 無線LANアクセスポイントのSSID
#define PASS "password"                     // パスワード
#define GScriptId "★ここにGoogle Apps Scriptのトークンを記入してください★"

/* GScriptId の設定方法 ********************************************************

Google Apps Script(https://script.google.com/)へアクセスし、GoogleCalendar.js
を転送し、[ファイル]メニューで[保存]を実行し、[公開]メニューから[ウェブアプリ
ケーションとして導入]を選択てください。
[次のユーザとしてアプリケーションを実行]の欄で、自分のアカウントを選択し、
[アプリケーションにアクセスできるユーザー]で[全員（匿名ユーザーを含む）]を
選択してから[導入]をクリックしてください。
下記のような公開URLが表示されます。

　　https://script.google.com/macros/s/???????????????????????????????/exec

???の部分を #define GScriptId の""内へコピーしてください。

※注意事項：???の部分が他人に知られると、アカウント内の予定表が閲覧されます。
*******************************************************************************/

#ifdef CQ_PUB_IOT_EXPRESS
    #define PIN_BUZZER 12                   // GPIO 12にスピーカを接続
#else
    #define PIN_BUZZER 18                   // GPIO 18にスピーカを接続
#endif
#define PIN_LED 2                           // GPIO 2(24番ピン)にLEDを接続
#define TIMEOUT 6000                        // タイムアウト 6秒
#define SLEEP_P 50*1000000ul                // スリープ時間 50秒(uint32_t)
#define HTTPTO "script.google.com"          // HTTPSアクセス先
#define HTRED "script.googleusercontent.com"// HTTPSリダイレクト先
#define PORT 443                            // HTTPSポート番号
#ifdef CQ_PUB_IOT_EXPRESS 
    LiquidCrystal lcd(17,26,13,14,15,16);   // CQ出版 IoT Express 用 LCD開始
#else
    LiquidCrystal lcd(12,13,17,16,27,14);   // ESPduino 32 WEMOS D1 32用 LCD開始
#endif
String url = String("/macros/s/") + String(GScriptId) + "/exec";

void setup() {
    pinMode(PIN_LED,OUTPUT);                // LEDを接続したポートを出力に
    pinMode(PIN_BUZZER,OUTPUT);             // ブザーを接続したポートを出力に
    Serial.begin(115200);                   // 動作確認のためのシリアル出力開始
    lcd.begin(16, 2);                       // 液晶の初期化(16桁×2行)
    lcd.print(utf_del_uni("Google ｶﾚﾝﾀﾞ LCD")); // タイトルをLCDに表示する
    Serial.println("Example 63 Google");
    unsigned long start_ms=millis();        // 初期化開始時のタイマー値を保存
    unsigned long wait_ms=20000;            // 起動待ち時間(ms)
    lcd.setCursor(0,1);                     // カーソル位置を液晶の左下へ
    WiFi.mode(WIFI_STA);                    // 無線LANを【STA】モードに設定
    WiFi.begin(SSID,PASS);                  // 無線LANアクセスポイントへ接続
    chimeBellsSetup(PIN_BUZZER);            // ブザー/LED用するPWM制御部の初期化
    lcd.setCursor(0,1);                     // カーソル位置を液晶の左下へ
    while(WiFi.status() != WL_CONNECTED){   // 接続に成功するまで待つ
        if(millis() - start_ms < wait_ms){  // 待ち時間後の処理
            lcd.print(".");                 // 接続進捗を表示
            digitalWrite(PIN_LED,!digitalRead(PIN_LED));        // LEDの点滅
            ledcWriteNote(0,NOTE_B,7);delay(50);ledcWrite(0,0); // ブザー音
            delay(450);                     // 待ち時間処理
        }else{
            lcd.clear(); lcd.print("To "); lcd.print(SSID); lcd.setCursor(0,1);
            WiFi.disconnect(); delay(3000); // WiFiアクセスポイントを切断する
            WiFi.begin(SSID,PASS);          // 無線LANアクセスポイントへ再接続
            wait_ms += 20000;               // 待ち時間に20秒追加
        }
    }
    Serial.println(WiFi.localIP());         // IPアドレスをシリアル出力表示
}

void loop() {
    String data;                            // 受信データ
    String event;                           // イベント名
    char disp[17];                          // 液晶表示データ
    int sp;                                 // 文字列位置用の変数
    int events_n;                           // イベント数
    int hour,minute;                        // 時刻
    int i;
    
    HTTPSRedirect client(PORT);             // リダイレクト可能なHTTP接続client
    if(!client.connect(HTTPTO,PORT))sleep();// HTTP接続の実行(失敗時はスリープ)
    data=client.getData(url,HTTPTO,HTRED);  // データ受信
    Serial.println(data);                   // 受信データをシリアルへ出力
    
    sp=data.indexOf("|Length,");
    if(sp>=0) sp+=8; else sleep();
    events_n=data.substring(sp).toInt();
    lcd.clear();
    for(i=0;i<events_n;i++){
        sp=data.indexOf("|",sp+1)+1;
        hour=data.substring(sp).toInt();
        sp=data.indexOf(",",sp+1)+1;
        minute=data.substring(sp).toInt();
        sp=data.indexOf(",",sp+1)+1;
        event = data.substring(sp,data.indexOf("|",sp+1));
        snprintf(disp,17,"%02d:%02d,%s",hour,minute,utf_del_uni(event));
        Serial.println(disp);
        if(i<=1){
            lcd.setCursor(0,i);             // カーソル位置を液晶の左下へ
            lcd.print(disp);                // アクセス先を液晶の2行目に表示
        }
    }
    sleep();                                // スリープの実行
}

void sleep(){
    delay(200);                             // 処理完了待ち時間
    esp_deep_sleep(SLEEP_P);                // Deep Sleepモードへ移行
}
