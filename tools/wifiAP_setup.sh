#!/bin/bash
echo "Raspberry Piを無線LANアクセスポイントにします。"
echo " - 無線LAN内の機器が SORACOM Air 経由でネット接続。"
echo " - 通常のネットワークに接続できなくなります。"
echo " - 設定を自動的に戻すことは出来ません。"
echo "yesを入力するとインストールと設定変更を実行します。"
echo -n "yes/no >"
read yes
if [ $yes != "yes" ]; then
echo "ダウンロードをキャンセルしました。"
exit
fi
echo "インストールを開始します。"
apt-get install hostapd
apt-get install isc-dhcp-server

echo "設定：/etc/network/interfaces"
cp -b /etc/network/interfaces /etc/network/interfaces~
cat << EOF > /etc/network/interfaces
# Please note that this file is written to be used with dhcpcd.
# For static IP, consult /etc/dhcpcd.conf and 'man dhcpcd.conf'.

auto lo
iface lo inet loopback

auto wlan0
allow-hotplug wlan0
iface wlan0 inet static
address 192.168.0.1
netmask 255.255.255.0
EOF

echo "設定：/etc/default/hostapd"
cp -b /etc/default/hostapd /etc/default/hostapd~
echo DAEMON_CONF=\"/etc/hostapd/hostapd.conf\" >> /etc/default/hostapd

echo "設定：/etc/dhcp/dhcpd.conf"
cp -b /etc/dhcp/dhcpd.conf /etc/dhcp/dhcpd.conf~
cat << EOF > /etc/dhcp/dhcpd.conf
#
# Sample configuration file for ISC dhcpd for Debian
#
#

# The ddns-updates-style parameter controls whether or not the server will
# attempt to do a DNS update when a lease is confirmed. We default to the
# behavior of the version 2 packages ('none', since DHCP v2 didn't
# have support for DDNS.)
ddns-update-style none;

# option definitions common to all supported networks...
#option domain-name "example.org";
#option domain-name-servers ns1.example.org, ns2.example.org;

default-lease-time 600;
max-lease-time 7200;

# If this DHCP server is the official DHCP server for the local
# network, the authoritative directive should be uncommented.
authoritative;

# Use this to send dhcp log messages to a different log file (you also
# have to hack syslog.conf to complete the redirection).
log-facility local7;

subnet 192.168.0.0 netmask 255.255.255.0 {
        range 192.168.0.2 192.168.0.254;
        option broadcast-address 192.168.0.255;
        option routers 192.168.0.1;
        option domain-name "local";
        option domain-name-servers 8.8.8.8,8.8.4.4;
}
EOF

echo "設定：/etc/default/isc-dhcp-server"
cp -b /etc/default/isc-dhcp-server /etc/default/isc-dhcp-server~
echo INTERFACES=\"wlan0\" >> /etc/default/isc-dhcp-server

echo "設定：/etc/rc.local"
cp -b /etc/rc.local /etc/rc.local~
cat << EOF > /etc/rc.local
#!/bin/sh -e
#
# rc.local
#
# This script is executed at the end of each multiuser runlevel.
# Make sure that the script will "exit 0" on success or any other
# value on error.
#
# In order to enable or disable this script just change the execution
# bits.
#
# By default this script does nothing.

# Print the IP address
_IP=$(hostname -I) || true
if [ "$_IP" ]; then
  printf "My IP address is %s\n" "$_IP"
fi

date >/home/pi/start.log
service networking restart >>/home/pi/start.log 2>&1
service isc-dhcp-server restart >>/home/pi/start.log 2>&1
service hostapd restart >>/home/pi/start.log 2>&1
/home/pi/esp/tools/soracom start >>/home/pi/start.log 2>&1
sh -c "echo 1 > /proc/sys/net/ipv4/ip_forward" >>/home/pi/start.log 2>&1
iptables -t nat -A POSTROUTING -o ppp0 -j MASQUERADE >>/home/pi/start.log 2>&1
iptables -A FORWARD -i ppp0 -o wlan0 -m state --state RELATED,ESTABLISHED -j ACCEPT >>/home/pi/start.log 2>&1
iptables -A FORWARD -i wlan0 -o ppp0 -j ACCEPT >>/home/pi/start.log 2>&1

exit 0
EOF
chmod a+x /etc/rc.local

echo "引き続いて soracom AK020 をセットアップします。"
echo "yesを入力するとインストールと設定変更を実行します。"
echo -n "yes/no >"
read yes
if [ $yes != "yes" ]; then
echo "ダウンロードをキャンセルしました。"
exit
fi
./soracom_setupAK020.sh
echo "セットアップを完了しました"
echo "sudo rebootで再起動すると自動実行します。"
exit
