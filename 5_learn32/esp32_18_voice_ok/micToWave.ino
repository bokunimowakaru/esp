/*******************************************************************************
下記のサンプルを修正
                                           Copyright (c) 2017-2019 Wataru KUNINO
********************************************************************************
Example 65 アナログ入力ポートから録音した音声を送信する

********************************************************************************
　マイク入力ポート：GPIO 34 ADC1_CH6
　　　　　　　　　　IoT Express     A2ピン(P3 3番ピン)
　　　　　　　　　　WeMos D1 R32        Analog 3ピン
　　　　　　　　　　ESP-WROOM-32        6番ピン

　録音開始入力ポート：GPIO 0 を Lレベルへ移行する
　　　　　　　　　　IoT Express     BOOTスイッチを押す

　受信用ソフト：toolsフォルダ内のget_sound.sh(Raspberry Pi用)などを使用する
*******************************************************************************/

#include <WiFi.h>                           // ESP32用WiFiライブラリ
#define TIMEOUT 20000                       // タイムアウト 20秒
#define SOUND_LEN 3*8000                    // 音声長 8000=約1秒

byte snd[SOUND_LEN];                        // 音声録音用変数
int snd_size=0;                             // 音声データの大きさ(バイト)
const byte Wave_Header0[2]={0,0};
const byte Wave_Header1[8]={16,0,0,0,1,0,1,0};
const byte Wave_Header2[4]={0x40,0x1F,0,0}; // 8000Hz -> 00 00 1F 40
const byte Wave_Header3[4]={1,0,8,0};

int snd_rec(byte *snd, int len, int port){
    unsigned long time,time_trig=0;
    int i=0, wait_us=125;                   // 8kHz時 サンプリング間隔 125us
    
    while(i<len){
        time=micros();
        if(time < time_trig) continue;      // 未達時
        snd[i]=(byte)(analogRead(PIN_AIN)>>4);  // アナログ入力 12ビット→8ビット
        time_trig = time + wait_us;
        i++;
        if(time_trig < wait_us) break;      // 時間カウンタのオーバフロー
    }
    return i;
}

int micToWave_rec(int pin_ain){ 
//  pinMode(pin_ain,INPUT);                 // アナログ入力端子の設定
    unsigned long TIME=millis();
    Serial.print("Recording... ");
    snd_size=snd_rec(snd,SOUND_LEN,pin_ain);// 音声入力
    Serial.print(millis()-TIME);
    Serial.println("msec. Done");
    if(snd_size<82) return 0;
    return snd_size+44;
}

/*******************************************************************************
FTP送信用クライアント for ESP-WROOM-02 Yahoo! ジオシティーズ対応版

                                           Copyright (c) 2016-2019 Wataru KUNINO

参考文献
  ■ 本ソースコードの作成に当たり、下記の情報を参考にしました(2016/12/14)
  |
  | FTP passive client for IDE v1.0.1 and w5100/w5200
  | Posted October 2012 by SurferTim
  | Modified 6 June 2015 by SurferTim
  |
  | http://playground.arduino.cc/Code/FTP
  
  ■ Yahoo! ジオシティーズ (Geo Cities) へのアップロード等に対応するために
  |下記への書き込み情報を参考にしました。
  | esp8266/Arduino Ussues
  | Add FTP Client library  #1183
  |
  | https://github.com/esp8266/Arduino/issues/1183
*******************************************************************************/

#define FTP_WAIT 1
#define BUFFER_SIZE 128

