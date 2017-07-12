#!/bin/sh

[ -z "$4" ] && echo "usage: $0 host[:port] rem_file loc_file filetype" && exit

timedwait_input() {
  n=$2
  while true; do
    echo -n -e "\r$1 [$((n--))] ..."
    read -r -t1 && return 0
    [ $n -eq 0 ] && echo -e "\r$1 [0]     " && return 1
  done
}

host=${1%:*}
port=${1#*:}
[ "$port" = "$host" ] && port=

rem_file=$2
loc_file=$3
filetype=$4

echo "Begin to get remote file $rem_file ... "
tftp -g -r $rem_file -l /tmp/$rem_file $host $port

[ $? != 0 ] && echo "FAIL" && exit

echo "OK"

tmp_remfile=/tmp/$rem_file
/opt/ejoin_fs/script/verify_bin.sh $tmp_remfile $filetype

[ $? != 0 ] && echo "FAIL" && exit

if [ "$filetype" = "vad" ] ;then
  echo -n "Extract the vad file $tmp_remfile ..."
  tar xzvf $tmp_remfile -C /
else
  echo -n "Save local file $loc_file ... "
  mv $tmp_remfile $loc_file
fi

echo -e "OK\n"

timedwait_input "Reboot the system" 3
[ $? != 0 ] && echo -e "\nRebooting ...\n" && reboot
