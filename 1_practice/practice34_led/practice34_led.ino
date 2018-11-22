/***********************************************************************
Practice 34(Practice 2+32): LEDを点滅させる

GPIO 2 へ LEDを接続してください。
                                  Copyright (c) 2016-2019 Wataru KUNINO
***********************************************************************/

#define PIN_LED 2               // IO 2にLEDを接続

void setup() {                  // 起動時に一度だけ実行される関数
    pinMode(PIN_LED,OUTPUT);    // LEDを接続したポートを出力に設定する
}

void loop() {                   // setup実行後に繰り返し実行される関数
    digitalWrite(PIN_LED,HIGH); // ポートをHレベル(3.3V)に設定（点灯）
    delay(200);                 // 時間待ち(200ms)
    digitalWrite(PIN_LED,LOW);  // ポートをLレベル(0V)に設定（消灯）
    delay(200);                 // 時間待ち(200ms)
}
