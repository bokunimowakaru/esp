Example 29u
# MACブロードキャスト／ Amazon Dash ボタン検出

スマートフォンやPCが自宅の無線LANへ自動接続したときや、Amazon Dashボタンが押下されたときなどに、送信するMACブロードキャストを検出します。  
プロミスキャスモードで待ち受け、予め設定した特定のMACアドレスの端末を検出した時にシリアル送信（UART出力）によって通知します。

設定はシリアル通信で行います（TFTPで設定ファイルを受けるバージョンはexample29_dashです）。  
<https://github.com/bokunimowakaru/esp/tree/master/2_example/example29_dash>
起動後、普段はプロミスキャスモードでリスト中のMACアドレスを待ち受けます。  

## 動作方法

ソースリスト中の#define部へ無線LANアクセスポイントのSSIDとパスワード（PASS）を設定して下さい。  
本機を起動し、シリアル通信ですると、TFTPサーバから設定ファイルを受信し、設定ファイルに記載した特定のMACアドレスの端末を待ち受けます。
同じ機器から同種別のデータを連続して受信した場合は出力しません(待機時間0.5秒)。
チャンネルを変更するには、シリアルから「channel=数字」と改行を入力してください。

## 仕様

* シリアル速度は115200bpsです。※9600bpsではありません

* 出力形式：先頭に'(0x27)とスペース(0x20)に続いて6桁のMACアドレスをテキスト出力

    ' xx:xx:xx:xx:xx:xx  
    ' xx:xx:xx:xx:xx:xx  
    ' xx:xx:xx:xx:xx:xx  

* 入力パラメータ：シリアルからコマンドを入力して変更
    
### ???

        シリアル動作確認

### channel=1～12

        検出したい Wi-Fiチャンネルを指定する

### channel?

        Wi-Fiチャンネルを確認する

### filter=0～5

        0: フィルタなし
        2: (標準)0.5秒以内に検出した同じMACの出力を保留
        5: 10秒以内の同じMACの出力を保留

### mode=0～3

        0: (標準) MAC出力モード
        1: adashモード(登録した5個までのAmazon Dashボタンの検出を出力する)
        2: phoneモード(登録した5台までのスマートフォンの検出を出力する)
        3: 1+2の混在モード

### adash=N,XX:XX:XX:XX:XX:XX

        N: adash番号1～5
        XX:XX:XX:XX:XX:XX: MACアドレス

### phone=N,XX:XX:XX:XX:XX:XX

        N: phone番号1～5
        XX:XX:XX:XX:XX:XX: MACアドレス

### phone?

        phone=N,macで登録したMACアドレスを表示する

### adash?

        adash=N,macで登録したMACアドレスを表示する

### time?

        各機器の待ち時間情報を表示する

### wifi=0～1

        0: OFF
        1: ON

### save!

        設定を保存する

## 参考文献

下記の情報およびソースコードを利用させていただきました(2017/9/16)。

プロミスキャスモードを用いたESP8266でのAmazon Dash Buttonのイベント取得  
<http://qiita.com/kat-kai/items/3b1d5c74138d77a27c4d>

ライセンス：Qiita利用規約に基づく  
権利者：kat-kai http://qiita.com/kat-kai

## ご注意

* プロミスキャスモードはESPモジュール内でWi-Fiで受信可能な全てのパケットを処理します。ネットセキュリティ上の脆弱性の十分に注意して利用してください。

Copyright (c) 2017-2018 Wataru KUNINO  
<https://bokunimo.net/>
