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

umount /mnt/sd

# The following variables define where qtopia.cramfs and tools.tgz
# are installed to.
QTOPIA_IMAGE=/
QTOPIA_TOOLS=/mnt/user/tools

$QTOPIA_TOOLS/gifanim $QTOPIA_TOOLS/splash.gif &

# start usb networking
$QTOPIA_TOOLS/usbnet.sh load
nohup $QTOPIA_TOOLS/usbnet.sh up > /dev/null 2>&1 &
$QTOPIA_TOOLS/usbnet.sh start_services


# check and repair root file system (/dev/tffsa)
mount -o remount,ro,noatime /
if fsck -p / 2>&1 | grep REBOOT ; then
    echo "TROLL: Filesystem errors fixed on /"
    reboot
fi
mount -o remount,rw,noatime /

# check and repair /opt/Qtopia.user (/dev/tffsb)
umount /mnt/disk2
if fsck -p /dev/tffsb 2>&1 | grep REBOOT ; then
    echo "TROLL: Filesystem errors fixed on /opt/Qtopia.user"
    reboot
fi
mount -o rw,noatime /dev/tffsb /mnt/disk2

# check and repair /mnt/user (/dev/tffsc)
#mount -o remount,ro,noatime /dev/tffsc
#if e2fsck -p -y /dev/tffsc 2>&1 | grep REBOOT ; then
#    echo "TROLL: Filesystem errors fixed on /mnt/user"
#    reboot
#fi
#mount -o remount,rw,noatime /dev/tffsc

# check and repair /mnt/user_local (/dev/tffsd)
dosfsck -a /dev/tffsd

# Set Qtopia image location dependent variables here.
# Set generic Greenphone Qtopia variables in qpe.sh
export HOME=/mnt/user/home
export QPEDIR=/opt/Qtopia.user
export QTDIR=$QPEDIR
export QTOPIA_PATH=/opt/Qtopia.user:/tmp/qpk_sd:/tmp/qpk_local
export PATH=$QPEDIR/bin:/opt/Qtopia/bin:$QTOPIA_TOOLS:$PATH
export LD_LIBRARY_PATH=$QPEDIR/lib:/opt/Qtopia/lib
export TZDIR=/opt/Qtopia.user/etc/zoneinfo

if [ -e $QTOPIA_IMAGE/qtopia.cramfs ] ; then
    # Run Qtopia from cramfs!
    if [ -e /dev/loop0 ]; then
        losetup /dev/loop0 $QTOPIA_IMAGE/qtopia.cramfs
        mount -t cramfs /dev/loop0 /opt/Qtopia

        if [ -x /opt/Qtopia.user/qpe.sh ]; then
            nohup /opt/Qtopia.user/qpe.sh>/dev/null 2>&1 &
            sleep 5
            exit 0
        elif [ -x /opt/Qtopia/qpe.sh ]; then
            nohup /opt/Qtopia/qpe.sh>/dev/null 2>&1 &
            sleep 5
            exit 0
        fi
    fi
fi

exit 0

