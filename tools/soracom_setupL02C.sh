#!/bin/bash

# docomo L02C用 セットアップ
# 2016/06/08 国野亘
#
# このスクリプトは下記のサイトの情報を参考にして作成しました。
# https://gist.githubusercontent.com/j3tm0t0/2a126ba2592202ba5de4498d8a56cfb2
# http://www.okahiro.info/gd/2016/03/18/post-1249/

# install required packages
apt-get install -y eject wvdial

# add wvdial config
cat << EOF > /etc/wvdial.conf
[Dialer Defaults]
Init1 = ATZ
Init2 = ATH
Init3 = ATQ0 V1 E1 S0=0 &C1 &D2 +FCLASS=0
Dial Attempts = 3
Modem Type = Analog Modem
Dial Command = ATD
Stupid Mode = yes
Baud = 460800
New PPPD = yes
Modem = /dev/ttyUSB2
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
cat soracom|sed "s/vendor=0x.*$/vendor=0x1004 product=0x618f/g">soracom~
mv soracom~ soracom
chmod a+rwx soracom
