#!/bin/sh

splash -p + "Checking for" "Qtopia update"

if grep mmca1 /proc/partitions ; then
    mount /dev/mmca1 /mnt/sd 2>/dev/null
elif grep mmca$ /proc/partitions ; then
    mount /dev/mmca /mnt/sd 2>/dev/null
fi

# If you don't want the flash process to ask questions
# uncomment the following line.
UPDATEQTOPIA_OPTIONS="--no-questions"
updateqtopia $UPDATEQTOPIA_OPTIONS /mnt/sd && exit 0

umount /mnt/sd 2>/dev/null

. /etc/profile

if [ -e /tmp/boot-fail ]; then
    splash -now "Boot failed" "Networking is up"
    exit 0
fi

splash -p + "Starting Qtopia"

if [ -e /opt/Qtopia/qpe.env ]; then
    . /opt/Qtopia/qpe.env
elif [ -e /opt/Qtopia.rom/qpe.env ]; then
    . /opt/Qtopia.rom/qpe.env
fi

if [ -x /opt/Qtopia/qpe.sh ]; then
    nohup /opt/Qtopia/qpe.sh > /dev/null 2>&1 &
elif [ -x /opt/Qtopia.rom/qpe.sh ]; then
    nohup /opt/Qtopia.rom/qpe.sh > /dev/null 2>&1 &
fi

# Wait for Qtopia to start
sleep 5

if [ "`pidof qpe`" = "" ]; then
    gifanim /usr/share/booterrors/boot-error-qpe.gif
fi

exit 0

