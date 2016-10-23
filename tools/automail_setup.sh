#!/bin/bash
MAILTO="pi@localhost" # メール送信先

echo "Raspberry Pi 起動後にメールを自動送信するための設定を行うスクリプトです。"
echo "注意：設定後は、起動しても自動的にシャットダウンしてしまいます。"
if [ -e /etc/ssmtp/ssmtp.conf ] && [ -e ~/.muttrc ]; then
	echo "メール設定済みと判断しました。"
else
	./gmail_setup.sh
fi
echo "-------------------------------------------------------------------------"
echo "USBカメラ用のソフトウェアfswebcamをインストールしますか？"
echo "詳細：https://www.sanslogic.co.uk/fswebcam/"
echo "インストールする場合は「yes」を入力してください。"
echo -n "yes/no >"
read yes
if [ $yes = "yes" ]; then
sudo apt-get install fswebcam
fi

echo "-------------------------------------------------------------------------"
echo "自動メール送信を設定します。"
echo "自動メールの送信先を入力してください"
read MAILTO
if [ $yes = "yes" ]; then
cat automail_cam.txt|sed "s/MAILTO=\"\"/MAILTO=\"$MAILTO\"/g" > automail.sh
else
cat automail.txt|sed "s/MAILTO=\"\"/MAILTO=\"$MAILTO\"/g" > automail.sh
fi
sudo chmod a+x automail.sh
sudo cp -b automail_rc.local /etc/rc.local
sudo chmod a+x /etc/rc.local
echo "設定を完了しました。"
