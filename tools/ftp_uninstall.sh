#!/bin/bash

# ftp_setup.shを使ってインストールしたFTPサーバ（vsftpd）を削除します。
#
# FTPではユーザ名やパスワード、データーが平文で転送されます。
# インターネット上で扱う場合は、セキュリティに対する配慮が必要です。
#
# 2018/11/17 国野亘

echo "vsftpdを削除します"
sudo service vsftpd stop
sleep 1
sudo apt-get remove vsftpd
