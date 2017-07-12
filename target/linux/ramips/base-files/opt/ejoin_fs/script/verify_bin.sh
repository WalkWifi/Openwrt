#!/bin/sh

[ -z "$2" ] && echo "usage: $0 file type" && exit 1

file=$1
type=$2

vfd="/opt/ejoin/bin/vfd"

[ ! -x "$vfd" ] && {
  vfd_tgz="/opt/ejoin_fs/vfd.tgz"
  [ ! -f "$vfd_tgz" ] && echo "$vfd_tgz: no such file" && exit 1

  vfd="/tmp/vfd"
  tar xzvf $vfd_tgz -C /tmp 1>/dev/null
  [ ! -x "$vfd" ] && echo "$vfd: command not found" && exit 1
}

$vfd -t $type -V $file
