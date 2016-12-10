/*******************************************************************************
HTMLコンテンツ LEDの輝度制御

                                            Copyright (c) 2016 Wataru KUNINO
*******************************************************************************/

void html(WiFiClient &client, int target, int R, int G, int B, uint32_t ip){
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
    client.println("<h3>RGB LED STATUS</h3>");
    if(target==0) sprintf(s,"<p>0 (LED OFF R=%d,G=%d,B=%d)</p>",R/100,G/100,B/100);
    if(target==1) sprintf(s,"<p>1 (LED ON R=%d,G=%d,B=%d)</p>",R/100,G/100,B/100);
    if(target>1)  sprintf(s,"<p>%d (キャンドル)</p>",target);
    if(target<0)  sprintf(s,"<p>%d (輝度=%d%%)</p>",target,-target*10);
    client.println(s);
    client.println("<hr>");
    client.println("<h3>HTTP GET</h3>");
    client.print("<p>http://");
    client.print(s_ip);
    client.println("/?L=n<br>(白色 n: 0=OFF, 1=ON, 2～10=キャンドル, -1～-10=輝度)</p>");
    sprintf(s,"<form method=\"GET\" action=\"http://%s/\">",s_ip);
    client.println(s);
    client.println("<input type=\"submit\" name=\"L\" value=\"0 (LED OFF)\">");
    client.println("<input type=\"submit\" name=\"L\" value=\"1 (LED ON)\">");
    client.println("<input type=\"submit\" name=\"L\" value=\"10 (キャンドル)\">");
    client.println("<input type=\"submit\" name=\"L\" value=\"-2 (輝度20%)\">");
    client.println("</form>");
    client.print("<p>http://");
    client.print(s_ip);
    client.println("/?RGB=nnn<br>(<font color=\"red\">R</font><font color=\"green\">G</font><font color=\"blue\">B</font>色)</p>");
    sprintf(s,"<form method=\"GET\" action=\"http://%s/\">",s_ip);
    client.println(s);
    client.println("<input type=\"submit\" name=\"RGB\" value=\"933 (赤)\" style=\"color:red\">");
    client.println("<input type=\"submit\" name=\"RGB\" value=\"951 (橙)\" style=\"color:orange\">");
    client.println("<input type=\"submit\" name=\"RGB\" value=\"771 (黄)\" style=\"color:yellow\">");
    client.println("<input type=\"submit\" name=\"RGB\" value=\"393 (緑)\" style=\"color:green\">");
    client.println("<input type=\"submit\" name=\"RGB\" value=\"339 (青)\" style=\"color:blue\">");
    client.println("<input type=\"submit\" name=\"RGB\" value=\"717 (紫)\" style=\"color:purple\">");
    client.println("</form>");
    client.print("<p>http://");
    client.print(s_ip);
    client.println("/?R=n<br>(<font color=\"red\">赤色</font>)</p>");
    sprintf(s,"<form method=\"GET\" action=\"http://%s/\">",s_ip);
    client.println(s);
    client.println("<input type=\"submit\" name=\"R\" value=\"0 (LED OFF)\" style=\"color:red\">");
    client.println("<input type=\"submit\" name=\"R\" value=\"-2 (輝度20%)\" style=\"color:red\">");
    client.println("<input type=\"submit\" name=\"R\" value=\"-4 (輝度40%)\" style=\"color:red\">");
    client.println("<input type=\"submit\" name=\"R\" value=\"-6 (輝度60%)\" style=\"color:red\">");
    client.println("<input type=\"submit\" name=\"R\" value=\"-8 (輝度80%)\" style=\"color:red\">");
    client.println("<input type=\"submit\" name=\"R\" value=\"1 (LED ON)\" style=\"color:red\">");
    client.println("</form>");
    client.print("<p>http://");
    client.print(s_ip);
    client.println("/?G=n<br>(<font color=\"green\">緑色</font>)</p>");
    sprintf(s,"<form method=\"GET\" action=\"http://%s/\">",s_ip);
    client.println(s);
    client.println("<input type=\"submit\" name=\"G\" value=\"0 (LED OFF)\" style=\"color:green\">");
    client.println("<input type=\"submit\" name=\"G\" value=\"-2 (輝度20%)\" style=\"color:green\">");
    client.println("<input type=\"submit\" name=\"G\" value=\"-4 (輝度40%)\" style=\"color:green\">");
    client.println("<input type=\"submit\" name=\"G\" value=\"-6 (輝度60%)\" style=\"color:green\">");
    client.println("<input type=\"submit\" name=\"G\" value=\"-8 (輝度80%)\" style=\"color:green\">");
    client.println("<input type=\"submit\" name=\"G\" value=\"1 (LED ON)\" style=\"color:green\">");
    client.println("</form>");
    client.print("<p>http://");
    client.print(s_ip);
    client.println("/?B=n<br>(<font color=\"blue\">青色</font>)</p>");
    sprintf(s,"<form method=\"GET\" action=\"http://%s/\">",s_ip);
    client.println(s);
    client.println("<input type=\"submit\" name=\"B\" value=\"0 (LED OFF)\" style=\"color:blue\">");
    client.println("<input type=\"submit\" name=\"B\" value=\"-2 (輝度20%)\" style=\"color:blue\">");
    client.println("<input type=\"submit\" name=\"B\" value=\"-4 (輝度40%)\" style=\"color:blue\">");
    client.println("<input type=\"submit\" name=\"B\" value=\"-6 (輝度60%)\" style=\"color:blue\">");
    client.println("<input type=\"submit\" name=\"B\" value=\"-8 (輝度80%)\" style=\"color:blue\">");
    client.println("<input type=\"submit\" name=\"B\" value=\"1 (LED ON)\" style=\"color:blue\">");
    client.println("</form>");
    client.println("</body>");
    client.println("</html>");
    sprintf(s,"LED=%d",target);         // 変数sに「LED=」とtarget値を代入
    Serial.println(s);                  // シリアルへコンテンツを出力
}
