/*******************************************************************************
Example 51 (=32+19): ESP32 Wi-Fi コンシェルジェ リモコン担当(赤外線リモコン制御)
赤外線リモコンに対応した家電機器を制御します。 

                                          Copyright (c) 2016-2019 Wataru KUNINO
*******************************************************************************/

#include <WiFi.h>                           // ESP32用WiFiライブラリ
#include <WiFiUdp.h>                        // UDP通信を行うライブラリ
#define TIMEOUT 20000                       // タイムアウト 20秒
#define DATA_LEN_MAX 16                     // リモコンコードのデータ長(byte)
#define PIN_IR_IN 4                         // GPIO 4(26番ピン) にIRセンサを接続
#define PIN_LED 2                           // GPIO 2(24番ピン)にLEDを接続
#define SSID "1234ABCD"                     // 無線LANアクセスポイントのSSID
#define PASS "password"                     // パスワード
#define SENDTO "192.168.0.255"              // 送信先のIPアドレス
#define PORT 1024                           // 送信のポート番号
#define DEVICE "ir_rc_1,"                   // デバイス名(5文字+"_"+番号+",")
#define AEHA        0                       // 赤外線送信方式(Panasonic、Sharp)
#define NEC         1                       // 赤外線送信方式 NEC方式
#define SIRC        2                       // 赤外線送信方式 SONY SIRC方式

WiFiUDP udp;                                // UDP通信用のインスタンスを定義
WiFiServer server(80);                      // Wi-Fiサーバ(ポート80=HTTP)定義
byte D[DATA_LEN_MAX];                       // 保存用・リモコン信号データ
int D_LEN;                                  // 保存用・リモコン信号長（bit）
int IR_TYPE=AEHA;                           // リモコン方式

void setup(){                               // 起動時に一度だけ実行する関数
    pinMode(PIN_IR_IN, INPUT);              // IRセンサの入力ポートの設定
    pinMode(PIN_LED,OUTPUT);                // LEDを接続したポートを出力に
    ir_send_init();                         // IR出力用LEDの設定(IO 14ポート)
    Serial.begin(115200);                   // 動作確認のためのシリアル出力開始
    Serial.println("ESP32 eg.19 ir_rc");    // 「Example 19」をシリアル出力表示
    WiFi.mode(WIFI_STA);                    // 無線LANをSTAモードに設定
    WiFi.begin(SSID,PASS);                  // 無線LANアクセスポイントへ接続
    while(WiFi.status() != WL_CONNECTED){   // 接続に成功するまで待つ
        delay(500);                         // 待ち時間処理
        digitalWrite(PIN_LED,!digitalRead(PIN_LED));    // LEDの点滅
    }
    server.begin();                         // サーバを起動する
    udp.begin(PORT);                        // UDP通信御開始
    Serial.println(WiFi.localIP());         // 本機のIPアドレスをシリアル表示
    morseIp0(PIN_LED,100,WiFi.localIP());   // IPアドレス終値をモールス信号出力
}

void loop(){
    WiFiClient client;                      // Wi-Fiクライアントの定義
    byte d[DATA_LEN_MAX];                   // リモコン信号データ
    int d_len;                              // リモコン信号長（bit）
    char c;                                 // 文字変数を定義
    char s[97];                             // 文字列変数を定義 97バイト96文字
    int len=0;                              // 文字列等の長さカウント用の変数
    int t=0;                                // 待ち受け時間のカウント用の変数
    int postF=0;                            // POSTフラグ(0:未 1:POST 2:BODY)
    int postL=96;                           // POSTデータ長

    /* 赤外線受信・UDP送信処理 */
    digitalWrite(PIN_LED,LOW);              // LEDを消灯状態に
    d_len=ir_read(d,DATA_LEN_MAX,255);      // 赤外線信号を読み取る
    if(d_len>=16){                          // 16ビット以上の時に以下を実行
        digitalWrite(PIN_LED,HIGH);         // LEDを点灯状態に
        udp.beginPacket(SENDTO, PORT);      // UDP送信先を設定
        udp.print(DEVICE);                  // デバイス名を送信
        udp.print(d_len);                   // 信号長を送信
        udp.print(",");                     // カンマ「,」を送信
        ir_data2txt(s,96,d,d_len);          // 受信データをテキスト文字に変換
        udp.println(s);                     // それを文字をUDP送信
        Serial.println(s);
        udp.endPacket();                    // UDP送信の終了(実際に送信する)
        memcpy(D,d,DATA_LEN_MAX);           // データ変数dを変数Dにコピーする
        D_LEN=d_len;                        // データ長d_lenをD_LENにコピーする
    }
    /* TCPサーバ・UDP受信処理 */
    client = server.available();            // 接続されたクライアントを生成
    if(!client){                            // TCPクライアントが無かった場合
        d_len=udp.parsePacket();            // UDP受信長を変数d_lenに代入
        if(d_len==0)return;                 // TCPとUDPが未受信時にloop()先頭へ
        memset(s, 0, 97);                   // 文字列変数sの初期化(97バイト)
        udp.read(s, 96);                    // UDP受信データを文字列変数sへ代入
        if(
            len>6 && (                      // データ長が6バイトより大きくて、
                strncmp(s,"ir_rc_",6)==0 || // 受信データが「ir_rc_」
                strncmp(s,"ir_in_",6)==0    // または「ir_in_」で始まる時
            )
        ){
            D_LEN=ir_txt2data(D,DATA_LEN_MAX,&s[6]);
        }                                   // 受信TXTをデータ列に変換
        return;                             // 非接続の時にloop()の先頭に戻る
    }
    Serial.println("Connected");            // 接続されたことをシリアル出力表示
    len=0;
    while(client.connected()){              // 当該クライアントの接続状態を確認
        if(client.available()){             // クライアントからのデータを確認
            t=0;                            // 待ち時間変数をリセット
            c=client.read();                // データを文字変数cに代入
            if(c=='\n'){                    // 改行を検出した時
                if(postF==0){               // ヘッダ処理
                    if (len>11 && strncmp(s,"GET /?TYPE=",11)==0){
                        IR_TYPE=atoi(&s[11]);
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
                if(len>=96) len=95;         // 文字列変数の上限
            }
            if(postF>=2){                   // POSTのBODY処理
                if(postL<=0){               // 受信完了時
                    if(len>3 && strncmp(s,"IR=",3)==0){
                        trUri2txt(&s[3]);
                        D_LEN=ir_txt2data(D,DATA_LEN_MAX,&s[3]);
                        ir_send(D,D_LEN,IR_TYPE);
                        break;              // 受信TXTをデータ列に変換
                    }
                }                           // BODYが「IR=」の場合に解析を終了
                postL--;                    // 受信済POSTデータ長の減算
            }
        }
        t++;                                // 変数tの値を1だけ増加させる
        if(t>TIMEOUT) break; else delay(1); // TIMEOUTに到達したらwhileを抜ける
    }
    delay(1);                               // クライアント側の応答待ち時間
    if(client.connected()){                 // 当該クライアントの接続状態を確認
        if(D_LEN==0)strcpy(s,"データ未受信");
        ir_data2txt(s,96,D,D_LEN);
        html(client,s,D_LEN,IR_TYPE,WiFi.localIP());    // HTMLコンテンツを出力
    }                                       // 負のときは-100を掛けて出力
    client.flush();                         // ESP32用 ERR_CONNECTION_RESET対策
    client.stop();                          // クライアントの切断
    Serial.println("Disconnected");         // シリアル出力表示
}
