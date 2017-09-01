/*******************************************************************************
Example 58(=32+26): センサデバイス用 TFTPクライアント 設定

本ESPモジュールは、TFTPサーバ上の設定ファイルをダウンロードし、モジュール内の
設定を変更します。

※本サンプル作成の段階では、(暫定的に)下記ライブラリを使用しました。
    https://github.com/copercini/arduino-esp32-SPIFFS
※今後、esp-idfや上記ライブラリを基にしたものが公式サポートされると思います。

Raspberry PiへのTFTPサーバのインストール方法
    $ sudo apt-get install tftpd-hpa
    
    設定ファイル(/etc/default/tftpd-hpa) 例
    # /etc/default/tftpd-hpa
    TFTP_USERNAME="tftp"
    TFTP_DIRECTORY="/srv/tftp"
    TFTP_ADDRESS="0.0.0.0:69"

TFTPサーバの起動と停止
    /etc/init.d/tftpd-hpa start
    /etc/init.d/tftpd-hpa stop

転送用のファイルを保存
    $ echo "Hello! This is from RasPi" > /srv/tftp/esp8266_tftpc_1.ini

【注意事項】
・ESPモジュールのTFTPクライアントを起動したままにすると、危険です
・TFTPサーバ起動したままにすると、Raspberry Piが脅威にさらされます。
・ウィルスやワームが侵入すると、同じネットワーク上の全ての機器へ感染する恐れが
　高まります。

                                           Copyright (c) 2016-2017 Wataru KUNINO
*******************************************************************************/
#include <WiFi.h>                           // ESP32用WiFiライブラリ
#include <WiFiUdp.h>                        // UDP通信を行うライブラリ
#include "esp_deep_sleep.h"                 // ESP32用Deep Sleep ライブラリ
#define PIN_EN 2                            // GPIO 2(24番ピン)をセンサの電源に
#define PIN_AIN 34                          // GPIO 34 ADC1_CH6(6番ピン)をADCに
#define SSID "1234ABCD"                     // 無線LANアクセスポイントのSSID
#define PASS "password"                     // パスワード
#define SENDTO "192.168.0.255"              // 送信先のIPアドレス
#define PORT 1024                           // 送信のポート番号
#define SLEEP_P 50*1000000                  // スリープ時間 50秒(uint32_t)
#define DEVICE "tftpc_1,"                   // デバイス名(5文字+"_"+番号+",")
#define TFTP_PORT_C 69                      // TFTP接続用ポート番号(既定)
#define TFTP_PORT_T 1234                    // TFTP転送用ポート番号(任意)
#define TFTP_TIMEOUT 10                     // TFTP待ち受け時間(ms)
#define TFTP_FILENAME "/srv/tftp/esp8266_tftpc_1.ini" // TFTP受信ファイル名
char data[512];                             // TFTPデータ用変数
WiFiUDP tftp;                               // TFTP通信用のインスタンスを定義

void setup(){                               // 起動時に一度だけ実行する関数
    pinMode(PIN_AIN,INPUT);                 // アナログ入力の設定
    pinMode(PIN_EN,OUTPUT);                 // センサ用の電源を出力に
    Serial.begin(115200);                   // 動作確認のためのシリアル出力開始
    Serial.println("ESP32 eg.26 TFTP");     // 「ESP32 eg.26」をシリアル出力表示
    WiFi.mode(WIFI_STA);                    // 無線LANをSTAモードに設定
    delay(10);                              // ESP32に必要な待ち時間
    WiFi.begin(SSID,PASS);                  // 無線LANアクセスポイントへ接続
    while(WiFi.status() != WL_CONNECTED){   // 接続に成功するまで待つ
        delay(500);                         // 待ち時間処理
        digitalWrite(PIN_EN,!digitalRead(PIN_EN));      // LEDの点滅
        Serial.print(".");
    }
    Serial.println(WiFi.localIP());         // 本機のIPアドレスをシリアル出力
    tftpStart();                            // TFTPの開始
    while( tftpGet() );                     // データ受信
}

