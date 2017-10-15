/*******************************************************************************
ESP32用 アナログ入力関数 readButtons mvAnalogIn
                                           Copyright (c) 2017 Wataru KUNINO
*******************************************************************************/

#include "readButtons.h"
// #define DEBUG_ADC

int _PIN_KEY_5V_DIV=2;

uint8_t readButtons(uint8_t PIN,int div_n){
    _PIN_KEY_5V_DIV=div_n;
    return readButtons(PIN);
}
uint8_t readButtons(uint8_t PIN){
    int in3 = analogRead(PIN);
    delay(1);
    int in3_2 = analogRead(PIN);
    
    if( abs(in3-in3_2)>5 ) return 0x1F;
    
    #ifdef DEBUG_ADC
        float ad3;
        if( in3 > 2599 ){
            ad3 = -1.457583e-7 * (float)in3 * (float)in3
                + 1.510116e-3 * (float)in3
                - 0.573300;
        }else{
            ad3 = 8.378998e-4 * (float)in3 + 1.891456e-1;
        }
        Serial.print("ADC = ");
        Serial.print(ad3);
        Serial.print(" [V], ");
        Serial.println(in3);
    #endif
    if(_PIN_KEY_5V_DIV < 0 ) return 0x1F;
    if(_PIN_KEY_5V_DIV < 2){
        if (in3 >=3257) return 0x00;            		// 実測値= 3.17V 4095
        if (in3 <  274) return BUTTON_RIGHT;    		// 実測値= 0.19V    0
        if (in3 < 1019) return BUTTON_UP;       		// 実測値= 0.65V  548
        if (in3 < 1955) return BUTTON_DOWN;     		// 実測値= 1.44V 1490
        if (in3 < 3257){
        	if(_PIN_KEY_5V_DIV==1) return BUTTON_LEFT;	// 実測値= 2.22V 2420
        	else return BUTTON_SELECT; // LEFTキーをSELECTキーとして使用する
        }
        return 0x1F;
    }else{
        if (in3 >=1558) return 0x00;            // 実測値= 1.58V 1660
        if (in3 <  224) return BUTTON_RIGHT;    // 実測値= 0.19V    0
        if (in3 <  680) return BUTTON_UP;       // 実測値= 0.56V  448
        if (in3 < 1056) return BUTTON_DOWN;     // 実測値= 0.95V  912
        if (in3 < 1328) return BUTTON_LEFT;     // 実測値= 1.19V 1200
        if (in3 < 1558) return BUTTON_SELECT;   // 実測値= 1.41V 1456
        return 0x1F;
    }
//  return 0x1F;
}

float mvAnalogIn(uint8_t PIN){
    int in0,in3;
    float ad0,ad3;
    
    analogSetPinAttenuation(PIN,ADC_11db);
    in3=analogRead(PIN);
    
    if( in3 > 2599 ){
        ad3 = -1.457583e-7 * (float)in3 * (float)in3
            + 1.510116e-3 * (float)in3
            - 0.573300;
    }else{
        ad3 = 8.378998e-4 * (float)in3 + 1.891456e-1;
    }
    #ifdef DEBUG_ADC
        Serial.print("ADC (ATT=3;11dB) = ");
        Serial.print(ad3,3);
        Serial.print(" [V], ");
        Serial.println(in3);
    #endif
    if( in3 < 200 ){
        analogSetPinAttenuation(PIN,ADC_0db);
        in0=analogRead(PIN);
        ad0 = 2.442116e-4 * (float)in0 + 1.075584e-1;
        #ifdef DEBUG_ADC
            Serial.print("ADC (ATT=0; 0dB) = ");
            Serial.print(ad0,3);
            Serial.print(" [V], "); 
            Serial.println(in0);
        #endif
        if( in3 >= 100 ){
            ad3 = ad3 * ((float)in3 - 100.) / 100.
                + ad0 * (200. - (float)in3) / 100.;
        }else{
            ad3 = ad0;
        }
    }
    return ad3 * 1000.;
}
