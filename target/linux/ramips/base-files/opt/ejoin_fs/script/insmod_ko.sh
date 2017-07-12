#!/bin/sh
echo "<<<<< insmod_ko.sh >>>>>"

#insmod /opt/ejoin_fs/driver/drv_startup.ko
insmod /opt/ejoin_fs/driver/drv_regopt.ko
#insmod /opt/ejoin_fs/driver/drv_lcd.ko
/opt/ejoin_fs/bin/gpio set 39 0
/opt/ejoin_fs/bin/gpio set 15 0


g2status=$(/opt/ejoin_fs/bin/gpio get 17)
if [ $g2status -eq '0' ]; then
   echo 'power on 2g'
   /opt/ejoin_fs/bin/gpio set 14 1
   /opt/ejoin_fs/bin/usleep 700000
   /opt/ejoin_fs/bin/gpio set 14 0
else
   echo 'quit data mode'
   #gcom -s /etc/gcom/quitdatamode.gcom -d /dev/ttyS0
fi

g4status=$(/opt/ejoin_fs/bin/gpio get 22)
if [ $g4status -eq '0' ]; then
    echo 'power on 4g'
    /opt/ejoin_fs/bin/gpio set 29 1
    /opt/ejoin_fs/bin/usleep 100000
    /opt/ejoin_fs/bin/gpio set 29 0
fi
/opt/ejoin_fs/bin/gpio set 18 1

sleep 10

gcom -s /etc/gcom/setvbatt.gcom  -d /dev/ttyS0
gcom -s /etc/gcom/setcfun.gcom  -d /dev/ttyUSB2
