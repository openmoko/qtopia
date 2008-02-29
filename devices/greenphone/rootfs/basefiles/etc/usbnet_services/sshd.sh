#!/bin/sh

case $1 in
start)
    HOSTKEYDIR=/mnt/user/etc/dropbear
    [ -d $HOSTKEYDIR ] || mkdir -p $HOSTKEYDIR
    for i in rsa dss; do
        if [ ! -f $HOSTKEYDIR/dropbear_${i}_host_key ]; then
            /usr/sbin/dropbearkey -t ${i} -f $HOSTKEYDIR/dropbear_${i}_host_key
        fi
    done
        
    /usr/sbin/dropbear
    ;;
stop)
    kill $( pidof dropbear ) 2>/dev/null
    ;;
esac

exit 0

