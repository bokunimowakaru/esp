/*******************************************************************************
防犯カメラ
                                            Copyright (c) 2016 Wataru KUNINO
*******************************************************************************/

void html(WiFiClient &client, int size, int update, uint32_t ip){
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
    if(update){
        client.print("<meta http-equiv=\"refresh\" content=\"");
        client.print(update);
        client.print("\">");
    }
    client.println("</head>");
    client.println("<body>");
    client.println("<h3>防犯カメラ STATUS</h3>");
    client.println("<img src=\"cam.jpg\">");
    client.print("<p>画像サイズ = 約 ");
    client.print(size/1000);
    client.print(" KBytes, 更新間隔 = ");
    client.print(update);
    client.println(" 秒</p>");
    client.println("<hr>");
    client.println("<h3>HTTP GET</h3>");
    client.print("<p>http://");
    client.print(s_ip);
    client.println("/cam.jpg</p>");
    sprintf(s,"<form method=\"GET\" action=\"http://%s/\">",s_ip);
    client.println(s);
    client.println("<input type=\"submit\" value=\"画像データの取得\">");
    client.println("<input type=\"submit\" name=\"INT\" value=\"0 自動更新の停止\">");
    client.println("<input type=\"submit\" name=\"INT\" value=\"60 自動更新(60秒)\">");
    client.println("</form>");
    client.println("</body>");
    client.println("</html>");
}
