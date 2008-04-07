#!/bin/sh
# this scripts gets run on qtopia sdk update/install

. /opt/Qtopia/SDK/scripts/functions

# symlink the toolchain to the proper place
if [ ! -e /opt/toolchains ]; then
	 mkdir /opt/toolchains
fi
if [ ! -e  /opt/toolchains/arm920t-eabi ]; then
	sudo ln -s /usr/local/arm920t-eabi /opt/toolchains/arm920t-eabi
fi

QTOPIA_VERSION=`version`
mkdir -p /home/user/Settings/Trolltech
cp /opt/Qtopia/SDK/$QTOPIA_VERSION/$DEVICE/devices/$DEVICE/etc/default/Trolltech/* /home/user/Settings/Trolltech
sudo echo "192.168.0.202 neo" >> /etc/hosts
#sudo echo "neosdk" > /etc/hostname
#sudo hostname neosdk
sudo ln -s /usr/local/arm920t-eabi /usr/local/arm-linux
