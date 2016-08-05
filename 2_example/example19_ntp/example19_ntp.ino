/*******************************************************************************
Example 19: NTPクライアント
                                            Copyright (c) 2016 Wataru KUNINO
*******************************************************************************/

#include <ESP8266WiFi.h>                    // ESP8266用ライブラリ
extern "C" {
#include "user_interface.h"                 // ESP8266用の拡張IFライブラリ
}
#include <WiFiUdp.h>                        // udp通信を行うライブラリ
#define SSID "1234ABCD"                     // 無線LANアクセスポイントのSSID
#define PASS "password"                     // パスワード
#define SENDTO "192.168.0.255"              // 送信先のIPアドレス
#define PORT 1024                           // 送信のポート番号
#define SLEEP_P 29*60*1000000               // スリープ時間 29分(uint32_t)
#define DEVICE "timer_1,"                   // デバイス名(5文字+"_"+番号+",")
#define NTP_SERVER "ntp.nict.jp"            // NTPサーバのURL
#define NTP_PORT 8888                       // NTP待ち受けポート
#define NTP_PACKET_SIZE 48                  // NTP時刻長48バイト

byte packetBuffer[NTP_PACKET_SIZE];         // 送受信用バッファ
WiFiUDP udp;                                // NTP通信用のインスタンスを定義
WiFiUDP udpTx;                              // UDP送信用のインスタンスを定義

void setup(){
    int waiting=0;                          // アクセスポイント接続待ち用
    Serial.begin(9600);                     // 動作確認のためのシリアル出力開始
    Serial.println("Example 19 NTP");       // 「Example 19」をシリアル出力表示
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
}

void loop(){
    unsigned long highWord;                 // 時刻情報の上位2バイト用
    unsigned long lowWord;                  // 時刻情報の下位2バイト用
    unsigned long time;                     // 1970年1月1日からの経過秒数
    int waiting=0,data;
    
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
    Serial.println(time);

    Serial.print("JST time = ");            // 日本時刻
    time += 32400UL;                        // +9時間を加算
    data=(int)((time  % 86400L) / 3600);    // 時を計算し変数dataへ代入
    udpTx.beginPacket(SENDTO, PORT);        // UDP送信先を設定
    udpTx.print(DEVICE);                    // デバイス名を送信
    udpTx.print(data);                      // データを送信
    udpTx.print(",");                       // 「,」カンマを送信
    Serial.print(data);
    Serial.print(':');
    data=(int)(time  % 3600) / 60;          // 分を計算し変数dataへ代入
    if( data < 10 ) Serial.print('0');      // 10未満の時に「0」を付与
    udpTx.print(data);                      // データを送信
    udpTx.print(",");                       // 「,」カンマを送信
    Serial.print(data);
    Serial.print(':');
    data=(int)(time  % 3600) / 60;          // 秒を計算し変数dataへ代入
    if( data < 10 ) Serial.print('0');      // 10未満の時に「0」を付与
    udpTx.print(data);                      // データを送信
    udpTx.endPacket();                      // UDP送信の終了(実際に送信する)
    Serial.println(data);
    sleep();
}

void sleep(){
    delay(200);                             // 送信待ち時間
    ESP.deepSleep(SLEEP_P,WAKE_RF_DEFAULT); // スリープモードへ移行する
    while(1){                               // 繰り返し処理
        delay(100);                         // 100msの待ち時間処理
    }                                       // 繰り返し中にスリープへ移行
}
