/*******************************************************************************
esp32 26 photof

有機ELディスプレイ フォトフレーム SSD1331ドライバ用

                                           Copyright (c) 2017-2019 Wataru KUNINO

・Wi-Fiカメラ子機が撮影した画像を表示します
・SDカード内の画像ファイル(JPEGとBMP)を表示するフォトフレーム機能も搭載

    OLED接続IOピン
        PIN_OLED_CS     17
        PIN_OLED_DC     26
        PIN_OLED_RST    25
        PIN_OLED_MOSI   23
        PIN_OLED_SCLK   18

    SDカード接続IOピン
        PIN_SD_CS       5
        PIN_SPI_MOSI    23
        PIN_SPI_MISO    19
        PIN_SPI_SCLK    18
*******************************************************************************/

#include <WiFi.h>                           // ESP32用WiFiライブラリ
#include <WiFiUdp.h>                        // UDP通信を行うライブラリ
#define PIN_BUZZER 12                       // GPIO 12にスピーカを接続
#define PIN_SW 0                            // IO 0にスイッチを接続
#define TIMEOUT 7000                        // タイムアウト 7秒
#define SSID "1234ABCD"                     // 無線LANアクセスポイントのSSID
#define PASS "password"                     // パスワード
#define SSID_AP "1234ABCD"                  // 本機の無線アクセスポイントのSSID
#define PASS_AP "password"                  // パスワード
#define PORT 1024                           // センサ機器 UDP受信ポート番号
#define DEVICE_CAM "cam_a"                  // カメラ(実習4/example15)名前5文字
#include "Adafruit_GFX.h"                   // OLED用の描画ライブラリ
#include "Adafruit_SSD1331.h"               // OLED用ドライバ
#include <SPI.h>
#include <SD.h>
#include <SPIFFS.h>
#include "JPEGDecoder.h"

Adafruit_SSD1331 oled = Adafruit_SSD1331(17, 26, 25);
WiFiUDP udpRx;                              // UDP通信用のインスタンスを定義
WiFiServer server(80);                      // Wi-Fiサーバ(ポート80=HTTP)定義
IPAddress ip;                               // IPアドレス保持用
unsigned long TIME;                         // タイマー用変数
int mode=1;                                 // Wi-Fiモード 0:親機AP 1:子機STA
int chime=0;                                // チャイムOFF

void setup(void){
    chimeBellsSetup(PIN_BUZZER);            // ブザー/LED用するPWM制御部の初期化
    Serial.begin(115200);
    oled.begin();
    oled.fillScreen(0x0000);
    oled.println("esp32 26 photo_f");       // タイトルをOLEDに表示する
    delay(500);
    if(!SD.begin()){                        // SDカードの使用開始
        oled.println("SD ERROR");           // エラー表示
        while(1);                           // 終了(動作停止)
    }
    WiFi.mode(WIFI_STA);                    // 無線LANを【STA】モードに設定
    WiFi.begin(SSID,PASS);                  // 無線LANアクセスポイントへ接続
    TIME=millis();
    while(WiFi.status() != WL_CONNECTED){   // 接続に成功するまで待つ
        oled.print("."); delay(500);        // 接続進捗を表示
        if(millis()-TIME>TIMEOUT||!digitalRead(PIN_SW)){    // 7秒経過orボタン押
            WiFi.disconnect();              // WiFiアクセスポイントを切断する
            oled.println("\nWi-Fi AP Mode");// 接続が出来なかったときの表示
            WiFi.mode(WIFI_AP); delay(100); // 無線LANを【AP】モードに設定
            WiFi.softAP(SSID_AP,PASS_AP);   // ソフトウェアAPの起動
            WiFi.softAPConfig(
                IPAddress(192,168,0,1),     // AP側の固定IPアドレスの設定
                IPAddress(0,0,0,0),         // 本機のゲートウェイアドレスの設定
                IPAddress(255,255,255,0)    // ネットマスクの設定
            );
            mode=0;
            oled.println(WiFi.softAPIP());
            break;
        }
    }
    oled.println("\nStart Servers");        // サーバ起動の表示
    server.begin();                         // Wi-Fiサーバを起動する
    udpRx.begin(PORT);                      // UDP待ち受け開始(STA側+AP側)
    if(mode)oled.println(WiFi.localIP());   // 本機のローカルアドレスを表示
    delay(TIMEOUT);
    jpegDrawSlideShowBegin(SD);
    TIME=millis();
}

