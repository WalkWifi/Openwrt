#/bin/sh

ltq5xx_cfg_get() {
  local acomapp_cfgfile="/tffs/cfgnew.txt"
  if [ -f $acomapp_cfgfile ] ; then
    cfg_val=`grep "$1 =" $acomapp_cfgfile | awk -F, '{ sub(/\r/,"",$2); print $2 }'`
  fi

  echo ${cfg_val:-$2}
}

deleteOldGdbs() {
#  local max_gdbs=`ltq5xx_cfg_get "System GdbFile Count" 10`
  local max_gdbs=10
  if [ $max_gdbs -le $# ] ;then
    local i=1
    max_gdbs=$(($#-$max_gdbs+1))

    while [ $i -le $max_gdbs ] ;do
      eval filename=\$$i
      echo "remove gdb file $filename"
      rm -f $filename

      i=$(($i+1))
    done
  fi
}

VAR_DIR=/opt/ejoin_fs/log/gdb

[ ! -d $VAR_DIR ] && mkdir -p $VAR_DIR

deleteOldGdbs `ls $VAR_DIR/*.gdb 2>/dev/null`

mdHM=`date +%m%d-%H%M`
mv $1 $VAR_DIR/$mdHM.gdb

