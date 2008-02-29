#!/bin/ash

QTOPIA_IMAGE_PATH=/
QTOPIA_TOOLS=/mnt/user/tools
QTOPIA_IMAGE=/

# Set generic Greenphone Qtopia variables here.
# Set Qtopia image location dependent variables in
# the appropriate trolltech_*_startup.sh script or
# the appropriate trolltech_*_flash.sh script.
export QWS_MOUSE_PROTO="greenphonemousehandler"
export QWS_KEYBOARD="greenphonekbdhandler greenphonedetecthandler"
export QWS_SIZE="240x320"
export QTOPIA_PHONE_DEVICE="/dev/ttyS0:115200"

restore_default()
{
    echo "TROLL: Restoring system default"

    # Restore $HOME
    rm -rf /mnt/user/home
    rm -rf /mnt/user/sd_home

    cd /mnt/user
    tar -xzf $QTOPIA_IMAGE_PATH/home_default.tgz

    # Restore writeable Qtopia filesystem
    rm -rf $QPEDIR/*
    cd $QPEDIR
    tar -xzf $QTOPIA_IMAGE_PATH/qtopia_default.tgz
    export PATH=$QTOPIA_TOOLS:$PATH

    startupflags.sh RESTOREDEFAULTS_FLAG 0

    # Restart this script. It is possible that it was changed.
    exec $0
}

# FIXME
# This is a temporary fix for the slow initial boot after the database
# has been deleted. The kernel process "tffs1" consumes nearly 100%
# CPU usage when recreating the database. This fix reduces the performance
# impact by storing the database journal on a tmpfs.
# Symlink $HOME/Applications/Qtopia to /tmp/Apps_Qtopia
# Symlink all known files stored in $HOME/Applications/Qtopia that should
# be on persistant storage from /tmp/Apps_Qtopia/* to $HOME/Applications/.Qtopia
if [ -L $HOME/Applications/Qtopia ]; then
    echo "TROLL: Implementing fast boot fix"
    rm -rf /tmp/Apps_Qtopia
    mkdir -p /tmp/Apps_Qtopia

    if [ ! -e $HOME/Applications/.Qtopia ]; then
        mkdir $HOME/Applications/.Qtopia
    fi

    if [ -d $HOME/Applications/.Qtopia ]; then
        # symlink back all known files which should be on persistent storage
        [ ! -e /tmp/Apps_Qtopia/qtopia_db.sqlite ] && ln -s $HOME/Applications/.Qtopia/qtopia_db.sqlite /tmp/Apps_Qtopia/qtopia_db.sqlite
        [ ! -e /tmp/Apps_Qtopia/drm ] && ln -s $HOME/Applications/.Qtopia/drm /tmp/Apps_Qtopia/drm
        [ ! -e /tmp/Apps_Qtopia/QUniqueIdTrackerFile ] && ln -s $HOME/Applications/.Qtopia/QUniqueIdTrackerFile /tmp/Apps_Qtopia/QUniqueIdTrackerFile
    else
        echo "TROLL: $HOME/Applications/.Qtopia is not a directory.  All changes to the database will be lost on reboot."
    fi
fi

KILLPROGS="qpe quicklauncher qss mediaplayer sipagent phonebounce modem_keep_alive.sh"

touch /tmp/restart-qtopia
while [ -e /tmp/restart-qtopia ]; do
    killall $KILLPROGS 2>/dev/null

    # load startup flags from conf file
    eval `startupflags.sh`

    # Restore default settings.
    if [ "$RESTOREDEFAULTS_FLAG" = 1 ]; then
        restore_default
    fi

    if [ "$PHONEDUMMY_FLAG" = 1 ]; then
        export QTOPIA_PHONE_DUMMY=1
    else
        export QTOPIA_PHONE_DUMMY=0
    fi

    if [ "$PHONEDEVICE_FLAG" != "" ]; then
        export QTOPIA_PHONE_DEVICE=$PHONEDEVICE_FLAG
    fi

    if [ "$PHONEBOUNCE_FLAG" = 1 ] ; then
        # Turn on the modem
        apm --modem-poweron
        apm --wakeup

        # Bring up networking
        usbnet.sh up &

        phonebounce $QTOPIA_PHONE_DEVICE 12345 &
        modem_keep_alive.sh &

        export QTOPIA_PHONE_DUMMY=1
    fi

    chvol SYSTEM 100
    chvol CALL 60
    qpe 2>&1 | logger -t 'Qtopia'
done

gifanim $QTOPIA_TOOLS/splash-shutdown-exit.gif &

