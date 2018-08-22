/*******************************************************************************
NTPクライアント
                                            Copyright (c) 2016 Wataru KUNINO
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

#define NTP_PACKET_SIZE 48                  // NTP時刻長48バイト
// #define DEBUG

unsigned long getNTP(const char* address, const int port){
	WiFiUDP udpNtp;                             // UDP送信用のインスタンスを定義
	byte packetBuffer[NTP_PACKET_SIZE];         // 送受信用バッファ
    int waiting=0,data;
    unsigned long highWord;                 // 時刻情報の上位2バイト用
    unsigned long lowWord;                  // 時刻情報の下位2バイト用
    unsigned long local;                     // 1970年1月1日からの経過秒数

    udpNtp.begin(port);                    // NTP待ち受け開始

    // send an NTP request to the time server at the given address

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
	udpNtp.beginPacket(address, 123); //NTP requests are to port 123
	udpNtp.write(packetBuffer, NTP_PACKET_SIZE);
	udpNtp.endPacket();
	
    while(udpNtp.parsePacket()<44){
        delay(100);                         // 受信待ち
        waiting++;                          // 待ち時間カウンタを1加算する
        if(waiting > 100) return 0;          // 100回(10秒)を過ぎたら終了
    }
    udpNtp.read(packetBuffer,NTP_PACKET_SIZE); // 受信パケットを変数packetBufferへ
    highWord=word(packetBuffer[40],packetBuffer[41]);   // 時刻情報の上位2バイト
    lowWord =word(packetBuffer[42],packetBuffer[43]);   // 時刻情報の下位2バイト
    
    local = highWord<<16 | lowWord;          // 時刻(1900年1月からの秒数)を代入
    #ifdef DEBUG
    	Serial.println(highWord);
    	Serial.println(lowWord);
    	Serial.println(local);
    #endif
    if( local < 2208988800UL ) return 0;	// 時刻以外が得られた
    local -= 2208988800UL;                   // 1970年と1900年の差分を減算
    #ifdef DEBUG
    	Serial.println(local);
    #endif
    local += 32400UL;                        // +9時間を加算
    #ifdef DEBUG
    	Serial.println(local);
    #endif
    
    #ifdef DEBUG
	    Serial.print("JST time = ");            // 日本時刻
	    data=(int)((local  % 86400L) / 3600);    // 時を計算し変数dataへ代入
	    Serial.print(data);
	    Serial.print(':');
	    data=(int)(local  % 3600) / 60;          // 分を計算し変数dataへ代入
	    if( data < 10 ) Serial.print('0');      // 10未満の時に「0」を付与
	    Serial.print(data);
	    Serial.print(':');
	    data=(int)(local  % 60);          // 秒を計算し変数dataへ代入
	    if( data < 10 ) Serial.print('0');      // 10未満の時に「0」を付与
	    Serial.println(data);
    #endif
    udpNtp.stop(); 
	return local;
}
