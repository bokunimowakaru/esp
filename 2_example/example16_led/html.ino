/*******************************************************************************
HTMLコンテンツ LEDの輝度制御

                                          Copyright (c) 2016-2019 Wataru KUNINO
*******************************************************************************/

void html(WiFiClient &client, int target, uint32_t ip){
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
    client.println("<head><title>Wi-Fi コンシェルジェ 照明担当</title>");
    client.println("<meta http-equiv=\"Content-type\" content=\"text/html; charset=UTF-8\">");
    client.println("</head>");
    client.println("<body>");
    client.println("<h3>LED STATUS</h3>");
    if(target==0) sprintf(s,"<p>0 (LED OFF)</p>");
    if(target==1) sprintf(s,"<p>1 (LED ON)</p>");
    if(target>1)  sprintf(s,"<p>%d (キャンドル)</p>",target);
    if(target<0)  sprintf(s,"<p>%d (輝度=%d%%)</p>",target,-target*10);
    client.println(s);
    client.println("<hr>");
    client.println("<h3>HTTP GET</h3>");
    client.print("<p>http://");
    client.print(s_ip);
    client.println("/?L=n<br>(n: 0=OFF, 1=ON, 2～10=キャンドル, -1～-10=輝度)</p>");
    sprintf(s,"<form method=\"GET\" action=\"http://%s/\">",s_ip);
    client.println(s);
    client.println("<input type=\"submit\" name=\"L\" value=\"0 (LED OFF)\">");
    client.println("<input type=\"submit\" name=\"L\" value=\"1 (LED ON)\">");
    client.println("<input type=\"submit\" name=\"L\" value=\"10 (キャンドル)\">");
    client.println("<input type=\"submit\" name=\"L\" value=\"-2 (輝度20%)\">");
    client.println("</form>");
    client.println(s);
    client.println("<input type=\"text\" name=\"L\" value=\"\">");
    client.println("<input type=\"submit\" value=\"送信\">");
    client.println("</form>");
    client.println("<hr>");
    client.println("<h3>HTTP POST</h3>");
    sprintf(s,"<form method=\"POST\" action=\"http://%s/\">",s_ip);
    client.println(s);
    client.println("<input type=\"submit\" name=\"L\" value=\"0 (LED OFF)\">");
    client.println("<input type=\"submit\" name=\"L\" value=\"1 (LED ON)\">");
    client.println("<input type=\"submit\" name=\"L\" value=\"10 (キャンドル)\">");
    client.println("<input type=\"submit\" name=\"L\" value=\"-2 (輝度20%)\">");
    client.println("</form>");
    client.println(s);
    client.println("<input type=\"text\" name=\"L\" value=\"\">");
    client.println("<input type=\"submit\" value=\"送信\">");
    client.println("</form>");
    client.println("</body>");
    client.println("</html>");
    sprintf(s,"LED=%d",target);         // 変数sに「LED=」とtarget値を代入
    Serial.println(s);                  // シリアルへコンテンツを出力
}
