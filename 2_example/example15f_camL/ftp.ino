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

byte doFTP(char *filename){
    char ftpBuf[BUFFER_SIZE];
    WiFiClient client;
    WiFiClient dclient;
    int i;
    
    File file = SPIFFS.open(filename,"r");
    if(!file){
        Serial.println("SPIFFS open fail");
        return 11;
    }
    Serial.println("SPIFFS opened");
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
        file.close();
        return 31;
    }
    sprintf(ftpBuf,"STOR %s%s\r\n",FTP_DIR,filename);
    client.print(ftpBuf);
    delay(FTP_WAIT);
    Serial.print(ftpBuf);
    if(eRcv(client,ftpBuf)){
        dclient.stop();
        file.close();
        return 32;
    }
    Serial.println("Writing");
    i=0;
    while(file.available()){
        ftpBuf[i]=file.read();
        i++;
        if(i >= BUFFER_SIZE){
            if(!dclient.connected()) break;
            dclient.write( (byte *)ftpBuf, BUFFER_SIZE);
            i=0;
            Serial.print(".");
            delay(1);
        }
    }
    if(i > 0){
        if(dclient.connected()){
            dclient.write((byte *)ftpBuf, i);
        }
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
    file.close();
    Serial.println("SPIFFS closed");
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
