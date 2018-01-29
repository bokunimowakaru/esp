#!/bin/bash
# UDPを監視する 指定した時間内にデータが得られなければメールを送信する
# Copyright (c) 2018 Wataru KUNINO

MAILTO="xbee@dream.jp"                              # メール送信先
PORT=1024                                           # 受信UDPポート番号を1024に
DEVS=(
    "humid_1 90"
    "humid_2 90"
    "humid_3 90"
    "humid_4 90"
    "e_co2_1 180"
    "e_co2_2 180"
    "e_co2_3 180"
    "ocean_1 180"
    "ocean_2 180"
    "ocean_3 180"
    "adcnv_1 180"
    "adcnv_2 180"
    "adcnv_3 180"
)                                                   # 監視センサ 監視間隔(分)
NUM=${#DEVS[*]}                                     # 監視センサ数
echo "udp_alive_mon, NUM = "${NUM}
for (( i=0; i < $NUM; i++ )); do
    time[i]=$SECONDS
done

echo "Listening UDP port "$PORT"..."                # ポート番号表示
while true; do                                      # 永遠に繰り返し
    UDP=`timeout 60 nc -luw0 $PORT|tr -d [:cntrl:]|\
    tr -d "\!\"\$\%\&\'\(\)\*\+\;\<\=\>\?\[\\\]\^\{\|\}\~/"`
                                                    # UDPパケットを取得
    DATE=`date "+%Y/%m/%d %R"`                      # 日時を取得
    DEV=${UDP#,*}                                   # デバイス名を取得(前方)
    DEV=${DEV%%,*}                                  # デバイス名を取得(後方)
    DET=0                                           # 変数DETの初期化
    MAIL=""
    for (( i=0; i < $NUM; i++ )); do
        dev=(${DEVS[i]})
        if [ "${DEV}" = "${dev[0]}" ]; then
            time[i]=$(( SECONDS / 60 ))
            echo -E $DATE, $DEV, ${time[i]}         # 取得日時とデバイス名、時間
        fi
        d=$(( SECONDS / 60 - time[i] ))             # 経過時間を計算
        if [ $d -ge ${dev[1]} ]; then
            MAIL="デバイス "${dev[0]}" の送信が "${d}" 分間、ありません"
            echo $MAIL
            time[i]=$(( SECONDS / 60 ))
        fi
    done
    if [ -n "$MAIL" ]; then                         # MAILが空で無いとき
        if [ -n "$MAILTO" ]; then                   # 送信先が設定されているとき
            echo -E $MAIL\
            |mutt -s "Alive_Mon ALERT!" -- $MAILTO  # メールを送信
        fi
    fi
done
