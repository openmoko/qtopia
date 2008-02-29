#!/bin/sh

case $1 in
    unload)
        rmmod bvd_bi
        rmmod net_fd
        rmmod g_file_storage
        rmmod serial_fd
        # notify Qtopia that an ethernet device may have disappeared 
        qcop send "QPE/NetworkState" "updateNetwork()" &
        ;;
    ether)
        grep usbdcore /proc/modules || insmod usbdcore
        insmod net_fd.o if_name=eth
        insmod bvd_bi

        # notify Qtopia that a new ethernet device may have appeared 
        qcop send "QPE/NetworkState" "updateNetwork()" &
        ;;
    serial)
        grep usbdcore /proc/modules || insmod usbdcore
        insmod serial_fd vendor_id=0x6666 product_id=0x0001
        insmod bvd_bi
        ;;
    storage)
        if grep /dev/tffsd /proc/mounts; then
            umount /dev/tffsd
        fi
        grep usbdcore /proc/modules || insmod usbdcore
        insmod g_file_storage file=/dev/tffsd
        insmod bvd_bi
        ;;
    *)
        echo "$0 {unload|ether|serial|storage}"
        exit 1;
        ;;
esac

