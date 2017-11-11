/*******************************************************************************
HTMLコンテンツ取得
                                          Copyright (c) 2016-2017 Wataru KUNINO
*******************************************************************************/
/*

# http://www.river.go.jp/kawabou/ipSuiiKobetu.do?obsrvId=2206800400013&gamenId=01-1003&stgGrpKind=survOnly&fvrt=yes&timeType=10

<table class="tb1" border="0" cellspacing="0" cellpadding="0" width="260px">
<table class="tb1"
<tr style="line-height:13px;">
<tr style="line-height:13px;">
<td width="30%" class="tb1td1Right">
10/25 20:20
</td>
<td width="21%" class="tb1td2Right" style="color:#000000; background-color:#FFFFFF;">
2.21
</td>
curl -v "http://www.river.go.jp/kawabou/ipSuiiKobetu.do?obsrvId=2206800400013&gamenId=01-1003&stgGrpKind=survOnly&fvrt=yes&timeType=10"|head
> GET /kawabou/ipSuiiKobetu.do?obsrvId=2206800400013&gamenId=01-1003&stgGrpKind=survOnly&fvrt=yes&timeType=10 HTTP/1.1
> User-Agent: curl/7.38.0
> Host: www.river.go.jp
> Accept: *\/*
>
< HTTP/1.1 200 OK
*/

// #define DEBUG_HTTPG

float _httpg_depth=-1;
int  _httpg_hour=-1;
int  _httpg_minute=-1;

float httpGetBufferedDepth(){
    return _httpg_depth;
}

float httpGetBufferedDepth(char *s,int len){
    snprintf(s,len,"%02d:%02d:00, %1.2f",_httpg_hour,_httpg_minute,_httpg_depth);
    s[len]=0;
    return _httpg_depth;
}

int httpGetBufferedHour(){
    return _httpg_hour;
}

int httpGetBufferedMinute(){
    return _httpg_minute;
}

int _httpGetFindTag(char *s, char *keyword){
    char *cp;
    int ret=0;
    
    cp=s;
    while(1){
        cp=strstr(cp,keyword);   // キーワード
        if( !cp || (int)(cp-s) >= 128 ) break;
        ret++;
        cp++;
    }
    return ret;
}

void _httpGetShiftBuff(char *s){
    memcpy(s,s+128,128);    // 変数sの後半を前半へ移動
    memset(s+128,0,129);    // 変数sの後半だけ初期化
}