void loop(){                                // 繰り返し実行する関数
    File file;
    WiFiClient client;                      // Wi-Fiクライアントの定義
    int i,t=0;                              // 整数型変数i,tを定義
    char c;                                 // 文字変数cを定義
    char s[65];                             // 文字列変数を定義 65バイト64文字
    char ret[65];                           // 文字列変数を定義 65バイト64文字
    int len=0;                              // 文字列長を示す整数型変数を定義
    int headF=0;                            // ヘッダフラグ(0:ヘッダ 1:BODY)

    client = server.available();            // 接続されたTCPクライアントを生成
    if(!client){                            // TCPクライアントが無かった場合
        len = udpRx.parsePacket();          // UDP受信パケット長を変数lenに代入
        if(len <= 0){                       // UDP受信無し時
            if(chime){                      // チャイムの有無
                chime=chimeBells(PIN_BUZZER,chime); // チャイム音を鳴らす
            }else if(!digitalRead(PIN_SW)){         // ボタン押下時
                oled.fillScreen(0x001F);            // ブルーバック画面
                oled.setCursor(1,1);                // 画面の左上
                oled.println("Halted!");            // Halt表示
                while(1);                           // 終了(動作停止)
            }else if(millis()-TIME > TIMEOUT){      // 画像表示タイミングの確認
                jpegDrawSlideShowNext(SD);          // SD内の画像を表示
                TIME=millis();                      // タイミングのリセット
            }
            return;
        }
        memset(s, 0, 65);                   // 文字列変数sの初期化(65バイト)
        udpRx.read(s, 64);                  // UDP受信データを文字列変数sへ代入
        if(len>64){len=64; udpRx.flush();}  // 受信データが残っている場合に破棄
        for(i=0;i<len;i++) if( !isgraph(s[i]) ) s[i]=' ';   // 特殊文字除去
        if(!strncmp(s,"Ping",4)) chime=2;   // 受信データがPingならチャイム鳴音
        if(len <= 8)return;                 // 8文字以下の場合はデータなし
        if(s[5]!='_' || s[7]!=',')return;   // 6文字目「_」8文字目「,」を確認
        if(strncmp(s,DEVICE_CAM,5)==0){     // カメラからの取得指示のとき
            char *cp=strchr(&s[8],',');     // cam_a_1,size, http://192.168...
            if(cp && strncmp(cp+2,"http://",7)==0) httpGet(cp+9,atoi(&s[8]));
            file = SD.open("/cam.jpg","r");
            jpegDrawSlide(file);
            file.close();
            TIME=millis();
        }
        return;                             // loop()の先頭に戻る
    }
    TIME=millis();
    memset(ret, 0, 65);                     // 文字列変数retの初期化(65バイト)
    while(client.connected()){              // 当該クライアントの接続状態を確認
        if(!client.available()){            // クライアントからのデータを確認
            if(millis()-TIME > TIMEOUT){    // TIMEOUT時にwhileを抜ける
                len=0; break;           
            }else continue;
        }
        TIME=millis();
        c=client.read();                    // データを文字変数cに代入
        if(c=='\n'){                        // 改行を検出した時
            if(headF==0){                   // ヘッダ処理
                if (len>6 && strncmp(s,"GET / ",6)==0){
                    len=0;
                    break;                     // 解析処理の終了
                }else if (len>6 && strncmp(s,"GET /",5)==0){
                    for(i=5;i<strlen(s);i++){  // 文字列を検索
                        if(s[i]==' '||s[i]=='&'||s[i]=='+'){        
                            s[i]='\0';         // 区切り文字時に終端する
                        }
                    }
                    snprintf(ret,64,"Get%s",&s[4]); oled.println(ret);
                    file = SD.open(&s[4],"r");
                    if(file==0){
                        client.println("HTTP/1.1 404 Not Found");
                        client.println("Connection: close");
                        client.println();
                        client.println("<HTML>404 Not Found</HTML>");
                        break;
                    }
                    client.println("HTTP/1.1 200 OK");
                    client.print("Content-Type: ");
                    if(strstr(&s[5],".jpg")) client.println("image/jpeg");
                    else client.println("text/plain");
                    client.println("Connection: close");
                    client.println();
                    t=0; while(file.available()){   // データ残確認
                        s[t]=file.read(); t++;      // ファイルの読込み
                        if(t >= 64){                // 転送処理
                            if(!client.connected()) break;
                            client.write((byte*)s,64);
                            t=0; delay(1);
                        }
                    }
                    if(t>0&&client.connected())client.write((byte*)s,t);
                    client.flush();         // ESP32用 ERR_CONNECTION_RESET対策
                    file.close();           // ファイルを閉じる
                    client.stop();          // クライアント切断
                    file = SD.open(&ret[3],"r");
                    jpegDrawSlide(file);    // 写真を表示する
                    file.close();
                    return;
                }
            }
            if( len==0 ) headF++;           // ヘッダの終了
            len=0;                          // 文字列長を0に
        }else if(c!='\r' && c!='\0'){
            s[len]=c;                       // 文字列変数に文字cを追加
            len++;                          // 変数lenに1を加算
            s[len]='\0';                    // 文字列を終端
            if(len>=64) len=63;             // 文字列変数の上限
        }
    }
    delay(10);                              // クライアント側の応答待ち時間
    if(client.connected()){                 // 当該クライアントの接続状態を確認
        html(client,ret,client.localIP());  // HTMLコンテンツを出力する
    }
    client.flush();                         // ESP32用 ERR_CONNECTION_RESET対策
    client.stop();                          // クライアントの切断
}
