/*******************************************************************************
Example 24: Wi-Fi コンシェルジェ 電源設備担当（ACリレー制御）
Wi-Fiコンシェルジェが AC100V対応のリレー・モジュールを制御します。

                                          Copyright (c) 2016-2019 Wataru KUNINO
*******************************************************************************/

#include <ESP8266WiFi.h>                    // Wi-Fi機能を利用するために必要
extern "C" {
#include "user_interface.h"                 // ESP8266用の拡張IFライブラリ
}
#include <WiFiUdp.h>                        // UDP通信を行うライブラリ
#define PIN_OUT 13                          // IO 13(5番ピン)にリレーを接続する
#define TIMEOUT 20000                       // タイムアウト 20秒
#define SSID "1234ABCD"                     // 無線LANアクセスポイントのSSID
#define PASS "password"                     // パスワード
#define NTP_SERVER "ntp.nict.jp"            // NTPサーバのURL
#define NTP_PORT 8888                       // NTP待ち受けポート

WiFiServer server(80);                      // Wi-Fiサーバ(ポート80=HTTP)定義
int TIMER_ON=-1;                            // ONタイマー無効
int TIMER_OFF=-1;                           // OFFタイマー無効
int TIMER_SLEEP=-1;                         // スリープタイマー無効
unsigned long TIME;                         // 1970年からmillis()＝0までの秒数
                                            // ※現在時刻は TIME+millis()/1000
void setup(){
    pinMode(PIN_OUT,OUTPUT);                // リレーを接続したポートを出力に
    Serial.begin(9600);                     // 動作確認のためのシリアル出力開始
    Serial.println("Example 24 AC Outlet"); // 「Example 24」をシリアル出力表示
    wifi_set_sleep_type(LIGHT_SLEEP_T);     // 省電力モードに設定する
    WiFi.mode(WIFI_STA);                    // 無線LANをSTAモードに設定
    WiFi.begin(SSID,PASS);                  // 無線LANアクセスポイントへ接続
    while(WiFi.status() != WL_CONNECTED){   // 接続に成功するまで待つ
        Serial.print('.');                  // 進捗表示
        digitalWrite(PIN_OUT,!digitalRead(PIN_OUT));    // リレーの点滅
        delay(500);                         // 待ち時間処理
    }
    server.begin();                         // サーバを起動する
    Serial.println("\nStarted");            // 起動したことをシリアル出力表示
    Serial.println(WiFi.localIP());         // 本機のIPアドレスをシリアル出力
    morseIp0(PIN_OUT,100,WiFi.localIP());   // IPアドレス終値をモールス信号出力
    while(TIME==0){
        TIME=getNTP(NTP_SERVER,NTP_PORT);   // NTPを用いて時刻を取得
    }
    TIME-=millis()/1000;                    // 起動後の経過時間を減算
}

void loop(){                                // 繰り返し実行する関数
    WiFiClient client;                      // Wi-Fiクライアントの定義
    char c;                                 // 文字変数を定義
    char s[65];                             // 文字列変数を定義 65バイト64文字
    char date[20];                          // 日付データ格納用
    int len=0;                              // 文字列の長さカウント用の変数
    int t=0;                                // 待ち受け時間のカウント用の変数
    unsigned long time=millis();            // ミリ秒の取得
    int i,minutes;

    if(time<100){
        time=getNTP(NTP_SERVER,NTP_PORT);   // NTPを用いて時刻を取得,timeへ代入
        if(time)TIME=time-millis()/1000;    // 取得成功時に経過時間をTIMEに保持
        else TIME+=4294967;                 // 取得失敗時に経過時間を加算
        while(millis()<100)delay(1);        // 100ms超過待ち
        time=millis();                      // ミリ秒の取得
    }
    time = TIME + time / 1000;              // 時刻で上書き
    time2txt(date,time);                    // 日時をテキストに変換する
    minutes = (int)((time/60)%1440);        // 分に変更(0="0:00" 1439="23:59")
    if(minutes == TIMER_ON ) digitalWrite(PIN_OUT,HIGH);   // リレーON
    if(minutes == TIMER_OFF) digitalWrite(PIN_OUT,LOW);    // リレーOFF
    if(minutes == TIMER_SLEEP){             // スリープタイマー
        digitalWrite(PIN_OUT,LOW);          // リレーOFF
        TIMER_SLEEP=-1;                     // スリープ解除
    }

    client = server.available();            // 接続されたクライアントを生成
    if(client==0) return;                   // 非接続の時にloop()の先頭に戻る
    Serial.println("Connected");            // 接続されたことをシリアル出力表示
    while(client.connected()){              // 当該クライアントの接続状態を確認
        if(client.available()){             // クライアントからのデータを確認
            t=0;                            // 待ち時間変数をリセット
            c=client.read();                // データを文字変数cに代入
            if(c=='\n'){                    // 改行を検出した時
                if(len>5 && strncmp(s,"GET /",5)==0) break;
                len=0;                      // 文字列長を0に
            }else if(c!='\r' && c!='\0'){
                s[len]=c;                   // 文字列変数に文字cを追加
                len++;                      // 変数lenに1を加算
                s[len]='\0';                // 文字列を終端
                if(len>=64) len=63;         // 文字列変数の上限
            }
        }
        t++;                                // 変数tの値を1だけ増加させる
        if(t>TIMEOUT) break; else delay(1); // TIMEOUTに到達したらwhileを抜ける
    }
    if(len>12 && strncmp(s,"GET /?RELAY=",12)==0){
        digitalWrite(PIN_OUT,atoi(&s[12])); // 入力値に応じてリレーを制御する
    }                                       // タイマーの設定を行う
    if(len> 9 && strncmp(s,"GET /?ON=",  9)==0) TIMER_ON =atoi(&s[9]);
    if(len>10 && strncmp(s,"GET /?OFF=",10)==0) TIMER_OFF=atoi(&s[10]);
    if(len>12 && strncmp(s,"GET /?SLEEP=",12)==0){
        i=atoi(&s[12]);                     // 変数iに入力値を代入
        if(i<0) TIMER_SLEEP=-1;             // 入力値が負だった時にスリープ切
        else TIMER_SLEEP=(i+minutes)%1440;  // 現在時刻+入力スリープ時間
    }
    if(client.connected()){                 // 当該クライアントの接続状態を確認
        i=digitalRead(PIN_OUT);             // リレーの状態を読み取り変数iへ代入
        html(client,i,date,TIMER_ON,TIMER_OFF,TIMER_SLEEP,WiFi.localIP());
    }                                       // 負のときは-100を掛けて出力
//  client.stop();                          // クライアントの切断
    Serial.println("Disconnected");         // シリアル出力表示
}
