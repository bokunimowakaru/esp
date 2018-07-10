#!/bin/bash
#########
# river #
#########

# 河川の水位情報をLCDへ表示する River Water Lev

# 主要機能：
#    ・国土交通省「川の防災情報」から水位を取得し、表示とUDP送信を行います。

# 地域設定：
#    ・RIVER_SPOT_ID に観測所コードを設定してください。
#    ・コードは川の防災情報の河川・観測所検索から検索してください。
#            https://www.river.go.jp/kawabou/schObsrv.do

# ご注意：
#    ・国土交通省・川の防災情報はブラウザで閲覧することを前提に公開されています。
#    ・実運用を行う場合は、一般財団法人・河川情報センターが配信する水防災オープン
#    　データ提供サービスなどの数値データを利用したシステムへ修正してください。
#    ・ブラウザでの閲覧よりもサーバへ負担がかかる使い方は控える必要があります。
#            https://www.river.go.jp/kawabou/qa/QA/chu-i3.html

# curl -v "http://www.river.go.jp/kawabou/ipSuiiKobetu.do?obsrvId=2206800400013&gamenId=01-1003&stgGrpKind=survOnly&fvrt=yes&timeType=10"
# DATA=`curl -v "http://www.river.go.jp/kawabou/ipSuiiKobetu.do?obsrvId="${RIVER_SPOT_ID}"&gamenId=01-1003&stgGrpKind=survOnly&fvrt=yes&timeType=10"|tr -d "\n\r\t"|tr '<' '\n'|grep -e "td width=\"30%\" class=\"tb1td1Right\">" -e "td width=\"21%\" class=\"tb1td2Right\""|cut -d'>' -f2|tail -2|tr '\n' ' '`

RIVER_SPOT_ID="2206800400013"           # 2206800400013 = 淀川
RIVER_DEPTH_MAX=500                     # 氾濫水位 1cm単位の整数
RIVER_DEPTH_ALRT=440                    # 避難水位 1cm単位の整数
RIVER_DEPTH_WARN=380                    # 待機水位 1cm単位の整数

IP_TO="127.0.0.1"                       # UDP送信先IPアドレス
PORT_TO=1024                            # UDP送信先ポート番号
DEVICE="river_1"                        # 本機デバイス名
URL_OPTION="&timeType=10"               # URLに付与するオプション

while true; do
    DATA=`curl -s "http://www.river.go.jp/kawabou/ipSuiiKobetu.do?obsrvId="${RIVER_SPOT_ID}"&gamenId=01-1003&stgGrpKind=survOnly&fvrt=yes${URL_OPTION}"\
        |tr -d "\n\r\t"|tr '<' '\n'\
        |grep -e "td width=\"30%\" class=\"tb1td1Right\">" -e "td width=\"21%\" class=\"tb1td2Right\""\
        |cut -d'>' -f2|tail -2|tr '\n' ' '`
    DATE_NOW=`date "+%Y/%m/%d %R"`
    DATE_DAT=`echo ${DATA}|cut -d' ' -f1-2|tr '/' ','|tr ' ' ','|tr ':' ','`
    DEPTH=`echo ${DATA}|cut -d' ' -f3`
    depth=`echo "${DEPTH} * 100"|bc|awk '{printf "%d",$1}'`
    if [ -z ${depth} ]; then
        if [ -n ${URL_OPTION} ]; then
            URL_OPTION=""
            echo "ERROR: no depth in 10 min.'s data"
            continue
        else
            echo "ERROR: no depth in 60 min.'s data"
            depth=0
        fi
    fi
    echo -E $DATE_NOW, ${DEVICE},${DATE_DAT},${depth}
#   sudo echo -E ${DEVICE},${DATE_DAT},${depth} > /dev/udp/${IP_TO}/${PORT_TO}
    if [ $depth -ge $RIVER_DEPTH_MAX ]; then
        sleep 59
        URL_OPTION="&timeType=10"
    elif [ $depth -ge $RIVER_DEPTH_ALRT ]; then
        sleep 178
        URL_OPTION="&timeType=10"
    elif [ $depth -ge $RIVER_DEPTH_WARN ]; then
        sleep 590
        URL_OPTION="&timeType=10"
    else
        sleep 3540
        URL_OPTION=""
    fi
done
