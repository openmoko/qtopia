#!/bin/sh
#
# This script creates a Qtopia image
# suitable for flashing onto the Greenphone
#
# See also: make_sdk_images.sh -help

HELP=0
STRIP=0

# Hopefully this will make the -strip argument useful even when the toolchain lives somewhere else
# (ie. just set PATH and it will work)
TOOLCHAIN=/opt/toolchains/greenphone/gcc-4.1.1-glibc-2.3.6/arm-linux/bin
PATH=$TOOLCHAIN:$PATH

if [ "$1" = "-strip" ]; then
    STRIP=1
    shift
fi

if [ $# -eq 1 ]; then
    IMAGE=$1
    OUTDIR=`pwd`
elif [ $# -eq 2 ]; then
    IMAGE=$1
    OUTDIR=`(cd $2 ; pwd)`
elif [ $# -eq 3 ] && [ $3 = "-perftest" ]; then
    PERFTEST=1
    IMAGE=$1
    OUTDIR=`(cd $2 ; pwd)`
else
    HELP=1;
    echo "$(basename $0) [-strip] <image> [Output dir]"
    echo "This script creates a cramfs image suitable for flashing onto the Greenphone"
    echo "Create qtopia.cramfs in Output dir or the current directory from the specified image."
    exit 1;
fi

# Check our pre-condition that a Qtopia image seems to be under $IMAGE
if [ ! -f $IMAGE/bin/qpe ]; then
    echo "Error : Missing $IMAGE/bin/qpe, check the value of <image> argument"
    exit 1;
fi

rm -f "$OUTDIR/qtopia.cramfs"
rm -f "$OUTDIR/qtopia_default.tgz"

echo "Creating working directory..."
WORK=`mktemp -d /tmp/greenphone.XXXXXXXX`
mkdir $WORK/opt
QPEROM=$WORK/opt/Qtopia.rom
QTOPIA_PREFIX=$WORK/opt/Qtopia
cp -R $IMAGE $QPEROM
mkdir $QTOPIA_PREFIX

if [ "$STRIP" = 1 ]; then
    echo "Stripping binaries..."
    files="$(find $QPEROM -type f | xargs file | grep ELF | grep 'not stripped' | awk '{print $1}' | sed 's/:$//')"
    for file in $files; do
        arm-linux-strip --strip-all -R .note -R .comment $file
    done
    files="$(find $QPEROM -type f | xargs file | grep ELF | grep 'not stripped' | awk '{print $1}' | sed 's/:$//')"
    for file in $files; do
        arm-linux-strip --strip-unneeded -R .note -R .comment $file
    done
fi

# setup the "dynamic" directories
setup_dynamic()
{
    rom=$1
    ram=$2
    mkdir -p $ram
    dirs="$(find $rom/ -type d | sed 's,^'$rom'/,,')"
    for dir in $dirs; do
        mkdir -p $ram/$dir
    done
    for file in $(ls -a $rom); do
        file=$rom/$file
        if test -L $file; then
            filename=$(basename $file)
            file=$(readlink $file)
            ln $ln_args $file $ram/$filename
        elif test -f $file; then
            file=$(echo $file | sed 's,^'$rom_prefix',,')
            ln $ln_args $file $ram
        fi
    done
    for dir in $dirs; do
        for file in $(ls -a $rom/$dir); do
            file=$rom/$dir/$file
            if test -L $file; then
                filename=$(basename $file)
                file=$(readlink $file)
                ln $ln_args $file $ram/$dir/$filename
            elif test -f $file; then
                file=$(echo $file | sed 's,^'$rom_prefix',,')
                ln $ln_args $file $ram/$dir
            fi
        done
    done
}

echo "Creating symlinks..."
ln_args="-sf"
rom_prefix=$WORK
setup_dynamic $QPEROM $QTOPIA_PREFIX

if [ "$PERFTEST" = "1" ]; then
    rm $QTOPIA_PREFIX/qpe.sh
    ln -s /opt/Qtopia.rom/qpe_performance.sh $QTOPIA_PREFIX/qpe.sh
fi

# This is a fix for Greenphone-specific tools that assume the PREFIX is writeable.
# The problem comes because you can't open a symlink for writing. This is why I
# want to get UnionFS going on the Greenphone (because then you would be able to
# do that)
rm -rf $QTOPIA_PREFIX/etc
cp -R $QPEROM/etc $QTOPIA_PREFIX/etc

# create qtopia_default.tgz
cd $QTOPIA_PREFIX
tar --owner=0 --group=0 -czf "${WORK}/opt/Qtopia.rom/qtopia_default.tgz" *

# create qtopia.cramfs
cd ${WORK}
/sbin/mkfs.cramfs ${WORK}/opt/Qtopia.rom "${OUTDIR}/qtopia.cramfs"
echo "Qtopia.rom image created as ${OUTDIR}/qtopia.cramfs"

cd "$OUTDIR"
rm -rf $WORK

