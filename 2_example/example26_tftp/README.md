Example 26:
# センサデバイス用 TFTPクライアント 設定

TFTPサーバ上から設定ファイルをダウンロードし、モジュール内の設定を変更します。  
本サンプルではESP-WROOM-02モジュールのディープスリープ時間を設定することが出来ます。

## TFTPとは

TFTPはネットワーク機器などの設定ファイルやファームウェアを転送するときなどに使用されているデータ転送プロトコルです。  
使い勝手が簡単で、プロトコルも簡単なので、機器のメンテナンスに向いています。  
認証や暗号化は行わないので、転送時のみ有効にする、もしくは侵入・ファイル転送されても問題の無い用途で利用します。  

TFTPについて、より詳しいサンプルも公開しました（2021/01/16）  
    複数ブロック受信,SDカードへの保存に対応：  
    https://github.com/bokunimowakaru/tftp  

## 本サンプルの仕様

「SLEEP_SEC=時間（秒）」をTFTPで受信すると、ESPモジュールのスリープ間隔を変更することが出来ます。  

## Raspberry PiへのTFTPサーバのインストール方法

    $ sudo apt-get install tftpd-hpa
    
## 設定ファイル(/etc/default/tftpd-hpa) の例

    # /etc/default/tftpd-hpa
    TFTP_USERNAME="tftp"
    TFTP_DIRECTORY="/srv/tftp"
    TFTP_ADDRESS="0.0.0.0:69"

## TFTPサーバの起動と停止

    $ chmod 755 /srv/tftp
    $ sudo /etc/init.d/tftpd-hpa start
    $ sudo /etc/init.d/tftpd-hpa stop

## 転送用のファイルを保存

    $ sudo echo "; Hello! This is from RasPi" | sudo tee /srv/tftp/tftpc_1.ini
    $ sudo echo "SLEEP_SEC=50" | sudo tee -a /srv/tftp/tftpc_1.ini
    $ sudo chmod 644 /srv/tftp/tftpc_1.ini
    $ cat /srv/tftp/tftpc_1.ini
    ; Hello! This is from RasPi
    SLEEP_SEC=50

## 注意事項

* TFTPクライアント(ESP側)やTFTPサーバ(PCやRaspberry Pi側)起動すると、各機器がセキュリティの脅威にさらされた状態となります。
* また、ウィルスやワームが侵入すると、同じネットワーク上の全ての機器へ感染する恐れが高まります。
* インターネットに接続すると外部からの侵入される場合があります。
* TFTPクライアントは少なくともローカルネット内のみで動作させるようにして下さい。
* TFTPが不必要なときは、停止させてください。

Copyright (c) 2016-2021 Wataru KUNINO  
<https://bokunimo.net/>
