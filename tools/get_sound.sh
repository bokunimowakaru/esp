#!/bin/bash
# マイクからの音声データを取得する
# Copyright (c) 2016-2017 Wataru KUNINO

DEVICE="sound_1"                                    # 配信デバイス名(必須)
PORT=1024                                           # UDPポート番号を1024に

echo "UDP Logger (usage: ${0} port)"                # タイトル表示
if [ ${#} = 1 ]; then                               # 入力パラメータ数が1つ
    if [ ${1} -ge 1 ] && [ ${1} -le 65535 ]; then   # ポート番号の範囲確認
        PORT=${1}                                   # ポート番号を設定
    fi                                              # ifの終了
fi                                                  # ifの終了
echo "Listening UDP port "${PORT}"..."              # ポート番号表示
mkdir sound >& /dev/null                            # 音声保存用フォルダ作成
while true                                          # 永遠に
do                                                  # 繰り返し
    UDP=`sudo netcat -luw0 ${PORT}|tr -d [:cntrl:]|\
    tr -d "\!\"\$\%\&\'\(\)\*\+\-\;\<\=\>\?\[\\\]\^\{\|\}\~"`
                                                    # UDPパケットを取得
    DATE=`date "+%Y/%m/%d %R"`                      # 日時を取得
    DEV=${UDP#,*}                                   # デバイス名を取得(前方)
    DEV=${DEV%%,*}                                  # デバイス名を取得(後方)
    echo -E $DATE, $UDP|tee -a log_${DEV}.csv       # 取得日時とデータを保存
    if [ ${DEVICE} = ${DEV} ]; then                 # マイクからの配信画像時
        DATE=`date "+%Y%m%d-%H%M"`                  # 日時を取得
        URL=`echo -E $UDP|cut -d' ' -f2`            # スペース区切りの2番目
        echo -n "Get "${URL}                        # 音声取得実行表示
        wget -qT10 ${URL} -Osound/${DEVICE}"_"${DATE}.wav   # wget実行
        echo " Done"                                # 終了表示
    fi
done                                                # 繰り返し範囲:ここまで
