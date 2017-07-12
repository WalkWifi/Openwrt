#!/bin/sh

[ -z "$1" ] && echo "usage: $0 host [rem_file]" && exit

host=$1
bin_name=${2:-vad_bin.tgz}
loc_file="/opt/ejoin_fs/vad_bin.tgz"

tftp_sh="/opt/ejoin_fs/script/tftp_load.sh"
[ ! -x "$tftp_sh" ] && "$tftp_sh: command not found" && exit

$tftp_sh $host $bin_name $loc_file "bin"