byte micToWave_FTP(const char *filename){
    char ftpBuf[BUFFER_SIZE];
    WiFiClient client;
    WiFiClient dclient;
    int i;
    /*
    File file = SPIFFS.open(filename,"r");
    if(!file){
        Serial.println("SPIFFS open fail");
        return 11;
    }
    Serial.println("SPIFFS opened");
    */
    if (client.connect(FTP_TO,21)) {
        Serial.println("Command connected");
    }
    if(eRcv(client,ftpBuf)) return 21;

    sprintf(ftpBuf,"USER %s\r\n",FTP_USER);
    client.print(ftpBuf);
    delay(FTP_WAIT);
    Serial.print(ftpBuf);
    if(eRcv(client,ftpBuf)) return 22;

    sprintf(ftpBuf,"PASS %s\r\n",FTP_PASS);
    client.print(ftpBuf);
    delay(FTP_WAIT);
    Serial.println("PASS");
    if(eRcv(client,ftpBuf)) return 23;
    
    client.print("Type I\r\n");
    delay(FTP_WAIT);
    Serial.println("Type i");
    if(eRcv(client,ftpBuf)) return 25;

    client.print("PASV\r\n");
    delay(FTP_WAIT);
    Serial.println("PASV");
    delay(100);
    if(eRcv(client,ftpBuf)) return 26;

    char *tStr = strtok(ftpBuf,"(,");
    if(tStr == NULL) return 27;
    int array_pasv[6];
    for (i = 0; i < 6; i++) {
        tStr = strtok(NULL,"(,");
        array_pasv[i] = atoi(tStr);
        if(tStr == NULL){
            Serial.println("Bad PASV Answer");
            return 28;
        }
    }
    
    unsigned int hiPort,loPort;
    hiPort = array_pasv[4] << 8;
    loPort = array_pasv[5] & 255;
    Serial.print("Data port: ");
    hiPort = hiPort | loPort;
    Serial.println(hiPort);
    if (dclient.connect(FTP_TO,hiPort)) {
        Serial.println("Data connected");
    }else{
        Serial.println("Data connection failed");
        client.stop();
//      file.close();
        return 31;
    }
    sprintf(ftpBuf,"STOR %s%s\r\n",FTP_DIR,filename);
    client.print(ftpBuf);
    delay(FTP_WAIT);
    Serial.print(ftpBuf);
    if(eRcv(client,ftpBuf)){
        dclient.stop();
//      file.close();
        return 32;
    }
    Serial.println("Writing");
    i=0;
    if(dclient.connected()){
		dclient.write("RIFF",4);                 // RIFF                     ->[4]
        i = snd_size + 44 - 8;                  // ファイルサイズ-8
        dclient.write(i & 0xFF);                 // サイズ・最下位バイト     ->[5]
        dclient.write((i>>8) & 0xFF);            // サイズ・第2位バイト      ->[6]
        dclient.write(Wave_Header0,2);           // サイズ・第3-4位バイト    ->[8]
        dclient.write("WAVEfmt ",8);             // "Wave","fmt "            ->[16]
        dclient.write(Wave_Header1,8);           // 16,0,0,0,1,0,1,0         ->[24]
        dclient.write(Wave_Header2,4);           // 68,172,0,0               ->[28]
        dclient.write(Wave_Header2,4);           // 68,172,0,0               ->[32]
        dclient.write(Wave_Header3,4);           // 1,0,8,0                  ->[36]
        dclient.write("data",4);                 //  "data"                  ->[40]
        i = snd_size + 44 - 126;                // ファイルサイズ-126
        dclient.write(i & 0xFF);                 // サイズ・最下位バイト     ->[41]
        dclient.write((i>>8) & 0xFF);            // サイズ・最下位バイト     ->[42]
        dclient.write("\0\0",2);                 // サイズ3-4
        for(i=0;i<snd_size;i++) dclient.write(snd[i]);
        dclient.flush();                         // ESP32用 ERR_CONNECTION_RESET対策
    }
    dclient.stop();
    Serial.println("Data disconnected");
    if(eRcv(client,ftpBuf)) return 33;
    client.print("QUIT\r\n");
    delay(FTP_WAIT);
    Serial.println("QUIT");
    if(eRcv(client,ftpBuf)) return 91;
    client.stop();
    Serial.println("Command disconnected");
//  file.close();
//  Serial.println("SPIFFS closed");
    return 0;
}

byte eRcv(WiFiClient &client,char *ftpBuf){
    byte thisByte,i=0,len=0;

    while(!client.available()){
        delay(FTP_WAIT);
        if(!client.connected()){
            Serial.println("FTP:eRcv:disC");
            return 11;
        }
        i++;
        if(i>1000){ // 200ms以上必要
            Serial.println("FTP:eRcv:noRes");
            return 12;
        }
    }
    while(client.connected()){
        if(!client.available()){
            delay(FTP_WAIT);
            if(!client.available()) break;
        }
        thisByte = client.read();
        if(thisByte==(byte)'\r');
        else if(thisByte==(byte)'\n'){
            Serial.write('>');
            Serial.println(ftpBuf);
            if(ftpBuf[0] >= '4'){
                client.print("QUIT\r\n");
                delay(FTP_WAIT);
                Serial.println("QUIT");
                return 1;
            }
            if(len>3 && ftpBuf[3] == ' '){
                return 0;
            }
            len = 0;
        }else if(len < BUFFER_SIZE - 1 ){
            ftpBuf[len] = thisByte;
            len++;      
            ftpBuf[len] = 0;
        }
    }
    return 0;
}

void efail(WiFiClient &client){
    byte thisByte = 0;
    client.print("QUIT\r\n");
    delay(FTP_WAIT);
    while(!client.available()){
        if(!client.connected()) return;
        delay(1);
    }
    while(client.available()){  
        thisByte = client.read();
        Serial.write(thisByte);
    }
    client.stop();
    Serial.println("Command disconnected");
}
