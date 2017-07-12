#!/bin/sh
echo "<<<<< background.sh >>>>>"
[ ! -d /tmp/ejoin ] && mkdir -p /tmp/ejoin
[ ! -L /opt/ejoin ] && ln -s /tmp/ejoin /opt/ejoin
[ ! -d /opt/ejoin/bin ] && mkdir -p /opt/ejoin/bin

#if it needs to update software.
if [ -f /opt/ejoin_fs/vad.tgz ];then
	tar -xzvf /opt/ejoin_fs/vad.tgz -C /
	rm /opt/ejoin_fs/vad.tgz
	reboot
	echo "update the system ok, restart..."
else
	echo "The system is the newest, run normally"
fi

#button for power off
#if [ -f /opt/ejoin_fs/button.tgz ];then
#	tar -xzvf /opt/ejoin_fs/button.tgz -C /opt/ejoin/bin
#	/opt/ejoin/bin/button &
#else
#	echo "the file named 'button.tgz' is lost"
#fi

#update app
sleep 15

if [ -f /opt/ejoin_fs/vfd.tgz ];then
	tar -xzvf /opt/ejoin_fs/vfd.tgz -C /opt/ejoin/bin 1>/dev/null
	/opt/ejoin/bin/vfd -d -L 3
else
	echo "the file named 'vfd.tgz' is lost"
fi
