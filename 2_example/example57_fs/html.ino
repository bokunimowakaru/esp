/*******************************************************************************
HTMLコンテンツ センサ受信データ・ファイルシステム
                                          Copyright (c) 2016-2019 Wataru KUNINO
*******************************************************************************/

void listDir(WiFiClient &client, char *s_ip, fs::FS &fs, const char * dirname, uint8_t levels) {
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
                client.print(file.name());
                client.println("</a></td>");
                client.print("<td align=\"right\">");
                client.print(file.size());
                client.println(" Bytes</td></tr>");
            }
            file = root.openNextFile();
        }
    }
} // https://github.com/copercini/arduino-esp32-SPIFFS/blob/master/examples/SPIFFS_Test/SPIFFS_Test.ino

void html(WiFiClient &client, uint32_t ip){
    char s[65],s_ip[16];
    
    // Dir dir = SPIFFS.openDir("/");
    
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
    client.println("</head>");
    client.println("<body>");
    client.println("<h3>UDP データ・ファイルシステム</h3>");
    client.println("<center><table border>");
    /*
    while (dir.next()) {
        client.print("<tr><td><a href=\"http://");
        client.print(s_ip);
        client.print(dir.fileName());
        client.print("\">");
        client.print(dir.fileName());
        client.println("</a></td>");
        client.print("<td align=\"right\">");
        File f = dir.openFile("r");
        client.print(f.size());
        client.println(" Bytes</td></tr>");
    } // https://github.com/esp8266/Arduino/blob/master/doc/filesystem.md
    */
    listDir(client,s_ip,SPIFFS, "/", 0);
    
    client.println("</table></center>");
    client.println("<hr>");
    client.println("<h3>HTTP GET</h3>");
    client.print("<p>http://");
    client.print(s_ip);
    client.println("/</p>");
    sprintf(s,"<form method=\"GET\" action=\"http://%s/\">",s_ip);
    client.println(s);
    client.println("<input type=\"submit\" value=\"更新\">");
    client.println("<input type=\"submit\" name=\"FORMAT\" value=\"初期化\">");
    client.println("</form>");
    client.println("</body>");
    client.println("</html>");
}


