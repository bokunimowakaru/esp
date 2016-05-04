#!/bin/bash
# UDPを受信する
# Copyright (c) 2016 Wataru KUNINO
echo "UDP Logger (Usage: ${0} port)"            # タイトル表示
echo "Listening UDP port "${1}"..."             # ポート番号表示
while [ ${1} -ge 1 ] && [ ${1} -lt 65536 ]      # ポート番号の範囲確認
do                                              # 繰り返し
    UDP=`sudo netcat -luw0 ${1}`                # UDPパケットを取得
    DATE=`date "+%Y/%m/%d %R"`                  # 日時を取得
    echo $DATE, $UDP                            # 取得日時とデータを表示
done                                            # 繰り返しここまで
