#!/bin/bash
if [ -e ~/RaspberryPi/network/i.myMimamoriPi ]; then
	git pull
	echo "更新しました"
else
	if [ -e ~/RaspberryPi ]; then
		mv ~/RaspberryPi ~/RaspberryPi_`date +%Y%m%d_%H%M`
		echo "RaspberryPiフォルダのバックアップを作成しました(日時を付与)"
	fi
	cd
	git clone https://www.github.com/bokunimowakaru/RaspberryPi.git
fi
echo "ダウンロードを終了しました。"
echo "i.MyMimamori Piをセットアップするには以下のコマンドを実行してください。"
echo "cd ~/RaspberryPi/network/i.myMimamoriPi/"
echo "./setup.sh"
exit
