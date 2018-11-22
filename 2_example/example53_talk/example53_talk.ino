/*******************************************************************************
Example 21: ESP32 Wi-Fi コンシェルジェ アナウンス担当（音声合成出力）
AquosTalkを使った音声合成でユーザへ気づきを通知することが可能なIoT機器です。

    AquosTalk接続用
    TXD0(35番ピン U0TXD)、AquosTalk側はRXD端子(2番ピン)

                                          Copyright (c) 2016-2019 Wataru KUNINO
*******************************************************************************/

#include <WiFi.h>                           // ESP32用WiFiライブラリ
#include <WiFiUdp.h>                        // UDP通信を行うライブラリ
#define TIMEOUT 20000                       // タイムアウト 20秒
#define SSID "1234ABCD"                     // 無線LANアクセスポイントのSSID
#define PASS "password"                     // パスワード
#define PORT 1024                           // 受信ポート番号

WiFiUDP udp;                                // UDP通信用のインスタンスを定義
WiFiServer server(80);                      // Wi-Fiサーバ(ポート80=HTTP)定義

void setup(){                               // 起動時に一度だけ実行する関数
    Serial.begin(9600);                     // AquesTalkとの通信ポート
    Serial.print("\r$");                    // ブレークコマンドを出力する
    delay(100);                             // 待ち時間処理
    Serial.print("$?kon'nnichi/wa.\r");     // 音声「こんにちわ」を出力する
    WiFi.mode(WIFI_STA);                    // 無線LANをSTAモードに設定
    WiFi.begin(SSID,PASS);                  // 無線LANアクセスポイントへ接続
    while(WiFi.status() != WL_CONNECTED){   // 接続に成功するまで待つ
        delay(500);                         // 待ち時間処理
    }
    server.begin();                         // サーバを起動する
    udp.begin(PORT);                        // UDP通信御開始
    Serial.print("<NUM VAL=");              // 数字読み上げ用タグ出力
    Serial.print(WiFi.localIP());           // IPアドレスを読み上げる
    Serial.print(">.\r");                   // タグの終了を出力する
}

void loop(){                                // 繰り返し実行する関数
    WiFiClient client;                      // Wi-Fiクライアントの定義
    char c;                                 // 文字変数cを定義
    char s[65];                             // 文字列変数を定義 65バイト64文字
    char talk[65]="";                       // 音声出力用の文字列変数を定義
    int len=0;                              // 文字列長を示す整数型変数を定義
    int t=0;                                // 待ち受け時間のカウント用の変数
    int postF=0;                            // POSTフラグ(0:未 1:POST 2:BODY)
    int postL=64;                           // POSTデータ長

    client = server.available();            // 接続されたTCPクライアントを生成
    if(!client){                            // TCPクライアントが無かった場合
        len = udp.parsePacket();            // UDP受信パケット長を変数lenに代入
        if(len==0)return;                   // TCPとUDPが未受信時にloop()先頭へ
        memset(s, 0, 49);                   // 文字列変数sの初期化(49バイト)
        udp.read(s, 48);                    // UDP受信データを文字列変数sへ代入
        Serial.print(s);                    // AquesTalkへ出力する
        Serial.print("\r");                 // 改行コード（CR）を出力する
        return;                             // loop()の先頭に戻る
    }
    while(client.connected()){              // 当該クライアントの接続状態を確認
        if(client.available()){             // クライアントからのデータを確認
            t=0;                            // 待ち時間変数をリセット
            c=client.read();                // データを文字変数cに代入
            if(c=='\n'){                    // 改行を検出した時
                if(postF==0){               // ヘッダ処理
                    if(len>11 && strncmp(s,"GET /?TEXT=",11)==0){
                        strncpy(talk,&s[11],64);     // 受信文字列をtalkへコピー
                        break;              // 解析処理の終了
                    }else if (len>5 && strncmp(s,"GET /",5)==0){
                        strcpy(talk,"de'-ta-o'nyu-ryo_kushiteku'dasai.");
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
                    if(len>5 && strncmp(s,"TEXT=",5)==0){
                        strncpy(talk,&s[5],64);      // 受信文字列をtalkへコピー
                    }
                    break;                  // 解析処理の終了
                }
                postL--;                    // 受信済POSTデータ長の減算
            }
        }
        t++;                                // 変数tの値を1だけ増加させる
        if(t>TIMEOUT) break; else delay(1); // TIMEOUTに到達したらwhileを抜ける
    }
    delay(1);                               // クライアント側の応答待ち時間
    if(talk[0]){                            // 文字列が代入されていた場合、
        trUri2txt(talk);                    // URLエンコードの変換処理
        Serial.print(talk);                 // 受信文字データを音声出力
        Serial.print("\r");                 // 改行コード（CR）を出力する
    }
    if(client.connected()){                 // 当該クライアントの接続状態を確認
        html(client,talk,WiFi.localIP());   // HTMLコンテンツを出力する
    }
    client.flush();                         // ESP32用 ERR_CONNECTION_RESET対策
    client.stop();                          // クライアントの切断
}
