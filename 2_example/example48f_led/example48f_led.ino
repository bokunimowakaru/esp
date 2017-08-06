/*******************************************************************************
Example 48f (=32+16f):  フルカラーLEDの製作

        GPIO 25(10番ピン)に赤色LEDを接続する
        GPIO 26(11番ピン)に緑色LEDを接続する
        GPIO 27(12番ピン)に青色LEDを接続する

                                           Copyright (c) 2016-2017 Wataru KUNINO
*******************************************************************************/

#include <WiFi.h>                           // ESP32用WiFiライブラリ
#define PIN_LED_R 25                        // GPIO 25(10番ピン)にLEDを接続する
#define PIN_LED_G 26                        // GPIO 26(11番ピン)にLEDを接続する
#define PIN_LED_B 27                        // GPIO 27(12番ピン)にLEDを接続する
#define TIMEOUT 20000                       // タイムアウト 20秒
#define SSID "1234ABCD"                     // 無線LANアクセスポイントのSSID
#define PASS "password"                     // パスワード
WiFiServer server(80);                      // Wi-Fiサーバ(ポート80=HTTP)定義
int ledR=0;                                 // 現在のLEDの輝度(0は消灯)
int ledG=0;                                 // 現在のLEDの輝度(0は消灯)
int ledB=0;                                 // 現在のLEDの輝度(0は消灯)
int target=0;                               // LED設定値(0は消灯)

void setup(){                               // 起動時に一度だけ実行する関数
    pinMode(PIN_LED_R,OUTPUT);              // LED_Rを接続したポートを出力に
    pinMode(PIN_LED_G,OUTPUT);              // LED_Gを接続したポートを出力に
    pinMode(PIN_LED_B,OUTPUT);              // LED_Bを接続したポートを出力に
    Serial.begin(115200);                   // 動作確認のためのシリアル出力開始
    Serial.println("ESP32 eg.16f LED HTTP");// 「Example 16」をシリアル出力表示
    WiFi.mode(WIFI_STA);                    // 無線LANをSTAモードに設定
    WiFi.begin(SSID,PASS);                  // 無線LANアクセスポイントへ接続
    morse(PIN_LED_R,50,"HELLO");            // 「HELLO」をモールス信号出力
    while(WiFi.status() != WL_CONNECTED){   // 接続に成功するまで待つ
        Serial.print('.');                  // 進捗表示
        digitalWrite(PIN_LED_G,!digitalRead(PIN_LED_G));    // LEDの点滅
        delay(500);                         // 待ち時間処理
    }
    morseIp0(PIN_LED_R,100,WiFi.localIP()); // IPアドレス終値をモールス信号出力
    ledSetup();                             // LED用セットアップ
    server.begin();                         // サーバを起動する
    Serial.println("\nStarted");            // 起動したことをシリアル出力表示
    Serial.println(WiFi.localIP());         // 本機のIPアドレスをシリアル出力
}

