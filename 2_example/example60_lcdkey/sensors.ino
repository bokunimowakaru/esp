/*******************************************************************************
管理機器のリスト

Ambient送信用のセンサ機器の設定
・機器・項目番号を設定する（データ1～8まで）
・送信不要の場合はnum=0

*******************************************************************************/

#define SENSOR_SW_PING 0                    // WiFiスイッチャ 2016/9 IOT製作(6)
#define SENSOR_SW_RDSW 1                    // WiFiドア開閉   2016/9 IOT製作(12)
#define SENSOR_SW_PIRS 2                    // WiFi人感センサ 2017/3 実習(1) 
#define SENSOR_SW_ACCM 3                    // WiFi加速センサ 2017/3 実習(2) 
#define DEBUG_SENSOR

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

boolean sensors_ckeckDevId(char *dev){
    if( strlen(dev)<7 ) return false;
    if( dev[5] != '_' ) return false;
    for(int i=0;i<7;i++){
        if( !isalnum(dev[i]) && dev[i] !='_' ) return false;
    }
    return true;
}

int sensors_getSwDevIndex(char *dev){
    int index;
    
    index=SENSOR_SW_PING; // WiFiスイッチャ 2016年9月 IOT製作(6) example02_sw, example34_sw
    if( strncmp(dev,"Ping",4)==0 || strncmp(dev,"Pong",4)==0) return index;
    if( !sensors_ckeckDevId(dev) ){
	    #ifdef DEBUG_SENSOR
	        Serial.print("ERROR in sensors_ckeckDevId, dev : ");
	        Serial.println(dev);
	    #endif
    	return -1;
    }
    index=SENSOR_SW_RDSW; // Wi-Fiドア開閉センサ 2016年9月 IOT製作(12) example08_sw, example40_sw
    if( strncmp(dev,"rd_sw_",6)==0 ) return index;
    
    index=SENSOR_SW_PIRS; // Wi-Fi人感センサ 2017年3月 実習(1) example11_pir, example43_pir
    if( strncmp(dev,"pir_s_",6)==0 ) return index;
    
    index=SENSOR_SW_ACCM; // Wi-Fi加速度センサ_X 2017年3月 実習(2) example12_acm, example44_acm
    if( strncmp(dev,"accem_",6)==0 ) return index;

    #ifdef DEBUG_SENSOR
        Serial.print("Sensor Device : ");
        Serial.println(dev);
    #endif
    return 999; // スイッチ以外のデバイス
}

int _sensors_csv2val(char *csv,int num){
    int val=-9999,i;
    char *p;

    for(i=1;i<num;i++){
        p=strchr(csv,',');
        if( !p ) break;
        csv=p+1;
    }
    if( strlen(csv) > 0){
        val = atoi(csv);
    }
    return val;
}

int sensors_isDetectedSw(char *dev, char *csv){
    int suff,index,det=0,ret=0;
    
    index=sensors_getSwDevIndex(dev);
    if( index < 0 || index > 100) return -1;
    #ifdef DEBUG_SENSOR
        Serial.print("Detected Switch : ");
        Serial.println(index);
    #endif
    
    if( index==SENSOR_SW_PING ){;       // WiFiスイッチャ
        if( dev[1]='i' ) det=1;
        if( dev[1]='o' ) det=0;
        return det;
    }
    
    if( strlen(csv)<1 ) return -1;
    suff=atoi(&dev[6]);     // アルファベットの時は0になる
    #ifdef DEBUG_SENSOR
        Serial.print("Suff = ");
        Serial.println(suff);
    #endif
    if( suff > 9 || suff < 0 ) suff=0;  // 在り得ないがセグフォ回避の確認
    
    if( index==SENSOR_SW_RDSW ){;       // ドア開閉センサ
	    #ifdef DEBUG_SENSOR
	        Serial.print("Detected SENSOR_SW_RDSW : ");
	        Serial.print(_sensors_csv2val(csv,1));
	        Serial.print(", ");
	        Serial.println(_sensors_csv2val(csv,2));
	    #endif
	    det=_sensors_csv2val(csv,1);
        if( det == 1 ) return 1;		// OFF 検出回路用（ドアが開いた時に検出する）
        if( det == 0 ) return 1;		// ON 検出回路用（ドアが閉じた時に検出する）
        return 0;
    }
    if( index==SENSOR_SW_PIRS ){;       // 人感センサ
        return _sensors_csv2val(csv,1);
    }
    if( index==SENSOR_SW_ACCM ){;       // 加速度センサ
        if( _sensors_csv2val(csv,1) == 0 &&
            _sensors_csv2val(csv,2) == 0 &&
            _sensors_csv2val(csv,3) == 0) return 0;
        return 1;
    }
    return -1;
}

int sensors_update(char *dev, char *csv){
    int ret=-1;
    int index,i,len;
    char *p;
    
    if( !sensors_ckeckDevId(dev) ) return -1;
    if( strlen(csv)<1 ) return -1;
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
