/*******************************************************************************
FTP送信用クライアント for ESP-WROOM-02

                                           Copyright (c) 2016-2017 Wataru KUNINO

参考文献
  |本ソースコードの作成に当たり、下記の情報を参考にしました(2016/12/14)
  |
  | FTP passive client for IDE v1.0.1 and w5100/w5200
  | Posted October 2012 by SurferTim
  | Modified 6 June 2015 by SurferTim
  |
  | http://playground.arduino.cc/Code/FTP
*******************************************************************************/

#define FTP_WAIT 1

byte doFTP(char *filename){
    char outBuf[128];
    byte outCount;
    WiFiClient client;
    WiFiClient dclient;
    
    File file = SPIFFS.open(filename,"r");
    if(!file){
        Serial.println(F("SPIFFS open fail"));
        return 11;
    }
    Serial.println(F("SPIFFS opened"));
    if (client.connect(FTP_TO,21)) {
        Serial.println(F("Command connected"));
    }
    if(eRcv(client,outBuf,&outCount)) return 21;

    sprintf(outBuf,"USER %s\r\n",FTP_USER);
    client.print(outBuf);
    delay(FTP_WAIT);
    Serial.print(outBuf);
    if(eRcv(client,outBuf,&outCount)) return 22;

    sprintf(outBuf,"PASS %s\r\n",FTP_PASS);
    client.print(outBuf);
    delay(FTP_WAIT);
    Serial.println(F("PASS"));
    if(eRcv(client,outBuf,&outCount)) return 23;

    client.print(F("Type I\r\n"));
    delay(FTP_WAIT);
    Serial.println(F("Type I"));
    if(eRcv(client,outBuf,&outCount)) return 25;

    client.print(F("PASV\r\n"));
    delay(FTP_WAIT);
    Serial.println(F("PASV"));
    if(eRcv(client,outBuf,&outCount)) return 26;

    char *tStr = strtok(outBuf,"(,");
    int array_pasv[6];
    for ( int i = 0; i < 6; i++) {
        tStr = strtok(NULL,"(,");
        array_pasv[i] = atoi(tStr);
        if(tStr == NULL){
            Serial.println(F("Bad PASV Answer"));    
        }
    }
    unsigned int hiPort,loPort;
    hiPort = array_pasv[4] << 8;
    loPort = array_pasv[5] & 255;
    Serial.print(F("Data port: "));
    hiPort = hiPort | loPort;
    Serial.println(hiPort);
    if (dclient.connect(FTP_TO,hiPort)) {
        Serial.println(F("Data connected"));
    }else{
        Serial.println(F("Data connection failed"));
        client.stop();
        file.close();
        return 31;
    }
    sprintf(outBuf,"STOR %s%s\r\n",FTP_DIR,filename);
    client.print(outBuf);
    delay(FTP_WAIT);
    Serial.print(outBuf);
    if(eRcv(client,outBuf,&outCount)){
        dclient.stop();
        file.close();
        return 32;
    }
    Serial.println(F("Writing"));
    while(file.available()){
        if(!dclient.connected()) break;
        dclient.write((byte)file.read());
    }
    dclient.stop();
    Serial.println(F("Data disconnected"));
    if(eRcv(client,outBuf,&outCount)) return 33;
    client.print(F("QUIT\r\n"));
    delay(FTP_WAIT);
    Serial.println(F("QUIT"));
    if(eRcv(client,outBuf,&outCount)) return 91;
    client.stop();
    Serial.println(F("Command disconnected"));
    file.close();
    Serial.println(F("SPIFFS closed"));
    return 0;
}

byte eRcv(WiFiClient &client,char *outBuf,byte *outCount){
    byte thisByte,i=0;

    while(!client.available()){
        delay(FTP_WAIT);
        if(!client.connected()){
            Serial.println(F("FTP:eRcv:disC"));
            return 11;
        }
        i++;
        if(i>1000){ // 少なくとも200ms以上必要
            Serial.println(F("FTP:eRcv:noRes"));
            return 12;
        }
    }
    (*outCount) = 0;
    while(client.connected()){
        if(!client.available()){
            delay(FTP_WAIT);
            if(!client.available()) break;
        }
        thisByte = client.read();
    //  Serial.write(thisByte);
        if(thisByte==(byte)'\r');
        else if(thisByte==(byte)'\n'){
            Serial.write('>');
            Serial.println(outBuf);
            if(outBuf[0] >= '4'){
                client.print(F("QUIT\r\n"));
                delay(FTP_WAIT);
                Serial.println(F("QUIT"));
                return 1;
            }
            (*outCount) = 0;
        }else if((*outCount) < 127){
            outBuf[(*outCount)] = thisByte;
            (*outCount)++;      
            outBuf[(*outCount)] = 0;
        }
    }
    return 0;
}

void efail(WiFiClient &client){
    byte thisByte = 0;
    client.print(F("QUIT\r\n"));
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
    Serial.println(F("Command disconnected"));
}
