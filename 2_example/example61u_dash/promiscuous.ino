/*******************************************************************************
Amazon Dash Button

*******************************************************************************/

/*
【参考文献】

下記の情報およびソースコードを利用させていただきました

プロミスキャスモードを用いたESP8266でのAmazon Dash Buttonのイベント取得(2017/9/16)
	http://qiita.com/kat-kai/items/3b1d5c74138d77a27c4d
	ライセンス：Qiita利用規約に基づく
	権利者：kat-kai http://qiita.com/kat-kai (2016年12月29日～2017年01月08日)

ESP32用(2021/02/23)
	https://www.hackster.io/p99will/esp32-wifi-mac-scanner-sniffer-promiscuous-4c12f4
*/

#include <WiFi.h>                           // ESP32用WiFiライブラリ
#include <esp_wifi.h>
#define MAC_SKIP_N 64						// MAC情報表示の表示間隔(重複作動)

const wifi_promiscuous_filter_t filt={ //Idk what this does
    .filter_mask=WIFI_PROMIS_FILTER_MASK_MGMT|WIFI_PROMIS_FILTER_MASK_DATA
};

typedef struct {
  int16_t fctl;
  int16_t duration;
  uint8_t da[6];
  uint8_t sa[6];
  uint8_t bssid[6];
  int16_t seqctl;
  unsigned char payload[];
} __attribute__((packed)) WifiMgmtHdr;

boolean DEBUG_MAC=1;
boolean _mac_fc_hold=0;
byte _mac_prev[6];
unsigned int _fc_prev;
volatile int _mac_prev_n=0;
volatile int _mac_read_i=0;
byte _recieved_mac[6];

//プロミスキャスモードでパケットを受け取ったときのcallback
void promisc_cb(void *buf, wifi_promiscuous_pkt_type_t type) {
	//This is where packets end up after they get sniffed
	wifi_promiscuous_pkt_t *p = (wifi_promiscuous_pkt_t*)buf;
	WifiMgmtHdr *mac = (WifiMgmtHdr*)p->payload;
	
	int len = p->rx_ctrl.sig_len;
	len -= sizeof(WifiMgmtHdr);
	if (len < 0){
		if(DEBUG_MAC) Serial.println("Received 0");
		return;
	}
	if(memcmp(mac->sa,mac->bssid,6)==0)return;	// ビーコンの除去
	if(_mac_prev_n && !memcmp(_mac_prev,mac->sa,6)){
		if( _mac_fc_hold || _fc_prev == mac->fctl){
			_mac_prev_n++;
			if( (_mac_prev_n%MAC_SKIP_N) !=(MAC_SKIP_N-1) ) return;
		}
	}
	memcpy(_mac_prev,mac->sa,6);
	_fc_prev=mac->fctl;
	_mac_prev_n=1;
	
	if(!_mac_read_i) memcpy(_recieved_mac,mac->sa,6);
	_mac_read_i++;

	if( DEBUG_MAC ){
		Serial.printf("FC=%04x from=%02x:%02x:%02x:%02x:%02x:%02x\n",
			_fc_prev,
			mac->sa[0],
			mac->sa[1],
			mac->sa[2],
			mac->sa[3],
			mac->sa[4],
			mac->sa[5]
		);
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
		Serial.print("ESP32 promiscuous mode started. Channel: ");
		Serial.println(channel);
		Serial.println("Please turn on your wi-fi STA or push Amazon Dash Button");
	}
	
	wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
	esp_wifi_init(&cfg);
	esp_wifi_set_storage(WIFI_STORAGE_RAM);
	esp_wifi_set_mode(WIFI_MODE_NULL);
	esp_wifi_start();
	esp_wifi_set_promiscuous(true);
	esp_wifi_set_promiscuous_filter(&filt);
	esp_wifi_set_promiscuous_rx_cb(&promisc_cb);
	esp_wifi_set_channel(channel, WIFI_SECOND_CHAN_NONE);
}

void promiscuous_stop(){
	Serial.println("promiscuous_stop: esp_wifi_set_promiscuous false");
	esp_wifi_set_promiscuous(false);
	delay(200);
}

void promiscuous_uart(boolean in){
	DEBUG_MAC=in;
}

int promiscuous_ready(){
	int n=_mac_prev_n;
	_mac_prev_n=0;
	return n;
}

void promiscuous_fchold(boolean in){
	_mac_fc_hold=in;
}
