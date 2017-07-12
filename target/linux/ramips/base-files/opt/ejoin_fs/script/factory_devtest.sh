#!/bin/sh

echo "factory 2g 4g test"                     
                                                       
batvol=`gcom -s /etc/gcom/getvol.gcom  -d /dev/ttyUSB2 | grep CBC | awk '{print$2}' | awk -F "," '{print$3}'`
csq_2g=`gcom -s /etc/gcom/getcsq.gcom  -d /dev/ttyS0 | grep CSQ | awk '{print$2}' | awk -F "," '{print$1}'`
csq_4g=`gcom -s /etc/gcom/getcsq.gcom  -d /dev/ttyUSB2 | grep CSQ | awk '{print$2}' | awk -F "," '{print$1}'`

i=0
sleep 2

cpin_4g=`gcom -s /etc/gcom/getcpin.gcom  -d /dev/ttyUSB2`
if [ $cpin_4g = '1' ]; then
  sleep 1  
  cpin_4g=`gcom -s /etc/gcom/getcpin.gcom  -d /dev/ttyUSB2`

  if [ $cpin_4g = '1' ]; then 
    echo "4g is READY."
    echo "4g dialing..."
    uci set network.wan.ifname=ppp1
    uci set network.wan.proto=3g
    uci set network.wan.apn=3gnet
    uci set network.wan.auto=1
    uci set network.wan.device=/dev/ttyUSB3
    uci commit
    ifup wan
   #pppd call sim1_pppdial                                                       
  else
    echo "4g is not READY."     
  fi
else
  sleep 1  
  cpin_4g=`gcom -s /etc/gcom/getcpin.gcom  -d /dev/ttyUSB2`
								 
  if [ $cpin_4g = '1' ]; then
    echo "4g is READY."
    echo "4g dialing..."
    uci set network.wan.ifname=ppp1
    uci set network.wan.proto=3g
    uci set network.wan.apn=3gnet
    uci set network.wan.auto=1
    uci set network.wan.device=/dev/ttyUSB3
    uci commit
    ifup wan
   #pppd call sim1_pppdial                                                       
  else
    echo "4g is not READY."     
  fi
fi                                

cpin_2g=`gcom -s /etc/gcom/getcpin.gcom  -d /dev/ttyS0`  
										                                                         
if [ $cpin_2g = '1' ]; then                              
  sleep 1  
  cpin_2g=`gcom -s /etc/gcom/getcpin.gcom  -d /dev/ttyS0`  
  if [ $cpin_2g = '1' ]; then 
    echo "2g is READY."                                    
    echo "2g dialing..."
    pppd call sim0_pppdial
  else
    echo "2g is not READY."     
  fi
else
  sleep 1  
  cpin_2g=`gcom -s /etc/gcom/getcpin.gcom  -d /dev/ttyS0`  
								 
  if [ $cpin_2g = '1' ]; then
    echo "2g is READY."                                    
    echo "2g dialing..."
    pppd call sim0_pppdial
  else
    echo "2g is not READY."     
  fi
fi
                                                                                     
while :                                
do                                     
	                                                     
  dial_2g=`ifconfig | grep ppp`                          
  if [ "$dial_2g" = "" ]; then                           
    dial_2g=ng                                           
  else                                                   
    dial_2g=ok                                           
  fi                                   
									                                        
  dial_4g=`ifconfig | grep 3g-wan`     
  if [ "$dial_4g" = "" ]; then                           
    dial_4g=ng                                           
  else                                                   
    dial_4g=ok                                           
  fi                                                     
								                                                         
  if [ "$dial_2g" == "ok" -a "$dial_4g" == "ok" ]; then         
    /opt/ejoin_fs/bin/gpio set 39 1                    
    /opt/ejoin_fs/bin/gpio set 15 1
    echo 0 > /opt/ejoin_fs/script/factory_result                                                 
    batvol=`gcom -s /etc/gcom/getvol.gcom  -d /dev/ttyUSB2 | grep CBC | awk '{print$2}' | awk -F "," '{print$3}'`
    csq_4g=`gcom -s /etc/gcom/getcsq.gcom  -d /dev/ttyUSB2 | grep CSQ | awk '{print$2}' | awk -F "," '{print$1}'`
    did=`cat opt/ejoin_fs/etc/did | grep DID | awk -F ":" '{print$2}' | cut -b 1-24`
    hdv=`cat opt/ejoin_fs/etc/did | grep DID | awk -F ":" '{print$2}' | cut -b 3-6`
    result="ok"
    echo "did=$did"                                                                                   
    echo "s2g=$csq_2g"                                                              
    echo "s4g=$csq_4g"                                                                        
    echo "vol=$batvol"                                                    
    echo "hdv=$hdv"                                                    
	
    wget -q -t 3 -T 10 "http://vrs.eoutech.net:8443/UUWiFiWS/devtest?did=$did&code=0&result=$result&s2g=$csq_2g&s4g=$csq_4g&swifi=4&voltage=$batvol&hdv=$hdv&fwv=1.0.0&soft=2.3.9"
    [ $? = 0 ] && echo "OK" || echo "FAIL"
    mv /opt/ejoin_fs/vad_bin.tgz.bak /opt/ejoin_fs/vad_bin.tgz    
    /opt/ejoin_fs/script/key_detect.sh &
    rm /opt/ejoin_fs/script/factory_devtest.sh
    mv /devtest* /opt/ejoin/log/
    break
  fi 

  led_status=`/opt/ejoin_fs/bin/gpio get 39`
  if [ $led_status -eq '1' ]; then
    /opt/ejoin_fs/bin/gpio set 39 0
    /opt/ejoin_fs/bin/gpio set 15 0
  else
    /opt/ejoin_fs/bin/gpio set 39 1
    /opt/ejoin_fs/bin/gpio set 15 1
  fi

  echo "$i"
  let i+=1;
  if [ $i -eq '20' ];then
    batvol=`gcom -s /etc/gcom/getvol.gcom  -d /dev/ttyUSB2 | grep CBC | awk '{print$2}' | awk -F "," '{print$3}'`
    csq_4g=`gcom -s /etc/gcom/getcsq.gcom  -d /dev/ttyUSB2 | grep CSQ | awk '{print$2}' | awk -F "," '{print$1}'`
    did=`cat opt/ejoin_fs/etc/did | grep DID | awk -F ":" '{print$2}' | cut -b 1-24`
    hdv=`cat opt/ejoin_fs/etc/did | grep DID | awk -F ":" '{print$2}' | cut -b 3-6`
    result="timeout"
    echo "did=$did"                                                                                   
    echo "s2g=$csq_2g"                                                              
    echo "s4g=$csq_4g"                                                                        
    echo "vol=$batvol"                                                    
    echo "hdv=$hdv"                                                    

    wget -q -t 3 -T 10 "http://vrs.eoutech.net:8443/UUWiFiWS/devtest?did=$did&code=1&result=$result&s2g=$csq_2g&s4g=$csq_4g&swifi=4&voltage=$batvol&hdv=$hdv&fwv=1.0.0&soft=2.3.9"
    [ $? = 0 ] && echo "OK" || echo "FAIL"
    /opt/ejoin_fs/script/key_detect.sh &
    mv /devtest* /opt/ejoin/log/

    break
  fi
  sleep 1                                                                                       
done
