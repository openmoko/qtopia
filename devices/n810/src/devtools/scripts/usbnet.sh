#!/bin/sh
KERNEL_VER=`uname -r`
case $1 in
    start)
        /sbin/rmmod /mnt/initfs/lib/modules/$KERNEL_VER/g_file_storage.ko
        /sbin/insmod /mnt/initfs/lib/modules/$KERNEL_VER/g_ether.ko
        /sbin/ifup usb0
        ;;
    stop)
        /sbin/rmmod /mnt/initfs/lib/modules/$KERNEL_VER/g_ether.ko
        /sbin/insmod /mnt/initfs/lib/modules/$KERNEL_VER/g_file_storage.ko
        /sbin/ifdown usb0
        ;;
    restart)
        usbnet stop
        wait 1
        usbnet start
        ;;
    *)
        echo "$0 {start|stop|restart}"
        exit 1;
        ;;
esac
