/*******************************************************************************
Practice 5: 良く使う演算子

シリアルモニタのビット・レート設定を115200 bpsに設定してください。
                                           Copyright (c) 2015-2019 Wataru KUNINO
*******************************************************************************/

void setup() {                          // 起動時に一度だけ実行される関数
    Serial.begin(115200);               // シリアル通信速度を115200bpsに設定する
}

void loop() {                           // setup実行後に繰り返し実行される関数
    int a = 12345;                      // 整数型の変数aを定義
    float v;                            // 浮動小数点数型変数v
    
    Serial.println("Practice 04");      // 「Practice 04」を表示
    
    Serial.print(a,DEC);                // 整数値変数aの値を表示
    a = a - 345;                        // a-345を計算してaに代入
    Serial.print(" - 345 = ");
    Serial.println(a,DEC);              // 整数値変数aの値を表示
    
    Serial.print(a,DEC);                // 整数値変数aの値を表示
    a = a / 1000;                       // a÷1000をaに代入
    Serial.print(" / 1000 = ");
    Serial.println(a,DEC);              // 整数値変数aの値を表示

    v = (float) a;                      // 浮動小数変数vにaを代入
    Serial.print("(int) ");
    Serial.print(a,DEC);                // 整数値変数aの値を表示
    a = a / 10;                         // a÷10をaに代入
    Serial.print(" / 10 = ");
    Serial.println(a,DEC);              // 整数値変数aの値を表示
    
    Serial.print("(float) ");
    Serial.print(v,3);                  // 浮動小数点数型変数vの値を表示
    v = v / 10;                         // v÷10をvに代入
    Serial.print(" / 10 = ");
    Serial.println(v,3);                // 浮動小数点数型変数vの値を表示

    for(a=0;a<10;a++) delay(100);       // 1秒の待ち時間処理
    Serial.println();
}
