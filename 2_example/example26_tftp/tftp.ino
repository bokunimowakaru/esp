/*******************************************************************************
TFTPクライアント

【注意事項】
・TFTPクライアント(ESP側)やTFTPサーバ(PCやRaspberry Pi側)起動すると、各機器が
　セキュリティの脅威にさらされた状態となります。
・また、ウィルスやワームが侵入すると、同じネットワーク上の全ての機器へ感染する
　恐れが高まります。
・インターネットに接続すると外部からの侵入される場合があります。
・TFTPクライアントは少なくともローカルネット内のみで動作させるようにして下さい。
・TFTPが不必要なときは、極力、停止させてください。

                                     Copyright (c) 2016-2021 Wataru KUNINO
*******************************************************************************/

#include <WiFiUdp.h>                        // UDP通信を行うライブラリ
#define TFTP_PORT_C 69                      // TFTP接続用ポート番号(既定)
#define TFTP_PORT_R 12345                   // TFTP待ち受け用ポート番号(任意)
#define TFTP_TIMEOUT 10                     // TFTP待ち受け時間(ms)
#define TFTP_FILE "tftpc_1.ini"             // TFTP受信ファイル名
WiFiUDP tftp;                               // TFTP通信用のインスタンスを定義

int tftpStart(IPAddress IP_TFTP, const char *filename){
    tftp.begin(TFTP_PORT_R);                // TFTP(受信)の開始
    tftp.beginPacket(IP_TFTP, TFTP_PORT_C); // TFTP送信先を設定
    tftp.write(0x0); tftp.write(0x01);      // Read Requestコマンド(RRQ)
    tftp.print(filename);                   // ファイル名
    tftp.write(0x0);                        // ファイル名の終端
    tftp.print("netascii");                 // ASCII:netascii バイナリ:octect
    tftp.write(0x0);                        // モード名の終端
    tftp.endPacket();                       // TFTP送信の終了(実際に送信する)
    Serial.print("Send TFTP RRQ to ");      // 送信完了をシリアル端末へ表示
    Serial.print(IP_TFTP);                  // サーバのIPアドレスを表示
    Serial.print(':');
    Serial.println(TFTP_PORT_C);            // サーバのポート番号を表示
    return 1; // OK
}

int tftpStart(IPAddress IP_TFTP){
    return tftpStart(IP_TFTP,TFTP_FILE);
}

int tftpStart(const char *s, const char *filename){     // sは文字列のIPアドレス
    byte ip[4];
    int v=0;
    int len=strlen(s);
    for(int i=0;i<4;i++) ip[i]=0;
    for(int i=0;i<len;i++){
        if(s[i] >= '0' && s[i] <= '9'){
            ip[v] *= 10;
            ip[v] += (byte)s[i] - (byte)'0';
        }
        if(s[i] == '.'){
            v++;
            if(v >= 4) break;
        }
    }
    if(v == 3){
        IPAddress IP_TFTP(ip[0],ip[1],ip[2],ip[3]);
        tftpStart(IP_TFTP,filename);
        return 1;
    }
    return -1;
}

int tftpStart(const char *s){    // sは文字列のIPアドレス
    return tftpStart(s,TFTP_FILE);
}

int tftpGet(char *data){
    int len=0,time=0,i;
    uint16_t num, port;
    char s[517];
    IPAddress ip;
    memset(s,0,516);
    
    while(len<5){                           // 未受信の間、繰り返し実行
        delay(1);                           // 1msの待ち時間
        len = tftp.parsePacket();           // 受信パケット長を変数lenに代入
        time++;                             // 時間のカウント
        if(time>TFTP_TIMEOUT) return 0;     // タイムアウト(応答値は0)
    }
    ip = tftp.remoteIP();                   // サーバのIPアドレスを取得
    port = tftp.remotePort();               // サーバのポート番号を取得
    tftp.read(s, 516);                      // 最大516バイトまで受信する
    Serial.print("\nRecieved (0x");
    for(int i=0;i<4;i++){
        if(i == 2) Serial.print(" 0x");
        if(s[i] <= 10) Serial.print('0');
        Serial.print(s[i],HEX);             // コマンド、ブロック番号
    }
    Serial.print(") ");
    Serial.print(len - 4);                  // 受信データ長を表示
    Serial.print(" Bytes from ");
    Serial.print(ip);                       // サーバのIPアドレスを表示
    Serial.print(':');
    Serial.println(port);                   // サーバのポート番号を表示
    if(len > 516 || len < 0){
        Serial.println("FILE SIZE ERROR");
        data[0] = '\0';
        return -1;                          // ERROR(応答値は-1),変数dataは破壊
    }
    s[len] = '\0';
    num = (((uint16_t)s[2]) << 8) + (uint16_t)s[3];
    if(s[0]==0x0 && s[1]==0x3){             // TFTP転送データの時
        Serial.print(&s[4]);                // TFTP受信データを表示する
        Serial.println("[EOF]");
        tftp.beginPacket(ip, port);         // TFTP ACK送信先を設定
        tftp.write(0x0); tftp.write(0x04);  // 受信成功コマンド(ACK)を送信
        tftp.write(s[2]);                   // ブロック番号の上位1バイトを送信
        tftp.write(s[3]);                   // ブロック番号の下位1バイトを送信
        tftp.endPacket();                   // TFTP送信の終了(実際に送信する)
        for(i=4;i<len;i++) data[i-4]=s[i];
        data[len-4]='\0';
        return len-4;
    }
    if(s[0]==0x0 && s[1]==0x5){             // ERROR 05の時
        Serial.print("ERROR TFTP: ");
        Serial.println(&s[4]);
    }
    data[0] = '\0';
    return -1;                              // ERROR(応答値は-1),変数dataは破壊
}
