/*******************************************************************************
Linksprite社の LinkSprite_cam_IR.ino を基に国野 亘が改変し、下記のライセンスの基に
配布します。

本ソースコードは、CC BY-SA（表示 - 継承） です。
https://creativecommons.org/licenses/by-sa/4.0/

http://linksprite.com/wiki/index.php5?title=JPEG_Color_Camera_Serial_Interface_with_Built-in_Infrared_(TTL_level)
*******************************************************************************/

#define DEBUG

int camReadADR=0x0000;

void CamReadADR0(){
	camReadADR=0x0000;
}

int CamRead(byte *data){
	// 応答値は読み取りバイト数
	// 応答値が負の時はEndFlag
	byte incomingbyte;
	int j,k,count;
	boolean EndFlag=0;
	
	j=0;
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
	if(EndFlag) count *=-1;
	return count;
}

//Send Reset command
void CamSendResetCmd()
{
	softwareSerial.write((byte)0x56);
	softwareSerial.write((byte)0x00);
	softwareSerial.write((byte)0x26);
	softwareSerial.write((byte)0x00);
}

//Send take picture command
void CamSendTakePhotoCmd()
{
	softwareSerial.write((byte)0x56);
	softwareSerial.write((byte)0x00);
	softwareSerial.write((byte)0x36);
	softwareSerial.write((byte)0x01);
	softwareSerial.write((byte)0x00);  
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

void CamStopTakePhotoCmd()
{
      softwareSerial.write((byte)0x56);
      softwareSerial.write((byte)0x00);
      softwareSerial.write((byte)0x36);
      softwareSerial.write((byte)0x01);
      softwareSerial.write((byte)0x03);        
}
