/*******************************************************************************
Example 25: センサ受信データ・ファイルシステム
                                           Copyright (c) 2016-2019 Wataru KUNINO
*******************************************************************************/
#include <FS.h>
#include <ESP8266WiFi.h>                    // ESP8266用ライブラリ
extern "C" {
#include "user_interface.h"                 // ESP8266用の拡張IFライブラリ
}
#include <WiFiUdp.h>                        // UDP通信を行うライブラリ
#define TIMEOUT 20000                       // タイムアウト 20秒
#define SSID "1234ABCD"                     // 無線LANアクセスポイントのSSID
#define PASS "password"                     // パスワード
#define PORT 1024                           // 送信のポート番号
#define NTP_SERVER "ntp.nict.jp"            // NTPサーバのURL
#define NTP_PORT 8888                       // NTP待ち受けポート

File file;
WiFiUDP udp;                                // UDP通信用のインスタンスを定義
WiFiServer server(80);                      // Wi-Fiサーバ(ポート80=HTTP)定義
int LCD_EN;                                 // LCDに何らかのメッセージを表示中
unsigned long TIME;                         // 1970年からmillis()＝0までの秒数
                                            // ※現在時刻は TIME+millis()/1000
void setup(){
    while(!SPIFFS.begin())delay(1000);      // ファイルシステムの開始
    lcdSetup(8,2);                          // 8桁×2行のI2C液晶の準備
//  wifi_set_sleep_type(LIGHT_SLEEP_T);     // 省電力モードに設定する
//  ↑設定解除中 ∵SPIFFSとの組み合わせ動作不安定 esp8266 Ver.2.4.2 + LwIP v1.4
    WiFi.mode(WIFI_STA);                    // 無線LANをSTAモードに設定
    WiFi.begin(SSID,PASS);                  // 無線LANアクセスポイントへ接続
    while(WiFi.status() != WL_CONNECTED){   // 接続に成功するまで待つ
        delay(500);                         // 待ち時間処理
    }
    server.begin();                         // サーバを起動する
    udp.begin(PORT);                        // UDP通信御開始
    lcdPrintIp(WiFi.localIP());             // 本機のIPアドレスをLCDに表示
    while(TIME==0){
        TIME=getNTP(NTP_SERVER,NTP_PORT);   // NTPを用いて時刻を取得
    }
    TIME-=millis()/1000;                    // 起動後の経過時間を減算
}

void loop() {
    WiFiClient client;                      // Wi-Fiクライアントの定義
    char c;                                 // 文字変数を定義
    char s[65];                             // 文字列変数を定義 65バイト64文字
    char filename[13];                      // ファイル名格納用
    char date[20];                          // 日付データ格納用
    int len=0,i;                            // 文字列等の長さカウント用の変数
    int t=0;                                // 待ち受け時間のカウント用の変数
    unsigned long time=millis();            // ミリ秒の取得

    if(time<100){
        time=getNTP(NTP_SERVER,NTP_PORT);   // NTPを用いて時刻を取得,timeへ代入
        if(time)TIME=time-millis()/1000;    // 取得成功時に経過時間をTIMEに保持
        else TIME+=4294967;                 // 取得失敗時に経過時間を加算
        while(millis()<100)delay(1);        // 100ms超過待ち
        time=millis();                      // ミリ秒の取得
    }
    if((time/20)%50==0){                    // 1秒間隔で以下を実行
        if(LCD_EN){                                         // LCD_ENが0以外の時
            LCD_EN--;                                       // LCD_ENから1を減算
        }else{                                              // LCD_ENが0の時
            if((time/5000)%2) lcdPrintIp(WiFi.localIP());   // IPアドレスを表示
            else lcdPrintTime(TIME+time/1000);              // または時刻を表示
        }
    }
    client = server.available();            // 接続されたクライアントを生成
    if(client==0){                          // TCPクライアントが無かった場合
        delay(11);
        len = udp.parsePacket();            // UDP受信パケット長を変数lenに代入
        if(len==0)return;                   // TCPとUDPが未受信時にloop()先頭へ
        memset(s, 0, 65);                   // 文字列変数sの初期化(65バイト)
        udp.read(s, 64);                    // UDP受信データを文字列変数sへ代入
        if(s[7]!=',')return;                // 8番目の文字が「,」で無ければ戻る
        for(i=0;i<7;i++) if(!isalnum(s[i])) s[i]='_';
        s[7]='\0';                          // 8番目の文字を文字列の終端に設定
        sprintf(filename,"/%s.txt",s);
        file = SPIFFS.open(filename,"a");   // 追記保存のためにファイルを開く
        if(file==0)return;                  // ファイルを開けれなければ戻る
        for(i=8;i<64;i++) if(s[i]=='\r'||s[i]=='\n') s[i]='\0';
        lcdPrint(s);                        // 液晶に表示する
        time2txt(date,TIME+time/1000);      // 日時をテキストに変換する
        file.print(date);                   // 日時を出力する
        file.print(',');                    // 「,」カンマをファイル出力
        file.println(&s[8]);                // 受信データをファイル出力
        file.close();                       // ファイルを閉じる
        return;                             // loop()の先頭に戻る
    }
    lcdPrint("Connect");                    // 接続されたことを表示
    LCD_EN=10;                              // 表示期間を10秒に設定
    len=0;
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
    if(strncmp(s,"GET / ",6)==0){           // コンテンツ要求があった時
        lcdPrint2(&s[6]);                   // ファイル名またはコマンドを表示
        html(client,WiFi.localIP());        // HTMLコンテンツを表示
    //  client.stop();                      // クライアントの切断
        return;
    }
    if(strncmp(s,"GET /?FORMAT",12)==0){    // ファイルシステム初期化コマンド
        lcdPrint2("FORMAT");                // フォーマットの開始表示
        SPIFFS.format();                    // ファイル全消去
        s[5]='\0';                          // 取得ファイル名なし
    }
    if(strncmp(s,"GET /",5)==0){            // コンテンツ要求時
        for(i=5;i<strlen(s);i++){                       // 文字列を検索
            if(s[i]==' '||s[i]=='&'||s[i]=='+'){        // 区切り文字のとき
                s[i]='\0';                              // 文字列を終端する
            }
        }
        lcdPrint2(&s[5]);                               // ファイル名を表示
        client.println("HTTP/1.1 200 OK");              // HTTP OKを応答
        client.println("Content-Type: text/plain");     // テキスト・コンテンツ
        client.println("Connection: close");            // 応答終了後に切断
        client.println();
        file = SPIFFS.open(&s[4],"r");      // データを読取ファイルを開く
        i=0;                                // 変数iに0を代入
        if(file==0){                        // ファイルが開けなかった時
            client.println("no data");      // ファイル無し表示
        }else{                              // ファイルが開けた時
            while(file.available()){        // データが残っているとき
                client.write(file.read());  // ファイルを読み取って転送
                i++;                        // 変数iに1を加算
            }
        }
        file.close();                       // ファイルを閉じる
    //  client.stop();                      // クライアントの切断
        lcdPrintVal("Bytes",i);             // ファイルサイズを表示
        return;                             // 処理の終了・loop()の先頭へ
    }
}
