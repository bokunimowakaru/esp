/*******************************************************************************
Example 29: MACブロードキャスト／ Amazon Dash ボタン検出

リスト中の特定のMACアドレスの端末のブロードキャストをプロミスキャスモードで
待ち受け、検出時にUART出力します。

スマートフォンやPCを自宅に持ち帰り無線LANへ自動接続するときや、Amazon Dashボタン
が押下されたときなどに、送信するMACブロードキャストを検出します。

同じ機器から同種別のデータを連続して受信した場合は出力しません(待機時間0.5秒)。
チャンネルを変更するには、シリアルから「channel=数字」と改行を入力してください。

                                           Copyright (c) 2017-2018 Wataru KUNINO
********************************************************************************
・シリアル速度は115200bpsです。

・出力形式：先頭に'(0x27)とスペース(0x20)に続いて6桁のMACアドレスをテキスト出力

            ' xx:xx:xx:xx:xx:xx
            ' xx:xx:xx:xx:xx:xx
*******************************************************************************/
#include <ESP8266WiFi.h>                    // ESP8266用ライブラリ
extern "C" {
#include "user_interface.h"                 // ESP8266用の拡張IFライブラリ
}
#include <WiFiUdp.h>                        // UDP通信を行うライブラリ
#define PIN_EN 13                           // IO 13 をLEDなどへ接続
#define PIN_HOLD 500                        // 検出時の保持時間を設定(500ms)
int channel;                                // 無線LAN物理チャンネル
unsigned long reset_time;                   // LED消灯時刻
char uart[17];                              // UART受信バッファ
byte uart_n=0;                              // UART受信文字数
byte mac[6];                                // プロミスキャス受信デバイスのMAC

void setup(){                               // 起動時に一度だけ実行する関数
    int len;                                // 設定データ長
    pinMode(PIN_EN,OUTPUT);                 // LEDなど用の出力
    Serial.begin(115200);                   // 動作確認のためのシリアル出力開始
    WiFi.mode(WIFI_STA);                    // 無線LANをSTAモードに設定
    channel=wifi_get_channel();             // チャンネルを取得
    promiscuous_uart(false);                // ライブラリ側のUART出力を無効に
    promiscuous_start(channel);             // プロミスキャスモードへ移行する
    memset(uart,0,17);                      // UART受信バッファの初期化
}

void loop(){
    while(promiscuous_get_mac(mac)){        // 検知時
        Serial.print("' ");
        for (int i=0; i<6; i++){            // MACアドレス6バイトを繰り返し
            if(mac[i] < 0x10) Serial.print(0);
            Serial.print(mac[i], HEX);      // MACアドレスを表示
            if(i != 5)Serial.print(":");
        }
        Serial.println();                   // 改行を出力
        digitalWrite(PIN_EN,HIGH);          // LEDなどを点灯
        reset_time=(millis()-1)%PIN_HOLD;   // 消灯時刻を設定
    }
    if(Serial.available()){                 // UART受信時
        char c=Serial.read();               // 受信文字を変数cで保持
        if( c=='\r' || c=='\n' ){           // 改行コードの時
            if(!strncmp(uart,"channel=",8)){// チャンネル設定コマンドの時
                promiscuous_stop();         // プロミスキャスを停止
                channel=atoi(uart+8);       // チャンネルを変更
                if(channel<1 || channel>12) channel=(channel%12)+1;
                Serial.print("' adash channel=");
                Serial.println(channel);
                promiscuous_start(channel); // プロミスキャスモードへ移行する
                promiscuous_ready();        // 検知処理完了
            }
            memset(uart,0,17);
            uart_n=0;
            return;
        }
        uart[uart_n]=c;
        uart_n++;
        if(uart_n>15)uart_n=15;             // 最大値は15(16文字)
    }
    if(millis()%PIN_HOLD != reset_time) return;
    digitalWrite(PIN_EN,LOW);               // LEDなどを点灯
    promiscuous_ready();                    // 検知処理完了
}
