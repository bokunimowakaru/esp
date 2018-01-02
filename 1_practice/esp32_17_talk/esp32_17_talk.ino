/*******************************************************************************
Practice esp32 17 talk 【Wi-Fi 音声アナウンス親機・子機】

    AquosTalk接続用
    TXD0(35番ピン U0TXD)、AquosTalk側はRXD端子(2番ピン)

                                           Copyright (c) 2016-2017 Wataru KUNINO
*******************************************************************************/

#include <WiFi.h>                           // ESP32用WiFiライブラリ
#include <WiFiUdp.h>                        // UDP通信を行うライブラリ
#define PIN_LED 2                           // IO 2にLEDを接続
#define PIN_SW 0                            // IO 0にスイッチを接続
#define SSID "1234ABCD"                     // 無線LANアクセスポイントのSSID
#define PASS "password"                     // パスワード
#define SSID_AP "1234ABCD"                  // 本機の無線アクセスポイントのSSID
#define PASS_AP "password"                  // パスワード
#define SENDTO "192.168.0.255"              // 送信先のIPアドレス
#define PORT 1024                           // 受信ポート番号
#define DEVICE "atalk_1,"                   // 子機デバイス名(5文字+"_"+ID+",")

WiFiUDP udp;                                // UDP通信用のインスタンスを定義
int mode;                                   // Wi-Fiモード 0:親機AP 1:子機STA

void setup(){                               // 起動時に一度だけ実行する関数
    pinMode(PIN_SW,INPUT_PULLUP);           // スイッチを接続したポートを入力に
    pinMode(PIN_LED,OUTPUT);                // LEDを接続したポートを出力に
    Serial.begin(9600);                     // AquesTalkとの通信ポート(9600bps)
    Serial.print("\r$"); delay(100);        // ブレークコマンドを出力する
    Serial.print("?kon'nnichi/wa.\r");      // 音声「こんにちわ」を出力する
    WiFi.mode(WIFI_STA);                    // 無線LANを【子機】モードに設定
    WiFi.begin(SSID,PASS);                  // 無線LANアクセスポイントへ接続
    while(WiFi.status() != WL_CONNECTED){   // 接続に成功するまで待つ
        digitalWrite(PIN_LED,!digitalRead(PIN_LED));    // LEDの点滅
        if(millis()>10000 || !digitalRead(PIN_SW)){     // 10秒経過orボタン押下
            WiFi.disconnect();              // WiFiアクセスポイントを切断する
            while(millis()<2000);           // 音声「こんにちわ」の終了待ち
            break;                          // whileを抜ける
        }
        delay(100);                         // 待ち時間処理(LED点滅用)
    }
    
    if(WiFi.status() == WL_CONNECTED){      // 接続に成功した時
        Serial.print("ko'kidesu.\r");       // 音声「子機です」を出力する
        mode=1;                             // 子機モードであることを保持
    }else{
        Serial.print("oya'kidesu.\r");      // 音声「親機です」を出力する
        WiFi.mode(WIFI_AP); delay(100);     // 無線LANを【親機】モードに設定
        WiFi.softAP(SSID_AP,PASS_AP);       // ソフトウェアAPの起動
        WiFi.softAPConfig(
            IPAddress(192,168,0,1),         // AP側の固定IPアドレス
            IPAddress(0,0,0,0),             // 本機のゲートウェイアドレス
            IPAddress(255,255,255,0)        // ネットマスク
        );
        mode=0;                             // 親機モードであることを保持
    }
    delay(2000);                            // 音声「親機／子機です」の終了待ち
    Serial.print("<NUM VAL=");              // 数字読み上げ用タグ出力
    if(mode) Serial.print(WiFi.localIP());  // 子機IPアドレスを読み上げる
      else   Serial.print(WiFi.softAPIP()); // 親機IPアドレスを読み上げる
    Serial.print(">.\r");                   // タグの終了を出力する
    udp.begin(PORT);                        // UDP待ち受け開始(STA側+AP側)
}

void loop(){                                // 繰り返し実行する関数
    char s[57],talk[49];                    // 文字列変数sとtalkを定義
    int len = udp.parsePacket();            // UDP受信パケット長を変数lenに代入
    if(len==0) return;                      // TCPとUDPが未受信時にloop()先頭へ
    
    memset(s, 0, 57);                       // 文字列変数sの初期化(57バイト)
    memset(talk, 0, 49);                    // 文字列変数talkの初期化(49バイト)
    udp.read(s, 56);                        // UDP受信データを文字列変数sへ代入
    for(int i=0;i<16;i++) if(iscntrl(s[i]))s[i]=0; // 制御文字をNullへ置き換え
    if(!strncmp(s,"Ping",4)) Serial.print("yobi'rinnga/osarema'_shita.\r");
    if(len<8 || !isalnum(s[6]) || s[7]!=',' ) return;   // フォーマット外を排除
    if(!strncmp(s,DEVICE,8) || (mode>0 && !strncmp(s,"atalk_0,",8)) ){
        Serial.print(s+8);                  // AquesTalkへ出力する
        Serial.print('\r');                 // 改行コード（CR）を出力する
    }
    if(mode==0) return;                     // 親機の時だけ以下を実施
    if(!strncmp(s,"onoff_",6)) strcpy(talk,"bo'tanga/osarema'_shita.");
    if(!strncmp(s,"pir_s_",6)) strcpy(talk,"jinn'kannse'nnsaga/hannno-.");
    if(!strncmp(s,"rd_sw_",6)) strcpy(talk,"do'aga/hirakima'_shita.");
    if(!strncmp(s,"alarm_",6)) strcpy(talk,"yotei'no/ji'kandesu.");
    if(!strncmp(s,"voice_",6)) strcpy(talk,"o'nse'iwo/jushinshima'_shita.");
    if(!strlen(talk)) return;               // talkに代入されていれば以下を実行
    Serial.print(talk);                     // AquesTalkへ出力する
    Serial.print('\r');                     // 改行コード（CR）を出力する
    udp.beginPacket(SENDTO, PORT);          // UDP送信先を設定
    udp.print("atalk_0,");                  // 親機デバイス名を送信
    udp.println(talk);                      // 音声用データを他の子機へ送信
    udp.endPacket();                        // UDP送信の終了(実際に送信する)
    delay(10);                              // 送信待ち時間
}
