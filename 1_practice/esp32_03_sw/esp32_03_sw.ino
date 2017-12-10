/*******************************************************************************
Practice 3: スイッチ入力
                                          Copyright (c) 2016-2018 Wataru KUNINO
*******************************************************************************/

#define PIN_LED 2                       // IO 2にLEDを接続
#define PIN_SW 0                        // IO 0にスイッチを接続

boolean button=1;                       // スイッチ状態を保存するための変数

void setup() {                          // 起動時に一度だけ実行される関数
    pinMode(PIN_SW,INPUT_PULLUP);       // スイッチを接続したポートを入力に設定
    pinMode(PIN_LED,OUTPUT);            // LEDを接続したポートを出力に設定する
    Serial.begin(115200);               // シリアル通信速度を115200bpsに設定する
}

void loop() {                           // setup実行後に繰り返し実行される関数
    boolean b=button;                   // 前回のスイッチ状態を変数bに保持
    
    while(b==button){                   // 前回の値bと同じ値の間に繰り返し実行
        button = digitalRead(PIN_SW);   // スイッチ状態の読み取り
    }                                   // 異なる値となったときに繰り返しを終了
    if(button==0){                      // スイッチが押されLOWレベルだった時
        Serial.println("Ping");         // シリアルへPingを出力表示
        digitalWrite(PIN_LED,1);        // ポートをHレベル(3.3V)に設定（点灯）
    }else{
        Serial.println("Pong");         // そうでないときに「Pong」を出力表示
        digitalWrite(PIN_LED,0);        // ポートをLレベル(0V)に設定（点灯）
    }
    delay(10);                          // 10msの時間待ち(チャタリング防止策)
}
