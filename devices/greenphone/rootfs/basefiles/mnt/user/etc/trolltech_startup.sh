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

# The following variables define where qtopia.cramfs and tools.tgz
# are installed to.
QTOPIA_IMAGE=/
QTOPIA_TOOLS=/mnt/user/tools

# start usb networking
$QTOPIA_TOOLS/usbnet.sh load
nohup $QTOPIA_TOOLS/usbnet.sh up > /dev/null 2>&1 &

if [ -e $QTOPIA_IMAGE/qtopia.cramfs ] ; then
    # Run Qtopia from cramfs!
    if [ -e /dev/loop0 ]; then
        mount -t cramfs -o ro,loop $QTOPIA_IMAGE/qtopia.cramfs /opt/Qtopia.rom

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

        sleep 5
        exit 0
    fi
fi

exit 0

