#!/bin/sh

lrz_sh="/opt/ejoin_fs/script/lrz_load.sh"
[ ! -x "$lrz_sh" ] && "$lrz_sh: command not found" && exit

$lrz_sh "vad_bin.tgz" "bin"
