#!/bin/sh
ROOT_UID=0
VMPLAYER_DIR=`which vmplayer`
clear
echo "Qtopia Greenphone SDK Linux Installation"
echo
if [ ! -n "$VMPLAYER_DIR" ] ; then
    echo "Installation of vmplayer"
    echo "If not installed download and install before continuing"
    echo "at http://www.vmware.com"
    echo 
    echo -n "Unable to find vmplayer. Have you installed vmplayer? (y/n) : "
    read option
    if [ "$option" != "y" ] ; then
        echo "Qtopia Greenphone SDK requires vmplayer or equivalent to be installed."
        echo
        exit
    fi
else
    echo "Found VMPlayer installed at: $VMPLAYER_DIR"
fi
echo
echo "NOTE: SDK requires 2.2GB free disk space and 512MB RAM"
echo
echo -n "What directory would you like to install the SDK? [$HOME/QtopiaGreenphoneSDK] : "
read dir
if [ ! -n "$dir" ] ; then
    dir="$HOME/QtopiaGreenphoneSDK"
fi
if [ ! -e "$dir" ] ; then
    echo
    echo -n "Directory does not exist. Do you wish to create the directory $dir? (y/n) : "
    read option
    if [ "$option" != "y" ] ; then
        echo "Installation Aborted by user."
        echo
        exit
    fi
    mkdir -p $dir
fi
if [ ! -w "$dir" ] ; then
    echo
    echo "Please choose a directory you have permission to write to."
    echo
    exit
fi 
echo
echo "Directory to install to: $dir"
echo 
echo -n "Continue with installation? (y/n) : "
read option
if [ "$option" != "y" ] ; then
    echo "Installation Aborted by user."
    echo
    exit
fi
clear
echo "Installing Qtopia Greenphone SDK....please wait"
#mkdir -p $dir
cp release.html $dir
cp greenphone.vmx $dir
cp license.txt $dir
export currentdir=$PWD
cd $dir
echo "Installing qtopia SDK...4.0 minutes remaining"
tar -xzf $currentdir/qtopia.dat >/dev/null 2>&1
echo "Installing qtopia src...2.5 minutes remaining"
tar -xzf $currentdir/qtopiasrc.dat >/dev/null 2>&1
echo "Installing toolchain....2.0 minutes remaining"
tar -xzf $currentdir/toolchain.dat >/dev/null 2>&1
echo "Installing home.........1.5 minutes remaining"
tar -xzf $currentdir/home.dat >/dev/null 2>&1
echo "Installing rootfs.......1.0 minutes remaining"
tar -xzf $currentdir/rootfs.dat >/dev/null 2>&1

if [ -e "$dir/home.vmdk" ] ; then
  if [ -e "$dir/qtopiasrc.vmdk" ] ; then
    if [ -e "$dir/qtopia.vmdk" ] ; then
      if [ -e "$dir/toolchain.vmdk" ] ; then
        if [ -e "$dir/rootfs.vmdk" ] ; then
          if [ -e "$dir/greenphone.vmx" ] ; then
            echo 
            echo "Installation successful."
            echo "to start cd $dir;vmplayer greenphone.vmx"
            echo
            exit
          fi
        fi
      fi
    fi
  fi
fi
echo
echo "Installation FAILED!"
echo
