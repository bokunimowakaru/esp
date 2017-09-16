/*******************************************************************************
Example 28: LCDへ表示する(HTTP版・時刻表示機能・NTP受信機能付き)
                                           Copyright (c) 2016-2017 Wataru KUNINO
*******************************************************************************/

#include <ESP8266WiFi.h>                    // Wi-Fi機能を利用するために必要
extern "C" {
#include "user_interface.h"                 // ESP8266用の拡張IFライブラリ
}
#include <WiFiUdp.h>                        // UDP通信を行うライブラリ
#include "LiquidCrystalDFR.h"

#define TIMEOUT 3000                        // タイムアウト 3秒
#define SSID "1234ABCD"                     // 無線LANアクセスポイントのSSID
#define PASS "password"                     // パスワード
#define PORT 1024                           // 受信ポート番号
#define NTP_SERVER "ntp.nict.jp"            // NTPサーバのURL
#define NTP_PORT 8888                       // NTP待ち受けポート
#define NTP_PACKET_SIZE 48                  // NTP時刻長48バイト
#define HIST_MAX 16                         // 過去データ保持件数(1以上)

byte packetBuffer[NTP_PACKET_SIZE];         // NTP送受信用バッファ
WiFiUDP udp;                                // NTP通信用のインスタンスを定義
WiFiUDP udpRx;                              // UDP通信用のインスタンスを定義
WiFiServer server(80);                      // Wi-Fiサーバ(ポート80=HTTP)定義
LiquidCrystal lcd(14,15,4,5,12,13);         // ESPduino用 LCD開始
// LiquidCrystal lcd(0,2,4,14,12,13);       // WEMOS D1用 LCD開始
unsigned long TIME=0;                       // 1970年からmillis()＝0までの秒数
char lcd0[17]="00:00:00 LCD MON";           // LCD表示用(1行目)文字列変数 16字
char lcd1[65]="";                           // LCD表示用(2行目)文字列変数 64字
int lcd_p=0;                                // LCD表示位置
char hist[HIST_MAX][65];                    // 過去データ デバイス7+Null+56字
int hist_p=0;                               // 過去データ位置
int disp_p=-1;                              // 表示データ位置

void setup(){                               // 起動時に一度だけ実行する関数
    lcd.begin(16, 2);                       // 液晶の初期化(16桁×2行)
    lcd.print("Example 28 LCD");            // 「Example 28」をLCDに表示する
    lcd.setCursor(0,1);                     // カーソル位置を液晶の左上へ
    wifi_set_sleep_type(LIGHT_SLEEP_T);     // 省電力モード設定
    WiFi.mode(WIFI_STA);                    // 無線LANをSTAモードに設定
    WiFi.begin(SSID,PASS);                  // 無線LANアクセスポイントへ接続
    while(WiFi.status() != WL_CONNECTED){   // 接続に成功するまで待つ
        delay(500);                         // 待ち時間処理
        lcd.print(".");                     // 接続進捗を表示
    }
    server.begin();                         // サーバを起動する
    udp.begin(NTP_PORT);                    // NTP待ち受け開始
    udpRx.begin(PORT);                      // UDP待ち受け開始
    lcd.setCursor(0,1);                     // カーソル位置を液晶の左上へ
    lcd.print(WiFi.localIP());              // IPアドレスを液晶の2行目に表示
    delay(1000);                            // 表示内容の確認待ち時間
    TIME=getNtp();                          // NTP時刻を取得
    TIME-=millis()/1000;                    // カウント済み内部タイマー事前考慮
    memset(hist,0,HIST_MAX*65);             // 過去データの初期化
}

