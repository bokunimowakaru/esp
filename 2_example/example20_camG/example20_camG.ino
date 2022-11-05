/*******************************************************************************
Example 20: Wi-Fi コンシェルジェ カメラ担当
 for SeeedStudio Grove Serial Camera Kit 
Webサーバ機能を使って、カメラのシャッターを制御し、撮影した写真を表示します。

                                          Copyright (c) 2016-2019 Wataru KUNINO
*******************************************************************************/
/*
ご注意
    ★★★ 写真データの転送に失敗するときは ★★★
    Arduino IDEの[ツール]メニューの[CPU Frequency]で[160MHz]を設定してください。
    （80MHzで正しく動作する場合は変更しないでください。）
    
    その他、上手く動作しないときは、本フォルダ内のREADME.mdをご覧ください。
*/

#include <SoftwareSerial.h>
#include <FS.h>
#include <ESP8266WiFi.h>                    // ESP8266用ライブラリ
extern "C" {
#include "user_interface.h"                 // ESP8266用の拡張IFライブラリ
}
#define PIN_CAM 13                          // IO 13(5番ピン)にPch-FETを接続する
#define TIMEOUT 20000                       // タイムアウト 20秒
#define SSID "1234ABCD"                     // 無線LANアクセスポイントのSSID
#define PASS "password"                     // パスワード

SoftwareSerial softwareSerial(12,14);       // IO12(4)をRX,IO14(3)をTXに設定
WiFiServer server(80);                      // Wi-Fiサーバ(ポート80=HTTP)定義
int size=0;                                 // 画像データの大きさ(バイト)
int update=60;                              // ブラウザのページ更新間隔(秒)

void setup(){ 
    lcdSetup(8,2);                          // 液晶の初期化(8桁×2行)
    pinMode(PIN_CAM,OUTPUT);                // FETを接続したポートを出力に
    digitalWrite(PIN_CAM,0);                // FETをLOW(ON)にする
    Serial.begin(9600);                     // 動作確認のためのシリアル出力開始
    Serial.println("Example 20 cam");       // 「Example 20」をシリアル出力表示
//  wifi_set_sleep_type(LIGHT_SLEEP_T);     // 省電力モードに設定する
//  ↑設定解除中 ∵SPIFFSとの組み合わせ動作不安定 esp8266 Ver.2.4.2 + LwIP v1.4
    WiFi.mode(WIFI_STA);                    // 無線LANをSTAモードに設定
    WiFi.begin(SSID,PASS);                  // 無線LANアクセスポイントへ接続
    delay(100);                             // カメラの起動待ち
    softwareSerial.begin(115200);           // カメラとのシリアル通信を開始する
    CamInitialize();                        // カメラの初期化コマンド
    CamSizeCmd(1);                          // 撮影サイズをQVGAに設定
    delay(4000);                            // 完了待ち(開始直後の撮影防止対策)
    while(WiFi.status() != WL_CONNECTED){   // 接続に成功するまで待つ
        delay(500);                         // 待ち時間処理
    }
    server.begin();                         // サーバを起動する
    Serial.println(WiFi.localIP());         // 本機のIPアドレスをシリアル表示
    lcdPrintIp(WiFi.localIP());             // 本機のIPアドレスを液晶に表示
}

void loop(){
    WiFiClient client;                      // Wi-Fiクライアントの定義
    char c;                                 // 文字変数を定義
    char s[65];                             // 文字列変数を定義 65バイト64文字
    int len=0;                              // 文字列等の長さカウント用の変数
    int t=0;                                // 待ち受け時間のカウント用の変数
    int i,j;
    
    client = server.available();            // 接続されたクライアントを生成
    if(client==0)return;                    // loop()の先頭に戻る
    Serial.println("Connected");            // シリアル出力表示
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
    delay(1);                               // クライアント側の応答待ち時間
    if(!client.connected()||len<6) return;  // 切断された場合はloop()の先頭へ
    Serial.println(s);                      // 受信した命令をシリアル出力表示
    lcdPrint(&s[5]);                        // 受信した命令を液晶に表示
    if(strncmp(s,"GET / ",6)==0){           // コンテンツ取得命令時
        html(client,size,update,WiFi.localIP()); // コンテンツ表示
    //  client.stop();                      // クライアントの切断
        return;                             // 処理の終了・loop()の先頭へ
    }
    if(strncmp(s,"GET /cam.jpg",12)==0){    // 画像取得指示の場合
        size=CamCapture();                  // カメラで写真を撮影する
        client.println("HTTP/1.0 200 OK");                  // HTTP OKを応答
        client.println("Content-Type: image/jpeg");         // JPEGコンテンツ
        client.println("Content-Length: " + String(size));  // ファイルサイズ
        client.println("Connection: close");                // 応答後に閉じる
        client.println();                                   // ヘッダの終了
        size=CamGetData(client);
    //  client.stop();                      // クライアントの切断
        Serial.print(size);                 // ファイルサイズをシリアル出力表示
        Serial.println(" Bytes");           // シリアル出力表示
        lcdPrintVal("TX Bytes",size);       // ファイルサイズを液晶へ表示
        return;                             // 処理の終了・loop()の先頭へ
    }
    if(strncmp(s,"GET /?INT=",10)==0){      // 更新時間の設定命令を受けた時
        update = atoi(&s[10]);              // 受信値を変数updateに代入
    }
    if(strncmp(s,"GET /?SIZE=",11)==0){     // JPEGサイズ設定命令時
        i = atoi(&s[11]);                   // 受信値を変数iに代入
        CamSizeCmd(i);                      // JPEGサイズ設定
    }
    if(!strncmp(s,"GET /favicon.ico",16)){  // Google Chrome対応(追加)
        client.println("HTTP/1.0 404 Not Found");
        client.println();                   // ヘッダの終了
    //  client.stop();                      // クライアントの切断
        return;                             // 処理の終了・loop()の先頭へ
    }
    
    for(i=6;i<strlen(s);i++) if(s[i]==' '||s[i]=='+') s[i]='\0';
    htmlMesg(client,&s[6],WiFi.localIP());  // メッセージ表示
//  client.stop();                          // クライアント切断
    Serial.println("Sent HTML");            // シリアル出力表示
}
