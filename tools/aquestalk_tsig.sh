#!/bin/bash
# 現在の時刻を再生します。
#
# 本プログラムを実行するには、AQUEST社の AquesTalkPiが必要です。
#
# AquesTalkPi のインストール方法
#       ./aquestalk_setup.sh

TIME_START=6                                # 時報の開始時刻(6:00～)
TIME_END=23                                 # 時報の終了時刻(～23:59)
HOUR=`date "+%_H"`                          # 現在の時刻(時)
MIN=`date "+%_M"`                           # 現在の時刻(分)
if [ ${HOUR} -ge ${TIME_START} ] && [ ${HOUR} -le ${TIME_END} ]; then
    kill `pidof aplay` &> /dev/null         # 再生中の音声を終了
    sleep 0.5
    if [ ${MIN} -eq 0 ]; then
        # 毎時0分のときは「分」を省略して再生
        TALK=${HOUR}"時です。"
    else
        # 毎時0分以外の時は「時」と「分」を再生
        TALK=${HOUR}"時"${MIN}"分です。"
    fi
    # 再生の実行と、60秒後にaplayプロセスの終了
    /home/pi/esp/tools/aquestalkpi/AquesTalkPi ${TALK}|aplay
	(sleep 60; kill `pidof aplay` &> /dev/null) &
fi
