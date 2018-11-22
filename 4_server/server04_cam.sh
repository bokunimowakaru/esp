#!/bin/bash
################################################################################
# Server Example 04: 防犯システム
#                                             Copyright (c) 2016-2019 Wataru KUNINO
################################################################################

PORT=1024                                           # 受信UDPポート番号を1024に
REED=1                                              # ドアスイッチON検出=0 OFF=1
IP_CAM="192.168.0.5"                                # カメラのIPアドレス
FILE="cam_a_1"                                      # 保存時のファイル名

echo "Server Example 04 Cam (usage: $0 port)"       # タイトル表示
if [ $# -ge 1 ]; then                               # 入力パラメータ数の確認
    if [ $1 -ge 1024 ] && [ $1 -le 65535 ]; then    # ポート番号の範囲確認
        PORT=$1                                     # ポート番号を設定
    fi                                              # ifの終了
fi                                                  # ifの終了
echo "Listening UDP port "$PORT"..."                # ポート番号表示
mkdir photo >& /dev/null                            # 写真保存用フォルダ作成
while true; do                                      # 永遠に繰り返し
    UDP=`nc -luw0 $PORT|tr -d [:cntrl:]|\
    tr -d "\!\"\$\%\&\'\(\)\*\+\;\<\=\>\?\[\\\]\^\{\|\}\~/"`
                                                    # UDPパケットを取得
    DATE=`date "+%Y/%m/%d %R"`                      # 日時を取得
    DEV=${UDP#,*}                                   # デバイス名を取得(前方)
    DEV=${DEV%%,*}                                  # デバイス名を取得(後方)
    echo -E $DATE, $UDP|tee -a log_$DEV.csv         # 取得日時とデータを保存
    CAM=0                                           # 変数CAMの初期化
    case "$DEV" in                                  # DEVの内容に応じて
        "rd_sw_"? ) DET=`echo -E $UDP|tr -d ' '|cut -d, -f2`
                    if [ $DET -eq $REED ]; then     # 応答値とREED値が同じとき
                        CAM=1
                    fi ;;
        "pir_s_"? ) DET=`echo -E $UDP|tr -d ' '|cut -d, -f2`
                    if [ $DET != 0 ]; then          # 応答値が0以外の時
                        CAM=1
                    fi ;;
        "Pong" )    CAM=1 ;;
    esac
    if [ $CAM != 0 ]; then                          # CAMが0で無いとき
        wget -qT10 $IP_CAM/cam.jpg                  # 写真撮影と写真取得
        SFX=`date "+%Y%m%d-%H%M"`                   # 撮影日時を取得し変数SFXへ
        mv cam.jpg photo/$FILE"_"$SFX.jpg >& /dev/null
    fi                                              # 写真の保存
done                                                # 繰り返しここまで
