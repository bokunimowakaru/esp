#!/bin/bash
# AQUEST社の AquesTalkをダウンロードし、インストールします。
# 
# http://www.a-quest.com/products/aquestalkpi.html
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
echo "再生を行います。"
aquestalkpi/AquesTalkPi -f aquestalkpi/test.txt |aplay
echo "セットアップ完了です。"
echo "詳細はこちら：http://blog-yama.a-quest.com/?eid=970157"
;;
*)
echo "ダウンロードをキャンセルしました。"
;;
esac
exit
