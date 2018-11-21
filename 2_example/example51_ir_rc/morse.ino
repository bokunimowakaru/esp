/*******************************************************************************
モールス

                                           Copyright (c) 2016-2019 Wataru KUNINO
*******************************************************************************/

/*
本スケッチは下記からダウンロードしたものを改変して作成しました。

http://darashi.net/2012/02/23/arduino-hello-world.html
HELLO WORLD from Arduino 
Feb 23 2012.
darashi/hello.c 

Yoji Shidara (https://github.com/darashi)
*/

// #define LED 13
// #define T 100 // speed

const char morse_pattern[26][5] = {
    "._", "_...", "_._.", "_..", ".", ".._.", "__.",
    "....", "..", ".___", "_._", "._..", "__",
    "_.", "___", ".__.", "__._", "._.", "...", "_",
    ".._", "..._", ".__", "_.._", "__._", "__.."
};


/*
void setup() {
    pinMode(LED, OUTPUT);
    digitalWrite(LED, LOW);
    delay(T*7);
}
*/

void morse_delay(int time){
    if(time > 10) for(int i=0;i<time/10;i++){
        delay(10);
    //  ESP.wdtFeed();
    }
    delay(time%10);
//  ESP.wdtFeed();
}

void morse(int output, int time, const char *str) {
    int i, j;
    char num[6]="_____";
    char morse_dot[7]="._._._";
    char *pattern;

    for (i=0; i<strlen(str); i++) {
        char *c = (char *)str+i;
        pattern=0;
        if ('A' <= *c && *c <= 'Z') pattern = (char *)morse_pattern[*c - 'A'];
        if ('a' <= *c && *c <= 'z') pattern = (char *)morse_pattern[*c - 'a'];
        if ('0' <= *c && *c <= '5'){
            for(j=0;j<5;j++) if( (int)(*c - '0') >= j+1 ) num[j]='.'; else num[j]='_';
            pattern = num;
        }
        if ('6' <= *c && *c <= '9'){
            for(j=0;j<5;j++) if( (int)(*c - '0') >= j+6 ) num[j]='_'; else num[j]='.';
            pattern = num;
        }
        if ( *c == '.') pattern=morse_dot;
        if(pattern){
            for (j=0; j<strlen(pattern); j++) {
                digitalWrite(output, HIGH);
                if(pattern[j] == '.') {
                    morse_delay(time);
                    Serial.print('.');
                } 
                else {
                    morse_delay(3 * time);
                    Serial.print('_');
                }
                digitalWrite(output, LOW);
                morse_delay(time);
            }
            morse_delay(2 * time);
        } else {
            morse_delay(3 * time);
        }
        Serial.print(' ');
    }
    Serial.println();
    morse_delay(4 * time);
    Serial.println("Done Morse");
}

/*
void loop() {
    char *str = "HELLO WORLD";
    morse(LED, T, str);
}
*/

void morseIp(int output, int time, uint32_t ip){
    byte i,j;
    char s[17];
    
    sprintf(s,"%d.%d.%d.%d",
        ip & 255,
        ip>>8 & 255,
        ip>>16 & 255,
        ip>>24
    );
    morse(output, time, s);
}

void morseIp0(int output, int time, uint32_t ip){
    byte i,j;
    char s[17];
    
    sprintf(s,".%i",
        ip>>24
    );
    morse(output, time, s);
}
