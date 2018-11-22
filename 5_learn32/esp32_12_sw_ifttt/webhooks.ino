/*******************************************************************************
webhooks.ino
                                          Copyright (c) 2016-2019 Wataru KUNINO
*******************************************************************************/


//Congratulations! You've fired the dash event
//Bad Request

int webhooks(char *eventName, char *talken, char *message){
    WiFiClient client;                      // Wi-Fiクライアントの定義
    int i,t=0,size=0,ret=0;                 // 変数i,j,t=待ち時間,size=保存容量
    char c,s[129];                          // 文字変数、to=アクセス先,s=汎用
    int headF=0;                            // ヘッダフラグ(0:HEAD 1:EOL 2:DATA)
    unsigned long time;                     // 時間測定用

    snprintf(s,128,"/trigger/%s/with/key/%s",eventName,talken);
    Serial.println("HTTP host : maker.ifttt.com");
    Serial.println("Requesting event...");
    i=0; while( !client.connect("maker.ifttt.com",80) ){   // 外部サイトへ接続を実行する
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
    client.println("Host: maker.ifttt.com");// 相手先ホスト名
    client.println("Accept: */*");
    client.println("Content-Type: application/json");
    client.println("Connection: close");    // セッションの都度切断を指定
    
    snprintf(s,128,"{\"value1\":\"%s\"}",message);  // JSON作成
    client.print("Content-Length: ");
    client.println(strlen(s));
    client.println();
    client.println(s);                      // メッセージ送信
    
    time=millis();
    memset(s,0,128);
    while(t<3000){
        if(client.available()){             // クライアントからのデータを確認
            t=0;
            c=client.read();                // TCPデータの読み取り
            if(headF==2 && isprint(c) ){
                s[size]=c;
                size++;
                if(size>127) break;
                if(!strcmp(s,"Congratulations! You've fired the dash event")){
                    ret=1;
                    break;
                }
                if(!strcmp(s,"Bad Request")){
                    ret=-1;
                    break;
                }
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
    Serial.println(s);                      // 受信データを表示
    Serial.print(size);                     // 保存したファイルサイズを表示
    Serial.print(" Bytes, ");
    Serial.print(millis()-time);
    Serial.println("ms, Done");
    return ret;
}
