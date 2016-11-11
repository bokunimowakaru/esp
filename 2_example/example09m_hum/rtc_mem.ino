/*******************************************************************************
RTC内のメモリの読み書きドライバ(整数値)

下記のウェブサイトの情報に基づいて作成しました。

http://jiwashin.blogspot.jp/2016/07/esp30.html
https://lowreal.net/2016/01/10/1
*******************************************************************************/
extern "C" {
#include "user_interface.h"                 // ESP8266用の拡張IFライブラリ
}
//#define DEBUG
#define kOffset         65
int WAKE_COUNT;

struct RtcStorage{
    char iot[4];
    int count;
    float data;
};

#ifdef DEBUG
void printRtcInt(char *iot,int count, float data,int size){
    Serial.print("Ident = "); Serial.print(iot);
    Serial.print(", Count = "); Serial.print(count);
    Serial.print(", Value = "); Serial.print(data,2);
    Serial.print(", Size  = "); Serial.println(size);
}
#endif

float readRtcInt(){
    struct RtcStorage mem_init={"IoT",0,0.};
    struct RtcStorage mem_read={"...",0,0.};
    bool ok;

    ok = system_rtc_mem_read(kOffset, &mem_read, sizeof(mem_read));
    #ifdef DEBUG
        Serial.println("Read from RTC Memory");
        printRtcInt(mem_read.iot, mem_read.count, mem_read.data, sizeof(mem_read));
    #else
        Serial.print("read ("); Serial.print(mem_read.count);
        Serial.print(")="); Serial.println(mem_read.data,2);
    #endif
    if(!ok || strncmp(mem_init.iot,mem_read.iot,3) ){
        Serial.println("Initializing RTC Memory");
        #ifdef DEBUG
            printRtcInt(mem_init.iot, mem_init.count, mem_init.data, sizeof(mem_init));
        #endif
        ok=system_rtc_mem_write(kOffset, &mem_init, sizeof(mem_init));
        if (!ok) Serial.println("readRtcInt : write fail");
        mem_read.count=0;
        mem_read.data=0.;
    }else{
        mem_read.count++;
        ok = system_rtc_mem_write(kOffset, &mem_read, 4 + sizeof(int) );
        if (!ok) Serial.println("readRtcInt : write fail");
    }
    WAKE_COUNT=mem_read.count;
    return mem_read.data;
}

bool writeRtcInt(float in) {
    struct RtcStorage mem={"IoT",WAKE_COUNT,in};
    bool ok;

    #ifdef DEBUG
        Serial.println("Write to RTC Memory");
        printRtcInt(mem.iot, mem.count, mem.data, sizeof(mem));
    #else
        Serial.print("write("); Serial.print(mem.count);
        Serial.print(")="); Serial.println(mem.data,2);
    #endif
    ok = system_rtc_mem_write(kOffset, &mem, sizeof(mem));
    if(!ok) Serial.println("Error : writeRtcInt : system_rtc_mem_write fail");
    return ok;
}
