#!/bin/sh

echo "<<<<< localcard task >>>>>"
#set AT+QCFG="nwscanmode",0 -->Auto
gcom -s /etc/gcom/setqcfg.gcom  -d /dev/ttyUSB2

username=`cat /opt/ejoin_fs/script/apn_config | grep username | awk -F ":" '{print$2}'`
password=`cat /opt/ejoin_fs/script/apn_config | grep password | awk -F ":" '{print$2}'`
apn=`cat /opt/ejoin_fs/script/apn_config | grep apn | awk -F ":" '{print$2}'`

uci set network.wan.ifname=ppp0
uci set network.wan.proto=3g
uci set network.wan.username=$username
uci set network.wan.password=$password
uci set network.wan.apn=$apn
uci set network.wan.auto=1
uci set network.wan.device=/dev/ttyUSB3
uci commit

ifup wan

sleep 5
pin_status=`gcom -s /etc/gcom/getcpin.gcom  -d /dev/ttyUSB2`

if [ $pin_status = '1' ]; then
  echo "local card is READY."
  pppd call sim2_pppdial
else
  echo "local card is not READY."
fi

/opt/ejoin_fs/script/led_detect.sh &
/opt/ejoin_fs/script/key_detect.sh &

