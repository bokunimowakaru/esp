/*******************************************************************************
HTMLコンテンツ取得
                                          Copyright (c) 2016-2017 Wataru KUNINO
*******************************************************************************/
/*
# http://rss.weather.yahoo.co.jp/rss/days/7.xml     福島県（福島市）
# http://rss.weather.yahoo.co.jp/rss/days/13.xml    東京都（東京）
# http://rss.weather.yahoo.co.jp/rss/days/23.xml    愛知県（西部・名古屋）
# http://rss.weather.yahoo.co.jp/rss/days/26.xml    京都府（南部・京都市）
# http://rss.weather.yahoo.co.jp/rss/days/27.xml    大阪府（大阪市）
# http://rss.weather.yahoo.co.jp/rss/days/28.xml    兵庫県（神戸市）
# http://rss.weather.yahoo.co.jp/rss/days/43.xml    熊本県（熊本市）

IP_LED="192.168.0.2"                                        # ワイヤレスLEDのIP
while true; do                                              # 永久ループ
WEATHER=`curl -s rss.weather.yahoo.co.jp/rss/days/43.xml\
|cut -d'<' -f17|cut -d'>' -f2|tail -1\
|cut -d' ' -f5|cut -c1-3`                                   # 天気を取得する

 GET /rss/days/29.xml HTTP/1.1
 User-Agent: curl/7.38.0
 Host: rss.weather.yahoo.co.jp
 Accept: 

*/

// #define DEBUG_HTTPG

int _httpg_day=0;                           // 天気予報日
char _httpg_weather[17]="";                 // 天気予報
char _httpg_weather_disp[9]="";             // 天気予報・表示用(Fine/Cloudy/Rainy)
char _httpg_weather_code=0;                 // 天気コード(3:Fine,2:Cloudy,1:Rainy)
int _httpg_temp_H=0;                        // 天気予想温度（最高気温）
int _httpg_temp_L=0;                        // 天気予想温度（最低気温）

int httpGetBufferedWeather(char *out, int out_len, int data_number){
    if(_httpg_day == 0) return 0;
    memset(out,0,out_len+1);
    switch(data_number){
        case 0:
            snprintf(out,out_len+1,"%d,%d,%d,%s,%s",_httpg_day,_httpg_temp_H,_httpg_temp_L,_httpg_weather_disp,_httpg_weather);
            break;
        case 1:
            snprintf(out,out_len+1,"%d",_httpg_day);
            break;
        case 2:
            snprintf(out,out_len+1,"%d",_httpg_temp_H);
            break;
        case 3:
            snprintf(out,out_len+1,"%d",_httpg_temp_L);
            break;
        case 4:
            snprintf(out,out_len+1,"%s",_httpg_weather_disp);
            break;
        case 5:
            snprintf(out,out_len+1,"%s",_httpg_weather);
            break;
        default:
            snprintf(out,out_len+1,"%s %d/%d%cC",_httpg_weather_disp,_httpg_temp_H,_httpg_temp_L,0xDF);
            break;
    }
    #ifdef DEBUG_HTTPG
        Serial.print(strlen(out));
        Serial.print(" Bytes \"");
        Serial.print(out);
        Serial.println("\"");
    #endif
    return strlen(out);
}

int httpGetBufferedWeather(char *out, int out_len){
    return httpGetBufferedWeather(out, out_len, -1);
}

int httpGetBufferedWeatherCode(){
    return _httpg_weather_code;
}

int httpGetBufferedTemp(){
    return (_httpg_temp_L + _httpg_temp_H)/2;
}

int httpGetBufferedTempH(){
    return _httpg_temp_H;
}

int httpGetBufferedTempL(){
    return _httpg_temp_L;
}

