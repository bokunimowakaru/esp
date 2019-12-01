/*******************************************************************************
Example 52 (=32+20): ESP32 Wi-Fi コンシェルジェ カメラ担当
 for SeeedStudio Grove Serial Camera Kit 
Webサーバ機能を使って、カメラのシャッターを制御し、撮影した写真を表示します。

    カメラ接続用
    GPIO16(27番ピン) U2RXD カメラ側はTXD端子(黄色)
    GPIO17(28番ピン) U2TXD カメラ側はRXD端子(白色)

    GPIO 2(24番ピン)にPch-FETを接続

                                          Copyright (c) 2016-2019 Wataru KUNINO
*******************************************************************************/

#include <WiFi.h>                           // ESP32用WiFiライブラリ
#include <WiFiUdp.h>                        // UDP通信を行うライブラリ
#define PIN_CAM 2                           // GPIO 2(24番ピン)にPch-FETを接続
#define TIMEOUT 20000                       // タイムアウト 20秒
#define SSID "1234ABCD"                     // 無線LANアクセスポイントのSSID
#define PASS "password"                     // パスワード

HardwareSerial hardwareSerial2(2);          // カメラ接続用シリアルポートESP32用
WiFiServer server(80);                      // Wi-Fiサーバ(ポート80=HTTP)定義
int size=0;                                 // 画像データの大きさ(バイト)
int update=60;                              // ブラウザのページ更新間隔(秒)

void setup(){ 
    lcdSetup(8,2);                          // 液晶の初期化(8桁×2行)
    pinMode(PIN_CAM,OUTPUT);                // FETを接続したポートを出力に
    digitalWrite(PIN_CAM,0);                // FETをLOW(ON)にする
    Serial.begin(115200);                   // 動作確認のためのシリアル出力開始
    Serial.println("Example 20 cam");       // 「Example 20」をシリアル出力表示
    WiFi.mode(WIFI_STA);                    // 無線LANをSTAモードに設定
    WiFi.begin(SSID,PASS);                  // 無線LANアクセスポイントへ接続
    delay(100);                             // カメラの起動待ち
    hardwareSerial2.begin(115200);          // カメラとのシリアル通信を開始する
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
    if(!client)return;                      // loop()の先頭に戻る
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
        client.flush();                     // ESP32用 ERR_CONNECTION_RESET対策
        client.stop();                      // クライアントの切断
        return;                             // 処理の終了・loop()の先頭へ
    }
    if(strncmp(s,"GET /cam.jpg",12)==0){    // 画像取得指示の場合
        CamCapture();                       // カメラで写真を撮影する
        size=CamGetData(client);
        client.println("HTTP/1.0 200 OK");                  // HTTP OKを応答
        client.println("Content-Type: image/jpeg");         // JPEGコンテンツ
        client.println("Content-Length: " + String(size));  // ファイルサイズ
        client.println("Connection: close");                // 応答後に閉じる
        client.println();                                   // ヘッダの終了
        client.println();                   // コンテンツの終了
        client.flush();                     // ESP32用 ERR_CONNECTION_RESET対策
        client.stop();                      // クライアントの切断
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
        client.flush();                     // ESP32用 ERR_CONNECTION_RESET対策
        client.stop();                      // クライアントの切断
        return;                             // 処理の終了・loop()の先頭へ
    }
    
    for(i=6;i<strlen(s);i++) if(s[i]==' '||s[i]=='+') s[i]='\0';
    htmlMesg(client,&s[6],WiFi.localIP());  // メッセージ表示
    client.flush();                         // ESP32用 ERR_CONNECTION_RESET対策
    client.stop();                          // クライアント切断
    Serial.println("Sent HTML");            // シリアル出力表示
}
