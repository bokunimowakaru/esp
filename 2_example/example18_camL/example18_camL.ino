/*******************************************************************************
Example 18: 監視カメラ for SparkFun SEN-11610 (LynkSprite JPEG Color Camera TTL)

                                            Copyright (c) 2016 Wataru KUNINO
*******************************************************************************/

#include <SoftwareSerial.h>
#include <FS.h>
#include <ESP8266WiFi.h>                    // ESP8266用ライブラリ
#include <WiFiUdp.h>                        // UDP通信を行うライブラリ
extern "C" {
#include "user_interface.h"                 // ESP8266用の拡張IFライブラリ
}
#define TIMEOUT 20000                       // タイムアウト 20秒
#define SSID "1234ABCD"                     // 無線LANアクセスポイントのSSID
#define PASS "password"                     // パスワード
#define FILENAME "/cam.jpg"

SoftwareSerial softwareSerial(12,13);       // IO12をRX,IO13をTXに設定
WiFiServer server(80);                      // Wi-Fiサーバ(ポート80=HTTP)定義
int size=0;                                 // 画像データの大きさ(バイト)
int update=60;                              // ブラウザのページ更新間隔(秒)

void setup(){ 
    Serial.begin(9600);                     // 動作確認のためのシリアル出力開始
    Serial.println("Example 18 cam");       // 「Example 18」をシリアル出力表示
    softwareSerial.begin(38400);
    wifi_set_sleep_type(LIGHT_SLEEP_T);     // 省電力モードに設定する
    WiFi.mode(WIFI_STA);                    // 無線LANをSTAモードに設定
    WiFi.begin(SSID,PASS);                  // 無線LANアクセスポイントへ接続
    CamSendResetCmd();
    delay(4000);
    while(WiFi.status() != WL_CONNECTED){   // 接続に成功するまで待つ
        delay(500);                         // 待ち時間処理
    }
    server.begin();                         // サーバを起動する
    Serial.println(WiFi.localIP());         // 本機のIPアドレスをシリアル表示
}

void loop() {
    WiFiClient client;                      // Wi-Fiクライアントの定義
    char c;                                 // 文字変数を定義
    char s[65];                             // 文字列変数を定義 65バイト64文字
    byte data[32];                          // 画像転送用の一時保存変数
    boolean EndFlag=0;
    int len;                                // 文字列等の長さカウント用の変数
    int t=0;                                // 待ち受け時間のカウント用の変数
    int i;
    
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
    if(!client.connected()||len<6) return;  // 切断された場合はloop()の先頭へ
    Serial.println(s);                      // 受信した命令をシリアル出力表示
    if(strncmp(s,"GET / ",6)==0){           // コンテンツ取得命令時
        html(client,size,update,WiFi.localIP()); // コンテンツ表示
        client.stop();                      // クライアントの切断
        return;                             // 処理の終了・loop()の先頭へ
    }
    if(strncmp(s,"GET /cam.jpg",12)==0){    // 画像取得指示の場合
        CamSendTakePhotoCmd();              // カメラを撮影モードに設定
        client.println("HTTP/1.0 200 OK");                  // HTTP OKを応答
        client.println("Content-Type: image/jpeg");         // JPEGコンテンツ
        client.println("Connection: close");                // 応答後に閉じる
        client.println();                                   // ヘッダの終了
        while(softwareSerial.available()>0) softwareSerial.read();
        size=0;                             // 変数sizeを0バイトへ
        while(!EndFlag){                    // 終了フラグが0の時に繰り返す
            len = CamRead(data);            // カメラからデータを取得
            if(len<0){
                EndFlag=1; len *= -1;       // 取得値が負の時に終了
            }
            size += len;                    // データサイズの合算
            for(i=0;i<len;i++) client.write((byte)data[i]); // データ送信
        }
        CamStopTakePhotoCmd();              // 撮影の停止
        CamReadADR0();                      // 読み出しアドレスのリセット
        client.println();                   // 改行を出力
        client.stop();                      // クライアントの切断
        Serial.print(size);                 // 画像サイズをシリアル出力表示
        Serial.println("Bytes");            // シリアル出力表示
        return;                             // 処理の終了・loop()の先頭へ
    }
    if(strncmp(s,"GET /?INT=",10)==0){      // 更新時間の設定命令を受けた時
        update = atoi(&s[10]);              // 受信値を変数updateに代入
    }
    if(strncmp(s,"GET /?BPS=",10)==0){      // ビットレート設定命令時
        i = atoi(&s[10]);                   // 受信値を変数iに代入
        CamBaudRateCmd(i);                  // ビットレート設定
        delay(100);
        softwareSerial.begin(i);            // ビットレート変更
    }
    if(strncmp(s,"GET /?SIZE=",11)==0){     // JPEGサイズ設定命令時
        i = atoi(&s[11]);                   // 受信値を変数iに代入
        CamSizeCmd(i);                      // JPEGサイズ設定
        delay(100);
        CamSendResetCmd();                  // リセットコマンド
        softwareSerial.begin(38400);
    }
    if(strncmp(s,"GET /?POWER=",12)==0){    // パワー設定命令時
        i = atoi(&s[12]);                   // 受信値を変数iに代入
        CamPowerCmd(i);                     // パワー設定コマンド
    }
    if(strncmp(s,"GET /?RESET=",12)==0){    // リセット命令時
        CamSendResetCmd();                  // リセットコマンド
        softwareSerial.begin(38400);
        s[11]='\0';
    }
    for(i=6;i<strlen(s);i++) if(s[i]==' '||s[i]=='+') s[i]='\0';
    htmlMesg(client,&s[6],WiFi.localIP()); 	// メッセージ表示
    client.stop();                          // クライアント切断
    Serial.println("Sent HTML");            // シリアル出力表示
}
