/*******************************************************************************
I2C LCD ドライバのテスト
                                           Copyright (c) 2014-2019 Wataru KUNINO
*******************************************************************************/

extern int ERROR_CHECK;

void setup(){
    delay(10);
    Serial.begin(115200);
    Serial.println("I2C LCD TESTER");
}

void loop(){
    char lcd[129];
    
    ERROR_CHECK=!ERROR_CHECK;
    if(ERROR_CHECK){
    	Serial.println("I2C LCD Testing ==========");
	    lcdSetup();
	    lcdPrint("I2C LCD");
	    lcdPrint2("TESTER");
	}else{
    	Serial.println("I2C without ACK ==========");
	    lcdSetup();
	    lcdPrint("TEST I2C");
	    lcdPrint2("W/O ACK");
	}
    
    Serial.println("lcdPrint");
    lcdPrint("Testing");
    lcdPrint2("lcdPrint");
    delay(1000);
    memset(lcd,0,129);
    for(int i=0 ; i< 33 ; i++ ){
        lcd[i]='A'+i;
        lcdPrint(lcd);
        delay(100);
    }
    Serial.println("lcdPrint2");
    lcdPrint("Testing");
    lcdPrint2("lcdPrint2");
    delay(1000);
    memset(lcd,0,129);
    for(int i=0 ; i< 9 ; i++ ){
        lcd[i]='a'+i;
        lcdPrint2(lcd);
        delay(100);
    }
    Serial.println("KATAKANA");
    lcdPrint("Testing");
    lcdPrint2("KATAKANA");
    delay(1000);
    memset(lcd,0,129);
    strcpy(lcd,"ｱｲｳｴｵｶｸｸｹｺｻｼｽｾｿ");
    lcdPrint(lcd);
    delay(1000);
    Serial.println("KATAKANA2");
    lcdPrint("ﾂｷﾞﾉﾃｽﾄﾊ");
    lcdPrint2("ｶﾀｶﾅ 2");
    delay(1000);
    memset(lcd,0,129);
    strcpy(lcd,"ﾀﾁﾂﾃﾄﾅﾆﾇﾈﾉﾊﾋﾌﾍﾎﾏﾐﾑﾒﾓ");
    lcdPrint(lcd);
    delay(1000);
    Serial.println("PrintIp");
    lcdPrint("ﾂｷﾞﾉﾃｽﾄﾊ");
    lcdPrint2("PrintIp");
    delay(1000);
    for(uint32_t i=3 ; i< UINT_MAX/2 ; i*=2 ){
        lcdPrintIp(i);
        delay(200);
    }
    Serial.println("PrintIp2");
    lcdPrint("ﾂｷﾞﾉﾃｽﾄﾊ");
    lcdPrint2("PrintIp2");
    delay(1000);
    for(uint32_t i=5 ; i< UINT_MAX/2 ; i*=2 ){
        lcdPrintIp2(i);
        delay(200);
    }
    Serial.println("PrintVal");
    lcdPrint("ﾂｷﾞﾉﾃｽﾄﾊ");
    lcdPrint2("PrintVal");
    delay(1000);
    for(int i=-2000 ; i< 2000 ; i+=125 ){
        lcdPrintVal("val test",i);
        delay(50);
    }
    Serial.println("PrintTime");
    lcdPrint("ﾂｷﾞﾉﾃｽﾄﾊ");
    lcdPrint2("PrintTime");
    delay(1000);
    for(unsigned long i=0ul ; i< (UINT_MAX)-(UINT_MAX)/16ul ; i+=(UINT_MAX)/16 ){
        lcdPrintTime(i);
        delay(200);
    }
    delay(1000);
}
