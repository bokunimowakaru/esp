/*******************************************************************************
HTMLコンテンツ DEMO

                                          Copyright (c) 2016-2017 Wataru KUNINO
*******************************************************************************/

extern int hall_prev;
extern int touch_prev[10];
extern boolean touchB[10];
extern uint8_t touch_pin[10];

void html(WiFiClient &client, int led, int hall, uint32_t ip){
    char s[65],s_ip[16];
    int i;
    
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
    client.println("<meta name=\"viewport\" content=\"width=240\">");
    client.print("<meta http-equiv=\"refresh\" content=\"10\">");
    client.println("</head>");
    
    client.println("<body>");
    client.println("<h3>DEMO STATUS</h3>");
    client.print("<p>");
    client.print("LED = ");
    client.print(led);
    client.println("</p>");
    client.print("<p>");
    client.print("hallRead = ");
    client.print(hall);
    client.print(" ( ");
    client.print(hall_prev);
    client.println(" )</p>");

    client.println("<p>touchRead : ");
    client.println("<center><table border=1><tr><td align=\"center\">Touch<br>Channel</td><td align=\"center\">GPIO</td><td align=\"center\">Status</td></tr>");
    client.println("<tr><td align=\"center\">T0</td><td align=\"center\">GPIO4</td><td align=\"center\">---</td></tr>");
    client.println("<tr><td align=\"center\">T1</td><td align=\"center\">GPIO0</td><td align=\"center\">(BOOT)</td></tr>");
    client.println("<tr><td align=\"center\">T2</td><td align=\"center\">GPIO2</td><td align=\"center\">(LED)</td></tr>");
    client.print("<tr><td align=\"center\">T3</td><td>GPIO15(MTDO)</td><td align=\"center\">");
    i=3; client.print(touch_prev[i]); if(touchB[i]) client.print("(ON"); else client.print("(OFF"); client.println(")</td></tr>");
    client.print("<tr><td align=\"center\">T4</td><td>GPIO13(MTCK)</td><td align=\"center\">");
    i=4; client.print(touch_prev[i]); if(touchB[i]) client.print("(ON"); else client.print("(OFF"); client.println(")</td></tr>");
    client.print("<tr><td align=\"center\">T5</td><td>GPIO12(MTD1)</td><td align=\"center\">");
    i=5; client.print(touch_prev[i]); if(touchB[i]) client.print("(ON"); else client.print("(OFF"); client.println(")</td></tr>");
    client.print("<tr><td align=\"center\">T6</td><td>GPIO14(MTMS)</td><td align=\"center\">");
    i=6; client.print(touch_prev[i]); if(touchB[i]) client.print("(ON"); else client.print("(OFF"); client.println(")</td></tr>");
    client.print("<tr><td align=\"center\">T7</td><td>GPIO27</td><td align=\"center\">");
    i=7; client.print(touch_prev[i]); if(touchB[i]) client.print("(ON"); else client.print("(OFF"); client.println(")</td></tr>");
    client.print("<tr><td align=\"center\">T8</td><td>GPIO33(32K_XN)</td><td align=\"center\">");
    i=8; client.print(touch_prev[i]); if(touchB[i]) client.print("(ON"); else client.print("(OFF"); client.println(")</td></tr>");
    client.print("<tr><td align=\"center\">T9</td><td>GPIO32(32K_XP)</td><td align=\"center\">");
    i=9; client.print(touch_prev[i]); if(touchB[i]) client.print("(ON"); else client.print("(OFF"); client.println(")</td></tr>");
    client.println("</table></center>");
    client.println("</p>");
    
    client.println("<hr>");
    client.println("<h3>HTTP GET</h3>");
    client.print("<p>http://");
    client.print(s_ip);
    client.println("/?L=n (n: 0=OFF, 1=ON)</p>");
    sprintf(s,"<form method=\"GET\" action=\"http://%s/\">",s_ip);
    client.println(s);
    client.println("<input type=\"submit\" name=\"L\" value=\"0 (LED OFF)\">");
    client.println("<input type=\"submit\" name=\"L\" value=\"1 (LED ON)\">");
    client.println("</form>");
    client.println("</body>");
    client.println("</html>");
}
