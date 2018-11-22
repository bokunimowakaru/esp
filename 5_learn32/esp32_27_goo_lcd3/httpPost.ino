/*******************************************************************************
HTTPアクセス POST

                                          Copyright (c) 2016-2019 Wataru KUNINO
*******************************************************************************/
#define DEBUG_HTTPG
#define HTTP_TIMEOUT 700                    // タイムアウト 0.7秒(LAN用)
#include <WiFi.h>                           // ESP32用WiFiライブラリ

int httpPost(const char *url,const char *message,char *out, int max_size){
    WiFiClient client;                      // Wi-Fiクライアントの定義
    int i,t=0,size=0,ret=0;                 // 変数i,j,t=待ち時間,size=保存容量
    char c,to[33],s[65];                    // 文字変数、to=アクセス先,s=汎用
    char *cp;
    int headF=0;                            // ヘッダフラグ(0:HEAD 1:EOL 2:DATA)
    unsigned long time;                     // 時間測定用

    memset(to,0,33);
    memset(s,0,65);
    if(!isgraph(url[0])) return 0;          // URLが無い時は処理を行わない
    cp=strchr(url,'/');                     // URL内の区切りを検索
    if(!cp){                                // 区切りが無かった場合は
        strncpy(to,url,32);                 // 入力文字はホスト名のみ
        strncpy(s,"/index.html",32);        // 取得ファイルはindex.html
    }else if(cp - url <= 32){               // 区切り文字までが32文字以下のとき
        strncpy(to,url,cp-url);             // 区切り文字までがホスト名
        strncpy(s,cp,64);                   // 区切り文字以降はファイル名
    }
    for(i=0;i<32;i++)if(!isgraph(to[i]))to[i]='\0'; to[32]=0;
    for(i=0;i<64;i++)if(!isgraph(s[i]))s[i]='\0'; s[64]=0;
    Serial.print("HTTP host : ");
    Serial.println(to);
    Serial.print("Filename  : ");
    Serial.println(s);
    Serial.println("Recieving contents...");
    i=0; while( !client.connect(to,80) ){   // 外部サイトへ接続を実行する
        i++; if(i>=3){                      // 失敗時のリトライ処理
            Serial.println("ERROR: Failed to connect");
            return 0;
        }
        Serial.println("WARN: Retrying");
        delay(10);                          // 10msのリトライ待ち時間
    }
    client.print("POST ");                  // HTTP POSTコマンドを送信
    client.print(s);                        // 相手先ディレクトリを指定
    client.println(" HTTP/1.0");            // HTTPプロトコル
    client.println("User-Agent: esp-wroom-32");
    client.print("Host: ");                 // ホスト名を指定
    client.print(to);                       // 相手先ホスト名
    client.println();                       // ホスト名の指定を終了
    client.println("Accept: */*");
//  client.println("Content-Type: application/json");
    client.println("Connection: close");    // セッションの都度切断を指定
    client.print("Content-Length: ");
    client.println(strlen(message));
    client.println();
    client.println(message);                // メッセージ送信
    
    time=millis();
    while(t<HTTP_TIMEOUT){
        if(client.available()){             // クライアントからのデータを確認
            t=0;
            c=client.read();                // TCPデータの読み取り
            if(headF==2 && isprint(c) ){
                out[size]=c;
                size++;
                if(size >= max_size) break;
            }
            #ifdef DEBUG_HTTPG
                Serial.print(c);            // ヘッダ部のシリアル出力表示
            #endif
            if(headF==1){                   // 前回が行端の時
                if(c=='\n') headF=2;        // 今回も行端ならヘッダ終了
                else if(c!='\r') headF=0;
                continue;
            }
            if(c=='\n') headF=1;            // 行端ならフラグを変更
            continue;
        }
        t++;
        delay(1);
    }
    client.flush();
    client.stop();                          // クライアントの切断
    Serial.println(out);                      // 受信データを表示
    Serial.print(size);                     // 保存したファイルサイズを表示
    Serial.print(" Bytes, ");
    Serial.print(millis()-time);
    Serial.println("ms, Done");
    return size;
}

int httpPost(const char *url,const char *message){
    char out[1];
    httpPost(url,message, out,1);
}
