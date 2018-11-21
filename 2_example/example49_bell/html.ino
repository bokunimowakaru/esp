/*******************************************************************************
HTMLコンテンツ チャイム

                                          Copyright (c) 2016-2019 Wataru KUNINO
*******************************************************************************/

void html(WiFiClient &client, int chime, uint32_t ip){
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
    client.println("<h3>chimeBells STATUS</h3>");
    if(chime==0) sprintf(s,"<p>0 (チャイム OFF)</p>");
    else if(chime>0)  sprintf(s,"<p>%d (チャイム %d回)</p>",chime,chime);
    else sprintf(s,"<p>%d (チャイム 単音)</p>",chime);
    client.println(s);
    client.println("<hr>");
    client.println("<h3>HTTP GET</h3>");
    client.print("<p>http://");
    client.print(s_ip);
    client.println("/?B=n<br>(n: 0=OFF, 2=ピンポン)</p>");
    sprintf(s,"<form method=\"GET\" action=\"http://%s/\">",s_ip);
    client.println(s);
    client.println("<input type=\"submit\" name=\"B\" value=\"0 (鳴音停止)\">");
    client.println("<input type=\"submit\" name=\"B\" value=\"2 (ピンポン)\">");
    client.println("<input type=\"submit\" name=\"B\" value=\"10 (5回連続)\">");
    client.println("</form>");
    client.println(s);
    client.println("<input type=\"text\" name=\"B\" value=\"\">");
    client.println("<input type=\"submit\" value=\"送信\">");
    client.println("</form>");
    client.println("<hr>");
    client.println("<h3>HTTP POST</h3>");
    sprintf(s,"<form method=\"POST\" action=\"http://%s/\">",s_ip);
    client.println(s);
    client.println("<input type=\"submit\" name=\"B\" value=\"0 (鳴音停止)\">");
    client.println("<input type=\"submit\" name=\"B\" value=\"2 (ピンポン)\">");
    client.println("<input type=\"submit\" name=\"B\" value=\"10 (5回連続)\">");
    client.println("</form>");
    client.println(s);
    client.println("<input type=\"text\" name=\"B\" value=\"\">");
    client.println("<input type=\"submit\" value=\"送信\">");
    client.println("</form>");
    client.println("</body>");
    client.println("</html>");
    sprintf(s,"BELL=%d",chime);         // 変数sに「BELL=」とchime値を代入
    Serial.println(s);                  // シリアルへコンテンツを出力
}
