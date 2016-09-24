#!/bin/bash
#
# EnOcean STM431J用 データロガー
#
# このスクリプトを実行するとSTM431Jから得られた温度データをAmbientに送信します。
# [保存先=Ambient(https://ambidata.io/)]
#
# 測定間隔と前回の値との差分値についても送信します。

AmbientChannelId=100                                    # AmbientチャネルID
AmbientWriteKey="0123456789abcdef"                      # ライトキー(16進数)
HOST="54.65.206.59"                                     # 送信先アドレス
DEV="ocean_1"                                           # デバイス名を定義
SECONDS=0                                               # 経過時間をリセット

stty -F /dev/ttyUSB0 57600 -icanon                      # シリアル設定
while true; do                                          # 永久ループの開始
    data=`timeout 1 cat /dev/ttyUSB0|od -An -v -tx1 -w1` # データ取得
    if [ -n "$data" ]; then                             # データがあった場合
        # echo $data                                    # データ列の表示
        head=`echo $data|cut -d' ' -f1-5`               # 先頭5バイトを抽出
        if [ "$head" == "55 00 0a 02 0a" ]; then        # データがあった場合
            DATE=`date "+%Y/%m/%d %R"`                  # 日時を取得
            TEMP=$(( 0x`echo $data|cut -d' ' -f14`))    # 14バイト目(温度)を抽出
            RSSI=$(( 0x`echo $data|cut -d' ' -f18`))    # 18バイト目(RSSI)を抽出
            TEMP=$(( (255 - $TEMP) * 400 / 255))        # 温度に変換(10倍値)
            DEC=$(( $TEMP / 10))                        # 整数部
            FRAC=$(( $TEMP - $DEC * 10))                # 小数部
            echo -E $DATE, $DEC.$FRAC, -$RSSI|tee -a log_${DEV}.csv
            DATA=\"d1\"\:\"$DEC.$FRAC\"                 # データ生成(温度)
            DATA=${DATA},\"d2\"\:\"-$RSSI\"             # データ生成(RSSI)
            DATA=${DATA},\"d3\"\:\"$SECONDS\"           # データ生成(経過時間)
            SECONDS=0                                   # 経過時間をリセット
            if [ $TEMP_ ]; then                         # 前回の温度値が存在する
                DATA=${DATA},\"d4\"\:\"$(( $TEMP - $TEMP_ ))\"
            fi                                          # 温度の差分(10倍値)
            TEMP_=$TEMP                                 # 今回の値を保存
            JSON="{\"writeKey\":\"${AmbientWriteKey}\",${DATA}}"
            curl -s ${HOST}/api/v2/channels/${AmbientChannelId}/data\
                 -X POST -H "Content-Type: application/json" -d ${JSON}
        fi                                              # 日時,温度の表示と保存
    fi
done                                                    # 永久に繰り返す
exit                                                    # 終了
