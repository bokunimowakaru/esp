#!/bin/bash

# Gmail(Google のメール) セットアップ
# 2016/07/02 国野亘
#
# このスクリプトは下記のサイトの情報を参考にして作成しました。
# http://www.tapun.net/raspi/raspberry-pi-ssmtp-mutt

echo "Gmail(Google のメール) セットアップ"
echo "Gmailのアカウントを入力してください"
read MAIL
echo "Gmailのパスワードを入力してください"
read PASS

echo
echo "sSMTPとMuttのインストールを開始します"
sudo apt-get install -y ssmtp
sudo apt-get install -y mutt

echo
echo "設定ファイルを作成します"
cat << EOF > ssmtp.conf
root=MAIL@gmail.com
mailhub=smtp.gmail.com:587
hostname=raspberrypi
AuthUser=MAIL@gmail.com
AuthPass=PASS
UseSTARTTLS=YES
EOF

cat ssmtp.conf|sed s/MAIL/${MAIL}/g|sed s/PASS/${PASS}/g > ssmtp.conf~
sudo mv /etc/ssmtp/ssmtp.conf /etc/ssmtp/ssmtp.conf~ >& /dev/null
sudo mv ssmtp.conf~ /etc/ssmtp/ssmtp.conf

cat << EOF > .muttrc
set sendmail="/usr/sbin/ssmtp"
set realname="My Raspberry Pi"
set from="MAIL@gmail.com"
EOF

cat .muttrc|sed s/MAIL/${MAIL}/g > .muttrc~
mv ~/.muttrc ~/.muttrc~ >& /dev/null
mv .muttrc~ ~/.muttrc
echo
echo "セットアップが完了しました"
