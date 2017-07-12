#!/bin/sh

hardware_test_status=0

did_get() {

  name=`echo "$1" | awk -F "-" '{print$1}'`
  date=`echo "$1" | awk -F "-" '{print$2}'`
  id=`echo "$1" | awk -F "-" '{print$3}'`

  date_len=${#date}
  if [ $date_len -eq 3 ]; then
    day=`echo "$date" | cut -b 2-3`
    date=`echo "$date" | cut -b 1`
  else
    echo "usage: date lenth must be 3" && echo "format:x-xxx-xxxxxxxx" && exit
  fi

  id_len=${#id}
  if [ $id_len -eq 8 ]; then
    id_6=`echo "$id" | cut -b 3-8`
  else
    echo "usage: id lenth must be 8" && echo "format:x-xxx-xxxxxxxx" && exit
  fi

  #---name get---#
  name_len=${#name}
  if [ $name_len -eq 6 ]; then
    name=$name
  else
    echo "usage: name lenth must be 6" && echo "format:x-xxx-xxxxxxxx" && exit
  fi

  #---date get---#
  if [ "$date" = 'A' ]; then
    date=201705
  elif [ "$date" = 'B' ]; then
    date=201706
  elif [ "$date" = 'C' ]; then
    date=201707
  elif [ "$date" = 'D' ]; then
    date=201708
  elif [ "$date" = 'E' ]; then
    date=201709
  elif [ "$date" = 'F' ]; then
    date=201710
  elif [ "$date" = 'G' ]; then
    date=201711
  elif [ "$date" = 'H' ]; then
    date=201712
  elif [ "$date" = 'I' ]; then
    date=201801
  elif [ "$date" = 'J' ]; then
    date=201802
  elif [ "$date" = 'K' ]; then
    date=201803
  elif [ "$date" = 'L' ]; then
    date=201804
  elif [ "$date" = 'M' ]; then
    date=201805
  elif [ "$date" = 'N' ]; then
    date=201806
  elif [ "$date" = 'O' ]; then
    date=201807
  elif [ "$date" = 'P' ]; then
    date=201808
  elif [ "$date" = 'Q' ]; then
    date=201809
  elif [ "$date" = 'R' ]; then
    date=201810
  elif [ "$date" = 'S' ]; then
    date=201811
  elif [ "$date" = 'T' ]; then
    date=201812
  elif [ "$date" = 'U' ]; then
    date=201901
  elif [ "$date" = 'V' ]; then
    date=201902
  elif [ "$date" = 'W' ]; then
    date=201903
  elif [ "$date" = 'X' ]; then
    date=201904
  elif [ "$date" = 'Y' ]; then
    date=201905
  elif [ "$date" = 'Z' ]; then
    date=201906
  else
	echo "usage: date must be [A-Z]" && echo "format:x-xxx-xxxxxxxx" && exit
  fi

  if [ $hardware_test_status -eq '1' ]; then
    echo "name:$name"
    echo "date:$date"
    echo "day:$day"
    echo "id:$id"
    echo "id_6:$id_6"

    #---did write---#
    echo -e "QWERasdf" > /opt/ejoin_fs/etc/did
    echo -e "WIFI:000C43"$id_6"" >> /opt/ejoin_fs/etc/did
    echo -e "WAN:000C"$id"" >> /opt/ejoin_fs/etc/did
    echo -e "LAN:000C43"$id_6"" >> /opt/ejoin_fs/etc/did
    echo -e "DID:"$name""$date""$day"00"$id"00000000" >> /opt/ejoin_fs/etc/did
    echo "write did OK"
  fi
}

hardware_test() {

ps |grep -w 'led_detect.sh' | grep -v grep | cut -c 2-5 | xargs kill -9
ps |grep -w 'key_detect.sh' | grep -v grep | cut -c 2-5 | xargs kill -9

test_status=1
 
pin_4G=`/opt/ejoin_fs/bin/gpio get 42`  
 
if [ $pin_4G -eq '1' ]; then  
  sleep 1  
  pin_4G=`/opt/ejoin_fs/bin/gpio get 42`  
	   
  if [ $pin_4G -eq '1' ]; then  
    echo "1:4G SIM DETECT OK"  
  fi  
else  
  sleep 1  
  pin_4G=`/opt/ejoin_fs/bin/gpio get 42`  
					   
  if [ $pin_4G -eq '0' ]; then  
    echo "1:4G SIM DETECT NG"  
    test_status=0
  fi  
fi  
								 
cpin_2G=`gcom -s /etc/gcom/getcpin.gcom  -d /dev/ttyS0`  
								 
if [ $cpin_2G = '1' ]; then
  sleep 1  
  cpin_2G=`gcom -s /etc/gcom/getcpin.gcom  -d /dev/ttyS0`  
								 
  if [ $cpin_2G = '1' ]; then
    echo "2:2G SIM READY OK"
  fi
else
  sleep 1  
  cpin_2G=`gcom -s /etc/gcom/getcpin.gcom  -d /dev/ttyS0`  
								 
  if [ $cpin_2G = '1' ]; then
    echo "2:2G SIM READY OK"
  else
    echo "2:2G SIM READY NG"
    test_status=0
  fi
fi
												   
cpin_4G=`gcom -s /etc/gcom/getcpin.gcom  -d /dev/ttyUSB2`
												 
if [ $cpin_4G = '1' ]; then
  sleep 1  
  cpin_4G=`gcom -s /etc/gcom/getcpin.gcom  -d /dev/ttyUSB2`
												 
  if [ $cpin_4G = '1' ]; then
    echo "3:4G SIM READY OK"
  fi
else
  sleep 1  
  cpin_4G=`gcom -s /etc/gcom/getcpin.gcom  -d /dev/ttyUSB2`
												 
  if [ $cpin_4G = '1' ]; then
    echo "3:4G SIM READY OK"
  else
    echo "3:4G SIM READY NG"
    test_status=0
  fi
fi

while :
do
  pwr_key=`/opt/ejoin_fs/bin/gpio get 24`

  if [ $pwr_key -eq '0' ]; then
    /opt/ejoin_fs/bin/usleep 500000
    pwr_key=`/opt/ejoin_fs/bin/gpio get 24`

    if [ $pwr_key -eq '0' ]; then
      echo "4:PWR KEY OK"
    fi
  fi

  reset_key=`/opt/ejoin_fs/bin/gpio get 43`

  if [ $reset_key -eq '0' ]; then
    /opt/ejoin_fs/bin/usleep 500000
    reset_key=`/opt/ejoin_fs/bin/gpio get 43`

    if [ $reset_key -eq '0' ]; then
      if [ $test_status -eq '1' ]; then
        echo "5:RESET KEY OK"
        cat /opt/ejoin_fs/script/factory_ok
        echo 1 > /opt/ejoin_fs/script/factory_result
        hardware_test_status=1
      else
        echo "5:RESET KEY OK"
        cat /opt/ejoin_fs/script/factory_ng
        hardware_test_status=0
      fi
      break
    fi
  fi

  /opt/ejoin_fs/bin/usleep 500000
 
done
}                                            
                                                          
if [ -z "$1" ]; then                                              
  hardware_test                                                   
else
  did_get $1    
  hardware_test
  did_get $1    
fi

