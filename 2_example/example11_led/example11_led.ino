/*******************************************************************************
Example 11: キャンドルLEDの製作
                                            Copyright (c) 2016 Wataru KUNINO
*******************************************************************************/

#include <ESP8266WiFi.h>                    // Wi-Fi機能を利用するために必要
extern "C" {
#include "user_interface.h"                 // ESP8266用の拡張IFライブラリ
}
#define PIN_LED 13                          // IO 13(5番ピン)にLEDを接続する
#define TIMEOUT 20000                       // タイムアウト 20秒
#define SSID "1234ABCD"                     // 無線LANアクセスポイントのSSID
#define PASS "password"                     // パスワード
WiFiServer server(80);                      // Wi-Fiサーバ(ポート80=HTTP)定義
int led=0;                                  // 現在のLEDの輝度(0は消灯)
int target=0;                               // LED設定値(0は消灯)

int ledc(int start,int end,int speed){      // ledのアナログ制御用の関数
    int i;                                  // (startからendへ輝度を推移する)
    if(speed<1)speed=1;
    if(start<=end){
        if(start<1) start=1;
        if(end>1023) end=1023;
        for(i=start;i<end;i<<=1){
            analogWrite(PIN_LED,i);
            delay(100/speed);
        }
    }else{
        if(start>1023) start=1023;
        if(end<0) end=0;
        for(i=start;i>end;i>>=1){
            analogWrite(PIN_LED,i);
            delay(100/speed);
        }
    }
    analogWrite(PIN_LED,end);
    return(end);
}

void setup(){                               // 起動時に一度だけ実行する関数
    pinMode(PIN_LED,OUTPUT);                // LEDを接続したポートを出力に
    Serial.begin(9600);                     // 動作確認のためのシリアル出力開始
    Serial.println("Example 11 LED HTTP");  // 「Example 11」をシリアル出力表示
    wifi_set_sleep_type(LIGHT_SLEEP_T);     // 省電力モードに設定する
    WiFi.mode(WIFI_STA);                    // 無線LANをSTAモードに設定
    WiFi.begin(SSID,PASS);                  // 無線LANアクセスポイントへ接続
    morse(PIN_LED,50,"HELLO");              // 「HELLO」をモールス信号出力
    while(WiFi.status() != WL_CONNECTED){   // 接続に成功するまで待つ
        Serial.print('.');                  // 進捗表示
        digitalWrite(PIN_LED,!digitalRead(PIN_LED));    // LEDの点滅
        delay(500);                         // 待ち時間処理
    }
    server.begin();                         // サーバを起動する
    Serial.println("\nStarted");            // 起動したことをシリアル出力表示
    Serial.println(WiFi.localIP());         // 本機のIPアドレスをシリアル出力
    morseIp0(PIN_LED,50,WiFi.localIP());    // IPアドレス終値をモールス信号出力
}

void loop(){                                // 繰り返し実行する関数
    WiFiClient client;                      // Wi-Fiクライアントの定義
    char c;                                 // 文字変数を定義
    char s[65];                             // 文字列変数を定義 65バイト64文字
    int len=0;                              // 文字列の長さカウント用の変数
    int i=0;                                // 待ち受け時間のカウント用の変数
    int headF=0;                            // HTTPヘッダ用フラグ(0:初期状態)
    
    client = server.available();            // 接続されたクライアントを生成
    if(client==0){
        if(target>1 && target<=10){         // 1よりも大きく10以下のとき
            i=23+random(0,target*100);      // 23～1023までの乱数値を発生
        //  i=1<<random(0,target-1);        // 別の発生方法
            led=ledc(led,i,20);             // LEDの輝度を値iに設定
        }
        return;                             // 非接続の時にloop()の先頭に戻る
    }
    Serial.println("Connected");            // 接続されたことをシリアル出力表示
    while(client.connected()){              // 当該クライアントの接続状態を確認
        if(client.available()){             // クライアントからのデータを確認
            i=0;                            // 待ち時間変数をリセット
            c=client.read();                // データを文字変数cに代入
            switch(c){                      // 文字cに応じて
                case '\0':                      // 文字変数cの内容が空のとき
                case '\r':                      // 文字変数cの内容がCRのとき
                    break;                      // 何もしない
                case '\n':                      // 文字変数cの内容がLFのとき
                    if(strncmp(s,"GET /L",6)==0 && len>6){
                        target=atoi(&s[6]);     // 変数targetに数字を代入
                    }else if( len==0 ) headF=1; // ヘッダの終了
                    len=0;                      // 文字列長を0に
                    break;
                default:                        // その他の場合
                    s[len]=c;                   // 文字列変数に文字cを追加
                    len++;                      // 変数lenに1を加算
                    s[len]='\0';                // 文字列を終端
                    if(len>=64) len=63;         // 文字列変数の上限
                    break;
            }
        }
        i++;                                // 変数iの値を1だけ増加させる
        if(headF) break;                    // HTTPヘッダが終わればwhileを抜ける
        if(i>TIMEOUT) break; else delay(1); // TIMEOUTに到達したらwhileを抜ける
    }
    if(client.connected()){                 // 当該クライアントの接続状態を確認
        client.println("HTTP/1.1 200 OK");  // HTTP OKを応答
        sprintf(s,"LED=%d",target);         // 変数sに「LED=」とtarget値を代入
        client.print("Content-Length: ");   // HTTPヘッダ情報を出力
        client.println(strlen(s)+2);        // コンテンツ長さを出力(改行2バイト)
        client.println();                   // HTTPヘッダの終了を出力
        client.println(s);                  // HTTPコンテンツを出力
        Serial.println(s);                  // シリアルへコンテンツを出力
        if(target==0) led=ledc(led,0,4);    // ゆっくりと消灯
        if(target==1) led=ledc(led,1023,4); // ゆっくりと点灯
        if(target<=0 && target>=-10) led=ledc(led,-100*target,4);   // 輝度変更
    }                                       // 負のときは-100を掛けて出力
    client.stop();                          // クライアントの切断
    Serial.println("Disconnected");         // シリアル出力表示
}
