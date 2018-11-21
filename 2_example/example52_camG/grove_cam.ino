/*******************************************************************************
監視カメラ用ドライバ grove_cam.ino for SeeedStudio Grove Serial Camera Kit 

                                          Copyright (c) 2016-2019 Wataru KUNINO

作成にあたり下記のソースコードを利用しました。
https://github.com/Seeed-Studio/Grove_Serial_Camera_Kit
http://www.seeedstudio.com/wiki/Grove_-_Serial_Camera_Kit
	SerialCamera_DemoCode_CJ-OV528.ino
*******************************************************************************/

#define DEBUG

//  File SerialCamera_DemoCode_CJ-OV528.ino
//  8/8/2013 Jack Shao
//  Demo code for using seeeduino or Arduino board to cature jpg format
//  picture from seeed serial camera and save it into sd card. Push the
//  button to take the a picture .
//  For more details about the product please check http://www.seeedstudio.com/depot/

// 下記のソースコードの一部(関数名)が混じっています。CC BY-SA（表示 - 継承） 
// http://linksprite.com/wiki/index.php5?title=JPEG_Color_Camera_Serial_Interface_with_Built-in_Infrared_(TTL_level)

#define PIC_PKT_LEN    128                  //data length of each read, dont set this too big because ram is limited
#define PIC_FMT_VGA    7
#define PIC_FMT_CIF    5
#define PIC_FMT_OCIF   3
#define CAM_ADDR       0
#define CAM_SERIAL     hardwareSerial2

#define PIC_FMT        PIC_FMT_VGA

const byte cameraAddr = (CAM_ADDR << 5);  // addr
unsigned long picTotalLen = 0;            // picture length
int picNameNum = 0;
unsigned int pktCnt = 0; 

/*********************************************************************/
void _Cam_clearRxBuf(){
  while (CAM_SERIAL.available()) 
  {
    CAM_SERIAL.read(); 
  }
}
/*********************************************************************/
void _Cam_sendCmd(char cmd[], int cmd_len){
  for (char i = 0; i < cmd_len; i++) CAM_SERIAL.write(cmd[i]); 
  delay(1);						// WDT対策
}
/*********************************************************************/
int _Cam_readBytes(char *dest, int len, unsigned int timeout){
  int read_len = 0;
  unsigned long t = millis();
  unsigned long wdt = t+100;	// WDT対策
  
  while (read_len < len)
  {
    while (CAM_SERIAL.available()<1)
    {
      if ((millis() - t) > timeout)
      {
        return read_len;
      }
      if (millis() > wdt)
      {
        wdt=millis()+100;
        delay(1);
      }
    }
    *(dest+read_len) = CAM_SERIAL.read();
//  Serial.write(*(dest+read_len));
    read_len++;
  }
  return read_len;
}
/*********************************************************************/

int CamGetData(WiFiClient &client){
    byte pkt[PIC_PKT_LEN];
    pktCnt = (picTotalLen) / (PIC_PKT_LEN - 6); 
    if ((picTotalLen % (PIC_PKT_LEN-6)) != 0) pktCnt += 1;
    char cmd[] = { 0xaa, 0x0e | cameraAddr, 0x00, 0x00, 0x00, 0x00 };  
    for (unsigned int i = 0; i < pktCnt; i++){
        cmd[4] = i & 0xff;
        cmd[5] = (i >> 8) & 0xff;

        int retry_cnt = 0;
        
        retry:
        
        delay(10);
        _Cam_clearRxBuf(); 
        _Cam_sendCmd(cmd, 6); 
        uint16_t cnt = _Cam_readBytes((char *)pkt, PIC_PKT_LEN, 200);

        unsigned char sum = 0; 
        for (int y = 0; y < cnt - 2; y++){
            sum += pkt[y];
        }
        if (sum != pkt[cnt-2]){
            if (++retry_cnt < 100) goto retry;
            else break;
        }

        client.write((const uint8_t *)&pkt[4], cnt-6); 
        //if (cnt != PIC_PKT_LEN) break;
    }
    cmd[4] = 0xf0;
    cmd[5] = 0xf0; 
    _Cam_sendCmd(cmd, 6); 
    return (int)picTotalLen;
}

