#!/bin/sh
# the system run the first shell,using this shell to run other user's shell file
echo "************************ < mtk_autorun.sh > *****************************"

/opt/ejoin_fs/script/insmod_ko.sh

/opt/ejoin_fs/script/background.sh &

/opt/ejoin_fs/script/main_task.sh

while : 
do
  sleep 10
done
