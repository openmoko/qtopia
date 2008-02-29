#!/bin/sh
#
# Build SDK greenphone Qtopia 4
#

if [ ! -e `pwd`/scripts/functions ] ; then
  echo "ERROR: This script must be run from the base directory of Qtopia source package" 
  exit
fi

. `pwd`/scripts/functions

output_error()
{
  make -k >/dev/null 2>&1
  make -j 1>$LOGFILE 2>&1
  tail -n 20 $LOGFILE
  echo "For more information on this error look at $LOGFILE for details"
}

output_install_error()
{
  make install -k >/dev/null 2>&1
  make install -j 1>$LOGFILE 2>&1
  tail -n 20 $LOGFILE
  echo "For more information on this error look at $LOGFILE for details"
}

skip_host_tools()
{
  if [ -e $QPEDIR/qtopiacore/host/bin/qmake ] ; then
    SKIP="-skip-dqt"
    if [ ! -e qtopiacore/host ] ; then
      ln -s ../qtopiacore/host qtopiacore/host
    fi
  else
    SKIP=""
  fi 
}

make_greenphone_sdk()
{
  rm -Rf $QPEDIR/../greenphone-sdk
  mkdir -p $QPEDIR/../greenphone-sdk/qtopiacore
  if [ ! -e $QPEDIR/../greenphone-sdk/qtopiacore/qtopiacore/qt ] ; then
    ln -s $QPEDIR/qtopiacore/qt $QPEDIR/../greenphone-sdk/qtopiacore/qt
  fi
  cd $QPEDIR/../greenphone-sdk
  LOGFILE=`pwd`/build.log
  echo 0>$LOGFILE
  skip_host_tools
  echo "TIME:`date "+%H:%M:%S"` Configuring Qtopia for Greenphone"

  if $QPEDIR/configure -device greenphone -platform $QPEDIR/devices/gcc411/mkspecs/linux-g++ -prefix /opt/Qtopia -sdk /opt/Qtopia/SDK/$QPEVER/greenphone -image /opt/Qtopia/SDK/$QPEVER/greenphone/image -ssl -no-helix -no-sxe -confirm-license -release $SKIP >${OUTPUT} 2>&1; then
    echo "DONE"
  else
    echo "$0 FAILED in configure for greenphone SDK"
    exit 1
  fi

  if [ "$CLEAN" = "1" ] ; then
    echo "TIME:`date "+%H:%M:%S"` Cleaning..."
    make clean >${OUTPUT} 2>&1
  fi
  echo "TIME:`date "+%H:%M:%S"` Building..."
  if make >${OUTPUT} 2>&1; then
    echo "DONE"
  else
    output_error
    echo "$0 FAILED in make for greenphone SDK"
    exit 1
  fi
  echo "TIME:`date "+%H:%M:%S"` Installing..."
  if make install >${OUTPUT} 2>&1; then
    echo "DONE"
  else
    output_install_error
    echo "$0 FAILED in make install for greenphone SDK"
    exit 1
  fi
}

make_x86_sdk()
{
  rm -Rf $QPEDIR/../x86-sdk
  mkdir -p $QPEDIR/../x86-sdk/qtopiacore
  if [ ! -e $QPEDIR/../x86-sdk/qtopiacore/qtopiacore/qt ] ; then
    ln -s $QPEDIR/qtopiacore/qt $QPEDIR/../x86-sdk/qtopiacore/qt
  fi
  cd $QPEDIR/../x86-sdk
  LOGFILE=`pwd`/build.log
  echo 0>$LOGFILE
  skip_host_tools
  echo "TIME:`date "+%H:%M:%S"` Configuring Qtopia for x86 GCC 4.1.1"

  if $QPEDIR/configure -device gcc411 -prefix /opt/Qtopia/SDK/$QPEVER/x86/image -sdk /opt/Qtopia/SDK/$QPEVER/x86 -image /opt/Qtopia/SDK/$QPEVER/x86/image -no-helix -no-sxe -no-drm -debug -confirm-license -extra-qt-config="-release" -extra-qtopiacore-config="-qt-libpng" >${OUTPUT} 2>&1; then
    echo "DONE"
  else
    echo "$0 FAILED in configure for x86 SDK"
    exit 1
  fi
  
  if [ "$CLEAN" = "1" ] ; then
    echo "TIME:`date "+%H:%M:%S"` Cleaning..."
    make clean >${OUTPUT} 2>&1
  fi
  echo "TIME:`date "+%H:%M:%S"` Building..."
  if make >${OUTPUT} 2>&1; then
    echo "DONE"
  else
    output_error
    echo "$0 FAILED in make for x86 SDK"
    exit 1
  fi
  echo "TIME:`date "+%H:%M:%S"` Installing..."
  if make install >${OUTPUT} 2>&1; then
    echo "DONE"
  else
    output_install_error
    echo "$0 FAILED in make install for x86 SDK"
    exit 1
  fi
}

