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
#define LEDC_CHANNEL_0     0                // use first channel of 16 channels (started from zero)
#define LEDC_TIMER_13_BIT  13               // use 13 bit precission for LEDC timer
#define LEDC_BASE_FREQ     5000             // use 5000 Hz as a LEDC base frequency

void ledcAnalogWrite(uint8_t channel, int value) {

  if(value > 1023) value = 1023;
  if(value < 0) value = 0;
  uint32_t duty = (uint32_t)value << 3;     // 10bit -> 13 bit へシフト

  ledcWrite(channel, duty);
}

void ledSetup(){
    Serial.print("ledSetup LEDC_CHANNEL_0 = ");
    Serial.print(LEDC_CHANNEL_0);
    Serial.print(", PIN_LED = ");
    Serial.print(PIN_LED);
    Serial.print(", freq. = ");
    Serial.println(ledcSetup(LEDC_CHANNEL_0, LEDC_BASE_FREQ, LEDC_TIMER_13_BIT),3);
    ledcAttachPin(PIN_LED, LEDC_CHANNEL_0);
}

void ledEnd(){
    ledcDetachPin(PIN_LED);
}

int ledCtrl(int start,int end,int speed){   // ledのアナログ制御用の関数
    int i;                                  // (startからendへ輝度を推移する)
    if(speed<1)speed=1;
    if(start<=end){
        if(start<1) start=1;
        if(end>1023) end=1023;
        for(i=start;i<end;i<<=1){
        //  analogWrite(PIN_LED,i);
            ledcAnalogWrite(LEDC_CHANNEL_0,i);
            delay(100/speed);
        }
    }else{
        if(start>1023) start=1023;
        if(end<0) end=0;
        for(i=start;i>end;i>>=1){
        //  analogWrite(PIN_LED,i);
            ledcAnalogWrite(LEDC_CHANNEL_0,i);
            delay(100/speed);
        }
    }
//  analogWrite(PIN_LED,end);
    ledcAnalogWrite(LEDC_CHANNEL_0,end);
    return(end);
}