float httpGetRiverDepth(char *id){
    File file;
    WiFiClient client;                      // Wi-Fiクライアントの定義
    int i,len,t=0,size=0;                   // 変数i,j,t=待ち時間,size=保存容量
    char c,to[33],s[257];                   // 文字変数、to=アクセス先,s=汎用
    char *cp,*cp2;
    int headF=0;                            // ヘッダフラグ(0:HEAD 1:EOL 2:DATA)
    unsigned long time;                     // 時間測定用
    float ret=-1.;

    if(strlen(id) != 13) return 0;
    memset(s,0,257);
    snprintf(s,256,"www.river.go.jp/kawabou/ipSuiiKobetu.do\?obsrvId=%s\&gamenId=01-1003\&stgGrpKind=survOnly\&fvrt=yes\&timeType=10",id);
    cp=strchr(s,'/');                       // URL内の区切りを検索
    len=(int)(cp-s);
    if( len<1 || len>32) return 0;
    strncpy(to,s,len);                      // 区切り文字までがホスト名
    to[len]='\0';
    strncpy(s,cp,224);                      // 区切り文字以降はファイル名(なお、s < cp)
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
    client.print("GET ");                   // HTTP GETコマンドを送信
    client.print(s);                        // 相手先ディレクトリを指定
    client.println(" HTTP/1.0");            // HTTPプロトコル
    client.println("User-Agent: esp-wroom-32");
    client.print("Host: ");                 // ホスト名を指定
    client.print(to);                       // 相手先ホスト名
    client.println();                       // ホスト名の指定を終了
//  client.println("Accept: *\/*");
    client.println("Accept: text/html");
    client.println("Connection: close");    // セッションの都度切断を指定
    client.println();
    
    time=millis(); s[0]='\0';
    while(t<TIMEOUT){
        if(client.available()){             // クライアントからのデータを確認
            t=0;
            c=client.read();                // TCPデータの読み取り
            if(headF>=2 && c){
                s[(size%128)+128]=c;
                size++;
                len = 128 - (size % 128);
                if(len >0 && len < 128 ) i=client.read((uint8_t *)(s+(size%128)+128),len);
                else i=0;
                size += i;
                if( size % 128 ) continue;
                if(headF ==2 || headF==3){
                    #ifdef DEBUG_HTTPG
                        Serial.print(size);
                        Serial.print(" bytes, F=");
                        Serial.println(headF);
                    //  Serial.println(s);
                    #endif
                    headF += _httpGetFindTag(s,"<table class=\"tb1\"");
                    _httpGetShiftBuff(s);
                    continue;
                }
                if(headF >= 4 && headF < 4 + 48){
                    #ifdef DEBUG_HTTPG
                        Serial.print(size);
                        Serial.print(" bytes, F=");
                        Serial.println(headF);
                    //  Serial.println(s);
                    #endif
                    headF += _httpGetFindTag(s,"<tr style=\"line-height:13px;\">");
                    if(headF<52){
                        _httpGetShiftBuff(s);
                        continue;
                    }
                    #ifdef DEBUG_HTTPG
                        Serial.print(size);
                        Serial.print(" bytes, F=");
                        Serial.println(headF);
                        Serial.println(s);
                    #endif
                    cp=strstr(s,"<tr style=\"line-height:13px;\">");   // キーワード
                    if( !cp ) break;        // 在り得ない異常
                    cp2=0;
                    if(strlen(cp)>30) cp2=strchr(cp+30,'<');  // 次のHTMLタグを探す
                    if(cp2){
                        cp=cp2;
                        i=256-(int)(cp-s);
                        if(i<0)return 0;    // 在り得ない異常
                        memcpy(s,cp,i);
                    }else{
                        i=256-(int)(cp-s)-30;
                        if(i<0)return 0;    // 在り得ない異常
                        memcpy(s,cp+30,i);
                    }
                    // データ取得部(同じものがもう一つある (グローバルclient,ローカルs,i)
                    while(t<TIMEOUT){
                        if(client.available()){
                            t=0;
                            s[i]=client.read();
                            size++;
                            i++;
                            if(i>255) break;
                        }else{
                            t++;
                            delay(1);
                        }
                    }
                    s[i]=0;
                    Serial.println("Fonud latest item (48th)");
                    #ifdef DEBUG_HTTPG
                        Serial.println(s);
                    #endif
                    cp=strchr(s,':');
                    if( !cp ){
                        Serial.println("no time data in buffer");
                        break;
                    }
                    _httpg_hour=atoi(cp-2);
                    Serial.print("HOUR      : ");
                    Serial.println(_httpg_hour);
                    if( *(cp+1) ){
                        _httpg_minute=atoi(cp+1);
                        if( _httpg_minute <= 5 ) _httpg_minute *= 10;
                        Serial.print("MINITUE   : ");
                        Serial.println(_httpg_minute);
                    }
                    i=256-(int)(cp-s);
                    memcpy(s,cp,i);
                    // データ取得部(同じものがもう一つある (グローバルclient,ローカルs,i)
                    while(t<TIMEOUT){
                        if(client.available()){
                            t=0;
                            s[i]=client.read();
                            size++;
                            i++;
                            if(i>255) break;
                        }else{
                            t++;
                            delay(1);
                        }
                    }
                    s[i]=0;
                    cp=strchr(s,'>');
                    if( !cp ){
                        Serial.println("no depth data, 1");
                        break;
                    }
                    cp=strchr(cp+1,'>');
                    if( !cp ){
                        Serial.println("no depth data, 2");
                        break;
                    }
                    cp=strchr(cp+3,'\n');   // 改行は \r\n
                    if( !cp ){
                        Serial.println("no depth data, 3");
                        break;
                    }
                    i=(int)(cp-s);
                    if(i<1) break;  // 在り得ない異常
                    if(s[i-2]=='>'){
                        Serial.println("format error, river depth");
                        break;
                    }
                    #ifdef DEBUG_HTTPG
                        Serial.print("String    : \"");
                        Serial.print(s);
                        Serial.println('\"');
                        Serial.print("DEPTH(Str): \"");
                        Serial.print(&s[i-5]);
                        Serial.println('\"');
                    #endif
                    _httpg_depth = atof(&s[i-5]);
                    Serial.print("DEPTH     : ");
                    Serial.println(_httpg_depth);
                    ret=_httpg_depth;
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
    client.stop();                          // クライアントの切断
    Serial.print(size);                     // 保存したファイルサイズを表示
    Serial.print(" Bytes, ");
    Serial.print(millis()-time);
    Serial.println("ms, Done");
    #ifdef DEBUG_HTTPG
        Serial.print(_httpg_hour);
        Serial.print(':');
        Serial.print(_httpg_minute);
        Serial.print(":00, depth = ");
        Serial.println(_httpg_depth);
    #endif
    return ret;
}
