#!/bin/bash
# UDPを受信しつつグラフを作成する [保存先=Ambient(https://ambidata.io/)]
# Copyright (c) 2016 Wataru KUNINO

echo "UDP Logger (usage: ${0} port)"                # タイトル表示
AmbientChannelId=100                                # AmbientチャネルID(整数)
AmbientWriteKey="0123456789abcdef"                  # ライトキー(16桁の16進数)
HOST="54.65.206.59"                                 # 送信先アドレス(変更不要)
if [ ${#} = 1 ];then                                # 入力パラメータ数が1つの時
    if [ ${1} -ge 1 ] && [ ${1} -le 65535 ];then    # ポート番号が1以上65535以下
        PORT=${1}                                   # ポート番号を設定
    else                                            # 範囲外だった時
        PORT=1024                                   # UDPポート番号を1024に
    fi                                              # ifの終了
else                                                # 1つでは無かった時
    PORT=1024                                       # UDPポート番号を1024に
fi                                                  # ifの終了
echo "Listening UDP port "${PORT}"..."              # ポート番号表示

while true;do                                       # 永久に繰り返し
UDP=`sudo netcat -luw0 ${PORT}|tr -d [:cntrl:]|\
tr -d "\!\"\$\%\&\'\(\)\*\+\-\;\<\=\>\?\[\\\]\^\{\|\}\~"`
                                                    # UDPパケットを取得
DATE=`date "+%Y/%m/%d %R"`                          # 日時を取得
DEV=${UDP#,*}                                       # デバイス名を取得(前方)
DEV=${DEV%%,*}                                      # デバイス名を取得(後方)
echo -E $DATE, $UDP|tee -a log_${DEV}.csv           # 取得日時とデータを保存
DATA=""                                             # 変数DATAの初期化
case "$DEV" in                                      # DEVの内容に応じて
"humid_1" ) DATA=\"d1\"\:\"`echo -E ${UDP}|tr -d ' '|cut -d, -f2-3|sed "s/,/\",\"d2\"\:\"/g"`\";;
"press_1" ) DATA=\"d3\"\:\"`echo -E ${UDP}|tr -d ' '|cut -d, -f2-3|sed "s/,/\",\"d4\"\:\"/g"`\";;
"illum_1" ) DATA=\"d5\"\:\"`echo -E ${UDP}|tr -d ' '|cut -d, -f2`\";;
"temp._1" ) DATA=\"d6\"\:\"`echo -E ${UDP}|tr -d ' '|cut -d, -f2`\";;
"rd_sw_1" ) DATA=\"d7\"\:\"`echo -E ${UDP}|tr -d ' '|cut -d, -f2-3|sed "s/,/\",\"d8\"\:\"/g"`\";;
esac
if [ -n "$DATA" ];then                              # 変数にデータが入っている時
JSON="{\"writeKey\":\"${AmbientWriteKey}\",${DATA}}"    # データを生成
curl -s ${HOST}/api/v2/channels/${AmbientChannelId}/data -X POST -H "Content-Type: application/json" -d ${JSON}
fi                                                  # データを送信
done                                                # 繰り返し