make_greenphone_img()
{
  rm -Rf $QPEDIR/greenphone-image
  cp -dpR /opt/Qtopia/SDK/$QPEVER/greenphone/image $QPEDIR/greenphone-image
}

update_extras()
{
  mkdir -p /opt/Qtopia/extras/bin
  cd /opt/Qtopia/extras/bin
  rm -f /opt/Qtopia/extras/bin/*
  wget -nv http://qtopiaweb:/dist/input/qtopia/$QPEVER/greenphone/extras/uic3
  chmod 555 uic3
  wget -nv http://qtopiaweb:/dist/input/qtopia/$QPEVER/greenphone/extras/qt3to4
  chmod 555 qt3to4
  wget -nv http://qtopiaweb:/dist/input/qtopia/$QPEVER/greenphone/extras/usbflash
  chmod 555 usbflash
}

make_flash()
{
  mkdir -p /opt/Qtopia/extras/images

  cd $QPEDIR
  $QPEDIR/scripts/greenphone-make-flash.sh --qtopia-build $QPEDIR/greenphone-image --flash
  cp -f $QPEDIR/qtopia-greenphone-flash /opt/Qtopia/extras/images/qtopia-greenphone-flash-$QPEVER
  cp -f $QPEDIR/qtopia-greenphone-update.tar.gz /opt/Qtopia/extras/images/qtopia-greenphone-update-$QPEVER.tar.gz

  cd /opt/Qtopia/extras/images
  rm -f qtopia-greenphone-update.tar.gz
  ln -s qtopia-greenphone-update-$QPEVER.tar.gz qtopia-greenphone-update.tar.gz
  rm -f qtopia-greenphone-flash
  ln -s qtopia-greenphone-flash-$QPEVER qtopia-greenphone-flash
  cd $QPEDIR
}


HELP=yes
OUTPUT="/dev/fd/0"
CLEAN=0
SDKGRN=0
SDKX86=0
SDKIMG=0
FLASH=0
EXTRAS=0
VMWARE=0

while [ -n "$1" ] ; do
  case $1 in
  -h | -help | --help)
    HELP=yes
    ;;
  -q | -quiet | --quiet)
    OUTPUT="build.log"
    HELP=no
    ;;
  -clean | --clean)
    CLEAN=1
    HELP=no
    ;;
  -sdk | --sdk)
    SDKGRN=1
    SDKX86=1
    SDKIMG=1
    HELP=no
    ;;
  -sdk-greenphone | --sdk-greenphone)
    SDKGRN=1
    HELP=no
    ;;
  -sdk-x86 | --sdk-x86)
    SDKX86=1
    HELP=no
    ;;
  -sdk-img | --sdk-img)
    SDKIMG=1
    HELP=no
    ;;
  -flash | --flash)
    FLASH=1
    HELP=no
    ;;
  -extras | --extras)
    EXTRAS=1
    HELP=no
    ;;
  *)
    echo $1: unknown argument
    ;;
  esac
  shift
done

if [ "$HELP" = "yes" ] ; then
  echo
  echo "FORMAT: $0 <options>"
  echo
  echo "Options:"
  echo 
  echo " --help            display command help"
  echo " --quiet           quiet mode"
  echo " --clean           clean"
  echo " --sdk             Build x86, greenphone and image for SDK"
  echo " --sdk-greenphone  Build greenphone SDK"
  echo " --sdk-x86         Build x86 SDK"
  echo " --sdk-img         Create Qtopia dir from greenphone sdk"
  echo " --flash           Create greenphone_flash file"
  echo " --extras          Update qt3to4, uic3 etc"
  echo
  exit
fi

if [ "`hostname`" = "greenphonesdk" ] ; then
  #We force if detect building inside vmware since don't have access to extras
  EXTRAS=0
fi

export QPEVER=`version`

export PATH=/opt/teambuilder/bin:/usr/local/gcc-4.1.1/bin:/opt/toolchains/greenphone/gcc-4.1.1-glibc-2.3.6/arm-linux/bin:$PATH
export TEAMBUILDER=1
export TEAMBUILDER_CC_VERSION=4.1.1

export QPEDIR=`pwd`
export PATH=${QPEDIR}/bin:${QPEDIR}/scripts:${PATH}

if [ "$SDKX86" = "1" ] ; then
  rm -Rf /opt/Qtopia/SDK/$QPEVER/x86
  make_x86_sdk
fi

if [ "$SDKGRN" = "1" ] ; then
  rm -Rf /opt/Qtopia/SDK/$QPEVER/greenphone
  make_greenphone_sdk
fi

if [ "$SDKIMG" = "1" ] ; then
  make_greenphone_img
fi

if [ "$EXTRAS" = "1" ] ; then
  rm -Rf /opt/Qtopia/extras
  update_extras
fi

if [ "$FLASH" = "1" ] ; then
  make_flash
fi

if [ -e /opt/Qtopia/extras/bin/uic3 ] ; then
  if [ -e /opt/Qtopia/SDK/$QPEVER/greenphone/qtopiacore/host/bin ] ; then
    rm -f /opt/Qtopia/SDK/$QPEVER/greenphone/qtopiacore/host/bin/uic3
    rm -f /opt/Qtopia/SDK/$QPEVER/greenphone/qtopiacore/host/bin/qt3to4
    rm -f /opt/Qtopia/SDK/scripts/usbflash
    ln -s /opt/Qtopia/extras/bin/uic3 /opt/Qtopia/SDK/$QPEVER/greenphone/qtopiacore/host/bin/uic3
    ln -s /opt/Qtopia/extras/bin/qt3to4 /opt/Qtopia/SDK/$QPEVER/greenphone/qtopiacore/host/bin/qt3to4
    ln -s /opt/Qtopia/extras/bin/usbflash /opt/Qtopia/SDK/scripts/usbflash
  fi
fi

if [ -e /opt/Qtopia/SDK/$QPEVER/x86/qtopiacore/host ] ; then
  if [ -e /opt/Qtopia/SDK/$QPEVER/greenphone/qtopiacore/host ] ; then
    rm -Rf /opt/Qtopia/SDK/$QPEVER/greenphone/qtopiacore/host
    cd /opt/Qtopia/SDK/$QPEVER/greenphone/qtopiacore
    ln -s /opt/Qtopia/SDK/$QPEVER/x86/qtopiacore/host
  fi
fi

if [ ! -e /opt/Qtopia/SDK/$QPEVER/greenphone/qtopiacore/qt/tools/qvfb/Greenphone.skin ] ; then
  cp -dpR $QPEDIR/devices/greenphone/Greenphone.skin /opt/Qtopia/SDK/$QPEVER/greenphone/qtopiacore/qt/tools/qvfb 
fi

mkdir -p /opt/Qtopia/SDK/scripts
cp -dpR $QPEDIR/scripts/greenphone-sdk/scripts/* /opt/Qtopia/SDK/scripts
cp $QPEDIR/doc/greenphone_quickstart.pdf /opt/Qtopia/SDK
cp $QPEDIR/doc/greenphone_userguide.pdf /opt/Qtopia/SDK
cp $QPEDIR/doc/greenphone_gettingstarted.pdf /opt/Qtopia/SDK

rm -f /opt/Qtopia/SDK/scripts/functions
echo -en "\043\041/bin/bash\nversion()\n{\necho \"$QPEVER\"\n}\n">/opt/Qtopia/SDK/scripts/functions
chmod 755 /opt/Qtopia/SDK/scripts/functions

echo "Built: `date`">/opt/Qtopia/SDK/versioninfo
echo "By:    `whoami`">>/opt/Qtopia/SDK/versioninfo
echo "Host:  `hostname`">>/opt/Qtopia/SDK/versioninfo
