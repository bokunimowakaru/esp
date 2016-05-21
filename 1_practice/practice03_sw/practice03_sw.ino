/*******************************************************************************
Practice 3: スイッチ入力
                                   Copyright (c) 2013-2016 Wataru KUNINO
*******************************************************************************/

#define PIN_SW 4                        // IO 4(10番ピン) にスイッチを接続する
#define PIN_LED 13                      // IO 13(5番ピン) にLEDを接続する
int prev = 0;                           // 整数型変数prevを定義し、初期値0を代入

void setup() {                          // 起動時に一度だけ実行される関数
    pinMode(PIN_SW,INPUT_PULLUP);       // スイッチを接続したポートを入力に設定
    pinMode(PIN_LED,OUTPUT);            // LEDを接続したポートを出力に設定する
    Serial.begin(9600);                 // シリアル通信速度を9600bpsに設定する
}

void loop() {                           // setup実行後に繰り返し実行される関数
    int in;                             // 整数型変数inを定義
    
    in = digitalRead(PIN_SW);           // デジタル入力値を変数inに代入
    
    if( in == 0 ){                      // in値が0、すなわちLレベル(0V)入力の時
        digitalWrite(PIN_LED,1);        // ポートをHレベル(3.3V)に設定（点灯）
    }else{                              // そうでは無い時
        digitalWrite(PIN_LED,0);        // ポートをLレベル(0V)に設定（点灯）
    }
    
    if( in != prev ){                   // inの値がprevと異なる時
        Serial.println(in);             // inの値をシリアル出力
        prev = in;                      // prevにin値を代入
    }
}
