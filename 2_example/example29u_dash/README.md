# MACブロードキャスト／ Amazon Dash ボタン検出

リスト中の特定のMACアドレスの端末のブロードキャストをプロミスキャスモードで待ち受け、検出時にUART出力します。  
スマートフォンやPCを自宅に持ち帰り無線LANへ自動接続するときや、Amazon Dashボタンが押下されたときなどに、送信するMACブロードキャストを検出します。  

Copyright (c) 2017-2018 Wataru KUNINO  
********************************************************************************

* 注意： デフォルトでは シリアルを115200bpsに設定している

* 出力形式：先頭に'(0x27)とスペース(0x20)に続いて6桁のMACアドレスをテキスト出力

`' xx:xx:xx:xx:xx:xx`  
`' xx:xx:xx:xx:xx:xx`  
`' xx:xx:xx:xx:xx:xx`  

* シリアル速度は115200bps

* 入力パラメータ：シリアルからコマンドを入力して変更
    
    ???
        シリアル動作確認
        
    channel=1～12
        検出したい Wi-Fiチャンネルを指定する
        
    channel?
        Wi-Fiチャンネルを確認する
        
    filter=0～5
        0: フィルタなし
        2: (標準)0.5秒以内に検出した同じMACの出力を保留
        5: 10秒以内の同じMACの出力を保留
    
    mode=0～3
        0: (標準) MAC出力モード
        1: adashモード(登録した5個までのAmazon Dashボタンの検出を出力する)
        2: phoneモード(登録した5台までのスマートフォンの検出を出力する)
        3: 1+2の混在モード
        
    adash=N,XX:XX:XX:XX:XX:XX
        N: adash番号1～5
        XX:XX:XX:XX:XX:XX: MACアドレス
        
    phone=N,XX:XX:XX:XX:XX:XX
        N: phone番号1～5
        XX:XX:XX:XX:XX:XX: MACアドレス
        
    phone?
        phone=N,macで登録したMACアドレスを表示する
        
    adash?
        adash=N,macで登録したMACアドレスを表示する
        
    time?
        各機器の待ち時間情報を表示する
        
    wifi=0～1
        0: OFF
        1: ON
        
    save!
        設定を保存する

* 参考文献

下記の情報およびソースコードを利用させていただきました(2017/9/16)。

プロミスキャスモードを用いたESP8266でのAmazon Dash Buttonのイベント取得  
<http://qiita.com/kat-kai/items/3b1d5c74138d77a27c4d>

ライセンス：Qiita利用規約に基づく  
権利者：kat-kai http://qiita.com/kat-kai
