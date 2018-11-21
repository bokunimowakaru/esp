/*******************************************************************************
HTMLコンテンツ Raspberry Pi

                                          Copyright (c) 2016-2019 Wataru KUNINO
*******************************************************************************/

void html(WiFiClient &client, char *tx, char *rx, uint32_t ip){
    char s[65],s_ip[16];
    
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
    client.println("<h3>Raspberry Pi STATUS</h3>");
    client.print("<p>");
    client.print(rx);    
    client.println("</p>");
    client.println("<hr>");
    client.println("<h3>HTTP GET</h3>");
    client.print("<p>http://");
    client.print(s_ip);
    client.println("/?COM=文字列</p>");
    sprintf(s,"<form method=\"GET\" action=\"http://%s/\">",s_ip);
    client.println(s);
    client.println("制御：<input type=\"submit\" name=\"POW\" value=\"1 (電源投入)\">");
    client.println("<input type=\"submit\" name=\"START\" value=\"2 (ログイン)\">");
    client.println("<input type=\"submit\" name=\"START\" value=\"0 (プロンプト)\">");
    client.println("<input type=\"submit\" name=\"GET\" value=\"(データ受信)\">");
    client.println("<br><br>");
    client.println("Bash：<input type=\"submit\" name=\"COM\" value=\"echo hello\">");
    client.println("<input type=\"submit\" name=\"COM\" value=\"wall hello\">");
    client.println("<input type=\"submit\" name=\"COM\" value=\"sudo shutdown -h now\">");
    client.println("<input type=\"submit\" name=\"COM\" value=\"logout\">");
    client.println("</form>");
    client.println(s);
    client.println("　　　<input type=\"text\" name=\"COM\" >");
    client.println("<input type=\"submit\" value=\"送信\">");
    client.println("</form>");
    client.println("</form>");
    client.println("</body>");
    client.println("</html>");
}
