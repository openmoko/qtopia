#!/bin/sh
# 
#  qtopia start-up script for Nokia n810
# 
# echo '1' >> /proc/sys/kernel/printk
export LOGNAME=user
export HOME=/home/$LOGNAME
export QPEDIR=/opt/Qtopia
export QTDIR=$QPEDIR
export LD_LIBRARY_PATH=$QTDIR/lib
export PATH=$QPEDIR/bin:$PATH

export QTOPIA_PHONE_DUMMY=1
export SXE_DISCOVERY_MODE=1

export QWS_MOUSE_PROTO="tslib:/dev/input/event3"
export QWS_SIZE="800x480"

BLK=`mount | grep mmcblk0p1`
if [ -z "$BLK" ]; then
    mount /media/mmc1
fi


if [ -e /sys/devices/platform/gpio-switch/slide/state ]; then
# n810
		export QWS_KEYBOARD="n810kbdhandler"
else
#n800
		export QWS_KEYBOARD="n800kbdhandler"
fi


KILLPROGS="qpe quicklauncher mediaserver sipagent messageserver telepathyagent"

    /sbin/rmmod /mnt/initfs/lib/modules/2.6.21-omap1/g_file_storage.ko
    /sbin/insmod /mnt/initfs/lib/modules/2.6.21-omap1/g_ether.ko
    /sbin/ifup usb0


killproc() {
        pid=`/bin/pidof $1'`
        [ "$pid" != "" ] && kill $pid
}

# Start up the mini-syslogd in our logread implementation.
# FIXME: Remove this once BusyBox is reconfigured with syslogd.
if [ -e $QPEDIR/bin/logread ]; then
    killall logread 2>/dev/null
    logread -d &
fi

case $1 in
'start')
    if [ -f /etc/mce/ ] ; then  /usr/sbin/update-rc.d -f mce remove; fi
	echo "Starting QPE..."

	cd $HOME
        echo "performance" > /sys/devices/system/cpu/cpu0/cpufreq/scaling_governor
        dsmetool -n 19 -t "/opt/Qtopia/bin/qpe 2>&1 | logger -t 'Qtopia'" 

	;;
'stop')
        echo "Killing QPE..."
        dsmetool -k "/opt/Qtopia/bin/qpe 2>&1 | logger -t 'Qtopia'"

    	  killall qpe $KILLPROGS 2>/dev/null
          dspctl reset;killall oss-media-server;killall multimediad
	
        ;;

esac