void loop(){                                // 繰り返し実行する関数
    WiFiClient client;                      // Wi-Fiクライアントの定義
    char c;                                 // 文字変数cを定義
    char s[65];                             // 文字列変数を定義 65バイト64文字
    char date[20];                          // 日時保持用
    uint8_t buttons;                        // キーパッド用
    int len=0;                              // 文字列長を示す整数型変数を定義
    int t=0;                                // 待ち受け時間のカウント用の変数
    int postF=0;                            // POSTフラグ(0:未 1:POST 2:BODY)
    int postL=64;                           // POSTデータ長
    unsigned long time=millis();            // ミリ秒の取得
    uint32_t ip = WiFi.localIP();

    client = server.available();            // 接続されたTCPクライアントを生成
    if(!client){                            // TCPクライアントが無かった場合
        if(time%200 < 1){                   // 200msに一回
            if(time%86400000ul==0){         // 24時間に1回
                TIME=getNtp();              // NTP時刻を取得
                TIME-=millis()/1000;
            }
            time2txt(date,TIME+time/1000);  // 以下、表示用コンテンツ作成
            strcpy(lcd0,&date[11]); lcd0[8]=' '; lcd0[16]='\0';
            buttons=lcd.readButtons(); switch( buttons ){
                case BUTTON_SELECT:
                    disp_p=-1; strcpy(&lcd0[9],"IP_ADDR");
                    sprintf(lcd1,"%d.%d.%d.%d",ip&255,ip>>8&255,ip>>16&255,ip>>24);
                    break;
                case BUTTON_UP: if(disp_p<0) disp_p=hist_p;
                    disp_p--; if(disp_p<0) disp_p=HIST_MAX-1; break;
                case BUTTON_DOWN: if(disp_p<0) disp_p=hist_p;
                    disp_p++;if(disp_p>=HIST_MAX) disp_p=0; break;
                case BUTTON_LEFT: lcd_p=-8; if(lcd_p<0) lcd_p=0; break;
                case BUTTON_RIGHT:
                    lcd_p=+8; if(strlen(&hist[disp_p][8])<=lcd_p)lcd_p=-8;break;
                default: break;
            }
            if(disp_p >=0){
                strncpy(&lcd0[9],hist[disp_p],8);
                strncpy(lcd1,&hist[disp_p][8],56);
            }
            if(TIME){ lcd.clear(); lcd.setCursor(0,0); lcd.print(lcd0); }
            if(strlen(lcd1)>16){
                memset(s, 0, 65);           // 文字列変数sの初期化(65バイト)
                if(lcd_p<0) strncat(s,lcd1,64);
                else strncat(s,&lcd1[lcd_p],64);
                lcd.setCursor(0,1); lcd.print(s); lcd_p++;
                if((int)strlen(lcd1)<=lcd_p)lcd_p=-8;
            }else{ lcd.setCursor(0,1); lcd.print(lcd1); }
            delay(1);
        }
        len = udpRx.parsePacket();          // UDP受信パケット長を変数lenに代入
        if(len==0)return;                   // TCPとUDPが未受信時にloop()先頭へ
        memset(s, 0, 65);                   // 文字列変数sの初期化(65バイト)
        udpRx.read(s, 64);                  // UDP受信データを文字列変数sへ代入
        for(int i=0;i<len;i++) if( !isgraph(s[i]) ) s[i]=' ';
        strncpy(&lcd0[9],s,8); lcd0[16]=0; lcd.setCursor(0,0); lcd.print(lcd0);
        strncpy(lcd1,&s[8],56);
        strcpy(hist[hist_p],&lcd0[9]); strncpy(&hist[hist_p][8],lcd1,56);
        hist_p++; if(hist_p>=HIST_MAX) hist_p=0;
        if(disp_p>=0) disp_p=hist_p;
        if(strlen(lcd1)>16) lcd_p=-8;
        else{ lcd.setCursor(0,1); lcd.print(lcd1); }
        if(strncmp(s,"timer_",6)==0 && strlen(s)>=27){
            TIME=atoi(&s[16]); TIME*=24;    TIME+=atoi(&s[19]); TIME*=60;
            TIME+=atoi(&s[22]); TIME*=60;   TIME+=atoi(&s[25]); 
            TIME-=millis()/1000;            // 012345678901234567890123456
        }                                   // timer_1,2016,10,11,18,46,36
        return;                             // loop()の先頭に戻る
    }
    lcd.clear();lcd.print("TCP Connected"); // 接続されたことを表示
    while(client.connected()){              // 当該クライアントの接続状態を確認
        if(client.available()){             // クライアントからのデータを確認
            c=client.read();                // データを文字変数cに代入
            if(c=='\n'){                    // 改行を検出した時
                if(postF==0){               // ヘッダ処理
                    if(len>11 && strncmp(s,"GET /?TEXT=",11)==0){
                        strncpy(lcd1,&s[11],64);    // 受信文字列をlcd1へコピー
                        trUri2txt(lcd1);            // URLエンコードの変換処理
                        if(strlen(lcd1)>16) lcd1[16]='\0';
                        strcpy(&lcd0[9],"Message");
                        break;              // 解析処理の終了
                    }else if (len>5 && strncmp(s,"GET /",5)==0){
                        len=0;
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
                        strncpy(lcd1,&s[5],64);     // 受信文字列をlcd1へコピー
                        trUri2txt(lcd1);            // URLエンコードの変換処理
                        if(strlen(lcd1)>16) lcd1[16]='\0';
                        strcpy(&lcd0[9],"HT_POST");
                    }else len=0;
                    break;                  // 解析処理の終了
                }
                postL--;                    // 受信済POSTデータ長の減算
            }
        }
        t++;                                // 変数tの値を1だけ増加させる
        if(t>TIMEOUT){                      // TIMEOUTに到達したらwhileを抜ける
            len=0; break;
        }else delay(1);
    }
    if(len){
        strncpy(hist[hist_p],&lcd0[9],8);
        strncpy(&hist[hist_p][8],lcd1,16);
        hist_p++; if(hist_p>=HIST_MAX) hist_p=0;
        if(disp_p>=0) disp_p=hist_p;
    }
    delay(1);                               // クライアント側の応答待ち時間
    if(client.connected()){                 // 当該クライアントの接続状態を確認
        html(client,lcd1,WiFi.localIP());   // HTMLコンテンツを出力する
    }
    client.stop();                          // クライアントの切断
}

