/*******************************************************************************
HTMLコンテンツ センサ受信データ・ファイルシステム
                                          Copyright (c) 2016-2019 Wataru KUNINO
*******************************************************************************/

void html(WiFiClient &client, uint32_t ip, char *date){
    char s[65],s_ip[16];
    Dir dir = SPIFFS.openDir("/");
    
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
    client.println("<head><title>Wi-Fi コンシェルジェ 情報担当</title>");
    client.println("<meta http-equiv=\"Content-type\" content=\"text/html; charset=UTF-8\">");
    client.print("<meta http-equiv=\"refresh\" content=\"20;URL=http://");
    client.print(s_ip);
    client.println("/\">");
    client.println("</head>");
    client.println("<body>");
    client.println("<h3>UDP データ・ファイルシステム</h3>");
    client.println("<center><table border>");
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
    client.println("<hr>");
    client.println("<h3>FTP Server</h3>");
    client.print("<p>ftp://");
    client.print(FTP_USER);
    client.print(":");
    client.print(FTP_PASS);
    client.print("@");
    client.print(s_ip);    
    client.println("/</p>");
    client.println("<center><table border>");
    client.println("<tr><td><b>項目</b></td><td><b>値</b></td>");
    client.print("<tr><td>FTP サーバ</td><td>");
    client.print(s_ip);
    client.println(":21</td>");
    client.print("<tr><td>ユーザ名</td><td>");
    client.print(FTP_USER);
    client.println("</td>");
    client.print("<tr><td>パスワード</td><td>");
    client.print(FTP_PASS);
    client.println("</td>");
    client.println("<tr><td>FTP モード</td><td>PASV</td>");
    client.println("<tr><td>暗号化</td><td>なし</td>");
    client.println("</table></center>");
    client.println("<hr>");
    client.println("<h3>NTP Client</h3>");
    client.print("<center><h4>");
    client.print(date);
    client.println("</h4></center>");
    client.println("</body>");
    client.println("</html>");
}
