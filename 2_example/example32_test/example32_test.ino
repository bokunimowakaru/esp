/*******************************************************************************
最新IoTモジュール ESP32-WROOM-32用 スイッチ早押し
********************************************************************************

ボタン（BOOT GPIO2）を2回押したときの速度をAmbientへ送信する実験です。

        IoTセンサ用クラウドサービスAmbient
                https://ambidata.io/ch/channel.html?id=725

        関連ブログ（筆者）
                http://blogs.yahoo.co.jp/bokunimowakaru/55622707.html

本スケッチ内の#defineのSSIDとPASSを、お手持ちの無線LANアクセスポイントに合わせて
修正してから、Arduino IDEを使ってESP-WROOM-32へ書き込んでください。
起動後、ボタンを2回押すと、速度値（Hz）がAmbientへ送信されます。上記のAmbientの
ウェブサイトで確認することが出来ます。

※シリアル通信の速度をESP32デフォルト設定の115.2kbpsへ変更しました。

※Ambientのライトキーの取り扱いには十分にご注意ください。
※不正な取り扱いが行われた場合は、実験を中止いたします。
※送信したデータは、インターネット上へ公開されます。

                                           Copyright (c) 2016-2017 Wataru KUNINO
*******************************************************************************/

#include <WiFi.h>                           // ESP32用WiFiライブラリ
#define PIN_LED 2                           // GPIO 2(24番ピン)にLEDを接続
#define PIN_SW 0                            // GPIO 0(25番ピン)にスイッチを接続
#define AmbientChannelId 725                // チャネルID(整数)
#define AmbientWriteKey "ad3e53b54fe16764"  // ライトキー(16桁の16進数)
#define HOST "54.65.206.59"                 // AmbientのIPアドレス(変更しない)
#define SSID "1234ABCD"                     // 無線LANアクセスポイントのSSID
#define PASS "password"                     // パスワード

void setup(){                               // 起動時に一度だけ実行する関数
    pinMode(PIN_LED,OUTPUT);                // LEDを接続したポートを出力に
    pinMode(PIN_SW,INPUT_PULLUP);           // スイッチを接続したポートを入力に
    Serial.begin(115200);                   // 動作確認のためのシリアル出力開始
    Serial.println("Example Ambient SW");   // 「Example 02」をシリアル出力表示

    WiFi.mode(WIFI_STA);                    // 無線LANをSTAモードに設定
    delay(10);                              // ESP32に必要な待ち時間
    WiFi.begin(SSID,PASS);                  // 無線LANアクセスポイントへ接続
    while(WiFi.status() != WL_CONNECTED){   // 接続に成功するまで待つ
        delay(500);                         // 待ち時間処理
        Serial.print('.');                  // 「.」をシリアル出力表示
    }Serial.println();                      // 改行をシリアル出力表示
    Serial.println(WiFi.localIP());         // 本機のIPアドレスをシリアル出力
}

void loop(){                                // 繰り返し実行する関数
    WiFiClient client;                      // 開発中のWiFiClientを使用してみる
    unsigned long t0,t1;                    // ボタン押下・解放時刻を保持
    char s[64],data[64],http[384];

    while(digitalRead(PIN_SW));             // Hレベル(スイッチ解放)時に繰り返し
    t0=millis();                            // 1回目のボタン押し時刻を保持
    while(!digitalRead(PIN_SW));            // ボタンが開放されるまで待つ
    digitalWrite(PIN_LED,HIGH);             // LEDを点灯
    delay(8);                               // 8msの待ち時間処理を実行
    while(digitalRead(PIN_SW));             // Hレベル(スイッチ解放)時に繰り返し
    t1=millis();                            // 2回目のボタン押し時刻を保持
    delay(100);                             // チャタリング防止措置
    while(!digitalRead(PIN_SW));            // 意図しない繰り返し処理の防止
    dtostrf(1000.L/(double)(t1-t0),-5,2,s); // 以下、送信データ作成
    sprintf(data,"{\"writeKey\":\"%s\",\"d1\":\"%s\"}",AmbientWriteKey,s);
    Serial.print(s);
    Serial.println(" Hz");
    sprintf(http,"POST /api/v2/channels/%d/data HTTP/1.0\r\n",AmbientChannelId);
    sprintf(s,"Host: %s\r\n",HOST);
    strcat(http,s);
    sprintf(s,"Content-Length: %d\r\n",strlen(data));
    strcat(http,s);
    strcat(http,"Content-Type: application/json\r\n");
    strcat(http,"\r\n");
    strcat(http,data);
    if(!client.connect(HOST,80)){           // AmbientへTCP接続を実行
        Serial.println("connection failed");    // 接続失敗表示
        return;                                 // loop関数の先頭へ戻る
    }
    client.print(http);                     // TCPによるデータ送信
    delay(30);                              // 送信完了待ち
    client.stop();                          // クライアントの切断
    digitalWrite(PIN_LED,LOW);              // LEDの消灯
}
