#!/bin/sh
#
# This script creates a Qtopia image
# suitable for flashing onto the Greenphone
#
# See also: make_sdk_images.sh -help

find_dirs()
{
    find $1 -type d -printf "%P\n"
}

find_files()
{
    find $1 -type f -printf "%P\n"
}

HELP=0

if [ $1 = "-h" -a $1 = "--help" ]; then
    HELP=1
fi

if [ $# -eq 1 ]; then
    QTOPIA=$1
    OUTDIR=`pwd`
elif [ $# -eq 2 ]; then
    QTOPIA=$1
    OUTDIR=`(cd $2 ; pwd)`
elif [ $# -eq 3 ] && [ $3 = "-perftest" ]; then
    PERFTEST=1
    QTOPIA=$1
    OUTDIR=`(cd $2 ; pwd)`
else
    HELP=1;
    echo "make_sdk_images.sh <Qtopia Tree> [Output dir]"
    echo "This script creates a Qtopia image suitable for flashing onto the Greenphone"
    echo "Create qtopia.cramfs and qtopia_default.tgz " \
         "in Output dir or the current directory from " \
         "the specified Qtopia tree."
    exit 1;
fi

# Check our pre-condition that a Qtopia image seems to be under $QTOPIA
if [ ! -f $QTOPIA/bin/qpe ]; then
    echo "Error : Missing $QTOPIA/bin/qpe, check the value of <Qtopia Tree> argument"
    exit 1;
fi

WORK=`mktemp -d /tmp/greenphone.XXXXXXXX`

mkdir ${WORK}/Qtopia.rom
cp -a $QTOPIA/* ${WORK}/Qtopia.rom/
cd ${WORK}

rm -f "$OUTDIR/qtopia.cramfs"
rm -f "$OUTDIR/qtopia_default.tgz"

# split Qtopia into Qtopia.rom and Qtopia.rw
mkdir -p ${WORK}/Qtopia.default
cd ${WORK}/Qtopia.default
mkdir bin lib packages
mv ../Qtopia.rom/etc .
mv ../Qtopia.rom/services .
mv ../Qtopia.rom/i18n .
mv ../Qtopia.rom/qtopia_db.sqlite .
if [ "$PERFTEST" = "1" ]; then
    mv ../Qtopia.rom/qpe_performance.sh ./qpe.sh 
    mv ../Qtopia.rom/qpe_original.sh .
else
    mv ../Qtopia.rom/qpe.sh .
fi

# Symlink qpe
ln -s /opt/Qtopia.rom/bin/qpe ${WORK}/Qtopia.default/bin/qpe

# Symlink themes
mkdir -p ${WORK}/Qtopia.default/pics/themes
for i in `find_dirs ../Qtopia.rom/pics/themes`; do
    ln -s /opt/Qtopia.rom/pics/themes/$i ${WORK}/Qtopia.default/pics/themes/$i
done

# Symlink plugins
for i in `find_dirs ../Qtopia.rom/plugins`; do
    mkdir -p ${WORK}/Qtopia.default/plugins/$i
done
for i in `find_files ../Qtopia.rom/plugins`; do
    ln -s /opt/Qtopia.rom/plugins/$i ${WORK}/Qtopia.default/plugins/$i
done

# Symlink qt_plugins
for i in `find_dirs ../Qtopia.rom/qt_plugins`; do
    mkdir -p ${WORK}/Qtopia.default/qt_plugins/$i
done
for i in `find_files ../Qtopia.rom/qt_plugins`; do
    ln -s /opt/Qtopia.rom/qt_plugins/$i ${WORK}/Qtopia.default/qt_plugins/$i
done

# Symlink other hardcoded files
[ -e ../Qtopia.rom/lib/fonts ] && ln -s /opt/Qtopia.rom/lib/fonts ${WORK}/Qtopia.default/lib/fonts
[ -e ../Qtopia.rom/bin/qtopia-pppd-internal ] && ln -s /opt/Qtopia.rom/bin/qtopia-pppd-internal ${WORK}/Qtopia.default/bin
[ -e ../Qtopia.rom/bin/ppp-network ] && ln -s /opt/Qtopia.rom/bin/ppp-network ${WORK}/Qtopia.default/bin
[ -e ../Qtopia.rom/bin/lan-network ] && ln -s /opt/Qtopia.rom/bin/lan-network ${WORK}/Qtopia.default/bin
[ -e ../Qtopia.rom/bin/dbmigrate ] && ln -s /opt/Qtopia.rom/bin/dbmigrate ${WORK}/Qtopia.default/bin
[ -e ../Qtopia.rom/bin/quicklauncher ] && ln -s /opt/Qtopia.rom/bin/quicklauncher ${WORK}/Qtopia.default/bin
[ -e ../Qtopia.rom/bin/qss ] && ln -s /opt/Qtopia.rom/bin/qss ${WORK}/Qtopia.default/bin
[ -e ../Qtopia.rom/bin/qcop ] && ln -s /opt/Qtopia.rom/bin/qcop ${WORK}/Qtopia.default/bin
[ -e ../Qtopia.rom/bin/sxemonitor ] && ln -s /opt/Qtopia.rom/bin/sxemonitor ${WORK}/Qtopia.default/bin

ln -s /usr/share/zoneinfo ${WORK}/Qtopia.default/etc/zoneinfo

# create qtopia_default.tgz
cd ${WORK}/Qtopia.default
tar --owner=0 --group=0 -czf "${WORK}/Qtopia.rom/qtopia_default.tgz" *

# create qtopia.cramfs
cd ${WORK}
/sbin/mkfs.cramfs ${WORK}/Qtopia.rom "${OUTDIR}/qtopia.cramfs"
echo "Qtopia.rom image created as ${OUTDIR}/qtopia.cramfs"

cd "$OUTDIR"
rm -rf $WORK

