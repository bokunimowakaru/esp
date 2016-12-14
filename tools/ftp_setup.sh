#!/bin/bash

# FTPサーバをセットアップします。
#
# FFTPではユーザ名やパスワードが平文で、データーが平文で転送されます。
# インターネット上で扱う場合は、セキュリティに対する配慮が必要です。
# 2016/12/14 国野亘

echo "vsftpdをインストールします"
sudo apt-get install vsftpd
sudo service vsftpd start
