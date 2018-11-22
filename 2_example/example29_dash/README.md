Example 29
# MACブロードキャスト／ Amazon Dash ボタン検出

スマートフォンやPCが自宅の無線LANへ自動接続したときや、Amazon Dashボタンが押下されたときなどに、送信するMACブロードキャストを検出します。  
プロミスキャスモードで待ち受け、設定ファイルに記載した特定のMACアドレスの端末を検出した時にUDP送信によって通知します。  

設定ファイルは本機の起動時にTFTPで受け取ることが出来ます（シリアル通信で受けるバージョンはexample29u_dashです）。  
起動後、普段はプロミスキャスモードでリスト中のMACアドレスを待ち受けます。  
リストに登録可能な最大件数はMAC_LIST_N_MAXで定義してください。  

## 動作方法

ソースリスト中の#define部へ無線LANアクセスポイントのSSIDとパスワード（PASS）を設定して下さい。  
本機を起動すると、TFTPサーバから設定ファイルを受信し、設定ファイルに記載した特定のMACアドレスの端末を待ち受けます。

## TFTPとは

TFTPはネットワーク機器などの設定ファイルやファームウェアを転送するときなどに使用されているデータ転送プロトコルです。  
使い勝手が簡単で、プロトコルも簡単なので、機器のメンテナンスに向いています。  
認証や暗号化は行わないので、転送時のみ有効にする、もしくは侵入・ファイル転送されても問題の無い用途で利用します。  

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

    $ echo "; List of MAC Address" > /srv/tftp/adash_1.mac
    $ echo "ACCEPT1=FF:FF:FF:FF:FF:FF" >> /srv/tftp/adash_1.mac
    $ echo "ACCEPT2=FF:FF:FF:FF:FF:FF" >> /srv/tftp/adash_1.mac
    $ echo "ACCEPT3=FF:FF:FF:FF:FF:FF" >> /srv/tftp/adash_1.mac
    $ chmod 644 /srv/tftp/adash_1.mac
    $ cat /srv/tftp/adash_1.mac
    ; List of MAC Address
    ACCEPT1=FF:FF:FF:FF:FF:FF
    ACCEPT2=FF:FF:FF:FF:FF:FF
    ACCEPT3=FF:FF:FF:FF:FF:FF
    
    ※FF:FF:FF:FF:FF:FFの部分をお手持ちの機器のアドレスへ書き換えてください

## 参考文献

下記の情報およびソースコードを利用させていただきました(2017/9/16)。

プロミスキャスモードを用いたESP8266でのAmazon Dash Buttonのイベント取得  
<http://qiita.com/kat-kai/items/3b1d5c74138d77a27c4d>

ライセンス：Qiita利用規約に基づく  
権利者：kat-kai http://qiita.com/kat-kai

## ご注意

* プロミスキャスモードはESPモジュール内でWi-Fiで受信可能な全てのパケットを処理します。ネットセキュリティ上の脆弱性の十分に注意して利用してください。
* TFTPクライアント(ESP側)やTFTPサーバ(PCやRaspberry Pi側)起動すると、各機器がセキュリティの脅威にさらされた状態となります。
* また、ウィルスやワームが侵入すると、同じネットワーク上の全ての機器へ感染する恐れが高まります。
* インターネットに接続すると外部からの侵入される場合があります。
* TFTPクライアントは少なくともローカルネット内のみで動作させるようにして下さい。
* TFTPが不必要なときは、停止させてください。

Copyright (c) 2017-2019 Wataru KUNINO  
<https://bokunimo.net/>
