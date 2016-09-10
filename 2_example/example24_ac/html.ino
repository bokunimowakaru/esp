/*******************************************************************************
HTMLコンテンツ LEDの輝度制御

                                            Copyright (c) 2016 Wataru KUNINO
*******************************************************************************/

void html(
	WiFiClient &client,
	int i,
	char *date,
	int TIMER_ON,
	int TIMER_OFF,
	int TIMER_SLEEP,
	uint32_t ip
){
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
    client.print("<meta http-equiv=\"refresh\" content=\"20;URL=http://");
    client.print(s_ip);
    client.println("/\">");
    client.println("</head>");
    client.println("<body>");
    client.println("<h3>AC Relay Controller STATUS</h3>");
    if(i==0) sprintf(s,"<p>0 (Relay OFF)</p>");
    if(i==1) sprintf(s,"<p>1 (Relay ON)</p>");
    client.println(s);
    date[16]='\0';
    sprintf(s,"<p>現在の時刻＝%s</p>",date);
    client.println(s);
	if(TIMER_ON>=0){
		sprintf(s,"<p>タイマー入(毎日)＝%d:%02d</p>",TIMER_ON/60,TIMER_ON%60);
    	client.println(s);
	}
	if(TIMER_OFF>=0){
		sprintf(s,"<p>タイマー切(毎日)＝%d:%02d</p>",TIMER_OFF/60,TIMER_OFF%60);
    	client.println(s);
	}
	if(TIMER_SLEEP>=0){
		sprintf(s,"<p>スリープ　(一回)＝%d:%02d</p>",TIMER_SLEEP/60,TIMER_SLEEP%60);
    	client.println(s);
	}

    client.println("<hr>");
    client.println("<h3>HTTP GET</h3>");
    client.print("<p>http://");
    client.print(s_ip);
    client.println("/?RELAY=n<br>(n: 0=OFF, 1=ON)</p>");
    sprintf(s,"<form method=\"GET\" action=\"http://%s/\">",s_ip);
    client.println(s);
    client.println("<input type=\"submit\" name=\"RELAY\" value=\"0 (RELAY OFF)\">");
    client.println("<input type=\"submit\" name=\"RELAY\" value=\"1 (RELAY ON)\">");
    client.println("<br><br>");
    client.println("<input type=\"submit\" name=\"SLEEP\" value=\"30 (30分後にOFF)\">");
    client.println("<input type=\"submit\" name=\"SLEEP\" value=\"60 (1時間後にOFF)\">");
    client.println("<input type=\"submit\" name=\"SLEEP\" value=\"120 (2時間後にOFF)\">");
    client.println("</form>");
    client.println(s);
    client.print("タイマー入：<input type=\"text\" name=\"ON\" value=\"");
    if(TIMER_ON>=0) client.print(TIMER_ON);
    else client.print("420");
    client.println("\">");
    client.println("<input type=\"submit\" value=\"送信\">");
    client.println("</form>");
    client.println(s);
    client.print("タイマー切：<input type=\"text\" name=\"OFF\" value=\"");
    if(TIMER_OFF>=0) client.print(TIMER_OFF);
    else client.print("480");
    client.println("\">");
    client.println("<input type=\"submit\" value=\"送信\">");
    client.println("</form>");
    client.println(s);
    client.println("<input type=\"submit\" name=\"ON\" value=\"-1 (タイマー入の解除)\">");
    client.println("<input type=\"submit\" name=\"OFF\" value=\"-1 (タイマー切の解除)\">");
    client.println("<input type=\"submit\" name=\"SLEEP\" value=\"-1 (スリープの解除)\">");
    client.println("</form>");
    client.println("<hr>");
    client.println("</body>");
    client.println("</html>");
    sprintf(s,"RELAY=%d",i);
    Serial.println(s);
}
