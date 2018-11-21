/*******************************************************************************
HTMLコンテンツ 液晶

                                          Copyright (c) 2016-2019 Wataru KUNINO
*******************************************************************************/

// extern unsigned long HTML_DIR_BYTES;

void listDir(WiFiClient &client, char *s_ip, fs::FS &fs, const char * dirname, uint8_t levels) {
    unsigned long bytes;
    Serial.printf("Listing directory: %s\n", dirname);
    File root = fs.open(dirname);
    if (!root) {
        Serial.println("Failed to open directory");
    }else if (!root.isDirectory()) {
        Serial.println("Not a directory");
    }else{
        File file = root.openNextFile();
        while (file) {
            if (file.isDirectory()) {
                Serial.print("  DIR : ");
                Serial.println(file.name());
                if (levels) {
                    listDir(client,s_ip,fs, file.name(), levels - 1);
                }
            } else {
                Serial.print("  FILE: ");
                Serial.print(file.name());
                Serial.print("  SIZE: ");
                Serial.println(file.size());
                /* HTML 出力 */
                client.print("<tr><td><a href=\"http://");
                client.print(s_ip);
                client.print(file.name());
                client.print("\">");
                client.print(file.name()+1);
                client.println("</a></td>");
                client.print("<td align=\"right\">");
                bytes=(unsigned long)(file.size());
                // HTML_DIR_BYTES += bytes;
                if(bytes >= 1024){
                    client.print(bytes/1024);
                    client.print(" K");
                }else{
                    client.print(bytes);
                    client.print(' ');
                }
                client.println("Bytes</td></tr>");
            }
            file = root.openNextFile();
        }
    }
} // https://github.com/copercini/arduino-esp32-SPIFFS/blob/master/examples/SPIFFS_Test/SPIFFS_Test.ino


void html(WiFiClient &client, char *date, char *lcd, uint32_t ip){
    char s[65],s_ip[16];
//  Dir dir = SPIFFS.openDir("/");
//  HTML_DIR_BYTES=0;
    
    sprintf(s_ip,"%d.%d.%d.%d",
        ip & 255,
        ip>>8 & 255,
        ip>>16 & 255,
        ip>>24
    );
    client.println("HTTP/1.1 200 OK");              // HTTP OKを応答
    client.println("Content-Type: text/html");      // HTMLコンテンツ
    client.println("Connection: close");            // 応答終了後にセッションを閉じる
    client.println();
    client.println("<html>");
    client.println("<head><title>Test Page</title>");
    client.println("<meta http-equiv=\"Content-type\" content=\"text/html; charset=UTF-8\">");
    client.print("<meta http-equiv=\"refresh\" content=\"30;URL=http://");
    client.print(s_ip);
    client.println("/\">");
    client.println("</head>");
    client.println("<body>");
    client.print("<h3>River Water Level ");
    client.print(date);    
    client.print(" </h3><p>");
    client.print(lcd);    
    client.println("</p>");
    sprintf(s,"<form method=\"GET\" action=\"http://%s/\">",s_ip);
    client.println(s);
    client.println("<input type=\"submit\" value=\"更新\">");
    client.println("<input type=\"submit\" name=\"DEPTH\" value=\"インターネット取得\">");
    client.println("<input type=\"submit\" name=\"FORMAT\" value=\"初期化\">");
    client.println("</form>");
    client.println("<hr>");
    
    client.println("<h3>ファイルシステム</h3>");
    client.println("<center><table border>");
    
    #ifdef SD_CARD_EN
        listDir(client,s_ip,SD, "/", 0);
    #else
        listDir(client,s_ip,SPIFFS, "/", 0);
    #endif
    client.print("<tr><td align=\"right\"> Used</td>");
    client.print("<td align=\"right\">");
//  client.print(HTML_DIR_BYTES/1024); client.println(" KBytes</td></tr>");
    #ifdef SD_CARD_EN
    client.print( (int)(SD.usedBytes()/1024/1024) ); client.println(" MBytes</td></tr>");
    #else
    client.print( SPIFFS.usedBytes()/1024 ); client.println(" KBytes</td></tr>");
    #endif
    client.print("<tr><td align=\"right\"> Remain</td>");
    client.print("<td align=\"right\">");
    #ifdef SD_CARD_EN
    client.print( (int)((SD.totalBytes()-SD.usedBytes())/1024/1024) ); client.println(" MBytes</td></tr>");
    #else
    client.print( (SPIFFS.totalBytes()-SPIFFS.usedBytes())/1024 ); client.println(" KBytes</td></tr>");
    #endif
    client.println("</table></center><br>");

    client.println("</body>");
    client.println("</html>");
}
