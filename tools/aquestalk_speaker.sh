#!/bin/bash
#
# Raspberry Pi版 ソフトウェア AquesTalk用 HTTP サーバ
# Copyright (c) 2017 Wataru KUNINO
#
# このスクリプトの実行には
# AquesTalkPi と nkf が必要です。
#
# AquesTalkPi のインストール方法
#       ./aquestalk_setup.sh
#
# nkf (Network Kanji Filter) のインストール方法
#       sudo apt-get install nkf
#
# 再生方法
#       curl -s -m3 127.0.0.1/?TEXT="こんにちわ"

amixer cset numid=1 200
IP=`hostname -I|cut -d" " -f1`
TALK="日本語を話します。"

HTML="\
HTTP/1.0 200 OK\n\
\n\
<html><head>\n\
<title>Test Page</title>\n\
<meta http-equiv=\"Content-type\" content=\"text/html; charset=UTF-8\">\n\
</head>\n\
<body>\n\
<h3>AquesTalkPi</h3>\n\
<form method=\"GET\" action=\"http://"${IP}"/\">\n\
<input type=\"text\" name=\"TEXT\" value=\"TALK\">\n\
<input type=\"submit\" value=\"送信\">\n\
</form>\n\
</html>\n\
\n\
"
# echo -e $HTML|sed -e "s/\"TALK\"/\"${TALK}\"/g"       # HTML表示
echo "Listening HTTP port 80..."                        # ポート番号表示
while true                                              # 永遠に
do                                                      # 繰り返し
    echo -e $HTML\
    |sed -e "s/\"TALK\"/\"${TALK}\"/g"\
    |sudo netcat -lw0 -v 80\
    |while read TCP
    do
        # DATE=`date "+%Y/%m/%d %R"`                    # 時刻を取得
        # echo -E $DATE, $TCP                           # 取得日時とデータを表示
        HTTP=`echo -E $TCP|cut -d"=" -f1`               # HTTPコマンドを抽出
        if [ "$HTTP" = "GET /?TEXT" ]; then
            TALK=`echo -E $TCP\
            |cut -d"=" -f2\
            |cut -d" " -f1\
            |sed -e "s/+/ /g"\
            |nkf --url-input`                           # 入力文字を抽出
            echo -E "TEXT="${TALK}
            kill `pidof aplay` &> /dev/null             # 再生中の音声を終了
            sleep 0.5
            aquestalkpi/AquesTalkPi "${TALK}"|aplay &   # 音声再生
        elif [ "$HTTP" = "GET /?VOL" ]; then
            VOL=`echo -E $TCP\
            |cut -d"=" -f2\
            |cut -d" " -f1`
            echo -E "VOL="${VOL}
            amixer cset numid=1 ${VOL}
        fi
    done
done                                                    # 繰り返しここまで