unsigned long getNtp(){
    unsigned long highWord;                 // 時刻情報の上位2バイト用
    unsigned long lowWord;                  // 時刻情報の下位2バイト用
    unsigned long time;                     // 1970年1月1日からの経過秒数
    int waiting=0;                          // 待ち時間カウント用
    char s[20];                             // 表示用
    
    sendNTPpacket(NTP_SERVER);              // NTP取得パケットをサーバへ送信する
    while(udp.parsePacket()<44){
        delay(100);                         // 受信待ち
        waiting++;                          // 待ち時間カウンタを1加算する
        if(waiting%10==0)Serial.print('.'); // 進捗表示
        if(waiting > 100) return 0ul;       // 100回(10秒)を過ぎたら戻る
    }
    udp.read(packetBuffer,NTP_PACKET_SIZE); // 受信パケットを変数packetBufferへ
    highWord=word(packetBuffer[40],packetBuffer[41]);   // 時刻情報の上位2バイト
    lowWord =word(packetBuffer[42],packetBuffer[43]);   // 時刻情報の下位2バイト
    
    Serial.print("UTC time = ");
    time = highWord<<16 | lowWord;          // 時刻(1900年1月からの秒数)を代入
    time -= 2208988800UL;                   // 1970年と1900年の差分を減算
    time2txt(s,time);                       // 時刻をテキスト文字に変換
    Serial.println(s);                      // テキスト文字を表示

    Serial.print("JST time = ");            // 日本時刻
    time += 32400UL;                        // +9時間を加算
    time2txt(s,time);                       // 時刻をテキスト文字に変換
    Serial.println(s);                      // テキスト文字を表示
    
    return time;
}
