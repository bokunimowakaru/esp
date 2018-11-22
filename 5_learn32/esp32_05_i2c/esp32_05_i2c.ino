/*******************************************************************************
Practice 5: I2Cインタフェース接続 湿度センサ
                                          Copyright (c) 2017-2019 Wataru KUNINO
*******************************************************************************/

#define PIN_EN 2                        // IO 2をセンサの電源に

void setup() {
    pinMode(PIN_EN,OUTPUT);             // センサ用の電源を出力に
    digitalWrite(PIN_EN,HIGH);          // センサ用の電源をONに
    shtSetup();
    Serial.begin(115200);               // シリアル通信速度を115200bpsに設定する
}

void loop() {
    float temp,hum;                     // 温度値と湿度値を保持する変数を定義
    temp=getTemp();                     // 温度値を取得し、変数tempへ代入
    hum =getHum();                      // 湿度値を変数humへ代入
    Serial.print(temp,2);               // 変数tempの値をシリアル出力
    Serial.print(", ");                 // カンマとスペースをシリアル出力
    Serial.println(hum,2);              // 変数tempの値をシリアル出力
    delay(3000);                        // 3秒間(3000ms)の待ち時間処理
}
