#!/bin/bash
# AQUEST社の AquesTalkをダウンロードし、インストールします。
# 
# http://www.a-quest.com/products/aquestalkpi.html

if [ ! -e "./aquestalkpi/AquesTalkPi" ]; then
    echo "AquesTalkのウェブサイトにアクセスして、ライセンスを確認してください。"
    echo "http://www.a-quest.com/products/aquestalkpi.html"
    echo "上記のライセンスに同意する場合は「yes」を入力してください。"
    echo -n "yes/no >"
    read yes
    case $yes in
    yes)
    echo "ダウンロードを実行中です。"
    wget http://www.a-quest.com/download/package/aquestalkpi-20130827.tgz
    echo "インストールを実行中です。"
    tar xzvf aquestalkpi-*.tgz
    echo "セットアップ完了です。"
    echo "詳細はこちら：http://blog-yama.a-quest.com/?eid=970157"
    ;;
    *)
    echo "ダウンロードをキャンセルしました。"
    ;;
    esac
fi
echo "時報をセットします。セットしたくない場合は「no」を入力してください。"
echo -n "yes/no >"
read yes

#if [ ! -e "/etc/cron.daily/talk_time_signal" ]; then
if [ $yes != "no" ]; then
    DIR=`pwd`
    EXIST=`grep aquestalk_tsig.sh /etc/crontab`
    if [ -z "$EXIST" ]; then
        echo "0  *    * * *   pi      "${DIR}"/aquestalk_tsig.sh" | sudo tee -a /etc/crontab
        sleep 0.1
        sudo /etc/init.d/rsyslog restart
        echo "/etc/crontab へ aquestalk_tsig.sh を設定しました。"
        # cat /etc/crontab
    fi
    echo "再生テストを行います。"
    aquestalkpi/AquesTalkPi -f aquestalkpi/test.txt |aplay
    echo "再生できればセットアップ完了です。"
fi
echo "終了"
exit
