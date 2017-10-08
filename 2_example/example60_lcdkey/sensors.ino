/*******************************************************************************
管理機器のリスト

Ambient送信用のセンサ機器の設定
・機器・項目番号を設定する（データ1～8まで）
・送信不要の場合はnum=0

*******************************************************************************/

extern Ambient ambient;
extern AmbData ambData[8];
/*
struct AmbData{                             // 履歴保持用 50 byte 17+32+1
    char dev[8]="unkwn_0";                  // LCD表示用(1行目)文字列変数 16字
    int num=0;                              // UDPデータ項目のnum番目を送信
    float value;                            // Ambientへ送信するデータ
    boolean flag=false;                     // 送信許可フラグ
}ambData[8];                                // クラウドへ送信するデータ
*/

#define DEBUG_SENSOR

void sensors_initAmbData(){
    int index;                              // Ambient データ番号 1～8
    
    index=1; ////////////// データd1, ワイヤレス照度センサ, 照度 //////////////
    strcpy(ambData[index-1].dev,"illum_?"); // 
    ambData[index-1].num = 1;
    
    index=2; ////////////// データd2, ワイヤレス温度センサ, 温度 //////////////
    strcpy(ambData[index-1].dev,"temp._?");
    ambData[index-1].num = 1;
    
    index=3; ////////////// データd3, ワイヤレスドア開閉センサ, 開閉状態 //////
    strcpy(ambData[index-1].dev,"rd_sw_?");
    ambData[index-1].num = 1;
    
    index=4; ////////////// データd4, ワイヤレス温湿度センサ, 温度 ////////////
    strcpy(ambData[index-1].dev,"humid_?");
    ambData[index-1].num = 1;               // 測定項目の1番目（温度）
    
    index=5; ////////////// データd5, ワイヤレス温湿度センサ, 湿度 ////////////
    strcpy(ambData[index-1].dev,"humid_?");
    ambData[index-1].num = 2;               // 測定項目の2番目（湿度）
    
    index=6; ////////////// データd1, ワイヤレス気圧センサ, 気圧 //////////////
    strcpy(ambData[index-1].dev,"press_?");
    ambData[index-1].num = 2;               // 2番目（気圧）
    
    index=7; ////////////// データd1, ワイヤレス人感センサ, 検出値 ////////////
    strcpy(ambData[index-1].dev,"pir_s_?");
    ambData[index-1].num = 1;               // 1番目（検出値）
    
    index=8; ////////////// データd1, ワイヤレス加速度センサ, 加速度 //////////
    strcpy(ambData[index-1].dev,"accem_?");
    ambData[index-1].num = 1;               // 1番目 (X軸の加速度）
    
    /* テスト用（上記にマッチしないデバイスについても送信 index=8 ）
        strcpy(ambData[7].dev,"?????_?");
        ambData[7].num = 2;
    */
}

int sensors_update(char *dev, char *csv){
    int ret=-1;
    int index,i,len;
    char *p;
    
    if( strlen(dev)<7 ) return -1;
    if( strlen(csv)<1 ) return -1;
    if( dev[5] != '_' ) return -1;
    for(index=1;index<=8;index++){
        len=7;
        p=strchr(ambData[index-1].dev,'?');
        if(p) len = p - ambData[index-1].dev;
        if(len<0 || len>7) len=7;
        if(strncmp(dev,ambData[index-1].dev,len)==0){
            #ifdef DEBUG_SENSOR
                Serial.print("Update Contents: ");
                Serial.print(ambData[index-1].dev);
                Serial.print(", ");
            #endif
            for(i=1;i<ambData[index-1].num;i++){
                p=strchr(csv,',');
                if( !p ) break; // i
                csv=p+1;
            }
            if( strlen(csv) > 0){
                ambData[index-1].value = (float)atof(csv);
                ambData[index-1].flag  = true;
                #ifdef DEBUG_SENSOR
                    Serial.println(ambData[index-1].value,3);
                #endif
            }
        }
    }
    if( index > 8 ) return -1;
    return index-1;
}

boolean sensors_available(){
    int index;
    for(index=1;index<=8;index++){
        if(ambData[index-1].flag) return true;
    }
    return false;
}

int sensors_sendToAmbient(){
    int index,ret=0;
    char s[16];
    
    if( !sensors_available() ) return 0;
    #ifdef DEBUG_SENSOR
        Serial.println("Send to Ambient:");
    #endif
    for(index=1;index<=8;index++){
        if(ambData[index-1].flag){
            dtostrf(ambData[index-1].value,-15,3,s);
            ambData[index-1].flag=false;
            ambient.set(index,s);            // Ambientへ送信
            #ifdef DEBUG_SENSOR
                Serial.print(ambData[index-1].dev); Serial.print(", d");
                Serial.print(index);                Serial.print(", ");
                Serial.println(ambData[index-1].value,3);
            #endif
            ret++;
        }
    }
    ambient.send();                         // Ambient送信の終了(実際に送信する)
    return ret;
}
