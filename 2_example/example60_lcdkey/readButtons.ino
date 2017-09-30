/*******************************************************************************
ESP32用 アナログ入力関数 readButtons mvAnalogIn
                                           Copyright (c) 2017 Wataru KUNINO
*******************************************************************************/

#define BUTTON_UP       0x08
#define BUTTON_DOWN     0x04
#define BUTTON_LEFT     0x10
#define BUTTON_RIGHT    0x02
#define BUTTON_SELECT   0x01
// #define DEBUG_ADC

uint8_t readButtons(uint8_t PIN) {
    int in3 = analogRead(PIN);
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
    if (in3 > 1558) return 0x00;            // 実測値= 1.58V 1660
    if (in3 <  224) return BUTTON_RIGHT;    // 実測値= 0.19V    0
    if (in3 <  680) return BUTTON_UP;       // 実測値= 0.56V  448
    if (in3 < 1056) return BUTTON_DOWN;     // 実測値= 0.95V  912
    if (in3 < 1328) return BUTTON_LEFT;     // 実測値= 1.19V 1200
    if (in3 < 1558) return BUTTON_SELECT;   // 実測値= 1.41V 1456
    return 0x1F;
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
