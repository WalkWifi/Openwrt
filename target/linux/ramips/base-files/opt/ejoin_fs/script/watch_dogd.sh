#!/bin/sh

wdg_name=$0

wdg_usage() {
  echo -e "\n$wdg_name: [-h|--help] [-d delay_time] [-i interval] p1:reboot1.sh [p2:reboot2.sh ...]"
  echo -e "\nNote: there is no any space between option name and value!!!\n"
  exit 0
}

wdg_logger() {
  logger -s -t vwdgd -p daemon.warn "$*"
}

# get the arguments
wdg_getopts() {
  for arg in "$@"
  do
    [ "${arg:0:1}" = "-" ] && {
      if [ "${arg:0:2}" = "-d" ] ;then
        delay_time=${arg:2}
      elif [ "${arg:0:2}" = "-i" ] ;then
        interval=${arg:2}
      elif [ "$arg" = "-h" -o "$arg" = "--help" ] ;then
        wdg_usage
      fi
    }
  done

  delay_time=${delay_time:-10}
  interval=${interval:-10}
}

wdg_chkproc() {
  local pname=$1
  local preboot_sh=$2

  [ -z "$(pidof $pname)" ] && {
    wdg_logger "$pname is stopped"

    if [ -x "${preboot_sh%% *}" ] ;then
      wdg_logger "run reboot script: $preboot_sh"
      $preboot_sh
    else
      wdg_logger "$preboot_sh: command not found"
    fi
  }
}

wdg_main() 
{
  echo "Run watchdog $wdg_name with interval ${interval}s after ${delay_time}s ..."
  sleep $delay_time

  while :
  do
    for proc in "$@"
    do
      [ -n "$proc" -a "${proc:0:1}" != "-" ] && wdg_chkproc "${proc%%:*}" "${proc#*:}"
    done

    sleep $interval
  done
}

echo "$*"

# get the options
wdg_getopts "$@"

# start the main function in daemon
wdg_main "$@"