void tftpStart(){
    tftp.begin(TFTP_PORT_T);                // TFTP(受信)の開始
    tftp.beginPacket(SENDTO, TFTP_PORT_C);  // TFTP送信先を設定
    tftp.write(0x0); tftp.write(0x01);      // Read Requestコマンド(RRQ)
    tftp.print(TFTP_FILENAME);              // ファイル名
    tftp.write(0x0);                        // ファイル名の終端
    tftp.print("netascii");                 // ASCIIモード
    tftp.write(0x0);                        // モード名の終端
    tftp.endPacket();                       // TFTP送信の終了(実際に送信する)
    Serial.println("Send TFTP RRQ");        // 送信完了をシリアル端末へ表示
}

int tftpGet(){
    int len=0,time=0;
    IPAddress ip;
    
    while(len<5){                           // 未受信の間、繰り返し実行
        delay(1);                           // 1msの待ち時間
        len = tftp.parsePacket();           // 受信パケット長を変数lenに代入
        time++;                             // 時間のカウント
        if(time>TFTP_TIMEOUT) return 0;     // タイムアウト(応答値は0)
    }
    ip=tftp.remoteIP();                     // サーバのIPアドレスを取得
    tftp.read(data, 511);                   // 最大511バイトまで受信する
    Serial.print("Recieved (");
    for(int i=0;i<4;i++) Serial.print(data[i],DEC); // コマンド、ブロック番号
    Serial.print(") ");
    Serial.print(len - 4);                  // 受信データ長を表示
    Serial.print(" Bytes from ");
    Serial.println(ip);                     // サーバのIPアドレスを表示
    if(len > 511){
        Serial.println("FILE SIZE ERROR");  // 複数ブロックの転送には対応しない
        return -1;                          // ERROR(応答値は-1)
    }
    if(data[0]==0x0 && data[1]==0x3){       // TFTP転送データの時
        Serial.print(&data[4]);             // TFTP受信データを表示する
        Serial.println("[EOF]");
        tftp.beginPacket(ip, TFTP_PORT_T);  // TFTP ACK送信先を設定
        tftp.write(0x0); tftp.write(0x04);  // 受信成功コマンド(ACK)を送信
        tftp.write(data[2]);                // ブロック番号の上位1バイトを送信
        tftp.write(data[3]);                // ブロック番号の下位1バイトを送信
        tftp.endPacket();                   // TFTP送信の終了(実際に送信する)
        return len;
    }
    return -1;                              // ERROR(応答値は-1)
}

void loop() {
    WiFiUDP udp;                            // UDP通信用のインスタンスを定義
    int adc;                                // 整数型変数adcを定義
    
    digitalWrite(PIN_EN,HIGH);              // センサ用の電源をONに
    delay(5);                               // 起動待ち時間
    adc=(int)mvAnalogIn(PIN_AIN);           // AD変換器から値を取得
    digitalWrite(PIN_EN,LOW);               // センサ用の電源をOFFに
    udp.beginPacket(SENDTO, PORT);          // UDP送信先を設定
    udp.print(DEVICE);                      // デバイス名を送信
    udp.println(adc);                       // 変数adcの値を送信
    Serial.println(adc);                    // シリアル出力表示
    udp.endPacket();                        // UDP送信の終了(実際に送信する)
    delay(200);                             // 送信待ち時間
    esp_deep_sleep(SLEEP_P);                // Deep Sleepモードへ移行
}

float mvAnalogIn(uint8_t PIN){
    int in0,in3;
    float ad0,ad3;
    
    analogSetPinAttenuation(PIN,ADC_11db);
    in3=analogRead(PIN);
    
    if( in3 > 2599 ){
        ad3 = -1.457583e-7 * (float)in3 * (float)in3
            + 1.510116e-3 * (float)in3
            - 0.573300;
    }else{
        ad3 = 8.378998e-4 * (float)in3 + 1.891456e-1;
    }
    if( in3 < 200 ){
        analogSetPinAttenuation(PIN,ADC_0db);
        in0=analogRead(PIN);
        ad0 = 2.442116e-4 * (float)in0 + 1.075584e-1;
        if( in3 >= 100 ){
            ad3 = ad3 * ((float)in3 - 100.) / 100.
                + ad0 * (200. - (float)in3) / 100.;
        }else{
            ad3 = ad0;
        }
    }
    return ad3 * 1000.;
}
