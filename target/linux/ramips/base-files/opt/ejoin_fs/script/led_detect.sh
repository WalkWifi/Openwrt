#!/bin/sh

BATFULL=4200
BATLOW=3660
BATDOWN=3500

/opt/ejoin_fs/bin/gpio set 41 0
while :
do

  wanip=`ubus call network.interface.wan status | grep \"address\" | grep -oE '[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}'`

  if [ "$wanip" = "" ]; then
    led_status=`/opt/ejoin_fs/bin/gpio get 39`
    
    if [ $led_status -eq '1' ]; then
      /opt/ejoin_fs/bin/gpio set 39 0
      /opt/ejoin_fs/bin/gpio set 15 0
    else
      /opt/ejoin_fs/bin/gpio set 39 1
      /opt/ejoin_fs/bin/gpio set 15 1
    fi

  else
    /opt/ejoin_fs/bin/gpio set 39 1
    /opt/ejoin_fs/bin/gpio set 15 1
  fi

  usb_status=`/opt/ejoin_fs/bin/gpio get 25`
  batvol=`gcom -s /etc/gcom/getvol.gcom  -d /dev/ttyUSB2 | awk '{print$2}' | awk -F "," '{print$3}'`

  echo $batvol > /tmp/batvol

  if [ $batvol -le $BATDOWN ]; then
    echo "Low power,Shut down UUWIFI."
    echo "poweroff 2g."
    /opt/ejoin_fs/bin/gpio set 14 1
    sleep 1
    /opt/ejoin_fs/bin/gpio set 14 0
    echo "poweroff 4g."
    /opt/ejoin_fs/bin/gpio set 29 1
    sleep 1
    /opt/ejoin_fs/bin/gpio set 29 0
    #gcom -s /etc/gcom/poweroff2g.gcom  -d /dev/ttyS0   
    #gcom -s /etc/gcom/poweroff4g.gcom  -d /dev/ttyUSB2      
    devmem 0x10000038 32 0xC0030300
    poweroff
    /opt/ejoin_fs/bin/gpio set 11 0      
  fi

  resetkey_status=`/opt/ejoin_fs/bin/gpio get 43`
  if [ $resetkey_status -eq '0' ]; then
    /opt/ejoin_fs/bin/usleep 100000
    resetkey_status=`/opt/ejoin_fs/bin/gpio get 43`
	
    if [ $resetkey_status -eq '0' ]; then
        /opt/ejoin_fs/bin/gpio set 41 1 
        /opt/ejoin_fs/bin/usleep 200000   
        /opt/ejoin_fs/bin/gpio set 41 0 
        /opt/ejoin_fs/bin/usleep 200000   
        /opt/ejoin_fs/bin/gpio set 41 1 
        /opt/ejoin_fs/bin/usleep 200000   
        /opt/ejoin_fs/bin/gpio set 41 0 
        /opt/ejoin_fs/bin/usleep 200000   
        /opt/ejoin_fs/bin/gpio set 41 1
	    
        uci set wireless.@wifi-iface[0].encryption=psk2
        uci set wireless.@wifi-iface[0].key=12345678
        uci set wireless.@wifi-iface[0].hidden=0
        uci commit
        wifi down && wifi up
    fi
  else
    if [ $usb_status -eq '0' ]; then
      if [ $batvol -ge $BATFULL ]; then
        /opt/ejoin_fs/bin/gpio set 41 1    
      else
        powerled_status=`/opt/ejoin_fs/bin/gpio get 41`
      
        if [ $powerled_status -eq '1' ]; then
          /opt/ejoin_fs/bin/gpio set 41 0
        else
          /opt/ejoin_fs/bin/gpio set 41 1
        fi
      fi
  
    else
      if [ $batvol -le $BATLOW ]; then
        /opt/ejoin_fs/bin/gpio set 41 1 
        /opt/ejoin_fs/bin/usleep 200000   
        /opt/ejoin_fs/bin/gpio set 41 0 
        /opt/ejoin_fs/bin/usleep 200000   
        /opt/ejoin_fs/bin/gpio set 41 1 
        /opt/ejoin_fs/bin/usleep 200000   
        /opt/ejoin_fs/bin/gpio set 41 0 
        /opt/ejoin_fs/bin/usleep 200000   
      else
        /opt/ejoin_fs/bin/gpio set 41 1
      fi
    fi
  fi
  sleep 1
done
