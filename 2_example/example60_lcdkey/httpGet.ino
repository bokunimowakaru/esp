/*******************************************************************************
HTMLコンテンツ取得
                                          Copyright (c) 2016-2017 Wataru KUNINO
*******************************************************************************/

// #define TIMEOUT 3000                     // タイムアウト 3秒
#include <SPIFFS.h>
#include <WiFi.h>                           // ESP32用WiFiライブラリ

int httpGet(char *url,int max_size){
    File file;
    WiFiClient client;                      // Wi-Fiクライアントの定義
    int i,t=0,size=0;                       // 変数i,t=待ち時間,size=保存容量
    char c,to[33],s[65];                    // 文字変数、to=アクセス先,s=汎用
    char *cp;
    int headF=0;                            // ヘッダフラグ(0:HEAD 1:EOL 2:DATA)

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
    Serial.print("Max Size  : ");
    Serial.println(max_size);
    Serial.println("Recieving contents");
    i=0; while( !client.connect(to,80) ){   // 外部サイトへ接続を実行する
        i++; if(i>=3){                      // 失敗時のリトライ処理
            Serial.println("ERROR: Failed to connect");
            return 0;
        }
        Serial.println("WARN: Retrying");
        delay(10);                          // 10msのリトライ待ち時間
    }
    file = SPIFFS.open(s,"w");              // 保存のためにファイルを開く
    if(file==0){
        Serial.println("ERROR: FALIED TO OPEN. Please format SPIFFS.");
        client.flush();                     // ESP32用 ERR_CONNECTION_RESET対策
        client.stop();                      // クライアントの切断
        return 0;                           // ファイルを開けれなければ戻る
    }
    client.print("GET ");                   // HTTP GETコマンドを送信
    client.print(s);                        // 相手先ディレクトリを指定
    client.println(" HTTP/1.1");            // HTTPプロトコル
    client.print("Host: ");                 // ホスト名を指定
    client.print(to);                       // 相手先ホスト名
    client.println();                       // ホスト名の指定を終了
    client.println("Connection: close");    // セッションの都度切断を指定
    
    // 以下の処理はデータの受信完了まで終了しないので、その間に届いたデータを
    // 損失してしまう場合があります。
    // Wi-Fiカメラの初期版では、送信完了まで20秒くらいかかります。
    while(t<TIMEOUT){             
        if(client.available()){             // クライアントからのデータを確認
            t=0;
            c=client.read();                // TCPデータの読み取り
            if(headF==2){
                // 複数バイトread命令を使用する
                // int WiFiClient::read(uint8_t *buf, size_t size)
                s[0]=c; size++;             // 既に取得した1バイト目を代入
                i=client.read((uint8_t *)s+1,63);   // 63バイトを取得
                // 戻り値はrecvが代入されている
                // int res = recv(fd(), buf, size, MSG_DONTWAIT);
                if(i>0){                            // 受信データがある時
                    file.write((const uint8_t *)s, i+1);
                    size += i;
                } else file.write(c);
                if(size%512==0){
                    Serial.print('.');
                    lcd.print('.');
                }
                if(size >= max_size) break;
                continue; 
            }
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
    if(i) file.write((const uint8_t *)s, i); 
    file.close();                           // ファイルを閉じる
    client.stop();                          // クライアントの切断
    Serial.println();
    Serial.print(size);                     // 保存したファイルサイズを表示
    Serial.println("Bytes, Done");
    return size;
}
