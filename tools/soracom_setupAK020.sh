#!/bin/bash

# j3tm0t0/AK-020_setup.sh
# run as root to configure your raspbian to use AK-020 with SORACOM Air SIM 
#
# このスクリプトは下記のサイトからダウンロードしたものを修正しました。
# 2016/06/08 国野亘
# https://gist.githubusercontent.com/j3tm0t0/2a126ba2592202ba5de4498d8a56cfb2

# install required packages
apt-get install -y eject wvdial

# add wvdial config
cat << EOF > /etc/wvdial.conf
[Dialer Defaults]
Init1 = ATZ
Init2 = AT+CFUN=1
Init3 = AT+CGDCONT=1,"IP","soracom.io"
Dial Attempts = 3
Modem Type = Analog Modem
Dial Command = ATD
Stupid Mode = yes
Baud = 460800
New PPPD = yes
Modem = /dev/ttyUSB0
ISDN = 0
APN = soracom.io
Phone = *99***1#
Username = sora
Password = sora
Carrier Check = no
Auto DNS = 1
Check Def Route = 1
EOF

# add replacedefaultroute option to ppp config
cat << EOF > /etc/ppp/peers/wvdial
noauth
name wvdial
usepeerdns
replacedefaultroute
EOF

# 接続コマンドsoracomのUSB ID設定
cat soracom|sed "s/vendor=0x.*$/vendor=0x15eb product=0x7d0e/g">soracom~
mv soracom~ soracom
chmod a+rwx soracom