int httpGetWeather(int city, char *out, int out_len, int data_number){
    File file;
    WiFiClient client;                      // Wi-Fiクライアントの定義
    int i,len,t=0,size=0,ret=0;             // 変数i,j,t=待ち時間,size=保存容量
    char c,to[33],s[257];                   // 文字変数、to=アクセス先,s=汎用
    char *cp,*cp2;
    int headF=0;                            // ヘッダフラグ(0:HEAD 1:EOL 2:DATA)
    unsigned long time;                     // 時間測定用

    memset(out,0,out_len+1);
    if(city<0 || city>100) return 0;
    snprintf(s,128,"rss.weather.yahoo.co.jp/rss/days/%d.xml",city);
    cp=strchr(s,'/');                       // URL内の区切りを検索
    len=(int)(cp-s);
    if( len<1 || len>32) return 0;
    strncpy(to,s,len);                      // 区切り文字までがホスト名
    to[len]='\0';
    strncpy(s,cp,32);                       // 区切り文字以降はファイル名(なお、s < cp)
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
    client.println("Accept: application/xml");
    client.println("Connection: close");    // セッションの都度切断を指定
    client.println();
    
    time=millis(); s[0]='\0';
    while(t<TIMEOUT){
        if(client.available()){             // クライアントからのデータを確認
            t=0;
            c=client.read();                // TCPデータの読み取り
            if(headF==2 && c){
                s[(size%128)+128]=c;
                size++;
                len = 128 - (size % 128);
                if(len >0 && len < 128 ) i=client.read((uint8_t *)(s+(size%128)+128),len);
                else i=0;
                size += i;
                if( size % 128 ) continue;
                cp=strstr(s,"<item><title>");   // 13文字のキーワード
                if( !cp || (int)(cp-s) >= 128 ){
                    memcpy(s,s+128,128);    // 変数sの後半を前半へ移動
                    memset(s+128,0,129);    // 変数sの後半だけ初期化
                    continue;
                }
                
                /* 解析処理 */
                // <item><title>【_22日（日）_熊本（熊本）_】_曇り_-_20℃/17℃_-_Yahoo!天気・災害
                cp2=strchr(cp,' ');         // 次の区切りスペース文字
                if( !cp2 ){
                    Serial.print("ERROR : parser [day] / ");
                    Serial.println(cp+1);
                    break;
                }
                _httpg_day=atoi(cp2+1);
                Serial.print("      day : ");
                Serial.println(_httpg_day);
                cp=strchr(cp2+1,' ');       // 次の区切りスペース文字
                if( !cp ) break;
                cp2=strchr(cp+1,' ');       // 次の区切りスペース文字
                if( !cp2 ) break;
                cp=strchr(cp2+1,' ');       // 次の区切りスペース文字
                if( !cp ){
                    Serial.print("ERROR : parser [Area] / ");
                    Serial.println(cp2+1);
                    break;
                }
                cp2=strchr(cp+1,' ');       // 次の区切りスペース文字
                len=(int)(cp2-cp-1);
                if(len<0|| len>16){
                    Serial.print("ERROR : parser [Weather] / ");
                    Serial.println(cp+1);
                    break;
                }
                strncpy(_httpg_weather,cp+1,len);
                strcpy(_httpg_weather_disp,"Unknown");
                for(i=0;i<len-2;i++){
                    if( strncmp(_httpg_weather+i,"晴",3) == 0 ){
                        strcpy(_httpg_weather_disp,"Fine");
                        _httpg_weather_code=3;
                        break;
                    }
                    if( strncmp(_httpg_weather+i,"曇",3) == 0 ){
                        strcpy(_httpg_weather_disp,"Cloudy");
                        _httpg_weather_code=2;
                        break;
                    }
                    if( strncmp(_httpg_weather+i,"雨",3) == 0 ){
                        strcpy(_httpg_weather_disp,"Rainy");
                        _httpg_weather_code=1;
                        break;
                    }
                }
                Serial.print("  weather : ");
                Serial.print(_httpg_weather_disp);
                Serial.print(" / ");
                Serial.println(_httpg_weather);
                
                cp=strchr(cp2+1,' ');       // 次の区切りスペース文字
                if( !cp ){
                    Serial.print("ERROR : parser [temp_H] / ");
                    Serial.println(cp2+1);
                    break;
                }
                _httpg_temp_H=atoi(cp+1);
                Serial.print("   temp_H : ");
                Serial.println(_httpg_temp_H);
                cp2=strchr(cp+1,'/');       // 次の区切りスペース文字
                if( !cp ){
                    Serial.print("ERROR : parser [temp_L] / ");
                    Serial.println(cp+1);
                    break;
                }
                _httpg_temp_L=atoi(cp2+1);
                Serial.print("   temp_L : ");
                Serial.println(_httpg_temp_L);
                ret=httpGetBufferedWeather(out, out_len, data_number);
                break;
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
        Serial.print(ret);
        Serial.print(" Bytes \"");
        Serial.print(out);
        Serial.println("\"");
    #endif
    return ret;
}

int httpGetWeather(int city, char *out, int out_len){
    return httpGetWeather(city, out, out_len, -1);
}

