#!/bin/sh

SAMBADIR=/usr/local/samba
SMBD=$SAMBADIR/bin/smbd
NMBD=$SAMBADIR/bin/nmbd
SMBLOG=$SAMBADIR/var/log.smbd
NMBLOG=$SAMBADIR/var/log.nmbd

case $1 in
start)
    # Start Samba
    rm -f $NMBLOG $SMBLOG
    $SMBD
    $NMBD
    ;;
stop)
    # Kill Samba
    kill $( pidof nmbd smbd ) 2>/dev/null
    ;;
esac

exit 0

