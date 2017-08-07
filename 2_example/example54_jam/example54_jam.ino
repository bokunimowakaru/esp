/*******************************************************************************
Example 54 (=32+22): IchigoJam をつかった情報表示器

IchigoJam Firmware 1.2以上を推奨
                                           Copyright (c) 2016-2017 Wataru KUNINO
*******************************************************************************/

#include <WiFi.h>                           // ESP32用WiFiライブラリ
#include <WiFiUdp.h>                            // UDP通信を行うライブラリ
#define TIMEOUT 20000                           // タイムアウト 20秒
#define SSID "1234ABCD"                         // 無線LANアクセスポイントのSSID
#define PASS "password"                         // パスワード
#define PORT 1024                               // 受信ポート番号
#define BUF_N 4096                              // バッファサイズ
WiFiUDP udp;                                    // UDP通信用のインスタンスを定義
WiFiServer server(80);                          // Wi-Fiサーバ(ポート80)定義
char tx[BUF_N+1]="\0";                          // 送信バッファ

void setup(){                                   // 起動時に一度だけ実行する関数
    Serial.begin(115200);                       // IchigoJamとの通信ポート
    delay(5000);                                // IchigoJamの起動・通信処理待ち
    WiFi.mode(WIFI_STA);                        // 無線LANをSTAモードに設定
    WiFi.begin(SSID,PASS);                      // 無線LANアクセスポイントへ接続
    Serial.write(25); Serial.write(16);         // 停止コマンドとDLEコードの送信
    Serial.print("cls:?\"Connecting to ");      // 「Connecting」を出力する
    Serial.print(SSID);                         // SSIDを出力する
    Serial.println("\":'");                     // コマンドとして実行する
    while(WiFi.status() != WL_CONNECTED){       // 接続に成功するまで待つ
        delay(500);                             // 待ち時間処理
        Serial.print("'");                      // 無線APへの接続プログレス表示
    }
    server.begin();                             // TCPサーバを起動する
    udp.begin(PORT);                            // UDP通信御開始
    Serial.println();delay(1000);               // 改行を出力
    Serial.print("' ");                         // コメント命令を送信
    Serial.println(WiFi.localIP());             // IPアドレスを出力する
}

