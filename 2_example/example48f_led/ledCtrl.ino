/*******************************************************************************
LEDの輝度制御

int ledCtrl(int start,int end,int speed);

int start   現在の輝度　（0～1023）
int end     制御後の輝度（0～1023）
int speed   制御速度    (1～100程度・1で約1秒、10で約0.1秒)
int 戻り値  制御後の輝度

参考：
https://github.com/espressif/arduino-esp32/blob/master/cores/esp32/esp32-hal-ledc.c
https://github.com/espressif/arduino-esp32/blob/master/libraries/ESP32/examples/AnalogOut/LEDCSoftwareFade/LEDCSoftwareFade.ino

                                            Copyright (c) 2016-2017 Wataru KUNINO
*******************************************************************************/

/* for LEDC */
#define LEDC_TIMER_13_BIT  13               // use 13 bit precission for LEDC timer
#define LEDC_BASE_FREQ     5000             // use 5000 Hz as a LEDC base frequency
uint8_t LEDC_CH_MAP[3];

void ledcAnalogWrite(uint8_t channel, int value) {

  if(value > 1023) value = 1023;
  if(value < 0) value = 0;
  uint32_t duty = (uint32_t)value << 3;     // 10bit -> 13 bit へシフト

  ledcWrite(channel, duty);
}

void ledSetup(){
    LEDC_CH_MAP[0]=(uint8_t)PIN_LED_R;
    LEDC_CH_MAP[1]=(uint8_t)PIN_LED_G;
    LEDC_CH_MAP[2]=(uint8_t)PIN_LED_B;
    for(uint8_t i=0;i<3;i++){
        Serial.print("ledSetup LEDC_CHANNEL = ");
        Serial.print(i);
        Serial.print(", PIN_LED = ");
        Serial.print(LEDC_CH_MAP[i]);
        Serial.print(", freq. = ");
        Serial.println(ledcSetup(i, LEDC_BASE_FREQ, LEDC_TIMER_13_BIT),3);
        ledcAttachPin(LEDC_CH_MAP[i], i);
    }
}

void ledEnd(){
    for(uint8_t i=0;i<3;i++) ledcDetachPin(LEDC_CH_MAP[i]);
}

int ledCtrl(int pin,int start,int end,int speed){   // ledのアナログ制御用の関数
    int i;                                  // (startからendへ輝度を推移する)
    uint8_t ch=0;
    for(i=0;i<3;i++){
        if( LEDC_CH_MAP[i] == (uint8_t)(pin) ) ch=i;
    }
    
    if(speed<1)speed=1;
    if(speed>100)speed=100;
    if(start<=end){
        if(start<1) start=1;
        if(end>1023) end=1023;
        for(i=start;i<end;i<<=1){
        //  analogWrite(PIN_LED,i);
            ledcAnalogWrite(ch,i);
            delay(100/speed);
        }
    }else{
        if(start>1023) start=1023;
        if(end<0) end=0;
        for(i=start;i>end;i>>=1){
        //  analogWrite(PIN_LED,i);
            ledcAnalogWrite(ch,i);
            delay(100/speed);
        }
    }
//  analogWrite(PIN_LED,end);
    ledcAnalogWrite(ch,end);
    return(end);
}
