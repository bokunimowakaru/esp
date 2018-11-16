# Example 58(=32+26):
# センサデバイス用 TFTPクライアント 設定
                                           Copyright (c) 2016-2018 Wataru KUNINO

TFTPサーバ上から設定ファイルをダウンロードし、モジュール内の設定を変更します。
本サンプルではADCの入力ピンとディープスリープ時間を設定することが出来ます。

## 解説



Raspberry PiへのTFTPサーバのインストール方法
    $ sudo apt-get install tftpd-hpa
    
    設定ファイル(/etc/default/tftpd-hpa) 例
    # /etc/default/tftpd-hpa
    TFTP_USERNAME="tftp"
    TFTP_DIRECTORY="/srv/tftp"
    TFTP_ADDRESS="0.0.0.0:69"

TFTPサーバの起動と停止
    $ chmod 755 /srv/tftp
    $ sudo /etc/init.d/tftpd-hpa start
    $ sudo /etc/init.d/tftpd-hpa stop

転送用のファイルを保存
    $ echo "; Hello! This is from RasPi" > /srv/tftp/tftpc_1.ini
    $ echo "ADC_PIN=32" >> /srv/tftp/tftpc_1.ini
    $ echo "SLEEP_SEC=50" >> /srv/tftp/tftpc_1.ini
    $ chmod 644 /srv/tftp/tftpc_1.ini
    $ cat /srv/tftp/tftpc_1.ini
    ; Hello! This is from RasPi
    ADC_PIN=32
    SLEEP_SEC=50

その他
*開発時に下記ライブラリを使用しました(現在はESP32ライブラリに含まれています。)
<https://github.com/copercini/arduino-esp32-SPIFFS>

## 注意事項

* TFTPクライアント(ESP側)やTFTPサーバ(PCやRaspberry Pi側)起動すると、各機器がセキュリティの脅威にさらされた状態となります。
* また、ウィルスやワームが侵入すると、同じネットワーク上の全ての機器へ感染する恐れが高まります。
* インターネットに接続すると外部からの侵入される場合があります。
* TFTPクライアントは少なくともローカルネット内のみで動作させるようにして下さい。
* TFTPが不必要なときは、極力、停止させてください。