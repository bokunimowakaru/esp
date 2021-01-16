/*******************************************************************************
Example 58(=32+26): ESP32 センサデバイス用 TFTPクライアント 設定

                                           Copyright (c) 2016-2019 Wataru KUNINO
                                           
TFTPサーバ上から設定ファイルをダウンロードし、モジュール内の設定を変更します。
本サンプルではADCの入力ピンとディープスリープ時間を設定することが出来ます。

TFTPとは
　TFTPはネットワーク機器などの設定ファイルやファームウェアを転送するときなどに
　使用されているデータ転送プロトコルです。  
　使い勝手が簡単で、プロトコルも簡単なので、機器のメンテナンスに向いています。
　認証や暗号化は行わないので、転送時のみ有効にする、もしくは侵入・ファイル転送
　されても問題の無い用途で利用します。

Raspberry PiへのTFTPサーバのインストール方法
    $ sudo apt-get install tftpd-hpa
    
    設定ファイル(/etc/default/tftpd-hpa) 例
    # /etc/default/tftpd-hpa
    TFTP_USERNAME="tftp"
    TFTP_DIRECTORY="/srv/tftp"
    TFTP_ADDRESS="0.0.0.0:69"
    TFTP_OPTIONS="--secure"

TFTPサーバの起動と停止
    $ chmod 755 /srv/tftp
    $ sudo /etc/init.d/tftpd-hpa start
    $ sudo /etc/init.d/tftpd-hpa stop

転送用のファイルを保存
    $ sudo echo "; Hello! This is from RasPi" | sudo tee /srv/tftp/tftpc_1.ini
    $ sudo echo "ADC_PIN=32" | sudo tee -a /srv/tftp/tftpc_1.ini
    $ sudo echo "SLEEP_SEC=50" | sudo tee -a /srv/tftp/tftpc_1.ini
    $ sudo chmod 644 /srv/tftp/tftpc_1.ini
    $ cat /srv/tftp/tftpc_1.ini
    ; Hello! This is from RasPi
    ADC_PIN=32
    SLEEP_SEC=50

その他
・開発時に下記ライブラリを使用しました(現在はESP32ライブラリに含まれています。)
　https://github.com/copercini/arduino-esp32-SPIFFS

【注意事項】
・TFTPクライアント(ESP側)やTFTPサーバ(PCやRaspberry Pi側)起動すると、各機器が
　セキュリティの脅威にさらされた状態となります。
・また、ウィルスやワームが侵入すると、同じネットワーク上の全ての機器へ感染する
　恐れが高まります。
・インターネットに接続すると外部からの侵入される場合があります。
・TFTPクライアントは少なくともローカルネット内のみで動作させるようにして下さい。
・TFTPが不必要なときは、停止させてください。
*******************************************************************************/

#include <WiFi.h>                           // ESP32用WiFiライブラリ
#include <WiFiUdp.h>                        // UDP通信を行うライブラリ
#include "esp_sleep.h"                      // ESP32用Deep Sleep ライブラリ
#define PIN_EN 2                            // GPIO 2(24番ピン)をセンサの電源に
int     PIN_AIN=34;                         // GPIO 34 ADC1_CH6(6番ピン)をADCに
#define SSID "1234ABCD"                     // ★要変更★無線LAN APのSSID
#define PASS "password"                     // ★要変更★パスワード
#define TFTP_SERV "192.168.0.123"           // ★要変更★TFTPサーバのIPアドレス
#define FILENAME  "tftpc_1.ini"             // ★要変更★転送ファイル名
#define SENDTO "192.168.0.255"              // UDP送信先のIPアドレス
#define PORT 1024                           // UDP送信のポート番号
uint32_t SLEEP_P=10*1000000;                // スリープ時間 10秒(uint32_t)
#define DEVICE "tftpc_1,"                   // デバイス名(5文字+"_"+番号+",")

