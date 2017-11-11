/*******************************************************************************
防犯カメラ
                                           Copyright (c) 2016-2017 Wataru KUNINO
*******************************************************************************/

#define PICT_NUM 6     // 表示枚数 3～6枚程度(6枚以上でエラーが発生しやすくなる)
extern int photo_size;
extern int update;
extern int photo_n;

void getHtml(char *html,int size) {
    char s_ip[16];
    int i;
    uint32_t ip = WiFi.localIP();
    
    sprintf(s_ip,"%d.%d.%d.%d",
        ip & 255,
        ip>>8 & 255,
        ip>>16 & 255,
        ip>>24
    );
    snprintf(html, size,"\
    <html>\n<head>\n<title>Test Page</title>\n\
    <meta http-equiv=\"Content-type\" content=\"text/html; charset=UTF-8\">\n\
    <meta http-equiv=\"refresh\" content=\"%d\";URL=http://%s/\">\n</head>\n\
    <body>\n<h3>防犯カメラ STATUS</h3>\
    ",update,s_ip);
    for(i=(PICT_NUM-1);i>=0;i--){
        if(photo_n-i-1>=0){
            snprintf(html, size,"%s<img width=160 height=120 src=\"cam%03d.jpg\"> ",html,photo_n-i-1);
        }else snprintf(html, size,"%s<img width=160 height=120 src=\"cam000.jpg\"> ",html);
        if( i%3==(PICT_NUM%3) ) snprintf(html, size,"%s<br><br>",html);
    }
    snprintf(html, size,"%s\
    画像サイズ = %d Bytes, 保存数 = %d 枚, 更新間隔 = %d 秒<br><br><hr>\
    <h3>HTTP GET</h3><p>http://%s/cam%03d.jpg</p>\
    <form method=\"GET\" action=\"http://%s/\"><input type=\"submit\" value=\"画面の更新\">\
    　<input type=\"submit\" name=\"SHOT\" value=\"写真撮影\"><br><br>\
    自動更新:<input type=\"submit\" name=\"INT\" value=\"20 秒\">\
    <input type=\"submit\" name=\"INT\" value=\"60 秒(1分)\">\
    <input type=\"submit\" name=\"INT\" value=\"120 秒(3分)\">\
    <input type=\"submit\" name=\"INT\" value=\"300 秒(5分)\">\
    <input type=\"submit\" name=\"INT\" value=\"600 秒(10分)\"><br><br>\
    SPIFFS:<input type=\"submit\" name=\"FORMAT\" value=\"初期化\"><br><br>\
    </form>\n</body>\n</html>\n\
    ",html,photo_size,photo_n,update,s_ip,photo_n-1,s_ip);
}

void sendError404(){
    String message = "File Not Found\n\n";
    message += "URI: ";
    message += server.uri();
    message += "\nMethod: ";
    message += ( server.method() == HTTP_GET ) ? "GET" : "POST";
    message += "\nArguments: ";
    message += server.args();
    message += "\n";
    for ( uint8_t i = 0; i < server.args(); i++ ) {
        message += " " + server.argName ( i ) + ": " + server.arg ( i ) + "\n";
    }
    server.send ( 404, "text/plain", message );
    Serial.println("Done handleNotFound");
}

/*
void getImgFile(int index) {
    char filename[]="/cam***.jpg";          // 画像ファイル名(ダウンロード用)
    int i=photo_n-index-1;
    if(i<0)i=0;
    sprintf(filename,"/cam%03d.jpg",i);
    Serial.print("SEND ");
    Serial.print(filename);                 // ファイル名を液晶へ出力表示
    file = SPIFFS.open(filename,"r");       // 読み込みのためにファイルを開く
    if(file){                               // ファイルを開けることが出来た時、
        server.streamFile(file,"image/jpeg");   // ファイル送信
        file.close();                       // ファイルを閉じる
    }else{
        handleNotFound();
    }
    Serial.println(" Done");
}
*/
