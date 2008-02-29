#!/bin/sh

# This flash script updates the Greenphone with a new Qtopia image.

# To use this script copy it onto a sd card.
# This script requires the following additional files on the sd card.
# qtopia.cramfs
# flash-status-*.gif

echo "Installing new Trolltech Qtopia image"

export PATH=/usr/local/bin:/usr/local/sbin:/usr/bin:/usr/sbin:/bin:/sbin

# Process command line options
OPTION_REMOVE_FLASH_FILES=0
OPTION_DONT_REBOOT=0
while [ $# -gt 0 ]; do
    case $1 in
        --remove-flash-files)
            OPTION_REMOVE_FLASH_FILES=1
            ;;
        --dont-reboot)
            OPTION_DONT_REBOOT=1
            ;;
        *)
            echo "TROLL: Unknown option $1, ignoring."
            ;;
    esac
    shift
done

QTOPIA_TOOLS=/mnt/user/tools

# The following variables define where qtopia.cramfs
QTOPIA_IMAGE_DEVICE="/dev/tffsa2"

# The following files are used by this script. If --remove-flash-files is
# sepcified all of these files will be removed from the installation medium.
QTOPIA_IMAGE="qtopia.cramfs"
STATUS_IMAGES="flash-status-fail-fserror.gif \
               flash-status-fail-fsversion.gif \
               flash-status-fail-files.gif \
               flash-status-fail-space.gif \
               flash-status-fail-initqtopia.gif \
               flash-status-flashing.gif \
               flash-status-pass.gif"

prepare_qtopia()
{
    rm -rf /mnt/disk2/Qtopia/*
    if tar -xzf /opt/Qtopia.rom/qtopia_default.tgz -C /mnt/disk2/Qtopia; then
        md5sum /opt/Qtopia.rom/qtopia_default.tgz > /mnt/disk2/.qtopia_default.md5
    fi
}

check_sd_card()
{
    # Check that required files exists and are readable
    for i in $STATUS_IMAGES $QTOPIA_IMAGE; do
        [ ! -r /mnt/sd/$i ] && return 1

        # The above only tests the permissions of the file. If the installation
        # medium is corrupted then the update is likely to fail. Verify that we
        # can actually read the contents of the required files.
        dd if=/mnt/sd/$i of=/dev/null >/dev/null 2>/dev/null || return 1
    done
    
    return 0
}

check_free_space()
{
    # Find size of QTOPIA_IMAGE_DEVICE
    local DEVICE=`basename $QTOPIA_IMAGE_DEVICE`
    if grep $DEVICE /proc/partitions; then
        local AVAILABLE_SPACE=`awk "/$DEVICE/ {print \\$3}" /proc/partitions`
    else
        local AVAILABLE_SPACE=0
    fi

    local REQUIRED_SPACE=`ls -s /mnt/sd/$QTOPIA_IMAGE | sed -e 's/^ *//' | cut -d ' ' -f 1`

    if [ $AVAILABLE_SPACE -lt $REQUIRED_SPACE ]; then
        return 1
    else
        return 0
    fi
}

flash_status()
{
    echo "TROLL: $2"
    gifanim /mnt/sd/$1.gif
}

check_and_repair_filesystems()
{
    # check and repair /mnt/disk2 (/dev/tffsb)
    umount /mnt/disk2
    if fsck -p -y /dev/tffsb 2>&1 | grep REBOOT ; then
        flash_status flash-status-fserror "Filesystem errors fixed on /mnt/disk2"
        reboot
    fi
    mount -o rw,noatime /dev/tffsb

    # check and repair /mnt/user (/dev/tffsc)
    #mount -o remount,ro,noatime /mnt/user
    #if fsck -p -y /dev/tffsc 2>&1 | grep REBOOT ; then
    #    flash_status flash-status-fserror "Filesystem errors fixed on /mnt/user"
    #    reboot
    #fi
    #mount -o remount,rw,noatime /dev/tffsc

    # check and repair /mnt/user_local (/dev/tffsd)
    # cannot check fs no fsck.vfat
}

check_rootfs_version()
{
    grep '^troll v2\.[0123456789]*$' /fs.ver >/dev/null 2>&1
}

###############################################
# Greenphone Qtoipa Flash for SDK Development #
###############################################

if [ -e $QTOPIA_TOOLS/.firstrun ] ; then
    # this is the first boot after flashing
    # don't reflash the device
    rm -f $QTOPIA_TOOLS/.firstrun
    exit 1
fi

flash_status flash-status-flashing "Installing Greenphone Qtopia image"

check_and_repair_filesystems

if ! check_sd_card; then
    flash_status flash-status-fail-files "SD Card check"
    exit 0
fi

if ! check_rootfs_version; then
    flash_status flash-status-fail-fsversion "Incompatible rootfs detected"
    exit 0
fi

if ! check_free_space; then
    flash_status flash-status-fail-space "Not enough freespace on /"
    exit 0
fi

echo "TROLL: Installing Qtopia Image"

# unmount Qtopia image
umount $QTOPIA_IMAGE_DEVICE

# install Qtopia image
dd if=/mnt/sd/$QTOPIA_IMAGE of=$QTOPIA_IMAGE_DEVICE bs=1024

# mount /opt/Qtopia.rom, needed for ldconfig
mount /opt/Qtopia.rom

prepare_qtopia

# update ld.so.cache
cat > /etc/ld.so.conf <<EOF
/lib
/usr/lib
/opt/Qtopia.rom/lib
/opt/Qtopia/lib
EOF
ldconfig

umount /opt/Qtopia.rom

flash_status flash-status-pass "Qtopia Installed"
touch $QTOPIA_TOOLS/.firstrun
sync

if [ $OPTION_REMOVE_FLASH_FILES = 1 ]; then
    for i in $QTOPIA_IMAGE $STATUS_IMAGES; do
        rm -f /mnt/sd/$i
    done
    sync
fi

if [ $OPTION_DONT_REBOOT = 0 ]; then
    sleep 10
    reboot
fi

exit 0

