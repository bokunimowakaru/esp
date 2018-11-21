/*******************************************************************************
監視カメラ用ドライバ linkSprite_cam_IR.ino for Linksprite JPEG Serial Camera

                                          Copyright (c) 2016-2019 Wataru KUNINO

Linksprite社の LinkSprite_cam_IR.ino を基に国野 亘が改変しました。
下記のライセンスの基に配布します。

本ソースコードは、CC BY-SA（表示 - 継承） です。
https://creativecommons.org/licenses/by-sa/4.0/

http://linksprite.com/wiki/index.php5?title=JPEG_Color_Camera_Serial_Interface_with_Built-in_Infrared_(TTL_level)
*******************************************************************************/

#define DEBUG

int camReadADR=0x0000;

void CamReadADR0(){
    camReadADR=0x0000;
}

int CamReadFileSize(){
    byte incomingbyte;
    int k,size=-1;
    boolean EndFlag=0;
    unsigned long t = millis();
    byte ret[7]={0x76,0x00,0x34,0x00,0x04,0x00,0x00};
    
    while(softwareSerial.available()>0) softwareSerial.read();  // 読み捨てる
    softwareSerial.write((byte)0x56);
    softwareSerial.write((byte)0x00);
    softwareSerial.write((byte)0x34);
    softwareSerial.write((byte)0x01);
    softwareSerial.write((byte)0x00);
    delay(25);
    k=0;
    while(softwareSerial.available()>0){
        incomingbyte=softwareSerial.read();
        if(k<7)if(incomingbyte != ret[k])break;
        if(k==7) size = (int)incomingbyte * 256;
        if(k==8) size += (int)incomingbyte;
        if(millis()-t>1000){
            size=-1;    // 異常終了
            break;
        }
        k++;
        if(k>8) break;
    }
    while(softwareSerial.available()>0) softwareSerial.read();
    return size;
}

int CamRead(byte *data){
    // 応答値は読み取りバイト数
    byte incomingbyte;
    int j,k,count;
    boolean EndFlag=0;      // このフラグを使用する場合はグローバル変数に変更すること
    
    j=0;                    // jもkもcountも同じようなもの
    k=0;
    count=0;
    CamSendReadDataCmd();
    delay(25);
    
    while(softwareSerial.available()>0){
        incomingbyte=softwareSerial.read();
        k++;
        if((k>5)&&(j<32)&&(!EndFlag)){
            data[j]=incomingbyte;
            if((data[j-1]==0xFF)&&(data[j]==0xD9)) EndFlag=1;     //Check if the picture is over
            j++;
            count++;
        }
    }
    /*
    for(j=0;j<count;j++){
        // Send jpeg picture over the serial port
        if(data[j]<0x10) Serial.print("0");
        Serial.print(data[j],HEX);
        Serial.print(" ");
    }
    Serial.println();
    */
//    if(EndFlag) count *=-1;     // 終了時はcount値を負に反転する
    return count;
}

//Send Reset command
void CamSendResetCmd(){
    softwareSerial.write((byte)0x56);
    softwareSerial.write((byte)0x00);
    softwareSerial.write((byte)0x26);
    softwareSerial.write((byte)0x00);
    delay(25);
    while(softwareSerial.available()>0) softwareSerial.read();
}

//Send take picture command
void CamSendTakePhotoCmd(){
    softwareSerial.write((byte)0x56);
    softwareSerial.write((byte)0x00);
    softwareSerial.write((byte)0x36);
    softwareSerial.write((byte)0x01);
    softwareSerial.write((byte)0x00);
    delay(25);
    while(softwareSerial.available()>0) softwareSerial.read();
}

