#!/bin/bash

# ログファイルを圧縮してUSBメモリへ保存します。
#
# バックアップ元のログファイル名の例 log_xxxxx_1.csv    xxxxx:5文字
# バックアップ先のログファイル名の例 log20170610.csv    数字は日付を表す
#
# 2017/06/10 国野亘

BU_FROM=""
BU_TO=""

if [ -z ${BU_FROM} ]; then
	BU_FROM="log_?????_?.csv"
fi
if [ -z ${BU_TO} ]; then
	BU_TO=`df|grep media|grep /dev/|grep -v SETTINGS|tail -1|sed -E 's/\s+/,/g'|cut -d, -f6`
fi
BU_TO=${BU_TO}"/log"`date "+%Y%m%d"`".tgz"
#echo "backup from "${BU_FROM}" to "${BU_TO}
PID=`ps a|grep "tar cvfz ${BU_TO}"|grep -v grep|sed -E 's/\s+/,/g'|cut -d, -f2`
if [ "$PID" != "" ];then
	echo "kill "${PID}
	kill $PID
	sleep 1
fi
nice -n 15 tar cvfz ${BU_TO} ${BU_FROM} &> log_bakup_z.log &
ps a|grep "tar cvfz ${BU_TO}"|grep -v grep
