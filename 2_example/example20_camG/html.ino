/*******************************************************************************
防犯カメラ
                                          Copyright (c) 2016-2019 Wataru KUNINO
*******************************************************************************/

void html(WiFiClient &client, int size, int update, uint32_t ip){
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
    client.println("<head><title>Wi-Fi コンシェルジェ カメラ担当</title>");
    client.println("<meta http-equiv=\"Content-type\" content=\"text/html; charset=UTF-8\">");
    if(update){
        client.print("<meta http-equiv=\"refresh\" content=\"");
        client.print(update);
        client.print(";URL=http://");
        client.print(s_ip);
        client.println("/\">");
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
    client.println("<input type=\"submit\" value=\"画像データの取得\"><br><br>");
    client.println("自動更新:<input type=\"submit\" name=\"INT\" value=\"0 停止\">");
    client.println("<input type=\"submit\" name=\"INT\" value=\"60 秒\">");
    client.println("　<input type=\"submit\" name=\"RESET\" value=\"リセット\"><br><br>");
//  client.println("速度:<input type=\"submit\" name=\"BPS\" value=\"38400 bps\">");
//  client.println("<input type=\"submit\" name=\"BPS\" value=\"115200 bps\"><br><br>");
    client.println("画像:<input type=\"submit\" name=\"SIZE\" value=\"0 640x480\">");
    client.println("<input type=\"submit\" name=\"SIZE\" value=\"1 320x240\">");
    client.println("<input type=\"submit\" name=\"SIZE\" value=\"2 160x120\"><br><br>");
//  client.println("画質:<input type=\"submit\" name=\"RATIO\" value=\"255 高圧縮\">");
//  client.println("<input type=\"submit\" name=\"RATIO\" value=\"155 中圧縮\">");
//  client.println("<input type=\"submit\" name=\"RATIO\" value=\"56 標準\">");
//  client.println("<input type=\"submit\" name=\"RATIO\" value=\"0 最高\"<br><br>");
/*
    client.println("<input type=\"submit\" name=\"POWER\" value=\"0 POWER OFF\">");
    client.println("<input type=\"submit\" name=\"POWER\" value=\"1 POWER ON\">");
*/
    client.println("</form>");
    client.println("</body>");
    client.println("</html>");
}

void htmlMesg(WiFiClient &client, char *txt, uint32_t ip){
    char s_ip[16];
    
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
    client.print("<meta http-equiv=\"refresh\" content=\"3;URL=http://");
    client.print(s_ip);
    client.println("/\">");
    client.print("<p>");
    client.print(txt);
    client.println("</p>");
    client.println("</body>");
    client.println("</html>");
}

