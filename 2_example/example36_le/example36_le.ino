/*******************************************************************************
Example 36(=32+4): 乾電池駆動に向けた低消費電力動作のサンプル
                                           Copyright (c) 2016-2017 Wataru KUNINO
*******************************************************************************/
#include <WiFi.h>                           // ESP32用WiFiライブラリ
#include <WiFiUdp.h>                        // UDP通信を行うライブラリ
#include "esp_deep_sleep.h"                 // ESP32用Deep Sleep ライブラリ
#define PIN_EN 2                            // GPIO 2(24番ピン)をセンサの電源に
#define PIN_AIN 34                          // GPIO 34 ADC1_CH6(6番ピン)をADCに
#define SSID "1234ABCD"                     // 無線LANアクセスポイントのSSID
#define PASS "password"                     // パスワード
#define SENDTO "192.168.0.255"              // 送信先のIPアドレス
#define PORT 1024                           // 送信のポート番号
#define SLEEP_P 50*1000000                  // スリープ時間 50秒(uint32_t)

void setup(){                               // 起動時に一度だけ実行する関数
    pinMode(PIN_EN,OUTPUT);                 // センサ用の電源を出力に
    pinMode(PIN_AIN,INPUT);
    Serial.begin(115200);                   // 動作確認のためのシリアル出力開始
    Serial.println("ESP32 eg.04 SW");       // 「ESP32 eg.04」をシリアル出力表示
    WiFi.mode(WIFI_STA);                    // 無線LANをSTAモードに設定
    WiFi.begin(SSID,PASS);                  // 無線LANアクセスポイントへ接続
    while(WiFi.status() != WL_CONNECTED){   // 接続に成功するまで待つ
        delay(500);                         // 待ち時間処理
        digitalWrite(PIN_EN,!digitalRead(PIN_EN));      // LEDの点滅
        Serial.print(".");
    }
    Serial.println(WiFi.localIP());         // 本機のIPアドレスをシリアル出力
}

void loop() {
    WiFiUDP udp;                            // UDP通信用のインスタンスを定義
    int adc;                                // 整数型変数adcを定義
    
    digitalWrite(PIN_EN,HIGH);              // センサ用の電源をONに
    delay(5);                               // 起動待ち時間
    adc=analogRead(PIN_AIN);                // AD変換器から値を取得
    digitalWrite(PIN_EN,LOW);               // センサ用の電源をOFFに
    udp.beginPacket(SENDTO, PORT);          // UDP送信先を設定
    udp.println(adc);                       // 変数adcの値を送信
    Serial.println(adc);                    // シリアル出力表示
    udp.endPacket();                        // UDP送信の終了(実際に送信する)
    delay(200);                             // 送信待ち時間
    esp_deep_sleep_enable_timer_wakeup(SLEEP_P);
    esp_deep_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH,ESP_PD_OPTION_OFF);
    esp_deep_sleep_pd_config(ESP_PD_DOMAIN_RTC_SLOW_MEM,ESP_PD_OPTION_OFF);
    esp_deep_sleep_pd_config(ESP_PD_DOMAIN_RTC_FAST_MEM,ESP_PD_OPTION_OFF);
    esp_deep_sleep_pd_config(ESP_PD_DOMAIN_MAX,ESP_PD_OPTION_OFF);
    esp_deep_sleep_start();                 // スリープモードへ移行する
    while(1) delay(100);                    // 終了(永久ループ)
}
/*
esp_deep_sleep_enable_timer_wakeup(uint64_t time_in_us);
    Enable wakeup by timer
    @param time_in_us   time before wakeup, in microseconds
    
esp_deep_sleep_enable_ext0_wakeup(gpio_num_t gpio_num, int level);
    Enable wakeup using [A] pin
    external wakeup feature of RTC_IO peripheral
    @param gpio_num     GPIO number used as wakeup source. Only GPIOs which are have RTC 
                        functionality can be used: 0,2,4,12-15,25-27,32-39. 

esp_deep_sleep_pd_config(esp_deep_sleep_pd_domain_t domain, 
                                                    esp_deep_sleep_pd_option_t option);
    Set power down mode for an RTC power domain in deep sleep
    @param domain       power domain to configure
                        ESP_PD_DOMAIN_RTC_PERIPH,      //!< RTC IO, sensors and ULP co-processor 
                        ESP_PD_DOMAIN_RTC_SLOW_MEM,    //!< RTC slow memory 
                        ESP_PD_DOMAIN_RTC_FAST_MEM,    //!< RTC fast memory 
                        ESP_PD_DOMAIN_MAX              //!< Number of domains 
    @param option       power down option (ESP_PD_OPTION_OFF, ESP_PD_OPTION_ON, or ESP_PD_OPTION_AUTO)
                        ESP_PD_OPTION_OFF,      //!< Power down the power domain in deep sleep 
                        ESP_PD_OPTION_ON,       //!< Keep power domain enabled during deep sleep 
                        ESP_PD_OPTION_AUTO      //!< Keep power domain enabled in deep sleep, 
                        if it is needed by one of the wakeup options. Otherwise power it down 
                        
esp_deep_sleep_start();
    Enter deep sleep with the configured wakeup options
    ### This function does not return

esp_deep_sleep(uint64_t time_in_us)
    Enter deep-sleep mode
    esp_deep_sleep DOES NOT shut down WiFi, BT, and higher level protocol connections gracefully. 
    @param time_in_us   deep-sleep time, unit: microsecond
    
参考文献：
https://github.com/espressif/esp-idf/blob/master/components/esp32/include/esp_deep_sleep.h
*/

