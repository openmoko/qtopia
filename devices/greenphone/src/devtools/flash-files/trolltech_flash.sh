#!/bin/sh

# This flash script updates the Greenphone with a new Qtopia image.

# To use this script copy it onto a sd card.
# This script requires the following additional files on the sd card.
# qtopia.cramfs
# qtopia_default.tgz
# user_default.tgz
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

# The following variables define where qtopia.cramfs, qtopia_default.tgz,
# user_default and user_tools.tgz are installed to.
QTOPIA_TOOLS=/mnt/user/tools
QTOPIA_IMAGE_PATH=/
QTOPIA_IMAGES="qtopia.cramfs qtopia_default.tgz"
QTOPIA_IMAGES_OPTIONAL="user_default.tgz user_tools.tgz home_default.tgz"
QTOPIA_IMAGES_DEPRECIATED=""

# Remove previous Qtopia image
clean_trolltech_qtopia()
{
    echo "TROLL: Removing previous Qtopia Image"

    for image in $QTOPIA_IMAGES; do
        if [ -r /mnt/sd/$image ]; then
            rm -f $QTOPIA_IMAGE_PATH/$image
        fi
    done

    for image in $QTOPIA_IMAGES_OPTIONAL; do
        if [ -r /mnt/sd/$image ]; then
            rm -f $QTOPIA_IMAGE_PATH/$image
        fi
    done

    for image in $QTOPIA_IMAGES_DEPRECIATED; do
        rm -f $QTOPIA_IMAGE_PATH/$image
    done

    rm -rf /opt/Qtopia/*
}

prepare_user()
{
    cd /mnt/user
    tar -xzf $QTOPIA_IMAGE_PATH/user_default.tgz

    rm -rf /mnt/user/tools
    tar -xzf $QTOPIA_IMAGE_PATH/user_tools.tgz

    # extract home_default only if it is missing
    if [ ! -d /mnt/user/home ]; then
        tar -xzf $QTOPIA_IMAGE_PATH/home_default.tgz
    fi
}

prepare_qtopia()
{
    tar -xzf $QTOPIA_IMAGE_PATH/qtopia_default.tgz -C /opt/Qtopia
}

check_sd_card()
{
    # Check installation messages
    [ ! -e /mnt/sd/flash-status-fail-fserror.gif ] && return 1
    [ ! -e /mnt/sd/flash-status-fail-files.gif ] && return 1
    [ ! -e /mnt/sd/flash-status-fail-generic.gif ] && return 1
    [ ! -e /mnt/sd/flash-status-fail-space.gif ] && return 1
    [ ! -e /mnt/sd/flash-status-flashing.gif ] && return 1
    [ ! -e /mnt/sd/flash-status-pass.gif ] && return 1

    # Check required Qtopia images
    for image in $QTOPIA_IMAGES; do
        [ ! -e /mnt/sd/$image ] && return 1
    done

    return 0
}

check_free_space()
{
    # Find free space on /
    AVAILABLE_SPACE=`df | grep '/$' | tail -1 | tr -s ' ' | cut -d ' ' -f 4`

    REQUIRED_SPACE=0
    for image in $QTOPIA_IMAGES; do
        IMAGE_SIZE=`ls -s /mnt/sd/$image | sed -e 's/^ *//' | cut -d ' ' -f 1`
        REQUIRED_SPACE=$(($REQUIRED_SPACE + $IMAGE_SIZE))
    done

    for image in $QTOPIA_IMAGES_OPTIONAL; do
        if [ -e /mnt/sd/$image ]; then
            IMAGE_SIZE=`ls -s /mnt/sd/$image | sed -e 's/^ *//' | cut -d ' ' -f 1`
            REQUIRED_SPACE=$(($REQUIRED_SPACE + $IMAGE_SIZE))
        fi
    done
    REQUIRED_SPACE=$(($REQUIRED_SPACE + 1024))

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
    # check and repair root file system (/dev/tffsa)
    mount -o remount,ro,noatime /
    if fsck -p -y /dev/tffsa 2>&1 | grep REBOOT ; then
        flash_status flash-status-fserror "Filesystem errors fixed on /"
        reboot
    fi
    mount -o remount,rw,noatime /

    # check and repair /opt/Qtopia.user (/dev/tffsb)
    umount /mnt/disk2
    if fsck -p -y /dev/tffsb 2>&1 | grep REBOOT ; then
        flash_status flash-status-fserror "Filesystem errors fixed on /opt/Qtopia.user"
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

###############################################
# Greenphone Qtoipa Flash for SDK Development #
###############################################

if [ -e $QTOPIA_TOOLS/.firstrun ] ; then
    # this is the first boot after flashing
    # don't reflash the device
    rm -f $QTOPIA_TOOLS/.firstrun
    exit 1
fi

check_and_repair_filesystems

if check_sd_card; then
    flash_status flash-status-flashing "Installing Greenphone Qtopia image"

    # clean up old software
    clean_trolltech_qtopia

    sync
    
    if check_free_space; then
        echo "TROLL: Installing Qtopia Image"

        # install Qtopia images
        for image in $QTOPIA_IMAGES; do
            cp /mnt/sd/$image $QTOPIA_IMAGE_PATH
        done
        sync

        # install optional Qtopia images
        for image in $QTOPIA_IMAGES_OPTIONAL; do
            if [ -e /mnt/sd/$image ]; then
                cp /mnt/sd/$image $QTOPIA_IMAGE_PATH
            fi
        done

        # populate default images
        prepare_qtopia
        prepare_user

        # mount /opt/Qtopia.rom, needed for ldconfig
        if [ -e $QTOPIA_IMAGE_PATH/qtopia.cramfs ]; then
            if [ -e /dev/loop0 ]; then
                losetup /dev/loop0 $QTOPIA_IMAGE_PATH/qtopia.cramfs
                mount -t cramfs /dev/loop0 /opt/Qtopia.rom
            fi
        fi

        # update ld.so.cache
        cat > /etc/ld.so.conf <<EOF
/lib
/opt/Qtopia.user/lib
/opt/Qtopia/lib
EOF
        ldconfig

        flash_status flash-status-pass "Qtopia Installed"
        touch $QTOPIA_TOOLS/.firstrun
        sync

        if [ $OPTION_REMOVE_FLASH_FILES = 1 ]; then
           rm -f `ls -1 /mnt/sd/* | grep -v trolltech_flash.sh`
           sync
        fi

        if [ $OPTION_DONT_REBOOT = 0 ]; then
            sleep 10
            reboot
        fi

        exit 0
    else
        flash_status flash-status-fail-space "Not enough freespace on /"
        exit 0
    fi
else
    flash_status flash-status-fail-files "SD Card check"
    exit 0
fi

exit 0

