#!/bin/sh
# 
#  qtopia start-up script for Nokia 770
# 
# echo '1' >> /proc/sys/kernel/printk
export LOGNAME=user
export HOME=/home/$LOGNAME
export QPEDIR=/opt/Qtopia
export QTDIR=$QPEDIR
export LD_LIBRARY_PATH=$QTDIR/lib
export PATH=$QPEDIR/bin:$PATH
export QTOPIA_PHONE_DUMMY=1


export QWS_MOUSE_PROTO="tslib:/dev/input/event2"
export QWS_KEYBOARD="nokiakbdhandler"
#export QWS_KEYBOARD="TTY"
export QWS_SIZE="800x480"

killproc() {
        pid=`/bin/ps -e | /bin/sed -n -e '/\<'$1'\>/ s/^ *\([0-9][0-9]*\).*/\1/p'`
        [ "$pid" != "" ] && kill $pid
}

case $1 in
'start')
	echo "Starting QPE..."

	cd $HOME
	/opt/killmaemo.sh &
	KILLPROGS="dbus-daemon bme-dbus-proxy dbus-daemon Xomap sapwood-server matchbox-window-manager clipboard-manager gconfd-2 qpe quicklauncher mediaserver mediaplayer sipagent"

	touch /tmp/restart-qtopia
 	while [ -e /tmp/restart-qtopia ]; do
     		killall $KILLPROGS 2>/dev/null
     		qpe 2>$HOME/log >$HOME/log
 	done


#	$QPEDIR/bin/qpe 2>/opt/log >/opt/log;
	;;
'stop')
        echo "Killing QPE..."
    	killall $KILLPROGS 2>/dev/null
	rm /tmp/restart-qtopia
        ;;
*)
	echo "Starting QPE..."

	KILLPROGS="dbus-daemon bme-dbus-proxy dbus-daemon Xomap sapwood-server matchbox-window-manager clipboard-manager gconfd-2 qpe quicklauncher mediaserver mediaplayer sipagent"

     	killall $KILLPROGS 2>/dev/null
     	qpe 2>log >log
	;;
esac

