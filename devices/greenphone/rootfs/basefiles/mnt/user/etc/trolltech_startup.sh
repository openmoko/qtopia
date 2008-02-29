#!/bin/sh

if grep mmca1 /proc/partitions ; then
    mount /dev/mmca1 /mnt/sd 2>/dev/null
elif grep mmca$ /proc/partitions ; then
    mount /dev/mmca /mnt/sd 2>/dev/null
fi

if [ -x /mnt/sd/trolltech_flash.sh ]; then
    /mnt/sd/trolltech_flash.sh && exit 0
fi

rm -f /mnt/user/tools/.firstrun

umount /mnt/sd 2>/dev/null

. /etc/profile

# The following variables define where user_tools.tgz is installed to.
QTOPIA_TOOLS=/mnt/user/tools

# start usb networking
nohup $QTOPIA_TOOLS/usbnet.sh up > /dev/null 2>&1 &

[ -e /tmp/boot-fail ] && exit 0

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
QPE_PIDS="`pidof qpe`"
if [ "$QPE_PIDS" = "" ]; then
    gifanim /usr/share/booterrors/boot-error-qpe.gif
fi

exit 0

