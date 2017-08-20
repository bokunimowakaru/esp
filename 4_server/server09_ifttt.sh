#!/bin/bash
################################################################################
# Server Example 09: IFTTT 送信
#
#                                              Copyright (c) 2017 Wataru KUNINO
################################################################################

# IFTTTのKeyを(https://ifttt.com/maker_webhooks)から取得し、変数KEYへ代入する
KEY="xx_xxxx_xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"   # IFTTTのKey(鍵)

PORT=1024                                           # 受信UDPポート番号を1024に
REED=1                                              # ドアスイッチON検出=0 OFF=1
URL="https://maker.ifttt.com/trigger/"              # IFTTTのURL(変更不要)

echo "Server Example 09 IFTTT (usage: $0 port)"     # タイトル表示
if [ $# -ge 1 ]; then                               # 入力パラメータ数の確認
    if [ $1 -ge 1024 ] && [ $1 -le 65535 ]; then    # ポート番号の範囲確認
        PORT=$1                                     # ポート番号を設定
    fi                                              # ifの終了
fi                                                  # ifの終了
echo "Listening UDP port "$PORT"..."                # ポート番号表示
while true; do                                      # 永遠に繰り返し
    UDP=`nc -luw0 $PORT|tr -d [:cntrl:]|\
    tr -d "\!\"\$\%\&\'\(\)\*\+\;\<\=\>\?\[\\\]\^\{\|\}\~/"`
                                                    # UDPパケットを取得
    DATE=`date "+%Y/%m/%d %R"`                      # 日時を取得
    DEV=${UDP#,*}                                   # デバイス名を取得(前方)
    DEV=${DEV%%,*}                                  # デバイス名を取得(後方)
    echo -E $DATE, $UDP                             # 取得日時とデータを表示
    DET=0                                           # 変数DETの初期化
    case "$DEV" in                                  # DEVの内容に応じて
        "rd_sw_"? ) DET=`echo -E $UDP|tr -d ' '|cut -d, -f2`
                    if [ $DET -eq $REED ]; then     # 応答値とREED値が同じとき
                        DET=1                       # 変数DETへ1を代入
                    else                            # 異なるとき
                        DET=0                       # 変数DETへ0を代入
                    fi;;
        "pir_s_"? ) DET=`echo -E $UDP|tr -d ' '|cut -d, -f2`;;
        "Ping" )    DET=1;;                         # 変数DETへ1を代入
    esac
    if [ $DET != 0 ]; then                          # 人感センサに反応あり
        RES=`curl -s -m3 -XPOST ${URL}${DEV}/with/key/${KEY}`   # IFTTTへ送信
        if [ -n "$RES" ]; then                      # 応答があった場合
            echo $RES                               # 応答内容を表示
        else                                        # 応答が無かった場合
            echo "ERROR"                            # ERRORを表示
        fi                                          # ifの終了
    fi
done                                                # 繰り返しここまで
