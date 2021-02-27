/*******************************************************************************
Example 29u + 32: (IoTセンサ) 無線LAN プロミスキャス センサ
MACブロードキャスト／ Amazon Dash ボタンを検出 [UART出力版]

スマートフォンやPCが自宅の無線LANへ自動接続したときや、Amazon Dashボタンが押下
されたときなどに、送信するMACブロードキャストを検出します。
プロミスキャスモードで待ち受け、予め設定した特定のMACアドレスの端末を検出した時に
シリアル送信（UART出力）によって通知します。

・設定ファイルはシリアル通信で受け取ることが出来ます。
　（TFTPで受けるバージョンはexample29_dashです）
・起動後、普段はプロミスキャスモードで特定のMACアドレスを待ち受けます。  

ご注意：ネットセキュリティ上の脆弱性の十分に注意して利用してください。

同じ機器から同種別のデータを連続して受信した場合は出力しません(待機時間0.5秒)。
チャンネルを変更するには、シリアルから「channel=数字」と改行を入力してください。

                                           Copyright (c) 2017-2021 Wataru KUNINO
********************************************************************************
・シリアル速度は115200bpsです。

・出力形式：先頭に'(0x27)とスペース(0x20)に続いて6桁のMACアドレスをテキスト出力

            ' xx:xx:xx:xx:xx:xx
            ' xx:xx:xx:xx:xx:xx
            
・入力パラメータ：シリアルからコマンドを入力して変更
    
    ???
        シリアル動作確認
        
    channel=1～12
        検出したい Wi-Fiチャンネルを指定する
        
    channel?
        Wi-Fiチャンネルを確認する
        
    filter=0～5
        0: フィルタなし
        2: (標準)0.5秒以内に検出した同じMACの出力を保留
        5: 10秒以内の同じMACの出力を保留
    
    mode=0～3
        0: (標準) MAC出力モード
        1: adashモード(登録した5個までのAmazon Dashボタンの検出を出力する)
        2: phoneモード(登録した5台までのスマートフォンの検出を出力する)
        3: 1+2の混在モード
        
    adash=N,XX:XX:XX:XX:XX:XX
        N: adash番号1～5
        XX:XX:XX:XX:XX:XX: MACアドレス
        
    phone=N,XX:XX:XX:XX:XX:XX
        N: phone番号1～5
        XX:XX:XX:XX:XX:XX: MACアドレス
        
    phone?
        phone=N,macで登録したMACアドレスを表示する
        
    adash?
        adash=N,macで登録したMACアドレスを表示する
        
    time?
        各機器の待ち時間情報を表示する
        
    wifi=0～1
        0: OFF
        1: ON
        
    save!
        設定を保存する

*******************************************************************************/
#include <SPIFFS.h>
#include <WiFi.h>                           // ESP32用WiFiライブラリ
// #include <BLEDevice.h>
#include <esp_wifi.h>
#define PIN_EN 13                           // IO 13 をLEDなどへ接続
#define WAIT_A 10000                        // adash用の保持時間 10 秒
#define WAIT_P 15000000                     // phone用の保持時間 4 時間 10分
#define FILENAME "/adash.ini"               // 設定ファイル名
#define SPIFFS_ON                           // SPIFFS使用時に定義する
int PIN_HOLD=500;                           // 検出時の保持時間を設定(500ms)
unsigned long reset_time;                   // LED消灯時刻
char uart[33];                              // UART受信バッファ
byte uart_n=0;                              // UART受信文字数
byte mac[6];                                // プロミスキャス受信デバイスのMAC
boolean wifi=1;
byte channel;                               // 無線LAN物理チャンネル
byte mode=0;
byte filter=2;
byte adash[5][6];
unsigned long adash_time[5];
byte adash_time_s=0x00;
byte phone[5][6];
unsigned long phone_time[5];
byte phone_time_s=0x00;

// BLEScan* bleScan;                           // BLE用インスタンス

boolean mac_parser(byte *mac, char *s){
    char in[3],*err;
    in[2]='\0';
    if(strlen(s)<17) return false;
    for(byte i=0; i<17 ; i+=3){
        strncpy(in,s+i,2);
        mac[i/3]=(byte)strtol(in,&err,16);
        if(strlen(err)) return false;
    }
    return true;
}

boolean mac_print(byte *mac){
    byte i;
    Serial.print("' ");
    for (i=0; i<6; i++){
        if(mac[i] < 0x10) Serial.print(0);
        Serial.print(mac[i], HEX);
        if(i != 5)Serial.print(":");
    }
    Serial.println();
    return true;
}

