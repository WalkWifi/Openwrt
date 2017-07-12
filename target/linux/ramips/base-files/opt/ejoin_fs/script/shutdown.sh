#!/bin/sh
# add shutdown commands in this script

echo "Run the script shutdown.sh ..."

# kill all uuwifi process
pids=$(pidof vdc vds vwa vfd)
for pid in $pids
do
  echo "kill -9 $Pid" >> /opt/ejoin_fs/message
  kill -9 $Pid
done

# copy the flash mmap file if exists
flash_mmap="/opt/ejoin/var/flash_mmap"
[ -f "$flash_mmap" ] && {
  /bin/cp -f $flash_mmap /opt/ejoin_fs/

  # to ensure the file is saved OK
  sleep 1
}
