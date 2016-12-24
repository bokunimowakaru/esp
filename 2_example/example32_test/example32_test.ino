/*******************************************************************************
最新IoTモジュール ESP-WROOM-32用 スイッチ早押し
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

※Ambientのライトキーの取り扱いには十分にご注意ください。
※不正な取り扱いが行われた場合は、実験を中止いたします。
※送信したデータは、インターネット上へ公開されます。

                                           Copyright (c) 2016-2017 Wataru KUNINO
*******************************************************************************/

#include <WiFi.h>                           // ESP32用WiFiライブラリ
#define PIN_LED 2                           // IO 2 にLEDを接続
#define PIN_SW 0                            // IO 0 にスイッチを接続
#define AmbientChannelId 725                // チャネルID(整数)
#define AmbientWriteKey "ad3e53b54fe16764"  // ライトキー(16桁の16進数)
#define HOST "54.65.206.59"                 // 
#define SSID "1234ABCD"                     // 無線LANアクセスポイントのSSID
#define PASS "password"                     // パスワード

void setup(){                               // 起動時に一度だけ実行する関数
    pinMode(PIN_LED,OUTPUT);                // LEDを接続したポートを出力に
    pinMode(PIN_SW,INPUT_PULLUP);           // スイッチを接続したポートを入力に
    Serial.begin(9600);                     // 動作確認のためのシリアル出力開始
    Serial.println("Example Ambient SW");   // 「Example 02」をシリアル出力表示

    WiFi.mode(WIFI_STA);                    // 無線LANをSTAモードに設定
    WiFi.begin(SSID,PASS);                  // 無線LANアクセスポイントへ接続
    while(WiFi.status() != WL_CONNECTED){   // 接続に成功するまで待つ
        delay(500);                         // 待ち時間処理
        Serial.print('.');
    }Serial.println();
    Serial.println(WiFi.localIP());         // 本機のIPアドレスをシリアル出力
}

void loop(){                                // 繰り返し実行する関数
    WiFiClient client;
    unsigned long t0,t1;
    char s[64],data[64],http[384];

    while(digitalRead(PIN_SW));             // Hレベル(スイッチ解放)時に繰り返し
    t0=millis();
    while(!digitalRead(PIN_SW));            // ボタンが開放されるまで待つ
    digitalWrite(PIN_LED,HIGH);
    delay(8);                               // 8msの待ち時間処理を実行
    while(digitalRead(PIN_SW));             // Hレベル(スイッチ解放)時に繰り返し
    t1=millis();
    delay(100);
    while(!digitalRead(PIN_SW));            // チャタリング防止措置
    dtostrf(1000.L/(double)(t1-t0),5,2,s);
    sprintf(data,"{\"writeKey\":\"%s\",\"d1\":\"%s\"}",AmbientWriteKey,s);
    Serial.print(s);
    Serial.println(" Hz");
    if(!client.connect(HOST,80)){
        Serial.println("connection failed");
        return;
    }
    sprintf(http,"POST /api/v2/channels/%d/data HTTP/1.0\r\n",AmbientChannelId);
    sprintf(s,"Host: %s\r\n",HOST);
    strcat(http,s);
    sprintf(s,"Content-Length: %d\r\n",strlen(data));
    strcat(http,s);
    strcat(http,"Content-Type: application/json\r\n");
    strcat(http,"\r\n");
    strcat(http,data);
    client.print(http);
    delay(30);
    client.stop();                          // クライアントの切断
    digitalWrite(PIN_LED,LOW);
}
