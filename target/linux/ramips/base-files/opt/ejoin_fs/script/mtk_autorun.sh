#!/bin/sh
# the system run the first shell,using this shell to run other user's shell file
#
# NOTE: this script MUST not return, or it will be respawned by /etc/inittab

echo "************************ < mtk_autorun.sh > *****************************"

# to avoid starting the main task repeatedly
main_task_lock="/tmp/.main_task.lock"

if ( set -o noclobber; echo "$$" > "$main_task_lock" ) 2>/dev/null; then
  main_task_sh="/opt/ejoin_fs/script/main_task.sh"
  if [ -x "$main_task_sh" ] ; then
    $main_task_sh &
  else
    echo "$main_task_sh: command not found"
  fi
else
  echo "The main task has been started before!"
fi

export LD_LIBRARY_PATH=/tmp/ejoin/lib:$LD_LIBRARY_PATH

# start watch dog daemon
watch_dogd_sh="/opt/ejoin_fs/script/watch_dogd.sh"
if [ -x "$watch_dogd_sh" ] ; then
  # define the process need to monitor by watchdog
  vdc_pars="vdc:/sbin/reboot"
  vfd_pars="vfd:/opt/ejoin/bin/vfd -d -L 3"
  vwad_pars="vwad:/tmp/ejoin/bin/vwad -d"

  localcard_status=`/opt/ejoin_fs/bin/gpio get 42`
  if [ $localcard_status -eq '1' ]; then
    sleep 1

    localcard_status=`/opt/ejoin_fs/bin/gpio get 42`
    if [ $localcard_status -eq '1' ]; then
      # no need to start the vwa daemon process
      vwad_pars=""
    fi
  fi

  # start the watchdog daemon after 600s with interval 10s
  # add a dot to use the same environment variables(such as LD_LIBRARY_PATH)
  # the watchdog script will not return forever
  . $watch_dogd_sh -d600 -i10 "$vdc_pars" "$vfd_pars" "$vwad_pars"
else
  while : 
  do
    sleep 10
  done
fi
