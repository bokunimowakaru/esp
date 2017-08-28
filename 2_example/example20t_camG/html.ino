/*******************************************************************************
防犯カメラ
                                           Copyright (c) 2016-2017 Wataru KUNINO
*******************************************************************************/
#define PICT_NUM 5 // 表示枚数(筆者の環境では最大5枚まででした)
extern int photo_n;

void html(WiFiClient &client, int size, int update, uint32_t ip){
    char s[65],s_ip[16];
    int i;
    
    sprintf(s_ip,"%i.%i.%i.%i",
        ip & 255,
        ip>>8 & 255,
        ip>>16 & 255,
        ip>>24
    );
    client.println("HTTP/1.0 200 OK");              // HTTP OKを応答
    client.println("Content-Type: text/html");      // HTMLコンテンツ
    client.println("Connection: close");            // 応答終了後にセッションを閉じる
    client.println();
    client.println("<html>");
    client.println("<head><title>Test Page</title>");
    client.println("<meta http-equiv=\"Content-type\" content=\"text/html; charset=UTF-8\">");
    if(update){
        client.print("<meta http-equiv=\"refresh\" content=\"");
        client.print(update);
        client.print(";URL=http://");
        client.print(s_ip);
        client.println("/\">");
    }
    client.println("</head>");
    if(client.available())return;
    if(!client.connected())return;
    client.println("<body>");
    client.println("<h3>防犯カメラ STATUS</h3>");
    for(i=(PICT_NUM-1);i>=0;i--){
        if(photo_n-i-1>=0){
            sprintf(s,"<img width=160 height=120 src=\"cam%03d.jpg\"> ",photo_n-i-1);
        }else sprintf(s,"<img width=160 height=120 src=\"cam000.jpg\"> ");
        client.println(s);
        if( i%3==(PICT_NUM%3) ) client.println("<br><br>");
    }
    delay(10);
    if(client.available())return;
    if(!client.connected())return;
    if(PICT_NUM%3) client.println("<br><br>");
    client.print("画像サイズ = 約 ");
    client.print(size/1000);
    client.print(" KBytes, 保存数 = ");
    client.print(photo_n);
    client.print(" 枚, 更新間隔 = ");
    client.print(update);
    client.println(" 秒<br><br>");
    client.println("<hr>");
    client.println("<h3>HTTP GET</h3>");
    client.print("<p>http://");
    client.print(s_ip);
    if(photo_n>0) sprintf(s,"/cam%03d.jpg",photo_n-1);
    else sprintf(s,"/cam000.jpg");
    client.print(s);
    client.println("</p>");
    delay(10);
    if(client.available())return;
    if(!client.connected())return;
    sprintf(s,"<form method=\"GET\" action=\"http://%s/\">",s_ip);
    client.println(s);
    client.println("<input type=\"submit\" value=\"画面の更新\">");
    client.println("　<input type=\"submit\" name=\"SHOT\" value=\"写真撮影\"><br><br>");
    client.println("自動更新:<input type=\"submit\" name=\"INT\" value=\"0 停止\">");
    client.println("<input type=\"submit\" name=\"INT\" value=\"60 秒(1分)\">");
    client.println("<input type=\"submit\" name=\"INT\" value=\"120 秒(3分)\">");
    client.println("<input type=\"submit\" name=\"INT\" value=\"300 秒(5分)\">");
    client.println("<input type=\"submit\" name=\"INT\" value=\"600 秒(10分)\"><br><br>");
    client.println("SPIFFS:<input type=\"submit\" name=\"FORMAT\" value=\"初期化\"><br><br>");
    client.println("</form>");
    client.println("</body>");
    client.println("</html>");
}

void htmlMesg(WiFiClient &client, char *txt, uint32_t ip){
    char s_ip[16];
    
    sprintf(s_ip,"%i.%i.%i.%i",
        ip & 255,
        ip>>8 & 255,
        ip>>16 & 255,
        ip>>24
    );
    client.println("HTTP/1.0 200 OK");              // HTTP OKを応答
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