void setup(){                               // 起動時に一度だけ実行する関数
    char data[512];                         // TFTPデータ用変数
    int len_tftp;                           // TFTPデータ長
    
    pinMode(PIN_EN,OUTPUT);                 // センサ用の電源を出力に
    Serial.begin(115200);                   // 動作確認のためのシリアル出力開始
    Serial.println("ESP32 eg.26 TFTP");     // 「ESP32 eg.26」をシリアル出力表示
    WiFi.mode(WIFI_STA);                    // 無線LANをSTAモードに設定
    WiFi.begin(SSID,PASS);                  // 無線LANアクセスポイントへ接続
    if(ini_init(data)) initialize(data);    // SPIFFSからINIファイルの読み込み
    while(WiFi.status() != WL_CONNECTED){   // 接続に成功するまで待つ
        delay(500);                         // 待ち時間処理
        digitalWrite(PIN_EN,!digitalRead(PIN_EN));      // LEDの点滅
        Serial.print(".");
    }
    Serial.println(WiFi.localIP());         // 本機のIPアドレスをシリアル出力
    tftpStart(TFTP_SERV, FILENAME);         // TFTPの開始
    do{
        len_tftp = tftpGet(data);           // TFTP受信(data=受信データ)
        if(len_tftp>0){
            initialize(data);               // INIファイル内の内容を変数へ代入
            ini_save(data);                 // INIファイルをSPIFFSへ書込み
            break;                          // 複数ブロックには対応しない
        }
    }while(len_tftp);
    pinMode(PIN_AIN,INPUT);                 // アナログ入力の設定
}

void initialize(char *data){
    /* INIファイル中のADC_PINの値をPIN_AINへ代入する */
    PIN_AIN=ini_parse(data,"ADC_PIN");
    if( PIN_AIN<0 || PIN_AIN==1 || PIN_AIN==3 || PIN_AIN>39
        || (PIN_AIN>4 && PIN_AIN<12)
        || (PIN_AIN>14 && PIN_AIN<25)
        || (PIN_AIN>27 && PIN_AIN<32)
        || (PIN_AIN>36 && PIN_AIN<39)
    ) PIN_AIN=34;
    /* INIファイル中のSLEEP_SECの値をSLEEP_Pへ代入する */
    SLEEP_P=(uint32_t)ini_parse(data,"SLEEP_SEC") * 1000000;
    if(SLEEP_P < 10 * 1000000) SLEEP_P=10 * 1000000;
}

void loop(){
    WiFiUDP udp;                            // UDP通信用のインスタンスを定義
    int adc;                                // 整数型変数adcを定義
    
    digitalWrite(PIN_EN,HIGH);              // センサ用の電源をONに
    delay(5);                               // 起動待ち時間
    adc=(int)mvAnalogIn(PIN_AIN);           // AD変換器から値を取得
    digitalWrite(PIN_EN,LOW);               // センサ用の電源をOFFに
    udp.beginPacket(SENDTO, PORT);          // UDP送信先を設定
    udp.print(DEVICE);                      // デバイス名を送信
    udp.println(adc);                       // 変数adcの値を送信
    Serial.print("ADC=");                   // シリアル出力表示
    Serial.println(adc);                    // シリアル出力表示
    udp.endPacket();                        // UDP送信の終了(実際に送信する)
    delay(200);                             // 送信待ち時間
    esp_deep_sleep(SLEEP_P);                // Deep Sleepモードへ移行
}

/* ADC入力用スケッチ */
float mvAnalogIn(uint8_t PIN){
    return mvAnalogIn(PIN, 0.0);            // 動作最小電圧 0.0 ～ 0.1(V)程度
//  return mvAnalogIn(PIN, 1.075584e-1);
}

float mvAnalogIn(uint8_t PIN, float offset){
    int in0,in3;
    float ad0,ad3;
    
    analogSetPinAttenuation(PIN,ADC_11db);
    in3=analogRead(PIN);
    
    if( in3 > 2599 ){
        ad3 = -1.457583e-7 * (float)in3 * (float)in3
            + 1.510116e-3 * (float)in3
            - 0.680858 + offset;
    }else{
        ad3 = 8.378998e-4 * (float)in3 + 8.158714e-2 + offset;
    }
    if( in3 < 200 ){
        analogSetPinAttenuation(PIN,ADC_0db);
        in0=analogRead(PIN);
        ad0 = 2.442116e-4 * (float)in0 + offset;
        if( in3 >= 100 ){
            ad3 = ad3 * ((float)in3 - 100.) / 100.
                + ad0 * (200. - (float)in3) / 100.;
        }else{
            ad3 = ad0;
        }
    }
    return ad3 * 1000.;
}
