#!/bin/sh

[ -z "$2" ] && echo "usage: $0 loc_file filetype"

timedwait_input() {
  n=$2
  while true; do
    echo -n -e "\r$1 [$((n--))] ..."
    read -r -t1 && return 0
    [ $n -eq 0 ] && echo -e "\r$1 [0]     " && return 1
  done
}

rem_file="vad*.tgz"
loc_file=$1
filetype=$2

cd /tmp

[ ! -x lrz ] && {
  lrzsz_tgz="/opt/ejoin_fs/bin/lrzsz.tgz"
  [ ! -f $lrzsz_tgz ] && echo "$lrzsz_tgz: no such file" && exit

  tar xzvf $lrzsz_tgz lrz
  [ ! -x lrz ] && echo "lrz: command not found" && exit
}

echo "Begin to lrz $loc_file ..."

rm -f $rem_file
./lrz
[ ! -f $rem_file ] && echo "$rem_file: no such file" && exit

/opt/ejoin_fs/script/verify_bin.sh $rem_file $filetype
[ $? != 0 ] && echo "FAIL" && exit

if [ "$filetype" = "vad" ] ;then
  echo -n "Extract the vad file $remfile ..."
  tar xzvf $remfile -C /
else
  echo -n "Saving local file $loc_file ... "
  mv $rem_file /opt/ejoin_fs/$loc_file
fi

echo -e "OK\n"

timedwait_input "Reboot the system" 3
[ $? != 0 ] && echo -e "\nRebooting ...\n" && reboot
