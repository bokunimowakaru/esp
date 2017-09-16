/*******************************************************************************
Example 29: MACブロードキャスト／ Amazon Dash ボタン検出

リスト中の特定のMACアドレスの端末のブロードキャストをプロミスキャスモードで
待ち受け、検出時にUDP送信します。

スマートフォンやPCを自宅に持ち帰り無線LANへ自動接続するときや、Amazon Dashボタン
が押下されたときなどに、送信するMACブロードキャストを検出します。

リストは本機の起動時にTFTPで受け取ることが出来ます。
起動後、普段はプロミスキャスモードでリスト中のMACアドレスを待ち受けます。
リストに登録可能な最大件数はMAC_LIST_N_MAXで定義してください。

ご注意：ネットセキュリティ上の脆弱性の十分に注意して利用してください。

                                           Copyright (c) 2016-2017 Wataru KUNINO
********************************************************************************

TFTPサーバ上から設定ファイルをダウンロードし、モジュール内の設定を変更します。
本サンプルではディープスリープ時間を設定することが出来ます。

Raspberry PiへのTFTPサーバのインストール方法
    $ sudo apt-get install tftpd-hpa
    
    設定ファイル(/etc/default/tftpd-hpa) 例
    # /etc/default/tftpd-hpa
    TFTP_USERNAME="tftp"
    TFTP_DIRECTORY="/srv/tftp"
    TFTP_ADDRESS="0.0.0.0:69"

TFTPサーバの起動と停止
    $ chmod 755 /srv/tftp
    $ sudo /etc/init.d/tftpd-hpa start
    $ sudo /etc/init.d/tftpd-hpa stop

転送用のファイルを保存 ※必ずFF:FF:FF:FF:FF:FFの部分を書き換えてください
    $ echo "; List of MAC Address" > /srv/tftp/adash_1.mac
    $ echo "ACCEPT1=FF:FF:FF:FF:FF:FF" >> /srv/tftp/adash_1.mac
    $ echo "ACCEPT2=FF:FF:FF:FF:FF:FF" >> /srv/tftp/adash_1.mac
    $ echo "ACCEPT3=FF:FF:FF:FF:FF:FF" >> /srv/tftp/adash_1.mac
    $ chmod 644 /srv/tftp/adash_1.mac
    $ cat /srv/tftp/adash_1.mac
    ; List of MAC Address
    ACCEPT1=FF:FF:FF:FF:FF:FF
    ACCEPT2=FF:FF:FF:FF:FF:FF
    ACCEPT3=FF:FF:FF:FF:FF:FF

【注意事項】
・TFTPクライアント(ESP側)やTFTPサーバ(PCやRaspberry Pi側)起動すると、各機器が
　セキュリティの脅威にさらされた状態となります。
・また、ウィルスやワームが侵入すると、同じネットワーク上の全ての機器へ感染する
　恐れが高まります。
・インターネットに接続すると外部からの侵入される場合があります。
・TFTPクライアントは少なくともローカルネット内のみで動作させるようにして下さい。
・TFTPが不必要なときは、極力、停止させてください。
*******************************************************************************/
#include <ESP8266WiFi.h>                    // ESP8266用ライブラリ
extern "C" {
#include "user_interface.h"                 // ESP8266用の拡張IFライブラリ
}
#include <WiFiUdp.h>                        // UDP通信を行うライブラリ
#define PIN_EN 13                           // IO 13(5番ピン)をセンサ用の電源に
#define SSID "1234ABCD"                     // 無線LANアクセスポイントのSSID
#define PASS "password"                     // パスワード
#define SENDTO "192.168.0.255"              // 送信先のIPアドレス
#define PORT 1024                           // 送信のポート番号
#define DEVICE "adash_1,"                   // デバイス名(5文字+"_"+番号+",")
#define MAC_LIST_N_MAX 8                    // MACリストの最大保持数

byte MAC_LIST[MAC_LIST_N_MAX][6];			// MACリスト
int MAC_LIST_N=0;
int channel;								// 無線LAN物理チャンネル

void connect(){
    WiFi.begin(SSID,PASS);                  // 無線LANアクセスポイントへ接続
    while(WiFi.status() != WL_CONNECTED){   // 接続に成功するまで待つ
        delay(500);                         // 待ち時間処理
        digitalWrite(PIN_EN,!digitalRead(PIN_EN));      // LEDの点滅
        Serial.print(".");
    }
    Serial.print(WiFi.localIP());           // 本機のIPアドレスをシリアル出力
	Serial.print(' ');
	Serial.println(WiFi.macAddress());      // 本機のMACアドレスをシリアル出力
}

void setup(){                               // 起動時に一度だけ実行する関数
    char data[512];                         // TFTPデータ用変数
    int len_tftp;                           // TFTPデータ長
    
    pinMode(PIN_EN,OUTPUT);                 // センサ用の電源を出力に
    Serial.begin(9600);                     // 動作確認のためのシリアル出力開始
    Serial.println("Example 29 ADASH");     // 「ESP32 eg.29」をシリアル出力表示
    WiFi.mode(WIFI_STA);                    // 無線LANをSTAモードに設定
    if(ini_init(data)) initialize(data);    // SPIFSSからINIファイルの読み込み
    connect();                              // Wi-Fiアクセスポイントへ接続
    tftpStart();                            // TFTPの開始
    do{
        len_tftp = tftpGet(data);           // TFTP受信(data=受信データ)
        if(len_tftp>0){
            initialize(data);               // INIファイル内の内容を変数へ代入
            ini_save(data);                 // INIファイルをSPIFSSへ書込み
        }
    }while(len_tftp);
    WiFi.disconnect();                      // WiFiアクセスポイントを切断する
    promiscuous_start(channel);             // プロミスキャスモードへ移行する
}

void initialize(char *data){
	char key[10]="ACCEPT";	
    for(int i=0;i<MAC_LIST_N_MAX; i++){
		sprintf(key+6,"%d",i+1);
		get_parsed_mac(MAC_LIST[i],data,key);
	}
}

void loop(){
    WiFiUDP udp;                            // UDP通信用のインスタンスを定義
    int i;
    byte mac[6];
    
    if(promiscuous_get_mac(mac)==0) return;
    for(i=0;i<MAC_LIST_N_MAX;i++){
		if(memcmp(MAC_LIST[i],mac,6)==0)break;
	}
	if(i==MAC_LIST_N_MAX) return;
	promiscuous_stop();

    connect();                              // Wi-Fiアクセスポイントへ接続
    digitalWrite(PIN_EN,HIGH);              // センサ用の電源をONに
    udp.beginPacket(SENDTO, PORT);          // UDP送信先を設定
    udp.print(DEVICE);                      // デバイス名を送信
    for(i=0;i<6;i++){
		udp.print(',');
		if(mac[i]<0x10) udp.print(0);
		udp.print(mac[i],HEX);
	}
	udp.println();
    udp.endPacket();                        // UDP送信の終了(実際に送信する)
    delay(200);                             // 送信待ち時間
    digitalWrite(PIN_EN,LOW);               // センサ用の電源をOFFに
    WiFi.disconnect();                      // WiFiアクセスポイントを切断する
    promiscuous_start(channel);             // プロミスキャスモードへ移行する
}

