/*******************************************************************************
Practice 3: 変数の使い方
                                           Copyright (c) 2015-2016 Wataru KUNINO
*******************************************************************************/

void setup() {                          // 起動時に一度だけ実行される関数
    Serial.begin(9600);                 // シリアル通信速度を9600bpsに設定する
}

void loop() {                           // setup実行後に繰り返し実行される関数
    int a = 12345;                      // 整数型の変数aを定義
    char c = 'R';                       // 文字変数cを定義
    char s[] = "Hello, world!";         // 文字列変数sを定義
    float v = 1.2345;                   // 浮動小数点数型変数v
    int size;                           // 変数sizeを定義

    Serial.println("Practice 03");      // 「Practice 03」を表示

    Serial.print("a=");
    Serial.println(a,DEC);              // 整数値変数aの値を表示

    Serial.print("c=");
    Serial.write(c);                    // 文字変数cの値を表示
    Serial.println();                   // 改行する

    Serial.print("s=");
    Serial.println(s);                  // 文字列変数sの値を表示

    size = sizeof(s);                   // sのサイズをsizeに代入
    Serial.print("sizeof(s)=");
    Serial.println(size,DEC);           // sizeの値を表示して改行

    Serial.print("v=");
    Serial.println(v,3);                // 浮動小数点数型変数vの値を表示

    for(a=0;a<10;a++) delay(100);       // 1秒の待ち時間処理
    Serial.println();
}
