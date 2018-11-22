/*******************************************************************************
Example 13c: NTP時刻データ転送機 NTPクライアント [Ambient対応版]

現在時刻をインターネットから取得してクラウドサービスAmbientへ送信する機器です。
あたかも「時刻センサ」のような動作を行います。

                                          Copyright (c) 2016-2019 Wataru KUNINO
*******************************************************************************/

#include <ESP8266WiFi.h>                    // ESP8266用ライブラリ
extern "C" {
#include "user_interface.h"                 // ESP8266用の拡張IFライブラリ
}
#include <WiFiUdp.h>                        // udp通信を行うライブラリ
#include "Ambient.h"                        // Ambient用のライブラリの組み込み
#define SSID "1234ABCD"                     // 無線LANアクセスポイントのSSID
#define PASS "password"                     // パスワード
#define AmbientChannelId 100                // チャネル名(整数)
#define AmbientWriteKey "0123456789abcdef"  // ライトキー(16桁の16進数)
#define SENDTO "192.168.0.255"              // 送信先のIPアドレス
#define PORT 1024                           // 送信のポート番号
#define SLEEP_P 59*60*1000000ul             // スリープ時間 59分(uint32_t)
#define DEVICE "timer_1,"                   // デバイス名(5文字+"_"+番号+",")
#define NTP_SERVER "ntp.nict.jp"            // NTPサーバのURL
#define NTP_PORT 8888                       // NTP待ち受けポート
#define NTP_PACKET_SIZE 48                  // NTP時刻長48バイト
void sleep();

Ambient ambient;
WiFiClient client;
byte packetBuffer[NTP_PACKET_SIZE];         // 送受信用バッファ
WiFiUDP udp;                                // NTP通信用のインスタンスを定義
WiFiUDP udpTx;                              // UDP送信用のインスタンスを定義
float v;                                    // 電圧保持用

void setup(){
    int waiting=0;                          // アクセスポイント接続待ち用
    v=(float)system_adc_read();             // AD変換器から値を取得
    v *= 5 / 1023.;                         // 電池電圧値に変換
    Serial.begin(9600);                     // 動作確認のためのシリアル出力開始
    Serial.println("Example 13 NTP->Amb");  // 「Example 13」をシリアル出力表示
    WiFi.mode(WIFI_STA);                    // 無線LANをSTAモードに設定
    WiFi.begin(SSID,PASS);                  // 無線LANアクセスポイントへ接続
    while(WiFi.status() != WL_CONNECTED){   // 接続に成功するまで待つ
        delay(100);                         // 待ち時間処理
        waiting++;                          // 待ち時間カウンタを1加算する
        if(waiting%10==0)Serial.print('.'); // 進捗表示
        if(waiting > 300) sleep();          // 300回(30秒)を過ぎたらスリープ
    }
    udp.begin(NTP_PORT);                    // NTP待ち受け開始
    Serial.println(WiFi.localIP());         // 本機のIPアドレスをシリアル出力
    ambient.begin(AmbientChannelId, AmbientWriteKey, &client);  // Ambient開始
}

void loop(){
    unsigned long highWord;                 // 時刻情報の上位2バイト用
    unsigned long lowWord;                  // 時刻情報の下位2バイト用
    unsigned long time;                     // 1970年1月1日からの経過秒数
    int waiting=0;                          // 待ち時間カウント用
    char s[20];                             // 表示用
    
    Serial.print("Voltage  = ");
    Serial.println(v,3);                    // 電圧値を表示
    sendNTPpacket(NTP_SERVER);              // NTP取得パケットをサーバへ送信する
    while(udp.parsePacket()<44){
        delay(100);                         // 受信待ち
        waiting++;                          // 待ち時間カウンタを1加算する
        if(waiting%10==0)Serial.print('.'); // 進捗表示
        if(waiting > 100) sleep();          // 100回(10秒)を過ぎたらスリープ
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
    
    udpTx.beginPacket(SENDTO, PORT);        // UDP送信先を設定
    udpTx.print(DEVICE);                    // デバイス名を送信
    s[4]=s[7]=s[13]=s[16]=',';              // 「/」と「:」をカンマに置き換える
    udpTx.println(s);                       // データを送信
    udpTx.endPacket();                      // UDP送信の終了(実際に送信する)
    /* クラウドへ */
    dtostrf(((double)(time%86400))/3600,-8,5,s);
    Serial.println(s);                      // テキスト文字を表示
    ambient.set(1,s);                       // Ambient(データ1)へ温度を送信
    dtostrf(v,-5,3,s);                      // 電圧値を文字列に変換
    ambient.set(8,s);                       // Ambient(データ8)へ送信
    ambient.send();                         // Ambient送信の終了(実際に送信する)
    sleep();
}

void sleep(){
    delay(200);                             // 送信待ち時間
    ESP.deepSleep(SLEEP_P,WAKE_RF_DEFAULT); // スリープモードへ移行する
    while(1){                               // 繰り返し処理
        delay(100);                         // 100msの待ち時間処理
    }                                       // 繰り返し中にスリープへ移行
}
