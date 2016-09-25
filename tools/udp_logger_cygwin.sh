#!/bin/bash
# UDPを受信する
# Copyright (c) 2016 Wataru KUNINO

echo "UDP Logger (usage: ${0} port)"                # タイトル表示
if [ ${#} = 1 ]                                     # 入力パラメータ数の確認
then                                                # 1つだった時
    if [ ${1} -ge 1 ] && [ ${1} -le 65535 ]         # ポート番号の範囲確認
    then                                            # 1以上65535以下の時
        PORT=${1}                                   # ポート番号を設定
    else                                            # 範囲外だった時
        PORT=1024                                   # UDPポート番号を1024に
    fi                                              # ifの終了
else                                                # 1つでは無かった時
    PORT=1024                                       # UDPポート番号を1024に
fi                                                  # ifの終了
echo "Listening UDP port "${PORT}"..."              # ポート番号表示
while true                                          # 永遠に
do                                                  # 繰り返し
    UDP=`nc -luw0 ${PORT}`                          # UDPパケットを取得
    DATE=`date "+%Y/%m/%d %R"`                      # 日時を取得
    echo -E $DATE, $UDP                             # 取得日時とデータを表示
done                                                # 繰り返しここまで
