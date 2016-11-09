/*******************************************************************************
RTC内のメモリの読み書きドライバ

下記のウェブサイトの情報に基づいて作成しました。

http://jiwashin.blogspot.jp/2016/07/esp30.html
https://lowreal.net/2016/01/10/1
*******************************************************************************/

#include <Arduino.h>
extern "C" {
#include <user_interface.h>
};

#define kOffset_CRC         65
#define kMaxDataArea  (512 - 4 - sizeof(int))

// system_rtc_mem_write() 先のブロックアドレス。
// 4 bytes で align されており、先頭256bytes はシステム予約領域
// 64 から書けるはずだが、65 以降でないとうまくいかなかった。。
#define kOffset_CRC         65

//  crc
//  thanks : https://github.com/vinmenn/Crc16/blob/master/examples/Crc16_Example/Crc16_Example.ino
//  below comment is from original source
//Check routine taken from
//http://web.mit.edu/6.115/www/miscfiles/amulet/amulet-help/xmodem.htm

int calcCRC(char *ptr,int count){
//  char *ptr = rtcStorage.buffer;
//  int   count = sizeof(rtcStorage.buffer);
	Serial.print("calc crc counter = ");
	Serial.println(count);

	int crc;
	char i;
	crc = 0;
	while(--count >= 0){
		crc = crc ^ (int) * ptr++ << 8;
		i = 8;
		do{
			if(crc & 0x8000) crc = crc << 1 ^ 0x1021;
			else crc = crc << 1;
		}while(--i);
	}
	return crc;
}

typedef struct{
  int crc;
  char buffer[kMaxDataArea];
}RtcStorage;

RtcStorage rtcStorage;
//
//  Storageを読み込む。もしCRCが不一致なら初期化
//
char *readRtcStorage(){
	bool ok;
	int crc;

	Serial.print("storage size = ");
	Serial.println(sizeof(rtcStorage));
	// データ読みこみ
	ok = system_rtc_mem_read(kOffset_CRC, &rtcStorage, kMaxDataArea);
	if(!ok){
		Serial.println("system_rtc_mem_read failed");
		return 0;
	}
	crc = calcCRC( rtcStorage.buffer, kMaxDataArea );
	Serial.print("crc = ");
	Serial.print(crc, HEX);
	Serial.print(", stored crc = ");
	Serial.println(rtcStorage.crc, HEX);
	if(crc != rtcStorage.crc || rtcStorage.buffer[kMaxDataArea-1]!='\0'){
		Serial.println("crc mismatch");
		memset(rtcStorage.buffer, 0, kMaxDataArea);
		crc = calcCRC( rtcStorage.buffer, kMaxDataArea );
		Serial.print("new crc = ");
		Serial.println(crc, HEX);
		rtcStorage.crc = crc;
		ok = system_rtc_mem_write(kOffset_CRC, &rtcStorage, sizeof(rtcStorage));
		if(!ok){
			Serial.println("readStorageAndInitIfNeeded : write fail");
		}
	}
	return rtcStorage.buffer;
}

bool writeRtcStorage(char *in) {
	bool ok;
	memset(rtcStorage.buffer, 0, kMaxDataArea);
	strncpy(rtcStorage.buffer,in,kMaxDataArea-1);
	rtcStorage.crc = calcCRC( rtcStorage.buffer, kMaxDataArea );
	ok = system_rtc_mem_write(kOffset_CRC, &rtcStorage, sizeof(rtcStorage));
	if(!ok){
		Serial.println("Error : writeStorage : system_rtc_mem_write fail");
	}
	return ok;
}

int readRtcInt(){
	return atoi(readRtcStorage());
}

bool writeRtcInt(int in){
	char s[12];
	itoa(in,s,10);
	return writeRtcStorage(s);
}

/*
float readRtcVal(){
	return strtof(readRtcStorage(),0);
}

bool writeRtcVal(float in){
	char s[9];
	dtostrf(in,8,2,s);
	return writeRtcStorage(s);
}
*/