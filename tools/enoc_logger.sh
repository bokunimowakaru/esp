#!/bin/bash
#
# EnOcean 用 データロガー
#
# このスクリプトを実行すると、EnOceanから得られたデータをファイルに保存し
# 続けます。
#
#       ファイル名： log_ocean_0.csv
#

DEV="ocean_0"                                           # デバイス名を定義
stty -F /dev/ttyUSB0 57600 -icanon                      # シリアル設定
while true; do                                          # 永久ループの開始
    data=`timeout 1 cat /dev/ttyUSB0|od -An -v -tx1 -w1` # データ取得
    data=`echo $data|tr " " ","`                        # カンマ区切りに変換
    if [ -n "$data" ]; then                             # データがあった場合
        DATE=`date "+%Y/%m/%d %R"`                      # 日時を取得
        echo -E $DATE, $data|tee -a log_${DEV}.csv      # 日時の表示と保存
    fi                                                  # ifの終了
done                                                    # 永久に繰り返す
exit                                                    # 終了
