#!/bin/sh

# This flash script runs Qtopia from the sd card. The Qtopia image
# to use is searched for in the following order:
# 1. Use partition 1 /qtopia.cramfs as /opt/Qtopia
# 2. Use partition 1 as /opt/Qtopia
# 3. Use partition 2 as /opt/Qtopia
# A valid Qtopia image is detected if $QTOPIA_IMAGE/qpe.sh exists and
# is executable.

# To use this script the device must have been previously flashed
# with trolltech_sdk_flash.sh. To use this script copy it to the first
# partition of an sd card as trolltech_flash.sh.

# This script requires the followig additional files on the sd card.
# qtopia.cramfs
# or
# Qtopia fs tree in first partition
# or
# Qtopia fs tree in second partition

echo "Trolltech run sd image startup hook"

# The following variables define where qtopia.cramfs and tools.tgz
# are installed to.  If you change these you will need to verify
# that the clean_unflashed_phone is still correct.
QTOPIA_TOOLS=/mnt/user/tools

# bring up usb networking
$QTOPIA_TOOLS/usbnet.sh load
nohup $QTOPIA_TOOLS/usbnet.sh up > /dev/null 2>&1 &
$QTOPIA_TOOLS/usbnet.sh start_services


# Mount Qtopia image
if [ -e /mnt/sd/qtopia.cramfs ]; then
    # Run Qtopia from cramfs!
    losetup /dev/loop0 /mnt/sd/qtopia.cramfs
    mount -t cramfs /dev/loop0 /opt/Qtopia
elif [ -e /mnt/sd/qpe.sh ]; then
    # Run Qtopia from /dev/mmca1
    mount /dev/mmca1 /opt/Qtopia
else
    # Run Qtopia from /dev/mmca2
    if grep mmca2 /proc/partitions ; then
        mount /dev/mmca2 /opt/Qtopia 2>/dev/null
    fi
fi

# Source Qtopia environment
if [ -e /opt/Qtopia/qpe.env ]; then
    . /opt/Qtopia/qpe.env
else
    # Set Qtopia image location dependent variables here.
    # Set generic Greenphone Qtopia variables in qpe.sh
    export HOME=/mnt/user/sd_home
    export QPEDIR=/opt/Qtopia
    export QTDIR=$QPEDIR
    export QTOPIA_PATH=/opt/Qtopia.rom:/tmp/qpk_sd:/tmp/qpk_local
    export PATH=$QPEDIR/bin:/opt/Qtopia.rom/bin:$QTOPIA_TOOLS:$PATH
    export LD_LIBRARY_PATH=$QPEDIR/lib:/opt/Qtopia.rom/lib
fi

[ ! -e $HOME ] && mkdir $HOME

if [ -x $QPEDIR/qpe.sh ]; then
    nohup $QPEDIR/qpe.sh >/dev/null 2>&1 &
    sleep 5
    exit 0
fi

# Trolltech hook failed, start oem software
exit 1

