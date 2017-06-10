#!/bin/bash

# backup_log.shを自動起動するように設定します。
# sudo を付けて実行してください。
#
# sudo ./backup_log_setup.sh
#
# 2017/06/10 国野亘

echo "-------------------------------------------------------------------------"
echo "backup_log.shを1日に1度、自動実行するように設定しますか？"
echo "「yes」を入力してください。"
echo -n "yes/no >"
read yes
if [ "$yes" != "yes" ]; then
	echo "キャンセルしました"
	exit
fi

cat << EOF > /etc/cron.daily/backup_log
#!/bin/bash
cd `pwd`
./backup_log.sh
EOF
chmod a+x /etc/cron.daily/backup_log
sudo service cron restart
