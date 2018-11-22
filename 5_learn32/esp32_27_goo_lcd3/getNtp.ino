/*******************************************************************************
NTPクライアント
                                           Copyright (c) 2016-2019 Wataru KUNINO
*******************************************************************************/
/*

 Udp NTP Client

 Get the time from a Network Time Protocol (NTP) time server
 Demonstrates use of UDP sendPacket and ReceivePacket
 For more on NTP time servers and the messages needed to communicate with them,
 see http://en.wikipedia.org/wiki/Network_Time_Protocol

 created 4 Sep 2010
 by Michael Margolis
 modified 9 Apr 2012
 by Tom Igoe

 This code is in the public domain.

 */

// send an NTP request to the time server at the given address

#include <WiFiUdp.h>                        // UDP通信を行うライブラリ
#define NTP_SERVER "ntp.nict.jp"            // NTPサーバのURL
#define NTP_PORT 8888                       // NTP待ち受けポート
#define NTP_PACKET_SIZE 48                  // NTP時刻長48バイト

byte packetBuffer[NTP_PACKET_SIZE];         // 送受信用バッファ
WiFiUDP udp_ntp;                            // NTP通信用のインスタンスを定義

unsigned long getNtp(){
    unsigned long highWord;                 // 時刻情報の上位2バイト用
    unsigned long lowWord;                  // 時刻情報の下位2バイト用
    unsigned long time;                     // 1970年1月1日からの経過秒数
    int waiting=0;                          // 待ち時間カウント用
    char s[20];                             // 表示用
    
    udp_ntp.begin(NTP_PORT);                    // NTP待ち受け開始
    sendNTPpacket(NTP_SERVER);              // NTP取得パケットをサーバへ送信する
    while(udp_ntp.parsePacket()<44){
        delay(100);                         // 受信待ち
        waiting++;                          // 待ち時間カウンタを1加算する
        if(waiting%10==0)Serial.print('.'); // 進捗表示
        if(waiting > 100){
            udp_ntp.stop();
            return 0ul;                     // 100回(10秒)を過ぎたら戻る
        }
    }
    udp_ntp.read(packetBuffer,NTP_PACKET_SIZE); // 受信パケットを変数packetBufferへ
    udp_ntp.stop();
    highWord=word(packetBuffer[40],packetBuffer[41]);   // 時刻情報の上位2バイト
    lowWord =word(packetBuffer[42],packetBuffer[43]);   // 時刻情報の下位2バイト
    time = highWord<<16 | lowWord;          // 時刻(1900年1月からの秒数)を代入
    time -= 2208988800UL;                   // 1970年と1900年の差分を減算
    time2txt(s,time);                       // 時刻をテキスト文字に変換
    time += 32400UL;                        // +9時間を加算
    time2txt(s,time);                       // 時刻をテキスト文字に変換
    return time;
}

unsigned long sendNTPpacket(const char* address)
{
  // set all bytes in the buffer to 0
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  packetBuffer[0] = 0b11100011;   // LI, Version, Mode
  packetBuffer[1] = 0;     // Stratum, or type of clock
  packetBuffer[2] = 6;     // Polling Interval
  packetBuffer[3] = 0xEC;  // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12]  = 49;
  packetBuffer[13]  = 0x4E;
  packetBuffer[14]  = 49;
  packetBuffer[15]  = 52;

  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp:
  udp_ntp.beginPacket(address, 123); //NTP requests are to port 123
  udp_ntp.write(packetBuffer, NTP_PACKET_SIZE);
  udp_ntp.endPacket();
}