//Read data
void CamSendReadDataCmd(){
    uint8_t MH,ML;

    MH=(uint8_t)(camReadADR/0x100);
    ML=(uint8_t)(camReadADR%0x100);
    softwareSerial.write((byte)0x56);
    softwareSerial.write((byte)0x00);
    softwareSerial.write((byte)0x32);
    softwareSerial.write((byte)0x0c);
    softwareSerial.write((byte)0x00); 
    softwareSerial.write((byte)0x0a);
    softwareSerial.write((byte)0x00);
    softwareSerial.write((byte)0x00);
    softwareSerial.write((byte)MH);
    softwareSerial.write((byte)ML);   
    softwareSerial.write((byte)0x00);
    softwareSerial.write((byte)0x00);
    softwareSerial.write((byte)0x00);
    softwareSerial.write((byte)0x20);
    softwareSerial.write((byte)0x00);  
    softwareSerial.write((byte)0x0a);
    camReadADR+=0x20; //address increases 32 set according to buffer size
}

void CamStopTakePhotoCmd(){
    softwareSerial.write((byte)0x56);
    softwareSerial.write((byte)0x00);
    softwareSerial.write((byte)0x36);
    softwareSerial.write((byte)0x01);
    softwareSerial.write((byte)0x03);        
    delay(25);
    while(softwareSerial.available()>0) softwareSerial.read();
}

void CamPowerCmd(int in){
    softwareSerial.write((byte)0x56);
    softwareSerial.write((byte)0x00);
    softwareSerial.write((byte)0x3E);
    softwareSerial.write((byte)0x03);
    softwareSerial.write((byte)0x00);
    softwareSerial.write((byte)0x01);
    if(in){  // Power ON
        softwareSerial.write((byte)0x00);
    }else{   // Power OFF (Power Save)
        softwareSerial.write((byte)0x01);
    }
    delay(25);
    while(softwareSerial.available()>0) softwareSerial.read();
}

void CamRatioCmd(int in){
    softwareSerial.write((byte)0x56);
    softwareSerial.write((byte)0x00);
    softwareSerial.write((byte)0x31);
    softwareSerial.write((byte)0x05);
    softwareSerial.write((byte)0x01);
    softwareSerial.write((byte)0x01);
    softwareSerial.write((byte)0x12);
    softwareSerial.write((byte)0x04);
    if(in>=0 && in<=255){  // Power ON
        softwareSerial.write((byte)in);
    }else{  // Power OFF (Power Save)
        softwareSerial.write((byte)0x36);
    }
    delay(25);
    while(softwareSerial.available()>0) softwareSerial.read();
}

void CamBaudRateCmd(int in){
    softwareSerial.write((byte)0x56);
    softwareSerial.write((byte)0x00);
    softwareSerial.write((byte)0x24);
    softwareSerial.write((byte)0x03);
    softwareSerial.write((byte)0x01);
    if(in==9600){
        softwareSerial.write((byte)0xAE);
        softwareSerial.write((byte)0xC8);
    }else if(in==19200){
        softwareSerial.write((byte)0x56);
        softwareSerial.write((byte)0xE4);
    }else if(in==57600){
        softwareSerial.write((byte)0x1C);
        softwareSerial.write((byte)0x4C);
    }else if(in==115200){
        softwareSerial.write((byte)0x0D);
        softwareSerial.write((byte)0xA6);
    }else{  // 38400 bps
        softwareSerial.write((byte)0x2A);
        softwareSerial.write((byte)0xF2);
    }
    delay(25);
    while(softwareSerial.available()>0) softwareSerial.read();
}

void CamSizeCmd(int in){
    softwareSerial.write((byte)0x56);
    softwareSerial.write((byte)0x00);
    softwareSerial.write((byte)0x31);
    softwareSerial.write((byte)0x05);
    softwareSerial.write((byte)0x04);
    softwareSerial.write((byte)0x01);
    softwareSerial.write((byte)0x00);
    softwareSerial.write((byte)0x19);
    if(in==0){          // 640 x 480
        softwareSerial.write((byte)0x00);
    }else if(in==2){    // 160 x 120
        softwareSerial.write((byte)0x22);
    }else{              // 320 x 240
        softwareSerial.write((byte)0x11);
    }
    delay(25);
    while(softwareSerial.available()>0) softwareSerial.read();
}