boolean load(File *file){
    byte in[10];
    file->read(in,9);
    if(strncmp((char *)in,"adash_ini",9)) return false;
    channel=(byte)file->read();
    mode=(byte)file->read();
    filter=(byte)file->read();
    file->read(adash[0],30);
    file->read(phone[0],30);
    return true;
}

void save(File *file){
    byte in[10];
    file->print("adash_ini");
    file->write(channel);
    file->write(mode);
    file->write(filter);
    file->write(adash[0],30);
    file->write(phone[0],30);
}

byte set_filter(byte i){
    switch(i){
        case 0:
            promiscuous_fchold(0);
            PIN_HOLD=1;
            break;
        case 1:
            promiscuous_fchold(0);
            PIN_HOLD=500;
            break;
        case 2:
            promiscuous_fchold(1);
            PIN_HOLD=500;
            break;
        case 3:
            promiscuous_fchold(1);
            PIN_HOLD=1000;
            break;
        case 4:
            promiscuous_fchold(1);
            PIN_HOLD=3000;
            break;
        case 5:
            promiscuous_fchold(1);
            PIN_HOLD=10000;
            break;
        default: i=0xFF;
    }
    return i;
}

void setup(){                               // 起動時に一度だけ実行する関数
    pinMode(PIN_EN,OUTPUT);                 // LEDなど用の出力
    pinMode(0,INPUT);                       // BOOTボタン入力
    Serial.begin(115200);                   // 動作確認のためのシリアル出力開始
    // Serial.println("Started");
    wifi_second_chan_t secondChan;
    esp_wifi_get_channel(&channel,&secondChan);         // チャンネルを取得
    memset(uart,0,33);                      // UART受信バッファの初期化
    memset(adash,0,30);
    memset(phone,0,30);
    for(byte i=0;i<5;i++) adash_time[i]=0;
    for(byte i=0;i<5;i++) phone_time[i]=0;
    #ifdef SPIFFS_ON
    if(!SPIFFS.begin()){                    // ファイルシステムSPIFFSの開始
        Serial.println("SPIFS Formatting"); // フォーマット中をLCDへ表示
        SPIFFS.format(); SPIFFS.begin();    // エラー時にSPIFFSを初期化
    }
    File file = SPIFFS.open(FILENAME,"r");  // 設定ファイルを開く
    if(file){
        load(&file);
        file.close();                       // ファイルを閉じる
    }
    #endif // SPIFFS
    set_filter(filter);
    promiscuous_uart(false);                // ライブラリ側のUART出力を無効に
    promiscuous_start(channel);             // プロミスキャスモードへ移行する
    // BLEDevice::init("");
}

