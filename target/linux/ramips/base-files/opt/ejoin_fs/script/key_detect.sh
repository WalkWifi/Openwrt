#!/bin/sh

while :
do
  key_status=`/opt/ejoin_fs/bin/gpio get 24`
    
  if [ $key_status -eq '0' ]; then
    sleep 2
    key_status=`/opt/ejoin_fs/bin/gpio get 24`
    
    if [ $key_status -eq '0' ]; then 
      echo "Shut down UUWIFI."
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
  fi

  localcard_status=`/opt/ejoin_fs/bin/gpio get 42`

  if [ $localcard_status -eq '0' ]; then
    sleep 1
    localcard_status=`/opt/ejoin_fs/bin/gpio get 42`
	
    if [ $localcard_status -eq '0' ]; then
      uci delete network.wan.device
      uci delete network.wan.apn
      uci delete network.wan.auto
      uci set network.wan.ifname=ppp1
      uci set network.wan.proto=static
      uci commit
      echo "reboot."
      reboot
    fi
  fi

  sleep 1
done
