/***********************************************************************
Practice 2: 2個のLEDを点滅させる
                                  Copyright (c) 2016-2019 Wataru KUNINO
***********************************************************************/

#define PIN_LED1 2              // IO 2にLED1を接続
#define PIN_LED2 23             // IO 23にLED2を接続

void setup() {                  // 起動時に一度だけ実行される関数
    pinMode(PIN_LED1,OUTPUT);   // LEDを接続したポートを出力に設定する
    pinMode(PIN_LED2,OUTPUT);   // LEDを接続したポートを出力に設定する
}

void loop() {                   // setup実行後に繰り返し実行される関数
    if(hallRead() < 30)return;  // ホールセンサ値30以下時は実行しない
    digitalWrite(PIN_LED1,HIGH);// LED1をHレベル(3.3V)に設定（点灯）
    digitalWrite(PIN_LED2,LOW); // LED2をLレベル(0V)に設定（消灯）
    delay(100);                 // 時間待ち(100ms)
    digitalWrite(PIN_LED1,LOW); // LED1をLレベル(0V)に設定（消灯）
    digitalWrite(PIN_LED2,HIGH);// LED2をHレベル(3.3V)に設定（点灯）
    delay(100);                 // 時間待ち(100ms)
}
