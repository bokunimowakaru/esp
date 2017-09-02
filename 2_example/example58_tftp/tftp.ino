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

                                               Copyright (c) 2017 Wataru KUNINO
*******************************************************************************/

#define TFTP_PORT_C 69                      // TFTP接続用ポート番号(既定)
#define TFTP_PORT_T 1234                    // TFTP転送用ポート番号(任意)
#define TFTP_TIMEOUT 10                     // TFTP待ち受け時間(ms)
#define TFTP_FILE "/srv/tftp/tftpc_1.ini"   // TFTP受信ファイル名
WiFiUDP tftp;                               // TFTP通信用のインスタンスを定義

void tftpStart(){
    tftp.begin(TFTP_PORT_T);                // TFTP(受信)の開始
    tftp.beginPacket(SENDTO, TFTP_PORT_C);  // TFTP送信先を設定
    tftp.write(0x0); tftp.write(0x01);      // Read Requestコマンド(RRQ)
    tftp.print(TFTP_FILE);                  // ファイル名
    tftp.write(0x0);                        // ファイル名の終端
    tftp.print("netascii");                 // ASCIIモード
    tftp.write(0x0);                        // モード名の終端
    tftp.endPacket();                       // TFTP送信の終了(実際に送信する)
    Serial.println("Send TFTP RRQ");        // 送信完了をシリアル端末へ表示
}

int tftpGet(char *data){
    int len=0,time=0,i;
    IPAddress ip;
    
    while(len<5){                           // 未受信の間、繰り返し実行
        delay(1);                           // 1msの待ち時間
        len = tftp.parsePacket();           // 受信パケット長を変数lenに代入
        time++;                             // 時間のカウント
        if(time>TFTP_TIMEOUT) return 0;     // タイムアウト(応答値は0)
    }
    ip=tftp.remoteIP();                     // サーバのIPアドレスを取得
    tftp.read(data, 511);                   // 最大511バイトまで受信する
    Serial.print("Recieved (");
    for(int i=0;i<4;i++) Serial.print(data[i],DEC); // コマンド、ブロック番号
    Serial.print(") ");
    Serial.print(len - 4);                  // 受信データ長を表示
    Serial.print(" Bytes from ");
    Serial.println(ip);                     // サーバのIPアドレスを表示
    if(len > 511){
        Serial.println("FILE SIZE ERROR");  // 複数ブロックの転送には対応しない
        return -1;                          // ERROR(応答値は-1),変数dataは破壊
    }
    if(data[0]==0x0 && data[1]==0x3){       // TFTP転送データの時
        Serial.print(&data[4]);             // TFTP受信データを表示する
        Serial.println("[EOF]");
        tftp.beginPacket(ip, TFTP_PORT_T);  // TFTP ACK送信先を設定
        tftp.write(0x0); tftp.write(0x04);  // 受信成功コマンド(ACK)を送信
        tftp.write(data[2]);                // ブロック番号の上位1バイトを送信
        tftp.write(data[3]);                // ブロック番号の下位1バイトを送信
        tftp.endPacket();                   // TFTP送信の終了(実際に送信する)
        for(i=4;i<len;i++) data[i-4]=data[i];
        data[len-4]='\0';
        return len-4;
    }
    return -1;                              // ERROR(応答値は-1),変数dataは破壊
}
