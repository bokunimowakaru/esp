/*******************************************************************************
Amazon Dash Button

*******************************************************************************/

/*
【参考文献】

下記の情報およびソースコードを利用させていただきました(2017/9/16)。

プロミスキャスモードを用いたESP8266でのAmazon Dash Buttonのイベント取得
http://qiita.com/kat-kai/items/3b1d5c74138d77a27c4d

ライセンス：Qiita利用規約に基づく
権利者：kat-kai http://qiita.com/kat-kai (2016年12月29日～2017年01月08日)
*/

#include <ESP8266WiFi.h>
#include "promiscuous.h"
extern "C" {
	#include <user_interface.h>
}
#define MAC_SKIP_N 64						// MAC情報表示の表示間隔(重複作動)

boolean DEBUG_MAC=1;
byte _mac_prev[6];
unsigned int _fc_prev;
volatile int _mac_prev_n=0;
volatile int _mac_read_i=0;
byte _recieved_mac[6];

//プロミスキャスモードでパケットを受け取ったときのcallback
static void ICACHE_FLASH_ATTR promisc_cb(uint8_t *buf, uint16_t len){
	if (len != 128) return; //In order to discard both unknown packets and data packets

	struct sniffer_buf2 *sniffer = (struct sniffer_buf2*) buf;
	struct MAC_header *mac = (struct MAC_header*) sniffer->buf;

	int i;
	boolean beaconFlag = true;
	const uint8_t broadcast[6]={0xff,0xff,0xff,0xff,0xff,0xff};
	
	if(memcmp(mac->addr2,mac->addr3,6)==0)return;	// ビーコンの除去
	if(_mac_prev_n && !memcmp(_mac_prev,mac->addr2,6) && _fc_prev == mac->frameControl){
		_mac_prev_n++;
		if( (_mac_prev_n%MAC_SKIP_N) !=(MAC_SKIP_N-1) ) return;
	}
	memcpy(_mac_prev,mac->addr2,6);
	_fc_prev=mac->frameControl;
	_mac_prev_n=1;
	
	if(!_mac_read_i) memcpy(_recieved_mac,mac->addr2,6);
	_mac_read_i++;

	/* デバッグ用の表示 */
	if( DEBUG_MAC ){
		Serial.print("FC=");
		if(mac->frameControl < 0x1000) Serial.print(0);
		if(mac->frameControl < 0x100) Serial.print(0);
		if(mac->frameControl < 0x10) Serial.print(0);
		Serial.print(mac->frameControl,HEX);
/*
		Serial.print(" us="); Serial.print(mac->duration);
*/
/*
		Serial.print(" to=");
		for (i=0; i<6; i++){
			if(mac->addr1[i] < 0x10) Serial.print(0);
			Serial.print(mac->addr1[i], HEX);
			if(i != 5)Serial.print(":");
		}
*/
		Serial.print(" from=");
		for (i=0; i<6; i++){
			if(mac->addr2[i] < 0x10) Serial.print(0);
			Serial.print(mac->addr2[i], HEX);
			if(i != 5)Serial.print(":");
		}
/*
		Serial.print(" i="); Serial.print(mac->sequenceControl);
*/
/*
		Serial.print(" bssid=");
		for (i=0; i<6; i++){
			if(mac->addr3[i] < 0x10) Serial.print(0);
			Serial.print(mac->addr3[i], HEX);
			if(i != 5)Serial.print(":");
		}
*/
/*
		Serial.print(" qos=");
		if(mac->qos < 0x1000) Serial.print(0);
		if(mac->qos < 0x100) Serial.print(0);
		if(mac->qos < 0x10) Serial.print(0);
		Serial.print(mac->qos,HEX);
		
		Serial.print(" hit=");
		for (i=0; i<4; i++){
			if(mac->hit[i] < 0x10) Serial.print(0);
			Serial.print(mac->hit[i], HEX);
		}
*/
		Serial.println();
	}
}

int promiscuous_get_mac(byte *mac){
	if(_mac_read_i==0) return 0;
	if(DEBUG_MAC && _mac_read_i>1){
		_mac_prev_n=0;
		Serial.print("WARNING: dropped n=");
		Serial.println(_mac_read_i-1);
	}
	memcpy(mac,_recieved_mac,6);
	_mac_read_i=0;
	return 1;
}

void promiscuous_start(byte channel){
	if( DEBUG_MAC ){
		Serial.print("ESP8266 promiscuous mode started. Channel: ");
		Serial.println(channel);
		Serial.println("Please turn on your wi-fi STA or push Amazon Dash Button");
	}
	wifi_set_opmode(STATION_MODE);
	wifi_set_channel(channel);
	wifi_set_promiscuous_rx_cb(promisc_cb);

	wifi_promiscuous_enable(1);
}

void promiscuous_stop(){
	wifi_promiscuous_enable(0);
}

void promiscuous_uart(boolean in){
	DEBUG_MAC=in;
}

int promiscuous_ready(){
	int n=_mac_prev_n;
	_mac_prev_n=0;
	return n;
}