void loop(){                                    // 繰り返し実行する関数
    WiFiClient client;                          // Wi-Fiクライアントの定義
    char c;                                     // 文字変数cを定義
    char s[65];                                 // 文字列変数を定義
    char com[65]="\0";                          // IchigoJamへの送信コマンド
    char rx[65]="起動しました";                 // IchigoJamからの応答(表示用)
    int i;
    int len=0;                                  // 文字列長を示す変数を定義
    int rxi=0;                                  // IchigoJamからの応答文字長
    int t=0;                                    // 待ち受け時間カウント用の変数
    int postF=0;                                // POSTフラグ(0:- 1:POST 2:BODY)
    int postL=0;                                // POSTデータ長

    client = server.available();                // TCPクライアントを生成
    if(!client){                            // TCPクライアントが無かった場合
        if(tx[0]){                              // 変数txに代入されていた場合
            c=trUri2c(tx[0]);                   // URIエンコード空白文字の変換
            if(c=='%'){                         // URIエンコード文字の検出
                c=trUri2s(tx);                  // アスキー文字へ変換して変数Cへ
            }
            Serial.write(c); delay(18);         // IchigoJamへ出力
            if(c=='\n') delay(100);             // IchigoJamの処理待ち
            trShift(tx,1);                      // FIFOバッファのシフト処理
        }else{
            len = udp.parsePacket();            // UDP受信パケット長をlenに代入
            if(len){                            // 受信データがあった場合、
                Serial.print("' ");             // コメント命令を送信
                memset(s, 0, 65);               // 文字列変数sの初期化(65バイト)
                udp.read(s, 64);                // UDP受信データを変数sへ代入
                utf_del_uni(s);                 // UTF8の制御コードの除去
                Serial.println(s);              // IchigoJamへ出力する
            }
        }
        return;                                 // loop()の先頭に戻る
    }
    while(client.connected()){                  // クライアントの接続状態を確認
        if(client.available()){                 // クライアントからのデータ確認
            t=0;                                // 待ち時間変数をリセット
            c=client.read();                    // データを文字変数cに代入
            if(c=='\n'){                        // 改行を検出した時
                if(postF==0){                   // ヘッダ処理
                    if(len>10 && strncmp(s,"GET /?COM=",10)==0){
                        strncpy(com,&s[10],64); // 受信文字列をcomへコピー
                        break;                  // 解析処理の終了
                    }else if (len>5 && strncmp(s,"GET /",5)==0){
                        break;                  // 解析処理の終了
                    }else if(len>6 && strncmp(s,"POST /",6)==0){
                        postF=1;                // POSTのBODY待ち状態へ
                    }
                }else if(postF==1){             // POSTのHEAD処理中のとき、
                    if(len>16 && strncmp(s,"Content-Length: ",16)==0){
                        postL=atoi(&s[16]);     // 変数postLにデータ値を代入
                    }
                }
                if( len==0 ) postF++;           // ヘッダの終了
                if( postF>=2) break;            // 解析処理の終了
                len=0;                          // 文字列長を0に
            }else if(c!='\r' && c!='\0'){
                s[len]=c;                       // 文字列変数に文字cを追加
                len++;                          // 変数lenに1を加算
                s[len]='\0';                    // 文字列を終端
                if(len>=64) len=63;             // 文字列変数の上限
            }
        }
        t++;                                    // 変数tの値を1だけ増加させる
        if(t>TIMEOUT) break; else delay(1);     // TIMEOUTしたらwhileを抜ける
    }
    delay(1);                                   // クライアント側の応答待ち時間
    while(Serial.available())Serial.read();     // シリアル受信バッファのクリア
    if(com[0]){                                 // コマンドあり
        trUri2txt(com);                         // URLエンコードの変換処理
        utf_del_uni(com);                       // UTF8の制御コードの除去
        Serial.write(16);                       // DLEコードの送信
        Serial.println(com);                    // 文字データを出力
        delay(200);
        rx[0]='\0';
        while(Serial.available() && rxi<64){
            c=Serial.read();
            if(c!='\r'){
                if(c=='\n'){
                    if(
                        (rxi>=2 && strncmp(&rx[rxi-2],"OK",2)==0) ||
                        (rxi>=5 && strncmp(&rx[rxi-5],"error",5)==0)
                    ) break;
                    if(rxi<=60){
                        strcpy(&rx[rxi],"<BR>");
                        rxi+=4;
                    }
                }else{
                    rx[rxi]=c; rxi++; rx[rxi]='\0';
                }
            }
        }
    }else if(postF>=2 && postL>4){
        strcpy(rx,"HTTP POST データ転送");
        com[4]='\0'; tx[0]='\0';                // 文字列変数の初期化
        for(i=0;i<4;i++) com[i]=client.read();
        if(strncmp(com,"PRG=",4)==0){           // プログラムモード時
            memset(tx, 0, BUF_N+1);             // 文字列変数txの初期化
            i=0; postL-=4; rxi=0;               // PCからの受信準備
            if(postL > BUF_N)postL=BUF_N;       // 送信バッファの上限を設定
            for(i=0;i<postL;i++){
                if(client.available()){
                    tx[i]=client.read();
                }else break;
            }
        }
        Serial.write(16);                       // IchigoJamへDLEコードを送信
    }
    if(client.connected()){                     // クライアントの接続状態を確認
        html(client,"",rx,WiFi.localIP());      // HTMLコンテンツを出力する
    }
    client.flush();                         // ESP32用 ERR_CONNECTION_RESET対策
    client.stop();                              // クライアントの切断
}
