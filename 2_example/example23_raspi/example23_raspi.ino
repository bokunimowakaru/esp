/*******************************************************************************
Example 23: Raspberry Pi を制御する
                                            Copyright (c) 2016 Wataru KUNINO
*******************************************************************************/

#include <SoftwareSerial.h>
#include <ESP8266WiFi.h>                        // Wi-Fi機能を利用するために必要
#define PIN_POW 13                              // IO 13(5番ピン)に電源制御回路
#define TIMEOUT 20000                           // タイムアウト 20秒
#define SSID "1234ABCD"                         // 無線LANアクセスポイントのSSID
#define PASS "password"                         // パスワード
#define PiUSER "pi"                             // Raspberry Pi ユーザ名

SoftwareSerial softSerial(12,14);               // IO12(4)をRX,IO14(3)をTXに設定
WiFiServer server(80);                          // Wi-Fiサーバ(ポート80)定義
int PiPASS=0;                                   // パスワード入力中

void setup(){                                   // 起動時に一度だけ実行する関数
    pinMode(PIN_POW,OUTPUT);                    // 電源制御回路ポートを出力に
    digitalWrite(PIN_POW,LOW);                  // Raspberry PiをLOW(OFF)にする
    Serial.begin(9600);                         // 動作確認のためのシリアル出力
    softSerial.begin(115200);                   // RaspberryPiとの通信ポート
    WiFi.mode(WIFI_STA);                        // 無線LANをSTAモードに設定
    WiFi.begin(SSID,PASS);                      // 無線LANアクセスポイント接続
    Serial.println("Example 23 Raspi");         // 「Example 23」をシリアル出力
    while(WiFi.status() != WL_CONNECTED){       // 接続に成功するまで待つ
        delay(500);                             // 待ち時間処理
        Serial.print('.');                      // 無線APへの接続プログレス表示
    }
    server.begin();                             // TCPサーバを起動する
    Serial.println();                           // 改行を出力する
    Serial.println(WiFi.localIP());             // IPアドレスを出力する
}

void loop(){                                    // 繰り返し実行する関数
    WiFiClient client;                          // Wi-Fiクライアントの定義
    char c;                                     // 文字変数cを定義
    char s[65];                                 // 文字列変数を定義
    char com[65]="\0";                          // RaspberryPiへの送信コマンド
    char rx[1025]="応答なし";                   // RaspberryPiからの応答(表示用)
    int i;
    int len=0;                                  // 文字列長を示す変数を定義
    int rxi=0;                                  // RaspberryPiからの応答文字長
    long t=0;                                   // 待ち受け時間カウント用の変数

    /* serverサーバ処理 */
    client=server.available();                  // TCPクライアントを生成
    if(!client) return;                         // 生成できなかった場合は戻る
    Serial.println("Connected");                // 接続があったことを表示する
    while(softSerial.available()) softSerial.read();    // 受信バッファのクリア
    while(client.available()) client.read();    // 受信バッファのクリア
    while(client.connected()){                  // クライアントの接続状態を確認
        if(client.available()){
            c=client.read();                    // データを文字変数cに代入
            if(c=='\n'){                        // 改行を検出した時
                if(len>12 && strncmp(s,"GET /?START=",12)==0){
                    while(softSerial.available())softSerial.read();
                    softSerial.print('\n');     // 改行コードLFを出力
                    delay(100);                 // プロンプト表示待ち時間
                    if(atoi(&s[12])!=2) break;  // 2以外の時(改行入力のみ)
                    PiPASS=1;                   // パスワード入力中フラグ
                    softSerial.print(PiUSER);   // ユーザ名を入力
                    softSerial.print('\n');     // 改行コードLFを出力
                    break;                      // 解析処理の終了
                }else if(len>11 && strncmp(s,"GET /?POW=1",11)==0){
                    digitalWrite(PIN_POW,HIGH); // Raspberry PiをHIGH(ON)にする
                    delay(100);                 // PIN_POW信号のホールド時間
                    digitalWrite(PIN_POW,LOW);  // PIN_POWをLOWに戻す
                    strcpy(rx,"電源を投入しました");
                    break;                      // 解析処理の終了
                }else if(len>10 && strncmp(s,"GET /?COM=",10)==0){
                    strncpy(com,&s[10],64);     // 受信文字列をcomへコピー
                    break;                      // 解析処理の終了
                }else if (len>5 && strncmp(s,"GET /",5)==0){
                    break;                      // 解析処理の終了
                }
                len=0;                          // 文字列長を0に
            }else if(c!='\r' && c!='\0'){
                s[len]=c;                       // 文字列変数に文字cを追加
                len++;                          // 変数lenに1を加算
                s[len]='\0';                    // 文字列を終端
                if(len>=64) len=63;             // 文字列変数の上限
            }
        }
        t++; delay(1);                          // 変数tの値を1だけ増加させる
        if(t>TIMEOUT) break;                    // TIMEOUTしたらwhileを抜ける
    }
    delay(1);                                   // クライアント側の応答待ち時間
    if(com[0]){                                 // コマンドあり
        trUri2txt(com);                         // URLエンコードの変換処理
        Serial.print("<- ");                    // 送信マークをシリアル表示する
        if(PiPASS){
            Serial.println("********");         // パスワードを伏字表示
            PiPASS=0;
        }else Serial.println(com);              // コマンドをシリアル表示する
        while(softSerial.available())softSerial.read(); // 受信バッファのクリア
        rx[0]='\0';                             // 受信データのクリア
        softSerial.print(com);                  // 文字データを出力
        softSerial.print('\n');                 // 改行コードLFを出力
    }
    while(rxi<1024){                            // シリアル受信ループ(1024文字)
        for(t=0;t<1000000;t++){                 // 約300msの期間、受信確認
            if(softSerial.available()) break;   // 受信があれば確認完了
        }
        if(t==1000000)break;                    // 受信が無ければ受信完了
        c=softSerial.read();                    // シリアルから受信
        if(c=='\n'){                            // 改行時
            if(rxi<=1020){                      // 1020文字以内の時
                strcpy(&rx[rxi],"<BR>");        // HTML用に<BR>に差し替え
                rxi+=4;                         // 文字数4
            }else break;                        // 1021文字以上の時、受信完了
        }else if(isprint(c)){                   // 表示可能な文字の場合、
            rx[rxi]=c; rxi++; rx[rxi]='\0';     // 変数rxへ代入・rxi加算・終端
        }
    }
    Serial.print("-> ");                        // 受信マークをシリアル表示する
    Serial.println(rx);                         // 受信結果を表示する
    if(client.connected()){                     // クライアントの接続状態を確認
        html(client,"",rx,WiFi.localIP());      // HTMLコンテンツを出力する
    }
    client.stop();                              // クライアントの切断
    Serial.println("Done");
}
