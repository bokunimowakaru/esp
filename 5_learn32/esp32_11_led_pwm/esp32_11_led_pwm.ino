/*******************************************************************************
Practice esp32 11 led PWM 【Wi-Fi インジケータ親機 UDP版】

・IO 2に電流制限抵抗100Ωと赤色LED OSDR3133Aを接続して実験を行います
・電流制限抵抗が1kΩの時は、#define LED_MAXを8000に修正してください
　（LED_MAXはPWMの変更幅で、上限は8191です。高いほど点灯時の輝度が上がります）

                                           Copyright (c) 2016-2019 Wataru KUNINO
*******************************************************************************/
#include <WiFi.h>                           // ESP32用WiFiライブラリ
#include <WiFiUdp.h>                        // UDP通信を行うライブラリ
#define PIN_LED 2                           // GPIO 2にLEDを接続
#define PMW_CH 0                            // LEDのPWM駆動用のチャンネル番号
#define LED_MAX 400                         // LEDの最大輝度(8191以下)
#define SSID_AP "1234ABCD"                  // 本機の無線アクセスポイントのSSID
#define PASS_AP "password"                  // パスワード
#define PORT 1024                           // 受信ポート番号
WiFiUDP udp;                                // UDP通信用のインスタンスを定義
int led=0;                                  // LEDの輝度値保持用の変数を定義

void setup(){                               // 起動時に一度だけ実行する関数
    pinMode(PIN_LED,OUTPUT);                // LEDを接続したポートを出力に
    ledcSetup(PMW_CH, 5000, 13);            // PWM用設定 PMW_CH 0ch, 5kHz, 13bit
    ledcAttachPin(PIN_LED, PMW_CH);         // PWM 0chをPIN_LEDへ割り当てる
    Serial.begin(115200);                   // 動作確認のためのシリアル出力開始
    Serial.println("Practice32 11 led");    // Practice32 01 ledをシリアル出力
    WiFi.mode(WIFI_AP); delay(100);         // 無線LANを【AP】モードに設定
    WiFi.softAP(SSID_AP);                   // ソフトウェアAPの起動
    WiFi.softAPConfig(
        IPAddress(192,168,0,1),             // 固定IPアドレス
        IPAddress(0,0,0,0),                 // 本機のゲートウェイアドレス(なし)
        IPAddress(255,255,255,0)            // ネットマスク
    );
    udp.begin(PORT);                        // UDP待ち受け開始
    Serial.println(SSID_AP);                // 本APのSSIDをシリアル出力
    Serial.println(WiFi.softAPIP());        // 本APのIPアドレスをシリアル出力
}

void loop(){                                // 繰り返し実行する関数
    char s[65];                             // 文字列変数を定義 65バイト64文字
    int len = udp.parsePacket();            // UDP受信パケット長を変数lenに代入
    if(len==0)return;                       // TCPとUDPが未受信時にloop()先頭へ
    memset(s, 0, 65);                       // 文字列変数sの初期化(65バイト)
    udp.read(s, 64);                        // UDP受信データを文字列変数sへ代入
    Serial.println(s);                      // 受信データを表示する
    
    if(!strncmp(s,"Test",4)){               // 受信データの先頭4文字がTestのとき
        if(led == 0){                       // 消灯時
            strcpy(s,"Ping");               // コマンドをPingに書き換える
        }else{                              // 点灯時
            strcpy(s,"Pong");               // コマンドをPongに書き換える
        }
    }
    if(!strncmp(s,"Ping",4)){               // 受信データの先頭4文字がPingのとき
        for(led=0; led<LED_MAX; led++){     // LED輝度を0から400まで徐々に上げる
            ledcWrite(PMW_CH,led);          // LEDへ輝度を設定
            delay(5);                       // LEDを徐々に変化させる為の待ち時間
        }
    }
    if(!strncmp(s,"Pong",4)){               // 受信データの先頭4文字がPongのとき
        for(led=LED_MAX; led>0; led--){     // LED輝度を400から1まで徐々に下げる
            ledcWrite(PMW_CH,led);          // LEDへ輝度を設定
            delay(5);                       // LEDを徐々に変化させる為の待ち時間
        }
    }
    if(!strncmp(s,"level_",6) && isalnum(s[6]) && s[7]==','){
        led=atoi(s+8);                      // UDPからの入力値をledへ代入
        if(led < 0) led=0;                  // ledの下限値0を下回らないように
        if(led > LED_MAX) led=LED_MAX;      // ledの輝度の最大値を400に
    }
    if(!strncmp(s,"onoff_",6) || !strncmp(s,"voice_",6)){
        if(isalnum(s[6]) && s[7]==',') led = LED_MAX * (atoi(s+8) & 1);
    }
    ledcWrite(PMW_CH,led);                  // LEDへ輝度を設定
}
