#!/bin/sh

mnt_vad=/mnt/vad

[ -d "$mnt_vad" ] && umount $mnt_vad 1>/dev/null 2>&1 || mkdir -p $mnt_vad

nfs_dir=$1
nfs_dir=${nfs_dir:-192.168.1.99:/opt/ejoin/vifi/vad}

while : ; do
  echo -e "\nmount $nfs_dir to $mnt_vad ...\n"
  mount -t nfs -o nolock $nfs_dir $mnt_vad
  [ "$?" = 0 ] && echo -e "OK\n" && break

  echo -e "failed to mount $nfs_dir!\n"

  read -n 1 -t 3 -p "retry after 3s! [Y/N] " yn
  echo -e "\n"

  [ "$yn" = "n" -o "$yn" = "N" ] && exit
done

cd $mnt_vad

tar cvf vad.tar lib/lib*so* bin/vdc bin/vdg bin/vds
tar xvf vad.tar -C /tmp/ejoin/

cd /tmp
umount $mnt_vad
rm -rf $mnt_vad