void loop(){                                // 繰り返し実行する関数
    WiFiClient client;                      // Wi-Fiクライアントの定義
    char c;                                 // 文字変数を定義
    char s[65];                             // 文字列変数を定義 65バイト64文字
    int len=0;                              // 文字列の長さカウント用の変数
    int t=0;                                // 待ち受け時間のカウント用の変数
    int postF=0;                            // POSTフラグ(0:未 1:POST 2:BODY)
    int postL=64;                           // POSTデータ長
    int pin=-1;
    
    client = server.available();            // 接続されたクライアントを生成
    if(!client){
        if(target>1 && target<=10){         // 1よりも大きく10以下のとき
            ledR=ledCtrl(PIN_LED_R,ledR,23+random(0,target*100),20);
            ledG=ledCtrl(PIN_LED_G,ledG,23+random(0,target*100),20);
            ledB=ledCtrl(PIN_LED_B,ledB,23+random(0,target*100),20);
        }                                   // LEDの輝度を乱数値23～1023に設定
        return;                             // 非接続の時にloop()の先頭に戻る
    }
    Serial.println("Connected");            // 接続されたことをシリアル出力表示
    while(client.connected()){              // 当該クライアントの接続状態を確認
        if(client.available()){             // クライアントからのデータを確認
            t=0;                            // 待ち時間変数をリセット
            c=client.read();                // データを文字変数cに代入
            if(c=='\n'){                    // 改行を検出した時
                if(postF==0){               // ヘッダ処理
                    if(len>8 && strncmp(s,"GET /?L=",8)==0){
                        target=atoi(&s[8]); // 変数targetにデータ値を代入
                        pin=0;
                        break;              // 解析処理の終了
                    }else if(len>10 && strncmp(s,"GET /?RGB=",10)==0){
                        target=atoi(&s[10]); // 変数targetにデータ値を代入
                        pin=3;
                        break;              // 解析処理の終了
                    }else if(len>8 && strncmp(s,"GET /?R=",8)==0){
                        target=atoi(&s[8]); // 変数targetにデータ値を代入
                        pin=PIN_LED_R;
                        break;              // 解析処理の終了
                    }else if(len>8 && strncmp(s,"GET /?G=",8)==0){
                        target=atoi(&s[8]); // 変数targetにデータ値を代入
                        pin=PIN_LED_G;
                        break;              // 解析処理の終了
                    }else if(len>8 && strncmp(s,"GET /?B=",8)==0){
                        target=atoi(&s[8]); // 変数targetにデータ値を代入
                        pin=PIN_LED_B;
                        break;              // 解析処理の終了
                    }else if (len>5 && strncmp(s,"GET /",5)==0){
                        break;              // 解析処理の終了
                    }else if(len>6 && strncmp(s,"POST /",6)==0){
                        postF=1;            // POSTのBODY待ち状態へ
                    }
                }else if(postF==1){
                    if(len>16 && strncmp(s,"Content-Length: ",16)==0){
                        postL=atoi(&s[16]); // 変数postLにデータ値を代入
                    }
                }
                if( len==0 ) postF++;       // ヘッダの終了
                len=0;                      // 文字列長を0に
            }else if(c!='\r' && c!='\0'){
                s[len]=c;                   // 文字列変数に文字cを追加
                len++;                      // 変数lenに1を加算
                s[len]='\0';                // 文字列を終端
                if(len>=64) len=63;         // 文字列変数の上限
            }
            if(postF>=2){                   // POSTのBODY処理
                if(postL<=0){               // 受信完了時
                    if(len>2 && strncmp(s,"L=",2)==0){
                        target=atoi(&s[2]); // 変数targetに数字を代入
                        pin=0;
                    }else if(len>4 && strncmp(s,"RGB=",4)==0){
                        target=atoi(&s[4]); // 変数targetにデータ値を代入
                        pin=3;
                    }else if(len>2 && strncmp(s,"R=",2)==0){
                        target=atoi(&s[2]); // 変数targetにデータ値を代入
                        pin=PIN_LED_R;
                    }else if(len>2 && strncmp(s,"G=",2)==0){
                        target=atoi(&s[2]); // 変数targetにデータ値を代入
                        pin=PIN_LED_G;
                    }else if(len>2 && strncmp(s,"B=",2)==0){
                        target=atoi(&s[2]); // 変数targetにデータ値を代入
                        pin=PIN_LED_B;
                    }
                    break;                  // 解析処理の終了
                }
                postL--;                    // 受信済POSTデータ長の減算
            }
        }
        t++;                                // 変数tの値を1だけ増加させる
        if(t>TIMEOUT) break; else delay(1); // TIMEOUTに到達したらwhileを抜ける
    }
    if(client.connected()){                 // 当該クライアントの接続状態を確認
        t=0;
        if(target==0) t=0;
        if(target==1) t=1023;
        if(target<=0 && target>=-10) t=-100*target;
        if(pin==0){
            ledR=ledCtrl(PIN_LED_R,ledR,t,1000);
            ledG=ledCtrl(PIN_LED_G,ledG,t,1000);
            ledB=ledCtrl(PIN_LED_B,ledB,t,1000);
        }else if(pin==3 && target>=0 ){
            ledB=ledCtrl(PIN_LED_B,ledB,(target%10)*100,1000);
            target/=10;
            ledG=ledCtrl(PIN_LED_G,ledG,(target%10)*100,1000);
            target/=10;
            ledR=ledCtrl(PIN_LED_R,ledR,(target%10)*100,1000);
            target=999;
        }else{
            switch(pin){
                case PIN_LED_R:
                    ledR=ledCtrl(PIN_LED_R,ledR,t,1000);
                    break;
                case PIN_LED_G:
                    ledG=ledCtrl(PIN_LED_G,ledG,t,1000);
                    break;
                case PIN_LED_B:
                    ledB=ledCtrl(PIN_LED_B,ledB,t,1000);
                    break;
            }
        }
        html(client,target,ledR,ledG,ledB,WiFi.localIP()); // HTMLコンテンツ出力
    }                                       // 負のときは-100を掛けて出力
    client.flush();                         // ESP32用 ERR_CONNECTION_RESET対策
    client.stop();                          // クライアントの切断
    Serial.println("Disconnected");         // シリアル出力表示
}
