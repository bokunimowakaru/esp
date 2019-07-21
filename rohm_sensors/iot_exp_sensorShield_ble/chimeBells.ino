/*******************************************************************************
チャイム

int chimeBells(int output, int count);

int output  LEDを接続したIOピン番号
int count   チャイム音が終わるまでのカウントダウン値（0で終了）
int 戻り値  count-1（0以上）

処理時間：約1秒

                                          Copyright (c) 2016-2019 Wataru KUNINO
*******************************************************************************/

/* for LEDC */
#define LEDC_CHANNEL_0     0                // use first channel of 16 channels (started from zero)
#define LEDC_TIMER_13_BIT  13               // use 13 bit precission for LEDC timer
#define LEDC_BASE_FREQ     5000             // use 5000 Hz as a LEDC base frequency

void chimeBellsSetup(int PIN){
    Serial.print("ledSetup LEDC_CHANNEL_0 = ");
    Serial.print(LEDC_CHANNEL_0);
    Serial.print(", PIN_BUZZER = ");
    Serial.print(PIN_BUZZER);
    Serial.print(", freq. = ");
    Serial.println(ledcSetup(LEDC_CHANNEL_0, LEDC_BASE_FREQ, LEDC_TIMER_13_BIT),3);
    ledcAttachPin(PIN, LEDC_CHANNEL_0);
}

int chimeBells(int output, int count) {
    int t;
    if(count==0)return 0;
    if(!(count%2)){
    //  tone(output,NOTE_CS6,800);
        ledcWriteNote(0,NOTE_Cs,6);		// uint8_t chan, note_t note, uint8_t octave
        for(t=0;t<8;t++) delay(100);
    //  noTone(output);
        ledcWrite(0, 0);				// uint8_t chan, uint32_t duty 0%
        for(t=0;t<2;t++) delay(100);
    }else{
    //  tone(output,NOTE_A5,800);
        ledcWriteNote(0,NOTE_A,5);		// uint8_t chan, note_t note, uint8_t octave
        for(t=0;t<8;t++) delay(100);
    //  noTone(output);
        ledcWrite(0, 0);				// uint8_t chan, uint32_t duty 0%
        for(t=0;t<2;t++) delay(100);
    }
    count--;
    if(count<0) count=0;
    return(count);
}
