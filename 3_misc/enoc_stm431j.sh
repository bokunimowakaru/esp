#!/bin/bash
#
# EnOcean STM431J用 データロガー
#
# このスクリプトを実行すると、STM431Jから得られた温度データをファイルに保存し
# 続けます。
#
#       ファイル名： log_ocean_1.csv
#
# ヒント
# 整数演算でデータを扱っています。bcをインストールすると小数演算も可能です。
#       sudo apt-get install bc
#       TEMP=`echo "scale=1; (255-$TEMP)*40/255"|bc`

DEV="ocean_1"                                           # デバイス名を定義
stty -F /dev/ttyUSB0 57600 -icanon                      # シリアル設定
while true; do                                          # 永久ループの開始
    data=`timeout 1 cat /dev/ttyUSB0|od -An -v -tx1 -w1` # データ取得
    if [ -n "$data" ]; then                             # データがあった場合
        head=`echo $data|cut -d' ' -f1-5`               # 先頭5バイトを抽出
        if [ "$head" == "55 00 0a 02 0a" ]; then        # データがあった場合
            DATE=`date "+%Y/%m/%d %R"`                  # 日時を取得
            TEMP=$(( 0x`echo $data|cut -d' ' -f14`))    # 14バイト目(温度)を抽出
            RSSI=$(( 0x`echo $data|cut -d' ' -f18`))    # 18バイト目(RSSI)を抽出
            TEMP=$(( (255 - $TEMP) * 400 / 255))        # 温度に変換(10倍値)
            DEC=$(( $TEMP / 10))                        # 整数部
            FRAC=$(( $TEMP - $DEC * 10))                # 小数部
            echo -E $DATE, $DEC.$FRAC, -$RSSI|tee -a log_${DEV}.csv
        fi                                              # 日時,温度の表示と保存
    fi
done                                                    # 永久に繰り返す
exit                                                    # 終了
