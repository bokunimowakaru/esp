#!/bin/bash
################################################################################
# Server Example 06: ホームオートメーション 熱中症予防
#                                             Copyright (c) 2016-2019 Wataru KUNINO
################################################################################

PORT=1024                                           # 受信UDPポート番号を1024に
IP_IR="192.168.0.5"                                 # 赤外線リモコン送信機のIP
AUTO="OFF"                                          # 運転中フラグ(ON/OFF)
IR_TYPE=0                                           # 方式 0=AEHA,1=NEC,2=SIRC
AC_ON="104,AA,5A,CF,10,00,11,20,3F,18,B0,00,F4,B1"  # エアコンの電源入コマンド
AC_OFF="104,AA,5A,CF,10,00,21,20,3F,18,B0,00,F4,81" # エアコンの電源切コマンド
TEMP=25                                             # 温度の初期値を常温に仮設定

echo "Server Example 06 Auto HVAC (usage: $0 port)" # タイトル表示
curl -s $IP_IR"?TYPE="$IR_TYPE > /dev/null          # リモコン方式の設定
if [ $# -ge 1 ]; then                               # 入力パラメータ数の確認
    if [ $1 -ge 1024 ] && [ $1 -le 65535 ]; then    # ポート番号の範囲確認
        PORT=$1                                     # ポート番号を設定
    fi                                              # ifの終了
fi                                                  # ifの終了
echo "Listening UDP port "$PORT"..."                # ポート番号表示
while true; do                                      # 永遠に繰り返し
    UDP=`timeout 10 nc -luw0 $PORT|tr -d [:cntrl:]|\
    tr -d "\!\"\$\%\&\'\(\)\*\+\;\<\=\>\?\[\\\]\^\{\|\}\~/"`
                                                    # UDPパケットを取得
    DATE=`date "+%Y/%m/%d %R"`                      # 日時を取得
    DEV=${UDP#,*}                                   # デバイス名を取得(前方)
    DEV=${DEV%%,*}                                  # デバイス名を取得(後方)
    if [ -n "$DEV" ]; then                          # デバイス名が得られたとき
        echo -E $DATE, $UDP|tee -a log_$DEV.csv     # 取得日時とデータを保存
    fi
    DET=0                                           # 変数DETの初期化
    VAL=-999                                        # 変数VALの初期化
    case "$DEV" in                                  # DEVの内容に応じて
        "pir_s_"? ) DET=`echo -E $UDP|tr -d ' '|cut -d, -f2`;;
        "temp._"? ) VAL=`echo -E $UDP|tr -d ' '|cut -d, -f2`;;
        "humid_"? ) VAL=`echo -E $UDP|tr -d ' '|cut -d, -f2`;;
        "press_"? ) VAL=`echo -E $UDP|tr -d ' '|cut -d, -f2`;;
    esac
    if [ $VAL != -999 ]; then
        TEMP=`echo $VAL|cut -d. -f1`                # 整数部の切り出し
    fi
    if [ $DET != 0 ]; then                          # 人感センサに反応あり
        if [ $TEMP -ge 28 ]||[ $TEMP -lt 15 ]; then # 28℃以上または15℃未満
            echo "エアコンの運転を開始します。"     # メッセージ表示
            curl -s $IP_IR -XPOST -d"IR="$AC_ON     # エアコンの電源をONに
            AUTO="ON"                               # 運転中
            SECONDS=0                               # 経過時間をリセット
        fi
    fi
    if [ $SECONDS -ge 1800 ]&&[ $AUTO = ON ]; then  # 制御後、30分が経過時
        echo "エアコンの運転を停止します。"         # メッセージ表示
        curl -s $IP_IR -XPOST -d"IR="$AC_OFF        # エアコンの電源をOFFに
        AUTO="OFF"                                  # 停止中
    fi
    # echo $DATE, $SECONDS $AUTO $TEMP $UDP         # デバッグ用
done                                                # 繰り返しここまで
