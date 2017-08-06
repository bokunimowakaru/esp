/*******************************************************************************
モールス

                                            Copyright (c) 2016 Wataru KUNINO
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

char *morse_pattern[] = {
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

void morse(int output, int time, char *str) {
    int i, j, t;
    char num[6]="_____";
    char *pattern;

    for (i=0; i<strlen(str); i++) {
        char *c = str+i;
        pattern=0;
        if ('A' <= *c && *c <= 'Z') pattern = morse_pattern[*c - 'A'];
        if ('a' <= *c && *c <= 'z') pattern = morse_pattern[*c - 'a'];
        if ('0' <= *c && *c <= '5'){
            for(j=0;j<5;j++) if( (int)(*c - '0') >= j+1 ) num[j]='.'; else num[j]='_';
            pattern = num;
        }
        if ('6' <= *c && *c <= '9'){
            for(j=0;j<5;j++) if( (int)(*c - '0') >= j+6 ) num[j]='_'; else num[j]='.';
            pattern = num;
        }
        if ( *c == '.') pattern="._._._";
        if(pattern){
            for (j=0; j<strlen(pattern); j++) {
                if(pattern[j] == '.') {
                //  tone(output, 800, time);
                    ledcWriteTone(0,800);           // double freq
                    delay(time);
                //  Serial.print('.');
                } 
                else {
                //  tone(output, 800, 3*time);
                    ledcWriteTone(0,800);           // double freq
                    for(t=0;t<3;t++) delay(time);
                //  Serial.print('_');
                }
            //  noTone(output);
                ledcWrite(0, 0);                    // uint8_t chan, uint32_t duty 0%
                delay(time);
            }
            for(t=0;t<2;t++) delay(time);
        } else {
            for(t=0;t<3;t++) delay(time);
        }
    //  Serial.println();
    }
    for(t=0;t<4;t++) delay(time);
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
    
    sprintf(s,"%i.%i.%i.%i",
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

