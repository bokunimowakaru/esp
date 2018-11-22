/*******************************************************************************
Practice 4: アナログ入力
                                          Copyright (c) 2016-2019 Wataru KUNINO
*******************************************************************************/

/*
関数mvAnalogInは、指定したIOポートについてAD変換と電圧の補正を行い、電圧値(mV)を
応答します。低電圧時にはアッテネータ切り替えも行うので、より低い電圧まで測定する
ことが出来ます。
ESP32マイコンの製造ばらつきによる補正を行いたい場合は、第2引数に0.05～0.10の範囲
で補正値を入力します。省略時は0.108が入力されます。今後、ESP32マイコンや開発環境
側のライブラリ等で改良されることも考えられるので、必ずしも改善されるとは限りませ
んが、所要の特性が得られない場合などにお試しください。
*/

#define PIN_AIN 33                      // アナログ入力 IO 33をADC入力として使用

void setup() {                          // 起動時に一度だけ実行される関数
    pinMode(PIN_AIN,INPUT);             // アナログ入力端子の設定
    Serial.begin(115200);               // シリアル通信速度を115200bpsに設定する
}

void loop() {                           // setup実行後に繰り返し実行される関数
    int adc;                            // AD変換値保存用の整数型変数adcを定義

    adc=(int)mvAnalogIn(PIN_AIN, 0.0);  // AD変換器から値を取得
    Serial.println(adc);                // 変数adcの値をシリアル出力表示
    delay(3000);                        // 3秒間(3000ms)の待ち時間処理
}

float mvAnalogIn(uint8_t PIN){
    return mvAnalogIn(PIN, 0.0);        // 動作最小電圧 0.0 ～ 0.1(V)程度
}

float mvAnalogIn(uint8_t PIN, float offset){
    int in0,in3;
    float ad0,ad3;
    
    analogSetPinAttenuation(PIN,ADC_11db);
    in3=analogRead(PIN);
    
    if( in3 > 2599 ){
        ad3 = -1.457583e-7 * (float)in3 * (float)in3
            + 1.510116e-3 * (float)in3
            - 0.680858 + offset;
    }else{
        ad3 = 8.378998e-4 * (float)in3 + 8.158714e-2 + offset;
    }
    Serial.print("ADC (ATT=3;11dB) = ");
    Serial.print(ad3,3);
    Serial.print(" [V], ");
    Serial.println(in3);
    if( in3 < 200 ){
        analogSetPinAttenuation(PIN,ADC_0db);
        in0=analogRead(PIN);
        ad0 = 2.442116e-4 * (float)in0 + offset;
        Serial.print("ADC (ATT=0; 0dB) = ");
        Serial.print(ad0,3);
        Serial.print(" [V], "); 
        Serial.println(in0);
        if( in3 >= 100 ){
            ad3 = ad3 * ((float)in3 - 100.) / 100.
                + ad0 * (200. - (float)in3) / 100.;
        }else{
            ad3 = ad0;
        }
    }
    return ad3 * 1000.;
}
