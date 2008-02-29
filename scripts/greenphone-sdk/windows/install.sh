#!/bin/bash
ROOT_UID=0
VMPLAYER_DIR=`which vmplayer`
if [ ! -n "$VMPLAYER_DIR" ] ; then
    VMPLAYER_DIR=`which vmware`
fi

SCRIPT="$0"
DIR=${SCRIPT%"install.sh"}
#echo $DIR
if [ "$DIR" != "./" ]; then
    cd $DIR
fi

cat license.txt
more release.txt
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
    mkdir -p "$dir"
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
if [ -e "$dir/release.html" ]; then
  rm -f "$dir/release.html"
fi
cp release.html "$dir"
chmod 664 "$dir/release.html"

if [ -e "$dir/greenphone.vmx" ]; then
  rm -f "$dir/greenphone.vmx"
fi
cp greenphone.vmx "$dir"
chmod 664 "$dir/greenphone.vmx"

if [ -e "$dir/license.txt" ]; then
  rm -f "$dir/license.txt"
fi
cp license.txt "$dir"
chmod 664 "$dir/license.txt"

export currentdir=$PWD
cd "$dir"

ERROR="0"

function unpack()
{
if ! tar -xzf "$1" >"$dir"/.install.log 2>"$dir"/.install.log
      then
      ERROR="1"
      echo "FAILED!"
      rm -f "$1"
  fi
}

if [ "$ERROR" -ne "1" ]; then
  echo "Installing qtopia SDK...4.0 minutes remaining"
  unpack "$currentdir/qtopia.dat"
fi

if [ "$ERROR" -ne "1" ]; then
  echo "Installing qtopia src...2.5 minutes remaining"
  unpack "$currentdir/qtopiasrc.dat"
fi

if [ "$ERROR" -ne "1" ]; then
  echo "Installing toolchain....2.0 minutes remaining"
  unpack "$currentdir/toolchain.dat"
fi

if [ "$ERROR" -ne "1" ]; then
  if [ -e "$dir/home.vmdk" ]; then
    echo -n "Do you want to update the home directory and settings? (y/n) : "
    read option
    if [ "$option" != "n" ] ; then
        echo "Updating home.........1.5 minutes remaining"
        unpack "$currentdir/home.dat"
    fi
  else
     echo "Installing home.........1.5 minutes remaining"
     unpack "$currentdir/home.dat"
  fi
fi

if [ "$ERROR" -ne "1" ]; then
  if [ -e "$dir/rootfs.vmdk" ]; then
    echo -n "Do you want to update the root filesystem? (y/n) : "
    read option
    if [ "$option" != "n" ] ; then
      echo "Updating rootfs.......1.0 minutes remaining"
      unpack "$currentdir/rootfs.dat"
    fi
  else
    echo "Installing rootfs.......1.0 minutes remaining"
    unpack "$currentdir/rootfs.dat"
  fi

fi

if [ "$ERROR" -ne "1" ]; then

if [ -e "$dir/home.vmdk" ] ; then
  if [ -e "$dir/qtopiasrc.vmdk" ] ; then
    if [ -e "$dir/qtopia.vmdk" ] ; then
      if [ -e "$dir/toolchain.vmdk" ] ; then
        if [ -e "$dir/rootfs.vmdk" ] ; then
          if [ -e "$dir/greenphone.vmx" ] ; then
            echo 
            echo "Installation successful."
            echo "to start cd \"$dir\";vmplayer greenphone.vmx"
            echo
            exit
          fi
        fi
      fi
    fi
  fi
fi

fi

echo "Installation FAILED!"
echo
