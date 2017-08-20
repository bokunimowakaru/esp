#!/bin/bash
# UDPを受信しつつグラフを作成する
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
while true                                          # 永久に
do                                                  # 繰り返し
    UDP=`sudo netcat -luw0 ${PORT}|tr -d [:cntrl:]|\
    tr -d "\!\"\$\%\&\'\(\)\*\+\;\<\=\>\?\[\\\]\^\{\|\}\~/"`
                                                    # UDPパケットを取得
    DATE=`date "+%Y/%m/%d %R"`                      # 日時を取得
    DEV=${UDP#,*}                                   # デバイス名を取得(前方)
    DEV=${DEV%%,*}                                  # デバイス名を取得(後方)
    echo -E $DATE, $UDP|tee -a log_${DEV}.csv       # 取得日時とデータを表示
    FLAG=`echo -E ${UDP}|cut -d, -f3`               # データ2を切り出す
    FLAG=`expr $FLAG : '[0-9]*' 2> /dev/null`       # 数値かどうかを代入する
if [ ${FLAG} ]                                      # 数値FLAGを確認する
then                                                # 数値の時(第2データあり)
    gnuplot << EOF                                  # グラフ描画ツールを起動
    set output 'log_${DEV}.png'                     # 出力ファイル名を設定
    set terminal png                                # 出力ファイル形式を設定
    set datafile separator ','                      # 区切り文字を設定
    set xlabel 'Date'                               # 横軸の名前を「Date」に
    set xdata time                                  # 横軸を時間表示に
    set timefmt'%Y/%m/%d %H:%M'                     # 時刻入力形式の設定
    set format x '%m/%d'                            # 時刻表示形式の設定
    set ytics nomirror                              # 左の縦軸を第1軸に
    set y2tics                                      # 右の縦軸を第2軸に
#   set yrange[0:100]                               # 縦軸の範囲を設定
#   set y2range[0:100]                              # 縦軸(右)の範囲を設定
    set grid                                        # 目盛線を表示
    plot 'log_${DEV}.csv' using 1:3 w lp t '#1',\
    'log_${DEV}.csv' using 1:4 w lp t '#2' axes x1y2    # データを描画
EOF
else
    gnuplot << EOF                                  # グラフ描画ツールを起動
    set output 'log_${DEV}.png'                     # 出力ファイル名を設定
    set terminal png                                # 出力ファイル形式を設定
    set datafile separator ','                      # 区切り文字を設定
    set xlabel 'Date'                               # 横軸の名前を「Date」に
    set xdata time                                  # 横軸を時間表示に
    set timefmt'%Y/%m/%d %H:%M'                     # 時刻入力形式の設定
    set format x '%m/%d'                            # 時刻表示形式の設定
    set ytics nomirror                              # 左の縦軸を第1軸に
#   set yrange[0:100]                               # 縦軸の範囲を設定
    set grid                                        # 目盛線を表示
    plot 'log_${DEV}.csv' using 1:3 w lp t '#1'     # データを描画
EOF
fi
done                                                # 繰り返しここまで
