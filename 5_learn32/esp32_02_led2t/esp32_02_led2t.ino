/***********************************************************************
Practice 2: 磁界や温度に応じて2個のLEDを点滅させる

・温度が上昇するとLED1が点灯し、下降すると消灯する
・N極の磁石を近づけると点灯し、S極だと点滅する

                                  Copyright (c) 2017-2019 Wataru KUNINO
***********************************************************************/

#define PIN_LED1 2                  // IO 2にLED1を接続
#define PIN_LED2 23                 // IO 23にLED2を接続

float temp=99.9;                    // 温度を保持するための変数定義

void setup() {                      // 起動時に一度だけ実行される関数
    pinMode(PIN_LED1,OUTPUT);       // LEDを接続したポートを出力に設定
    pinMode(PIN_LED2,OUTPUT);       // LEDを接続したポートを出力に設定
}

void loop() {                       // 繰り返し実行される関数
    float tp=temperatureRead();     // 内蔵の温度センサから値を取得
    int hall=hallRead();            // 内蔵のホールセンサから値を取得
    if( tp - temp > 1.0 ){          // 1℃を超える温度上昇があったとき
        digitalWrite(PIN_LED1,HIGH);// LED1をHレベル(3.3V)に設定（点灯）
        temp=tp;                    // 今回の測定温度を保存
    }
    if( temp - tp > 1.0 ){          // 1℃を超える温度下降があったとき
        digitalWrite(PIN_LED1,LOW); // LED1をLレベル(0V)に設定（消灯）
        temp=tp;                    // 今回の測定温度を保存
    }
    if( hall < 30 ){                // 磁石のN極を近づけたとき(磁界－)
        digitalWrite(PIN_LED2,HIGH);// LED2をHレベル(3.3V)に設定（点灯）
    }else if( hall > 60 ){          // 磁石のS極を近づけたとき(磁界＋)
        digitalWrite(PIN_LED2,HIGH);// LED2をHレベル(3.3V)に設定（点灯）
        delay(100);                 // 時間待ち(100ms)
        digitalWrite(PIN_LED2,LOW); // LED2をLレベル(0V)に設定（消灯）
    }else{                          // 磁石が近くに無いとき
        digitalWrite(PIN_LED2,LOW); // LED2をLレベル(0V)に設定（消灯）
    }
    delay(100);                     // 時間待ち(100ms)
}