//Send Reset command
void CamInitialize(){
    char cmd[] = {0xaa,0x0d|cameraAddr,0x00,0x00,0x00,0x00} ;
    unsigned char resp[6];

    while (1)
    {
        //_Cam_clearRxBuf();
        _Cam_sendCmd(cmd,6);
        if (_Cam_readBytes((char *)resp, 6,500) != 6)
        {
            continue;
        }
        if (resp[0] == 0xaa && resp[1] == (0x0e | cameraAddr) && resp[2] == 0x0d && resp[4] == 0 && resp[5] == 0)
        {
            if (_Cam_readBytes((char *)resp, 6, 500) != 6) continue;
            if (resp[0] == 0xaa && resp[1] == (0x0d | cameraAddr) && resp[2] == 0 && resp[3] == 0 && resp[4] == 0 && resp[5] == 0) break;
        }
    }
    cmd[1] = 0x0e | cameraAddr;
    cmd[2] = 0x0d;
    _Cam_sendCmd(cmd, 6);
//    Serial.println("\nCamera initialization done.");
}

//Send pre take picture command
void CamPreCapture(){
    char cmd[] = { 0xaa, 0x01 | cameraAddr, 0x00, 0x07, 0x00, PIC_FMT };
    unsigned char resp[6];

    while (1)
    {
        _Cam_clearRxBuf();
        _Cam_sendCmd(cmd, 6);
        if (_Cam_readBytes((char *)resp, 6,100) != 6) continue;
        if (resp[0] == 0xaa && resp[1] == (0x0e | cameraAddr) && resp[2] == 0x01 && resp[4] == 0 && resp[5] == 0) break;
    }
}

void CamSizeCmd(int format){
    char cmd[] = { 0xaa, 0x01 | cameraAddr, 0x00, 0x07, 0x00, PIC_FMT };
    unsigned char resp[6];

	if(format==0) cmd[5]=0x07;			// 640 x 480	VGA
	else if(format==2) cmd[5]=0x03;		// 160 x 120	QQVGA (仕様書は160 x 128につき要確認)
	else cmd[5]=0x05;					// 320 x 240	QVGA
	
    while (1)
    {
        _Cam_clearRxBuf();
        _Cam_sendCmd(cmd, 6);
        if (_Cam_readBytes((char *)resp, 6,100) != 6) continue;
        if (resp[0] == 0xaa && resp[1] == (0x0e | cameraAddr) && resp[2] == 0x01 && resp[4] == 0 && resp[5] == 0) break;
    }
}

//Send take picture command
void CamCapture(){
  char cmd[] = { 0xaa, 0x06 | cameraAddr, 0x08, PIC_PKT_LEN & 0xff, (PIC_PKT_LEN>>8) & 0xff ,0}; 
  unsigned char resp[6];

  while (1)
  {
    _Cam_clearRxBuf();
    _Cam_sendCmd(cmd, 6);
    if (_Cam_readBytes((char *)resp, 6, 100) != 6) continue;
    if (resp[0] == 0xaa && resp[1] == (0x0e | cameraAddr) && resp[2] == 0x06 && resp[4] == 0 && resp[5] == 0) break; 
  }
  cmd[1] = 0x05 | cameraAddr;
  cmd[2] = 0;
  cmd[3] = 0;
  cmd[4] = 0;
  cmd[5] = 0; 
  while (1)
  {
    _Cam_clearRxBuf();
    _Cam_sendCmd(cmd, 6);
    if (_Cam_readBytes((char *)resp, 6, 100) != 6) continue;
    if (resp[0] == 0xaa && resp[1] == (0x0e | cameraAddr) && resp[2] == 0x05 && resp[4] == 0 && resp[5] == 0) break;
  }
  cmd[1] = 0x04 | cameraAddr;
  cmd[2] = 0x1;
  while (1) 
  {
    _Cam_clearRxBuf();
    _Cam_sendCmd(cmd, 6);
    if (_Cam_readBytes((char *)resp, 6, 100) != 6) continue;
    if (resp[0] == 0xaa && resp[1] == (0x0e | cameraAddr) && resp[2] == 0x04 && resp[4] == 0 && resp[5] == 0)
    {
      if (_Cam_readBytes((char *)resp, 6, 1000) != 6)
      {
        continue;
      }
      if (resp[0] == 0xaa && resp[1] == (0x0a | cameraAddr) && resp[2] == 0x01)
      {
        picTotalLen = (resp[3]) | (resp[4] << 8) | (resp[5] << 16); 
        //Serial.print("picTotalLen:");
        //Serial.println(picTotalLen);
        break;
      }
    }
  }
}


void CamStopTakePhotoCmd(){
}

void CamPowerCmd(int in){
}

void CamRatioCmd(int in){
}

void CamBaudRateCmd(int in){
}


