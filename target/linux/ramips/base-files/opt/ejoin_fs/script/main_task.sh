#!/bin/sh

echo "<<<<< main task >>>>>"

export HOME=$(grep -e "^${USER:-root}:" /etc/passwd | cut -d ":" -f 6)
export LD_LIBRARY_PATH=/tmp/ejoin/lib:$LD_LIBRARY_PATH

/opt/ejoin_fs/script/insmod_ko.sh

/opt/ejoin_fs/script/background.sh &

confdir="/etc/config"

#creat the backup of the network file.
if [ ! -f "$confdir/network.bak" ]; then
    cp "$confdir/network" "$confdir/network.bak"
#else
#    cp "$confdir/network.bak" "$confdir/network"
fi
echo ">>>>> creat network.bak done."

#creat file system of the ejoin.
[ ! -d /tmp/ejoin ] && mkdir -p /tmp/ejoin
[ ! -L /opt/ejoin ] && ln -s /tmp/ejoin /opt/ejoin
mkdir /opt/ejoin/lib
mkdir /opt/ejoin/bin
mkdir /opt/ejoin/log
mkdir /opt/ejoin/var
echo ">>>>> creat link done./opt/ejoin-->/tmp/ejoin"

#creat the link of "ejoin_fs/etc".
[ ! -L /opt/ejoin/etc ] && ln -s /opt/ejoin_fs/etc /opt/ejoin/etc

#creat log direction.
[ ! -d /opt/ejoin_fs/log/gdb] && mkdir -p /opt/ejoin_fs/log/gdb
if [ ! -L /opt/ejoin/log/gdb ]; then
  ln -s /opt/ejoin_fs/log/gdb /opt/ejoin/log/gdb
fi
echo ">>>>> creat link done./opt/ejoin/log/gdb-->/opt/ejoin_fs/log/gdb."

# copy the flash mmap file to /opt/ejoin/var
flash_mmap="/opt/ejoin_fs/flash_mmap"
[ -f "$flash_mmap" ] && /bin/cp -f $flash_mmap /opt/ejoin/var/

# sleep 15

#copy the rootlib to system direction.
if [ ! -f "/opt/ejoin_fs/rootlib_ok" ]; then
    tar -xzvf /opt/ejoin_fs/rootlib.tgz  -C /
    touch /opt/ejoin_fs/rootlib_ok
    echo ">>>>> copy /opt/ejoin_fs/rootlib to / done."
else
    echo ">>>>> root direction is the newest."
fi

#uncompress the application to the ram.
tar -xzvf /opt/ejoin_fs/vad_bin.tgz -C /opt/ejoin/
#create the bridge field
brctl addif br-lan ra0

#sleep 1
#option the route.
#route del default
#route add default gw 192.168.1.1
#echo -e  "nameserver 8.8.8.8 \nnameserver 114.114.114.114 \n" > /var/resolv.conf.auto
#echo -e  "nameserver 8.8.8.8 \nnameserver 114.114.114.114 \n" > /etc/resolv.conf 
#/opt/ejoin/bin/vds -v
gcom -s /etc/gcom/quitdatamode.gcom  -d /dev/ttyS0

/opt/ejoin_fs/script/task_vdc.sh &
sleep 1

localcard_status=`/opt/ejoin_fs/bin/gpio get 42`                                 
factory_status=`cat /opt/ejoin_fs/script/factory_result`

if [ $localcard_status -eq '1' ]; then
  sleep 1  
  localcard_status=`/opt/ejoin_fs/bin/gpio get 42`

  if [ $localcard_status -eq '1' ]; then
    if [ $factory_status -eq '1' ]; then
      if [ -e /opt/ejoin_fs/script/factory_devtest.sh ]; then
        /opt/ejoin_fs/script/factory_devtest.sh
      else
        echo "localcard start."
        /opt/ejoin_fs/script/localcard_task.sh
      fi
    else
      echo "localcard start."
      /opt/ejoin_fs/script/localcard_task.sh
    fi  
  else
    echo "remotecard start."
    sleep 3
    /opt/ejoin_fs/script/task_vwa.sh &
  fi
else         
  echo "remotecard start."
  sleep 3
  /opt/ejoin_fs/script/task_vwa.sh &

:<<BLOCK
while true; do
    pppd_pid=`ps | grep pppd | grep -v grep`
    if [ -z "$pppd_pid" ] ;then
        pppd call ppp-net
        echo "pppd is dialing ..."
    fi

    sleep 5
    pppdump=`ifconfig ppp0 2>/dev/null`

    if [ -z "$pppdump" ]; then
        echo "ppp0 is disconnected"
    else
        ifconfig ppp0
    echo "ppp0 is connected"
        break;
    fi
done
iptables -I INPUT -p udp --dport 8090 -j ACCEPT
BLOCK

fi

:<<BLOCK
while : 
do
  sleep 10
done
BLOCK