void loop(){
    unsigned long p_time=millis();
    if(!p_time){adash_time_s=0x00; phone_time_s=0x00; }
    byte i,j;
    while(promiscuous_get_mac(mac)){        // 検知時
        if(!mode) mac_print(mac);
        else{
            if(mode & 0x1){                 // mode=1または3のとき
                for(i=0;i<5;i++) if(!memcmp(adash[i],mac,6)) break;
                if(i!=5){
                    if(adash_time[i] < p_time && !((adash_time_s>>i)&0x1) ){
                        Serial.print("' adash=");
                        Serial.println(i+1);
                    }
                    adash_time[i] = p_time+WAIT_A;
                    if(p_time < WAIT_A ) adash_time_s |= 0x1<<i;
                }
            }
            if(mode & 0x2){                 // mode=2または3のとき
                for(i=0;i<5;i++) if(!memcmp(phone[i],mac,6)) break;
                if(i!=5){
                    if(phone_time[i] < p_time && !((phone_time_s>>i)&0x1) ){
                        Serial.print("' phone=");
                        Serial.println(i+1);
                    }
                    phone_time[i] = p_time+WAIT_P;
                    if(p_time < WAIT_P ) phone_time_s |= 0x1<<i;
                }
            }
        }
        digitalWrite(PIN_EN,HIGH);          // LEDなどを点灯
        reset_time=(p_time-1)%PIN_HOLD;     // 消灯時刻を設定
    }
    if(Serial.available()){                 // UART受信時
        char c=Serial.read();               // 受信文字を変数cで保持
        if( c=='\r' || c=='\n' ){           // 改行コードの時
            if(!strncmp(uart,"wifi=",5)){   // スキャンの有効/無効
                i=atoi(uart+5);             // 値を取得
                if(i<2){
                    if(!wifi && i) promiscuous_start(channel);
                    if(wifi && !i) promiscuous_stop();
                    wifi=(boolean)i;
                    uart[4]='?';
                }
            }
            if(!strncmp(uart,"wifi?",5)){   // スキャンの有効/無効
                Serial.print("' adash wifi=");
                Serial.println(wifi);
            }
            if(!strncmp(uart,"channel=",8)){// チャンネル設定コマンドの時
                promiscuous_stop();         // プロミスキャスを停止
                i=atoi(uart+8);             // 値を取得
                if(i > 0 && i <= 12){
                    channel=i;              // チャンネルを変更
                    promiscuous_start(channel); // プロミスキャスモードへ移行する
                    promiscuous_ready();        // 検知処理完了
                    uart[7]='?';
                }
            }
            if(!strncmp(uart,"channel?",8)){// チャンネル設定コマンドの時
                Serial.print("' adash channel=");
                Serial.println(channel);
            }
            if(!strncmp(uart,"filter=",7)){ // フィルタのレベル入力
                i=(byte)atoi(uart+7);
                i=set_filter(i);
                if(i<=5){
                    filter=i;
                    promiscuous_ready();
                    uart[6]='?';
                }
            }
            if(!strncmp(uart,"filter?",7)){   // モード設定
                Serial.print("' adash filter=");
                Serial.println(filter);
            }
            if(!strncmp(uart,"mode=",5)){   // モード設定
                i=(byte)atoi(uart+5);
                if(i>=0 && i<=3){
                    mode=i;
                    uart[4]='?';
                }
            }
            if(!strncmp(uart,"mode?",5)){   // モード設定
                Serial.print("' adash mode=");
                Serial.println(mode);
            }
            if(!strncmp(uart,"adash=",6)){  // adash登録
                i=(byte)atoi(uart+6);
                if(i>0 && i<=5){
                    if(mac_parser(mac,uart+8)){
                        memcpy(adash[i-1],mac,6);
                        Serial.print("' adash adash=");
                        Serial.println(i);
                    }
                }
            }
            if(!strncmp(uart,"adash?",6)){   // 登録済みMAC表示
                Serial.println("' adash list_mac");
                for(i=0;i<5;i++) mac_print(adash[i]);
            }
            if(!strncmp(uart,"phone=",6)){  // phone登録
                i=(byte)atoi(uart+6);
                if(i>0 && i<=5){
                    if(mac_parser(mac,uart+8)){
                        memcpy(phone[i-1],mac,6);
                        Serial.print("' adash phone=");
                        Serial.println(i);
                    }
                }
            }
            if(!strncmp(uart,"phone?",6)){   // 登録済みMAC表示
                Serial.println("' phone list_mac");
                for(i=0;i<5;i++) mac_print(phone[i]);
            }
            if(!strncmp(uart,"time?",5)){   // 登録済みMAC表示
                Serial.print("' adash time=");
                Serial.println(p_time);
                Serial.println("' adash list_time");
                for(i=0;i<5;i++){
                    if( (adash_time_s>>i)&0x1 ) Serial.print("' 1 ");
                    else Serial.print("' 0 ");
                    Serial.println(adash_time[i]);
                }
                Serial.println("' phone list_time");
                for(i=0;i<5;i++){
                    if( (adash_time_s>>i)&0x1 ) Serial.print("' 1 ");
                    else Serial.print("' 0 ");
                    Serial.println(phone_time[i]);
                }
            }
            if(!strncmp(uart,"save!",5)){   // 設定の保存
                #ifdef SPIFFS_ON
                File file = SPIFFS.open(FILENAME,"w");
                if(file){
                    save(&file);
                    Serial.print("' save ");
                    Serial.println(FILENAME);
                    file.close();
                }
                #endif // SPIFFS
            }
            if(!strncmp(uart,"???",3)){     // 登録済みMAC表示
                Serial.println("' adash https://goo.gl/McJuV5");
            }
            memset(uart,0,33);
            uart_n=0;
            return;
        }
        uart[uart_n]=c;
        uart_n++;
        if(uart_n>31)uart_n=31;             // 最大値は31(32文字)
    }
    if(digitalRead(0) == 0){
        Serial.println("QUITTED!");
        promiscuous_stop();
        delay(100);
    }
    if(p_time%PIN_HOLD != reset_time) return;
    digitalWrite(PIN_EN,LOW);               // LEDなどを点灯
    promiscuous_ready();                    // 検知処理完了
}
