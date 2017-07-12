#! /bin/sh

/opt/ejoin_fs/bin/gsmmuxd -p /dev/ttyS0 -b 115200 -s /dev/mux -w /dev/ptmx /dev/ptmx &
