#!/bin/bash
################################################################################
# Server Example 01: 玄関呼鈴システム
#                                              Copyright (c) 2016 Wataru KUNINO
################################################################################

PORT=1024                                           # 受信UDPポート番号を1024に
REED=1                                              # ドアスイッチON検出=0 OFF=1
DEV_BELL="192.168.0.2"                              # ワイヤレスBELLのIPアドレス

echo "Server Example 01 Bell (usage: $0 port)"      # タイトル表示
if [ $# -ge 1 ]; then                               # 入力パラメータ数の確認
    if [ $1 -ge 1024 ] && [ $1 -le 65535 ]; then    # ポート番号の範囲確認
        PORT=$1                                     # ポート番号を設定
    fi                                              # ifの終了
fi                                                  # ifの終了
echo "Listening UDP port "$PORT"..."                # ポート番号表示
while true; do                                      # 永遠に繰り返し
    UDP=`nc -luw0 $PORT|tr -d [:cntrl:]|\
    tr -d "\!\"\$\%\&\'\(\)\*\+\-\;\<\=\>\?\[\\\]\^\{\|\}\~"`
                                                    # UDPパケットを取得
    DATE=`date "+%Y/%m/%d %R"`                      # 日時を取得
    DEV=${UDP#,*}                                   # デバイス名を取得(前方)
    DEV=${DEV%%,*}                                  # デバイス名を取得(後方)
    echo -E $DATE, $UDP                             # 取得日時とデータを表示
    BELL=0                                          # 変数BELLの初期化
    case "$DEV" in                                  # DEVの内容に応じて
        "rd_sw_1" ) DET=`echo -E $UDP|tr -d ' '|cut -d, -f2`
                    if [ $DET -eq $REED ]; then     # 応答値とREED値が同じとき
                        BELL=2                      # 変数BELLへ2を代入
                    fi
                    ;;
        "pir_s_1" ) DET=`echo -E $UDP|tr -d ' '|cut -d, -f2`
                    if [ $DET != 0 ]; then          # 応答値が0以外の時
                        BELL=1                      # 変数BELLへ1を代入
                    fi
                    ;;
        "Ping" )    BELL=-2                         # 変数BELLへ-2を代入
                    ;;
        "Pong" )    BELL=-1                         # 変数BELLへ-1を代入
                    ;;
    esac
    if [ -n "$DEV_BELL" ] && [ $BELL != 0 ]; then   # BELLが0で無いとき
        echo -n "BELL="                             # 「BELL=」を表示
        RES=`curl -s -m3 $DEV_BELL -XPOST -d"B=$BELL"\
        |grep "<p>"|grep -v "http"\
        |cut -d'>' -f2|cut -d'<' -f1`               # ワイヤレスBELL制御
        if [ -n "$RES" ]; then                      # 応答があった場合
            echo $RES                               # 応答内容を表示
        else                                        # 応答が無かった場合
            echo "ERROR"                            # ERRORを表示
        fi                                          # ifの終了
    fi
done                                                # 繰り返しここまで
