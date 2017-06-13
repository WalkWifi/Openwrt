#!/bin/sh
echo "<<<<< insmod_ko.sh >>>>>"

insmod /opt/ejoin_fs/driver/drv_startup.ko
insmod /opt/ejoin_fs/driver/drv_regopt.ko
insmod /opt/ejoin_fs/driver/drv_lcd.ko

