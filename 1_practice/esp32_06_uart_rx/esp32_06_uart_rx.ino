/*******************************************************************************
Practice 5: UARTでLED制御コマンドを受信する
                                          Copyright (c) 2017-2018 Wataru KUNINO
*******************************************************************************/

#define PIN_LED 2                           // GPIO 2にLEDを接続

void setup(){                               // 起動時に一度だけ実行する関数
    pinMode(PIN_LED,OUTPUT);                // LEDを接続したポートを出力に
    Serial.begin(115200);                   // 動作確認のためのシリアル出力開始
    Serial.println("Practice32 06 uart RX"); // Practice32 06をシリアル出力
}

void loop(){                                // 繰り返し実行する関数
    char s[65];                             // 文字列変数を定義 65バイト64文字
    int i,len = Serial.available();         // 受信データ長を変数lenに代入
    if(len==0)return;                       // 未受信時にloop()先頭へ
    if(len>64) len=64;                      // 最大受信長は64文字
    for(i=0;i<len;i++) s[i]=Serial.read();  // UARTシリアルからデータを受信
    s[i]='\0';                              // 受信データの終了を代入
    Serial.println(s);                      // 受信データを表示する
    
    if(!strncmp(s,"Ping",4)){               // 受信データの先頭4文字がPingのとき
        digitalWrite(PIN_LED,HIGH);         // LEDの点灯
    }
    if(!strncmp(s,"Pong",4)){               // 受信データの先頭4文字がPongのとき
        digitalWrite(PIN_LED,LOW);          // LEDの消灯
    }
}
