#!/bin/bash

# FTPサーバをセットアップします。
#
# FTPではユーザ名やパスワード、データーが平文で転送されます。
# インターネット上で扱う場合は、セキュリティに対する配慮が必要です。
# 2016/12/14 国野亘

echo "vsftpdをインストールします"
sudo apt-get install vsftpd
sudo service vsftpd stop
cat /etc/vsftpd.conf|sed -e "s/#write_enable=YES/write_enable=YES/g" > vsftpd.tmp
sudo mv -b vsftpd.tmp /etc/vsftpd.conf
sudo chown root:root /etc/vsftpd.conf
sudo service vsftpd start
