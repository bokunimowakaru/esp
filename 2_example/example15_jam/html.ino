/*******************************************************************************
HTMLコンテンツ IchigoJam

                                            Copyright (c) 2016 Wataru KUNINO
*******************************************************************************/

void html(WiFiClient &client, char *tx, char *rx, uint32_t ip){
    char s[65],s_ip[16];
    
    sprintf(s_ip,"%i.%i.%i.%i",
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
    client.println("<h3>IchigoJam STATUS</h3>");
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
    client.println("<input type=\"submit\" name=\"COM\" value=\"LED 1\">");
    client.println("<input type=\"submit\" name=\"COM\" value=\"LED 0\">");
    client.println("<input type=\"submit\" name=\"COM\" value=\"BEEP\">");
    client.println("<input type=\"submit\" name=\"COM\" value=\"?ANA()\">");
    client.println("<input type=\"submit\" name=\"COM\" value=\"?VER()\">");
    client.println("</form>");
    client.println(s);
    client.println("<input type=\"text\" name=\"COM\" >");
    client.println("<input type=\"submit\" value=\"送信\">");
    client.println("</form>");
    client.println("<hr>");
    client.println("<h3>HTTP POST</h3>");
    sprintf(s,"<form method=\"POST\" action=\"http://%s/\">",s_ip);
    client.println(s);
    client.println("<textarea rows=6 cols=32 type=\"text\" name=\"PRG\">");
    if(tx[0]=='\0'){
	    client.println("10 LED 1");
	    client.println("20 WAIT 60");
	    client.println("30 LED 0");
	    client.println("40 BEEP");
	    client.println("RUN");
	}else client.print(tx);
    client.println("</textarea>");
    client.println("<input type=\"submit\" value=\"送信\">");
    client.println("</form>");
    client.println("</body>");
    client.println("</html>");
}
