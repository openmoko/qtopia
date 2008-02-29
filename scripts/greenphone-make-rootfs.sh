#!/bin/bash

install()
{
    local src=$1
    local dest=$2
    shift 2

    [ -e $dest ] || mkdir -p $dest

    for i in "$@"; do
        cp -a $src/$i $dest/$i
    done
}

strip_install()
{
    local stripwhat=$1
    local src=$2
    local dest=$3
    shift 3

    [ -e $dest ] || mkdir -p $dest

    for i in "$@"; do
        cp -a $src/$i $dest/$i
        [ -f $dest/$i ] && arm-linux-strip --strip-$stripwhat $dest/$i
    done
}

die()
{
    echo "Error: $@"
    exit 1
}

source_changed()
{
    local SOURCE_STAMP=1
    local BUILD_STAMP=0

    if [ $# -ge 2 ]; then
        SOURCE_STAMP=`find $1 -type f -printf '%T@\n' 2>/dev/null | sort -n | tail -1`
        shift
    fi

    if [ -e $1 ]; then
        BUILD_STAMP=`stat -c %Y $1 2>/dev/null`
    fi

    if [ $BUILD_STAMP -lt $SOURCE_STAMP ]; then
        # source changed require rebuild
        return 0
    else
        # source not changed don't need to rebuild
        return 1
    fi
}

clean_package()
{
    echo "Cleaning $1"
    
    case $1 in
    toolchain)
        rm -rf $CROSSTOOL
        rm -f $CROSSTOOL.buildstamp
        ;;
    busybox)
        rm -rf $BUSYBOX
        rm -f $BUSYBOX.buildstamp
        ;;
    libungif)
        rm -rf $LIBUNGIF
        rm -f $LIBUNGIF.buildstamp
        ;;
    dosfstools)
        rm -rf $DOSFSTOOLS
        rm -f $DOSFSTOOLS.buildstamp
        ;;
    strace)
        rm -rf $STRACE
        rm -f $STRACE.buildstamp
        ;;
    ppp)
        rm -rf $PPP
        rm -f $PPP.buildstamp
        ;;
    fbv)
        rm -rf $FBV
        rm -f $FBV.buildstamp
        ;;
    wu-ftpd)
        rm -rf $WUFTPD
        rm -f $WUFTPD.buildstamp
        ;;
    samba)
        rm -rf $SAMBA
        rm -rf host/$SAMBA
        rm -f $SAMBA.buildstamp
        ;;
    armioctl)
        rm -rf armioctl
        rm -f armioctl.buildstamp
        ;;
    tat)
        rm -rf tat
        rm -f tat.buildstamp
        ;;
    prelink)
        rm -rf $LIBELF
        rm -rf $PRELINK
        rm -f $PRELINK.buildstamp
        ;;
    sqlite)
        rm -rf $SQLITE
        rm -f $SQLITE.buildstamp
        ;;
    expat)
        rm -rf $EXPAT
        rm -f $EXPAT.buildstamp
        ;;
    dbus)
        rm -rf $DBUS
        rm -f $DBUS.buildstamp
        ;;
    bluez-libs)
        rm -rf $BLUEZLIBS
        rm -f $BLUEZLIBS.buildstamp
        ;;
    bluez-utils)
        rm -rf $BLUEZUTILS
        rm -f $BLUEZUTILS.buildstamp
        ;;
    bluez-hcidump)
        rm -rf $BLUEZHCIDUMP
        rm -f $BLUEZHCIDUMP.buildstamp
        ;;
    bluez-firmware)
        rm -rf $BLUEZFIRMWARE
        rm -f $BLUEZFIRMWARE.buildstamp
        ;;
    initrd)
        rm -rf initrd
        rm -f initrd.buildstamp
        ;;
    linux)
        rm -rf $GPH_LINUX
        rm -f $GPH_LINUX.buildstamp
        ;;
    basefiles)
        ;;
    *)
        die "Unknown package $1"
        ;;
    esac
}

download_package()
{
    echo "Downloading $1"

    case $1 in
    toolchain)
        wget -nc -q $PACKAGE_LOCATION/$CROSSTOOL.tar.gz || die "downloading toolchain/$CROSSTOOL"
        for file in $BINUTILS $GCCSTRAP $GCC $GDB $GLIBC $LINUXTHREADS $LINUX; do
            wget -nc -q $PACKAGE_LOCATION/$file.tar.bz2 || die "downloading toolchain/$file"
        done
        ;;
    busybox)
        wget -nc -q $PACKAGE_LOCATION/$BUSYBOX.tar.bz2 || die "downloading $BUSYBOX"
        ;;
    libungif)
        wget -nc -q $PACKAGE_LOCATION/$LIBUNGIF.tar.bz2 || die "downloading $LIBUNGIF"
        ;;
    dosfstools)
        wget -nc -q $PACKAGE_LOCATION/$DOSFSTOOLS.src.tar.gz || die "downloading $DOSFSTOOLS"
        ;;
    strace)
        wget -nc -q $PACKAGE_LOCATION/$STRACE.tar.bz2 || die "downloading $STRACE"
        ;;
    ppp)
        wget -nc -q $PACKAGE_LOCATION/$PPP.tar.gz || die "downloading $PPP"
        ;;
    fbv)
        wget -nc -q $PACKAGE_LOCATION/$FBV.tar.gz || die "downloading $FBV"
        ;;
    wu-ftpd)
        wget -nc -q $PACKAGE_LOCATION/$WUFTPD.tar.gz || die "downloading $WUFTPD"
        ;;
    samba)
        wget -nc -q $PACKAGE_LOCATION/$SAMBA.tar.gz || die "downloading $SAMBA"
        ;;
    armioctl)
        ;;
    tat)
        ;;
    prelink)
        wget -nc -q $PACKAGE_LOCATION/$LIBELF.tar.gz || die "downloading prelink/$LIBELF"
        wget -nc -q $PACKAGE_LOCATION/$PRELINK.orig.tar.gz || die "downloading $PRELINK"
        ;;
    sqlite)
        wget -nc -q $PACKAGE_LOCATION/$SQLITE.tar.gz || die "downloading $SQLITE"
        ;;
    expat)
        wget -nc -q $PACKAGE_LOCATION/$EXPAT.tar.gz || die "downloading $EXPAT"
        ;;
    dbus)
        wget -nc -q $PACKAGE_LOCATION/$DBUS.tar.gz || die "downloading $DBUS"
        ;;
    bluez-libs)
        wget -nc -q $PACKAGE_LOCATION/$BLUEZLIBS.tar.gz || die "downloading $BLUEZLIBS"
        ;;
    bluez-utils)
        wget -nc -q $PACKAGE_LOCATION/$BLUEZUTILS.tar.gz || die "downloading $BLUEZUTILS"
        ;;
    bluez-hcidump)
        wget -nc -q $PACKAGE_LOCATION/$BLUEZHCIDUMP.tar.gz || die "downloading $BLUEZHCIDUMP"
        ;;
    bluez-firmware)
        wget -nc -q $PACKAGE_LOCATION/$BLUEZFIRMWARE.tar.gz || die "downloading $BLUEZFIRMWARE"
        ;;
    initrd)
        wget -nc -q $PACKAGE_LOCATION/$BUSYBOX.tar.bz2 || die "downloading $BUSYBOX"
        ;;
    linux)
        wget -nc -q $PACKAGE_LOCATION/$GPH_LINUX.tar.bz2 || die "downloading $GPH_LINUX"
        ;;
    basefiles)
        ;;
    *)
        die "Unknown package $1"
        ;;
    esac
}

build_package()
{
    # download 3rd party source packages if required
    download_package $1

    echo "Building $1"
    case $1 in
    toolchain)
        source_changed $ROOTFS_SOURCE_PATH/toolchain $CROSSTOOL.buildstamp || return 0

        rm -rf $CROSSTOOL

        tar -xzf $CROSSTOOL.tar.gz || die "toolchain extract"
        cp -a $ROOTFS_SOURCE_PATH/toolchain/* $CROSSTOOL ||
            die "toolcahin patch"

        cd $CROSSTOOL
        TEAMBUILDER=0 ./trolltech-greenphone.sh 2>&1 > /dev/null || die "toolchain build"
        cd ..
        
        touch $CROSSTOOL.buildstamp
        ;;
    busybox)
        source_changed $ROOTFS_SOURCE_PATH/busybox $BUSYBOX.buildstamp || return 0

        rm -rf $BUSYBOX

        tar -xjf $BUSYBOX.tar.bz2 || die "busybox extract"

        cd $BUSYBOX
        for i in $ROOTFS_SOURCE_PATH/busybox/*.patch; do
            patch -p 1 < $i || die "busybox patch"
        done
        cp $ROOTFS_SOURCE_PATH/busybox/config-greenphone .config ||
            die "busybox configure"
        make >$LOGFILE 2>&1 || die "busybox build"
        cd ..

        touch $BUSYBOX.buildstamp
        ;;
    libungif)
        source_changed $LIBUNGIF.buildstamp || return 0

        tar -xjf $LIBUNGIF.tar.bz2 || die "libungif extract"

        cd $LIBUNGIF
        ./configure --prefix=/usr --host=arm-linux ||
            die "libungif configure"
        make >$LOGFILE 2>&1 || die "libungif build"
        cd ..

        touch $LIBUNGIF.buildstamp
        ;;
    dosfstools)
        source_changed $DOSFSTOOLS.buildstamp || return 0

        tar -xzf $DOSFSTOOLS.src.tar.gz || die "dosfstools extract"

        cd $DOSFSTOOLS
        make CC=arm-linux-gcc >$LOGFILE 2>&1 || die "dosfstools build"
        cd ..

        touch $DOSFSTOOLS.buildstamp
        ;;
    strace)
        source_changed $ROOTFS_SOURCE_PATH/strace $STRACE.buildstamp || return 0

        tar -xjf $STRACE.tar.bz2 || die "strace extract"

        cd $STRACE
        for i in $ROOTFS_SOURCE_PATH/strace/*.patch; do
            patch -p 1 < $i || die "strace patch"
        done
        ./configure --prefix=/usr --host=arm-linux ||
            die "strace configure"
        make >$LOGFILE 2>&1 || die "strace build"
        cd ..

        touch $STRACE.buildstamp
        ;;
    ppp)
        source_changed $ROOTFS_SOURCE_PATH/ppp $PPP.buildstamp || return 0

        tar -xzf $PPP.tar.gz || die "ppp extract"

        cd $PPP
        for i in $ROOTFS_SOURCE_PATH/ppp/*.patch; do
            patch -p 1 < $i || die "ppp patch"
        done
        ./configure --prefix=/usr || die "ppp configure"
        make CC=arm-linux-gcc >$LOGFILE 2>&1 || die "ppp build"
        cd ..

        touch $PPP.buildstamp
        ;;
    fbv)
        source_changed $FBV.buildstamp || return 0
        
        rm -rf $FBV

        tar -xzf $FBV.tar.gz || die "fbv extract"

        cd $FBV
        ./configure --prefix=/usr --without-libjpeg \
                                  --without-libpng \
                                  --without-bmp || die "fbv configure"
        make CC=arm-linux-gcc >$LOGFILE 2>&1 || die "fbv build"
        cd ..

        touch $FBV.buildstamp
        ;;
    wu-ftpd)
        source_changed $ROOTFS_SOURCE_PATH/wu-ftpd $WUFTPD.buildstamp || return 0

        rm -rf $WUFTPD

        tar -xzf $WUFTPD.tar.gz || die "wu-ftpd extract"

        cd $WUFTPD
        for i in $ROOTFS_SOURCE_PATH/wu-ftpd/*.patch; do
            patch -p 1 < $i || die "wu-ftpd patch"
        done
        ./configure --prefix=/ --host=arm-linux --enable-anononly \
                               --disable-dns --disable-mail ||
            die "wu-ftpd configure"
        make >$LOGFILE 2>&1 || die "wu-ftpd build"
        cd ..

        touch $WUFTPD.buildstamp
        ;;
    samba)
        source_changed $SAMBA.buildstamp || return 0

        rm -rf $SAMBA

        tar -xzf $SAMBA.tar.gz || die "samba extract"

        cd $SAMBA/source
        CC=arm-linux-gcc ./configure --host=arm-linux || die "samba configure"
        echo '#define HAVE_GETTIMEOFDAY_TZ 1' >> include/config.h
        echo '#define uint32 unsigned int' >> include/config.h
        echo '#define int32 int' >> include/config.h
        make >$LOGFILE 2>&1 || die "samba build"
        cd ../..

        # host tools
        rm -rf host/$SAMBA

        mkdir -p host
        tar -xzf $SAMBA.tar.gz -C host || die "samba/make_unicodemap host extract"

        cd host/$SAMBA/source
        ./configure || die "samba/make_unicodemap configure"
        make bin/make_unicodemap >$LOGFILE 2>&1 || die "samba/make_unicodemap build"
        cd ../../..

        touch $SAMBA.buildstamp
        ;;
    armioctl)
        source_changed $ROOTFS_SOURCE_PATH/armioctl armioctl.buildstamp || return 0

        rm -rf armioctl

        mkdir armioctl
        cp $ROOTFS_SOURCE_PATH/armioctl/* armioctl/ || die "armioctl extract"

        cd armioctl
        CC=arm-linux-gcc make >$LOGFILE 2>&1 || die "armioctl build"
        cd ..

        touch armioctl.buildstamp
        ;;
    tat)
        source_changed $ROOTFS_SOURCE_PATH/tat tat.buildstamp || return 0

        rm -rf tat

        mkdir tat
        cp $ROOTFS_SOURCE_PATH/tat/* tat/ || die "tat extract"

        cd tat
        CFLAGS=-I$QTOPIA_SOURCE_PATH/devices/greenphone/include CC=arm-linux-gcc make -e >$LOGFILE 2>&1 || die "tat 
build"
        cd ..

        touch tat.buildstamp
        ;;
    prelink)
        if source_changed $LIBELF.buildstamp; then
            rm -rf $LIBELF

            tar -xzf $LIBELF.tar.gz || die "prelink/libelf extract"

            cd $LIBELF
            CC=arm-linux-gcc ./configure --host=arm-linux --prefix=/usr >/dev/null 2>/dev/null ||
                die "prelink/libelf configure"
            make >$LOGFILE 2>&1 || die "prelink/libelf build"
            cd ..

            touch $LIBELF.buildstamp
        fi

        source_changed $ROOTFS_SOURCE_PATH/prelink $PRELINK.buildstamp || return 0

        rm -rf $PRELINK

        tar -xzf $PRELINK.orig.tar.gz || die "prelink extract"

        cd $PRELINK
        for i in $ROOTFS_SOURCE_PATH/prelink/*.patch; do
            patch -p 1 < $i || die "prelink patch"
        done

        mkdir -p libelf/include/libelf libelf/lib
        for file in elf_repl.h gelf.h libelf.h nlist.h sys_elf.h; do
            cp ../$LIBELF/lib/$file libelf/include/libelf/ || die "prelink libelf include"
        done
        cp ../$LIBELF/lib/libelf.a libelf/lib/ || die "prelink libelf install"

        export CFLAGS="-I`pwd`/libelf/include -I`pwd`/libelf/include/libelf"
        export CPPFLAGS=$CFLAGS
        export LDFLAGS="-L`pwd`/libelf/lib"
        ./configure --host=arm-linux --prefix=/usr >/dev/null 2>/dev/null ||
            die "prelink configure"

        # src/execstack fails to build for arm
        # force make to continue anyway and test
        # if src/prelink has been built.
        make -k >$LOGFILE 2>&1
        [ -e src/prelink ] || die "prelink build"

        unset LDFLAGS
        unset CPPFLAGS
        unset CFLAGS
        cd ..
        
        touch $PRELINK.buildstamp
        ;;
    sqlite)
        source_changed $ROOTFS_SOURCE_PATH/sqlite $SQLITE.buildstamp || return 0

        rm -rf $SQLITE

        tar -xzf $SQLITE.tar.gz || die "sqlite extract"

        cd $SQLITE
        for i in $ROOTFS_SOURCE_PATH/sqlite/*.patch; do
            patch -p 1 < $i || die "sqlite patch"
        done
        mkdir build
        cd build
        make -f ../Makefile.linux-gcc TOP=.. sqlite3 >$LOGFILE 2>&1 || die "sqlite build"
        cd ../..

        touch $SQLITE.buildstamp
        ;;
    expat)
        source_changed $EXPAT.buildstamp || return 0

        rm -rf $EXPAT

        tar -xzf $EXPAT.tar.gz || die "expat extract"

        cd $EXPAT
        ./configure --host=arm-linux --prefix=/usr || die "expat configure"
        make >$LOGFILE 2>&1 || die "expat build"
        cd ..

        touch $EXPAT.buildstamp
        ;;
    dbus)
        source_changed $ROOTFS_SOURCE_PATH/dbus $DBUS.buildstamp || return 0

        rm -rf $DBUS

        tar -xzf $DBUS.tar.gz || die "dbus extract"

        cd $DBUS
        for i in $ROOTFS_SOURCE_PATH/dbus/*.patch; do
            patch -p 1 < $i || die "dbus patch"
        done
        autoreconf

        export CFLAGS="-I$ROOTFS_BUILD_PATH/$EXPAT/lib"
        export LDFLAGS="-L$ROOTFS_BUILD_PATH/$EXPAT/.libs"

        ./configure --host=arm-linux --prefix=/usr --sysconfdir=/etc \
                    --localstatedir=/var --cache-file=config.cache \
                    --disable-qt --disable-qt3 --disable-gtk --disable-glib \
                    --disable-gcj --disable-mono --disable-python \
                    --disable-selinux \
                    --with-xml=expat --without-x >/dev/null 2>/dev/null ||
            die "dbus configure"

        make >$LOGFILE 2>&1 || die "dbus build"

        unset CFLAGS LDFLAGS

        cd ..

        touch $DBUS.buildstamp
        ;;
    bluez-libs)
        source_changed $BLUEZLIBS.buildstamp || return 0

        rm -rf $BLUEZLIBS

        tar -xzf $BLUEZLIBS.tar.gz || die "bluez-libs extract"

        cd $BLUEZLIBS
        ./configure --host=arm-linux --prefix=/usr >/dev/null 2>/dev/null ||
            die "bluez-libs configure"
        make >$LOGFILE 2>&1 || die "bluez-libs build"
        cd ..

        touch $BLUEZLIBS.buildstamp
        ;;
    bluez-utils)
        source_changed $BLUEZUTILS.buildstamp || return 0

        rm -rf $BLUEZUTILS

        tar -xzf $BLUEZUTILS.tar.gz || die "bluez-utils extract"

        cd $BLUEZUTILS
        export BLUEZ_LIBS="-L$ROOTFS_BUILD_PATH/$BLUEZLIBS/src/.libs -lbluetooth"
        export BLUEZ_CFLAGS="-I$ROOTFS_BUILD_PATH/$BLUEZLIBS/include"
        export DBUS_CFLAGS="-I$ROOTFS_BUILD_PATH/$DBUS"
        export DBUS_LIBS="-L$ROOTFS_BUILD_PATH/$DBUS/dbus/.libs -ldbus-1"

        ./configure --host=arm-linux --prefix=/usr --sysconfdir=/etc \
                    --localstatedir=/mnt/user \
                    --enable-test \
                    --disable-glib --disable-obex --disable-alsa \
                    --disable-bccmd --disable-avctrl --disable-hid2hci \
                    --disable-dfutool --without-cups >/dev/null 2>/dev/null ||
            die "bluez-utils configure"

        make >$LOGFILE 2>&1 || die "bluez-libs build"

        unset BLUEZ_LIBS BLUEZ_CFLAGS DBUS_LIBS DBUS_CFLAGS

        cd ..

        touch $BLUEZUTILS.buildstamp
        ;;
    bluez-hcidump)
        source_changed $BLUEZHCIDUMP.buildstamp || return 0

        rm -rf $BLUEZHCIDUMP

        tar -xzf $BLUEZHCIDUMP.tar.gz || die "bluez-hcidump extract"

        cd $BLUEZHCIDUMP
        export BLUEZ_LIBS="-L$ROOTFS_BUILD_PATH/$BLUEZLIBS/src/.libs -lbluetooth"
        export BLUEZ_CFLAGS="-I$ROOTFS_BUILD_PATH/$BLUEZLIBS/include"

        ./configure --host=arm-linux --prefix=/usr >/dev/null 2>/dev/null ||
            die "bluez-hcidump configure"
        make >$LOGFILE 2>&1 || die "bluez-hcidump build"

        unset BLUEZ_LIBS BLUEZ_CFLAGS

        cd ..

        touch $BLUEZHCIDUMP.buildstamp
        ;;
    bluez-firmware)
        source_changed $BLUEZFIRMWARE.buildstamp || return 0

        rm -rf $BLUEZFIRMWARE

        tar -xzf $BLUEZFIRMWARE.tar.gz || die "bluez-firmware extract"

        cd $BLUEZFIRMWARE
        ./configure --host=arm-linux --prefix=/usr >/dev/null 2>/dev/null ||
            die "bluez-firmware configure"
        make >$LOGFILE 2>&1 || die "bluez-firmware build"
        cd ..

        touch $BLUEZFIRMWARE.buildstamp
        ;;
    initrd)
        source_changed $ROOTFS_SOURCE_PATH/initrd initrd.buildstamp || return 0

        rm -rf initrd

        mkdir initrd

        # insmod for initrd
        tar -C initrd -xjf $BUSYBOX.tar.bz2 || die "initrd busybox extract"

        cd initrd/$BUSYBOX
        for i in $ROOTFS_SOURCE_PATH/busybox/*.patch; do
            patch -p 1 < $i || die "initrd busybox patch"
        done
        cp $ROOTFS_SOURCE_PATH/initrd/busybox-config .config ||
            die "initrd busybox configure"
        make oldconfig >$LOGFILE 2>&1 || die "initrd busybox configure"
        make >$LOGFILE 2>&1 || die "initrd busybox build"
        cd ../..

        # linuxrc for initrd
        cd initrd
        cp $ROOTFS_SOURCE_PATH/initrd/linuxrc.c $ROOTFS_SOURCE_PATH/initrd/Makefile .
        make CROSS=arm-linux linuxrc >$LOGFILE 2>&1 || die "build initrd linuxrc"
        cd ..

        # make initrd image
        cd initrd
        dd if=/dev/zero of=$ROOTFS_BUILD_PATH/$INITRD_FILENAME bs=1024 seek=1024 count=0
        /sbin/mkfs.ext2 -F $ROOTFS_BUILD_PATH/$INITRD_FILENAME
        MNTPNT=`mktemp -d initrd.XXXXXX`
        sudo mount -o loop $ROOTFS_BUILD_PATH/$INITRD_FILENAME $MNTPNT
        
        sudo mkdir -p $MNTPNT/dev
        sudo mknod $MNTPNT/dev/console c 5 1
        sudo cp -a linuxrc $MNTPNT/
        sudo cp -a $ROOTFS_SOURCE_PATH/initrd/tffs.o $MNTPNT/
        sudo umount $MNTPNT
        rmdir $MNTPNT
        cd ..

        touch initrd.buildstamp
        ;;
    linux)
        source_changed $ROOTFS_SOURCE_PATH/linux $GPH_LINUX.buildstamp || return 0

        rm -rf $GPH_LINUX

        tar -xjf $GPH_LINUX.tar.bz2 || die "linux extract"

        cd $GPH_LINUX

        chmod u+w init/initrd.bin
        cp $ROOTFS_BUILD_PATH/$INITRD_FILENAME init/initrd.bin || die "linux initrd image"

#        make greenphone_config >$LOGFILE 2>&1 || die "linux greenphone_config"
        cp $ROOTFS_SOURCE_PATH/linux/config .config || die "linux greenphone_config"
        make oldconfig >$LOGFILE 2>&1 || die "linux config"
        
        if [ $OPTION_CONFIG_LINUX -eq 1 ]; then
            make menuconfig
        fi

        make dep >$LOGFILE 2>&1 || die "linux build dep"
        make zImage >$LOGFILE 2>&1 || die "linux build"
        make modules >$LOGFILE 2>&1 || die "linux modules build"

        cd ..

        touch $GPH_LINUX.buildstamp
        ;;
    basefiles)
        ;;
    *)
        die "Unknown package $1"
        ;;
    esac
}

install_package()
{
    echo "Installing $1"

    case $1 in
    toolchain)
        TOOLCHAIN_ROOT=$RESULT_TOP/gcc-4.1.1-glibc-2.3.6/arm-linux/arm-linux
        LIB_LINKS=`find $TOOLCHAIN_ROOT/lib -type l -printf "%P\n"`
        LIBS=`echo $LIB_LINKS | tr ' ' '\n' | xargs -iXFILEX ls -l $TOOLCHAIN_ROOT/lib/XFILEX | awk '{print $NF}'`
        strip_install unneeded $TOOLCHAIN_ROOT/lib $ROOTFS_IMAGE_DIR/lib $LIBS $LIB_LINKS
        strip_install unneeded $TOOLCHAIN_ROOT/lib $ROOTFS_IMAGE_DIR/lib libSegFault.so libmemusage.so libpcprofile.so

        strip_install all $TOOLCHAIN_ROOT/../bin $ROOTFS_IMAGE_DIR/usr/bin gdbserver 
        strip_install all $ROOTFS_BUILD_PATH/$CROSSTOOL/build/arm-linux/gcc-4.1.1-glibc-2.3.6/build-glibc/elf $ROOTFS_IMAGE_DIR/sbin ldconfig
        install $ROOTFS_BUILD_PATH/$CROSSTOOL/build/arm-linux/gcc-4.1.1-glibc-2.3.6/build-glibc/elf $ROOTFS_IMAGE_DIR/sbin ldd

        install $TOOLCHAIN_ROOT/etc $ROOTFS_IMAGE_DIR/etc rpc
        ;;
    busybox)
        cd $BUSYBOX
        make install PREFIX=$ROOTFS_IMAGE_DIR >$LOGFILE 2>&1
        cd ..
        ;;
    libungif)
        cd $LIBUNGIF
        strip_install unneeded lib/.libs $ROOTFS_IMAGE_DIR/usr/lib libungif.so.4 libungif.so.4.1.4
        cd ..
        ;;
    dosfstools)
        cd $DOSFSTOOLS
        strip_install all dosfsck $ROOTFS_IMAGE_DIR/sbin dosfsck
        strip_install all mkdosfs $ROOTFS_IMAGE_DIR/sbin mkdosfs
        cd ..
        ;;
    strace)
        cd $STRACE
        strip_install all . $ROOTFS_IMAGE_DIR/usr/bin strace
        cd ..
        ;;
    ppp)
        cd $PPP
        strip_install all pppdump $ROOTFS_IMAGE_DIR/usr/bin pppdump
        strip_install all pppstats $ROOTFS_IMAGE_DIR/usr/bin pppstats
        strip_install all chat $ROOTFS_IMAGE_DIR/usr/sbin chat
        strip_install all pppd $ROOTFS_IMAGE_DIR/usr/sbin pppd
        install etc.ppp $USERFS_IMAGE_DIR/etc/ppp chap-secrets options pap-secrets
        ln -s /mnt/user/etc/ppp $ROOTFS_IMAGE_DIR/etc/ppp
        cd ..
        ;;
    fbv)
        strip_install all $FBV $ROOTFS_IMAGE_DIR/usr/bin fbv
        ;;
    wu-ftpd)
        strip_install all $WUFTPD/src $ROOTFS_IMAGE_DIR/usr/sbin ftpd
        ;;
    samba)
        cd $SAMBA/source
        mkdir -p $ROOTFS_IMAGE_DIR/usr/local/samba/bin
        mkdir -p $ROOTFS_IMAGE_DIR/usr/local/samba/lib
        mkdir -p $ROOTFS_IMAGE_DIR/usr/local/samba/lib/codepages
        mkdir -p $ROOTFS_IMAGE_DIR/var/run/samba
        ln -s /var/run/samba $ROOTFS_IMAGE_DIR/usr/local/samba/var

        strip_install all bin $ROOTFS_IMAGE_DIR/usr/local/samba/bin smbd nmbd

        mkdir -p $USERFS_IMAGE_DIR/etc/samba
        mkdir -p $ROOTFS_IMAGE_DIR/usr/local/samba/private
        ln -s /mnt/user/etc/samba/secrets.tdb $ROOTFS_IMAGE_DIR/usr/local/samba/private/secrets.tdb
        cd ../..

        cd host/$SAMBA/source
        bin/make_unicodemap 932 codepages/CP932.TXT $ROOTFS_IMAGE_DIR/usr/local/samba/lib/codepages/unicode_map.932
        cd ../../..
        ;;
    armioctl)
        strip_install all armioctl $USERFS_IMAGE_DIR/tools arm_ioctl
        ;;
    tat)
        strip_install all tat $USERFS_IMAGE_DIR/tools tat
        ;;
    prelink)
        strip_install all $PRELINK/src $ROOTFS_IMAGE_DIR/usr/sbin prelink
        ;;
    sqlite)
        strip_install all $SQLITE/build $ROOTFS_IMAGE_DIR/usr/bin sqlite3
        ;;
    expat)
        cd $EXPAT
        strip_install unneeded .libs $ROOTFS_IMAGE_DIR/usr/lib libexpat.so.1 libexpat.so.1.5.0
        cd ..
        ;;
    dbus)
        cd $DBUS
        make DESTDIR=`pwd`/.install install >$LOGFILE 2>&1

        mkdir -p $ROOTFS_IMAGE_DIR/var/run/dbus

        install .install/etc/dbus-1 $ROOTFS_IMAGE_DIR/etc/dbus-1 session.conf system.conf
        strip_install all .install/usr/bin $ROOTFS_IMAGE_DIR/usr/bin dbus-cleanup-sockets dbus-launch dbus-send
        strip_install all bus/.libs $ROOTFS_IMAGE_DIR/usr/bin dbus-daemon
        strip_install unneeded .install/usr/lib $ROOTFS_IMAGE_DIR/usr/lib libdbus-1.so.3 libdbus-1.so.3.0.0
        cd ..
        ;;
    bluez-libs)
        cd $BLUEZLIBS
        strip_install unneeded src/.libs $ROOTFS_IMAGE_DIR/usr/lib libbluetooth.so.2 libbluetooth.so.2.4.1
        cd ..
        ;;
    bluez-utils)
        cd $BLUEZUTILS
        make DESTDIR=`pwd`/.install install >$LOGFILE 2>&1

        mkdir -p $ROOTFS_IMAGE_DIR/var/lib/bluetooth

        install .install/etc/bluetooth $ROOTFS_IMAGE_DIR/etc/bluetooth hcid.conf rfcomm.conf
        install .install/etc/dbus-1/system.d $ROOTFS_IMAGE_DIR/etc/dbus-1/system.d bluez-hcid.conf
        strip_install all .install/usr/bin $ROOTFS_IMAGE_DIR/usr/bin ciptool dund hcitool hidd l2ping pand rfcomm sdptool
        strip_install all .install/usr/sbin $ROOTFS_IMAGE_DIR/usr/sbin hciattach hciconfig hcid sdpd
        strip_install all test/.libs $ROOTFS_IMAGE_DIR/usr/sbin bdaddr
        cd ..
        ;;
    bluez-hcidump)
        cd $BLUEZHCIDUMP
        strip_install all src $ROOTFS_IMAGE_DIR/usr/sbin hcidump
        cd ..
        ;;
    bluez-firmware)
        cd $BLUEZFIRMWARE
        install st $ROOTFS_IMAGE_DIR/usr/lib/firmware STLC2500_R4_00_03.ptc STLC2500_R4_00_06.ssf STLC2500_R4_02_02_WLAN.ssf STLC2500_R4_02_04.ptc
        ln -s /usr/lib/firmware $ROOTFS_IMAGE_DIR/lib/firmware
        ln -s STLC2500_R4_02_02_WLAN.ssf $ROOTFS_IMAGE_DIR/usr/lib/firmware/STLC2500_R0_01_02.ssf
        ln -s STLC2500_R4_02_04.ptc $ROOTFS_IMAGE_DIR/usr/lib/firmware/STLC2500_R0_01_04.ptc
        cd ..
        ;;
    initrd)
        ;;
    linux)
        cd $GPH_LINUX
        make MODLIB=$ROOTFS_IMAGE_DIR/lib/modules DEPMOD=true modules_install >$LOGFILE 2>&1
        rm -f $ROOTFS_IMAGE_DIR/lib/modules/build

        cp arch/arm/boot/zImage $ROOTFS_BUILD_PATH/$KERNEL_FILENAME

        # Check if kernel will fit in allocated flash space
        if [ ! -e $ROOTFS_BUILD_PATH/$KERNEL_FILENAME ]; then
            die "linux install kernel doesn't exist."
        fi

        KERNEL_SIZE=`stat -c '%s' $ROOTFS_BUILD_PATH/$KERNEL_FILENAME`

        [ $KERNEL_SIZE -le $KERNEL_SIZELIMIT ] ||
            die "linux install kernel larger than allocated flash size. $KERNEL_SIZE > $KERNEL_SIZELIMIT."

        cd ..
        ;;
    basefiles)
        install_basefiles || die "Basefiles install"
        ;;
    *)
        die "Unknown package $package"
        ;;
    esac
}

install_basefiles()
{
    cd $ROOTFS_IMAGE_DIR
    mkdir -p mnt/disk2 mnt/nfs mnt/sd mnt/user mnt/user_local
    mkdir -p proc opt/Qtopia.rom
    mkdir -p lib/modules etc
    mkdir -p var var/log var/lock/subsys var/mnt var/run var/spool/at var/tmp

    ln -s /mnt/disk2/Qtopia opt/Qtopia
    ln -s Qtopia opt/Qtopia.user
    ln -s var/tmp tmp


    # Create minimal /dev
    mkdir -p $ROOTFS_IMAGE_DIR/dev
    cd $ROOTFS_IMAGE_DIR/dev

    sudo mknod initctl p

    sudo mknod mem     c 1 1
    sudo mknod kmem    c 1 2
    sudo mknod null    c 1 3
    sudo mknod zero    c 1 5
    sudo mknod random  c 1 8
    sudo mknod urandom c 1 9

    sudo mknod ttyS0 c 4 64
    sudo mknod ttyS1 c 4 65
    sudo mknod ttyS2 c 4 66

    sudo mknod tty     c 5 0
    sudo mknod console c 5 1

    sudo mknod fb0 c 29 0
    sudo ln -s fb0 fb

    sudo mknod tffsa  b 100  0
    sudo mknod tffsa1 b 100  1
    sudo mknod tffsa2 b 100  2
    sudo mknod tffsb  b 100 32
    sudo mknod tffsc  b 100 64
    sudo mknod tffsd  b 100 96
    sudo ln -s tffsa1 root

    sudo mknod mmca  b 241 0
    sudo mknod mmca1 b 241 1
    sudo mknod mmca2 b 241 2


    # Create full /dev
    cd $DEVFS_IMAGE_DIR

    mkdir -p pts

    sudo mknod initctl p

    sudo mknod mem c 1 1
    sudo mknod kmem c 1 2
    sudo mknod null c 1 3
    sudo mknod zero c 1 5
    sudo mknod random c 1 8
    sudo mknod urandom c 1 9

    sudo mknod ptyp0 c 2 0

    sudo mknod ttyp0 c 3 0

    sudo mknod tty0 c 4 0
    sudo mknod tty1 c 4 1
    sudo mknod tty2 c 4 2
    sudo mknod tty3 c 4 3
    sudo mknod tty4 c 4 4
    sudo mknod tty5 c 4 5
    sudo mknod tty6 c 4 6
    sudo mknod tty7 c 4 7
    sudo mknod tty8 c 4 8

    sudo mknod ttyS0 c 4 64
    sudo mknod ttyS1 c 4 65
    sudo mknod ttyS2 c 4 66

    sudo mknod tty c 5 0
    sudo mknod console c 5 1
    sudo mknod ptmx c 5 2

    sudo mknod ts c 10 17
    sudo mknod imm c 10 63
    sudo mknod dpmc c 10 90
    sudo mknod ipmc c 10 90
    sudo mknod watchdog c 10 130

    sudo mknod mixer0 c 14 0
    sudo mknod sequencer c 14 1
    sudo mknod midi00 c 14 2
    sudo mknod dsp0 c 14 3
    sudo mknod audio0 c 14 4
    sudo mknod dspW0 c 14 5
    sudo mknod sndstat c 14 6
    sudo mknod dmfm0 c 14 7
    sudo mknod mixer1 c 14 16
    sudo mknod midi01 c 14 18
    sudo mknod dsp1 c 14 19
    sudo mknod audio1 c 14 20
    sudo mknod dspW1 c 14 21
    sudo mknod mixer2 c 14 32
    sudo mknod midi02 c 14 34
    sudo mknod dsp2 c 14 35
    sudo mknod audio2 c 14 36
    sudo mknod dspW2 c 14 37
    sudo mknod mixer3 c 14 48
    sudo mknod midi03 c 14 50
    sudo mknod dsp3 c 14 51
    sudo mknod audio3 c 14 52
    sudo mknod dspW3 c 14 53
    sudo ln -s audio0 audio
    sudo ln -s mixer0 audioctl
    sudo ln -s dmfm0 dmfm
    sudo ln -s dsp0 dsp
    sudo ln -s dspW0 dspW
    sudo ln -s dsp0 dspdefault
    sudo ln -s midi00 midi
    sudo ln -s mixer0 mixer

    sudo mknod fb0 c 29 0
    sudo mknod fb1 c 29 32
    sudo mknod fb2 c 29 64
    sudo mknod fb3 c 29 96
    sudo mknod fb4 c 29 128
    sudo mknod fb5 c 29 160
    sudo mknod fb6 c 29 192
    sudo mknod fb7 c 29 224
    sudo ln -s fb0 fb

    sudo mknod ttyP0 c 57 0
    sudo mknod ttyP1 c 57 1
    sudo mknod ttyP2 c 57 2
    sudo mknod ttyP3 c 57 3

    sudo mknod ixs_oscr c 59 0

    sudo mknod video0 c 81 0
    sudo ln -s video0 video

    sudo mknod ppp c 108 0

    sudo mknod ttyUSB0 c 188 0

    for i in {0..31}; do
        sudo mknod rfcomm$i c 216 $i
    done

    sudo mknod omega_bt c 250 0
    sudo mknod omega_vibrator c 57 0
    sudo mknod omega_detect c 102 0
    sudo mknod omega_alarm c 120 0
    sudo mknod omega_rtcalarm c 121 0
    sudo mknod omega_kpbl c 127 0
    sudo mknod omega_bcm2121 c 200 0
    sudo mknod IPMC_FORCE_SLEEP c 201 0
    sudo mknod omega_chgled c 251 0
    sudo mknod sdcard_pm c 252 0
    sudo mknod camera c 253 0
    sudo mknod lcdctrl c 254 0
    sudo mknod omega_lcdctrl c 254 0

    sudo mknod loop0 b 7 0
    sudo mknod loop1 b 7 1
    sudo mknod loop2 b 7 2
    sudo mknod loop3 b 7 3
    sudo mknod loop4 b 7 4
    sudo mknod loop5 b 7 5
    sudo mknod loop6 b 7 6
    sudo mknod loop7 b 7 7

    sudo mknod docparatable b 58 1
    
    sudo mknod flh0 b 60 0
    sudo mknod flh1 b 60 1
    sudo mknod flh2 b 60 2
    sudo mknod flh3 b 60 3
    
    sudo mknod mmca  b 241 0
    sudo mknod mmca1 b 241 1
    sudo mknod mmca2 b 241 2

    sudo mknod ram0 b 1 0
    sudo mknod ram1 b 1 1
    sudo mknod ram2 b 1 2
    sudo mknod ram3 b 1 3

    sudo mknod tffsa  b 100 0
    sudo mknod tffsa1 b 100 1
    sudo mknod tffsa2 b 100 2
    sudo mknod tffsb  b 100 32
    sudo mknod tffsc  b 100 64
    sudo mknod tffsd  b 100 96
    sudo ln -s tffsa1 root

    # Install oui.txt required for Bluetooth
    cd $ROOTFS_IMAGE_DIR
    mkdir -p usr/share/misc/
    cd $ROOTFS_IMAGE_DIR/usr/share/misc
    wget -nc -q http://standards.ieee.org/regauth/oui/oui.txt

    # Greenphone base files
    files=`find $ROOTFS_SOURCE_PATH/basefiles -mindepth 1 -maxdepth 1 2>/dev/null`
    for i in $files; do
        if [ `basename $i` != "mnt" ]; then
            sudo cp -a $i $ROOTFS_IMAGE_DIR
        fi
    done
    ln -s . $ROOTFS_IMAGE_DIR/lib/modules/2.4.19-rmk7-pxa2-greenphone

    files=`find $ROOTFS_SOURCE_PATH/basefiles/mnt -mindepth 1 -maxdepth 1 -name user -prune -o -print 2>/dev/null`
    for i in $files; do
        sudo cp -a $i $ROOTFS_IMAGE_DIR/mnt
    done

    files=`find $ROOTFS_SOURCE_PATH/basefiles/mnt/user -mindepth 1 -maxdepth 1 2>/dev/null`
    for i in $files; do
        sudo cp -a $i $USERFS_IMAGE_DIR
    done


    # Setup default home directory
    mkdir -p $ROOTFS_IMAGE_DIR/mnt/disk2/home
    ln -s /mnt/user_local $ROOTFS_IMAGE_DIR/mnt/disk2/home/Documents

    #### FIXME: The following symlinks implement the fast boot fix. ####
    ####        See devices/greenphone/src/devtools/startup/qpe.sh. ####
    mkdir -p $ROOTFS_IMAGE_DIR/mnt/disk2/home/Applications
    ln -s /tmp/Apps_Qtopia $ROOTFS_IMAGE_DIR/mnt/disk2/home/Applications/Qtopia


    # Setup symlink between /mnt/user/etc and /etc
    mkdir -p $USERFS_IMAGE_DIR/etc/resolvconf
    files=`find $USERFS_IMAGE_DIR/etc -mindepth 1 -maxdepth 1 2>/dev/null`
    for i in $files; do
        if [ ! -L $ROOTFS_IMAGE_DIR/etc/`basename $i` ] &&
           [ ! -e $ROOTFS_IMAGE_DIR/etc/`basename $i` ]; then
            sudo ln -s /mnt/user/etc/`basename $i` $ROOTFS_IMAGE_DIR/etc
        fi
    done
    sudo ln -s /proc/mounts $ROOTFS_IMAGE_DIR/etc/mtab
    sudo ln -s /mnt/user/etc/resolv.conf $ROOTFS_IMAGE_DIR/etc

    cd ..
}

create_default_tgzs()
{
    echo "Creating default tarballs"
    sudo chown -R root.root $USERFS_IMAGE_DIR/*

    cd $USERFS_IMAGE_DIR
    sudo tar --owner=0 --group=0 -czf $ROOTFS_IMAGE_DIR/user_default.tgz etc fs.ver
    sudo tar --owner=0 --group=0 -czf $ROOTFS_IMAGE_DIR/user_tools.tgz tools

    cd $DEVFS_IMAGE_DIR
    sudo tar --owner=0 --group=0 -czf $ROOTFS_IMAGE_DIR/dev.tgz *

    cd $ROOTFS_IMAGE_DIR/var
    sudo tar --owner=0 --group=0 -czf $ROOTFS_IMAGE_DIR/var.tgz *

    cd $ROOTFS_IMAGE_DIR/mnt/disk2/home
    sudo tar --owner=0 --group=0 -czf $ROOTFS_IMAGE_DIR/home_default.tgz .

    cd $ROOTFS_BUILD_PATH
}

make_rootfs_image()
{
    echo "Creating rootfs image"

    MNTPNT=`mktemp -d rootfs.XXXXXX`
    [ -d $MNTPNT ] || die "Could not create temporary directory"

    rm -f $ROOTFS_FILENAME
    dd if=/dev/zero of=$ROOTFS_FILENAME bs=1 count=0 seek=44040192 2>/dev/null
    /sbin/mkfs.ext2 -F $ROOTFS_FILENAME >/dev/null 2>&1
    sudo mount -t ext2 -o loop $ROOTFS_FILENAME $MNTPNT

    sudo mkdir -p $MNTPNT/var

    files=`find $ROOTFS_IMAGE_DIR -mindepth 1 -maxdepth 1 2>/dev/null`
    for i in $files; do
        if [ `basename $i` != "var" ]; then
            sudo cp -a $i $MNTPNT
            sudo chown -R root.root $MNTPNT/`basename $i`
        fi
    done

    sudo umount $MNTPNT
    rmdir $MNTPNT
}


usage()
{
    echo -e "Usage: `basename $0` [--clean] [--build] [--install] [--image] [--packages=<package list>] [--qtopia-source <path>] [--source-url <url>]"
    echo -e "If no options are specified defaults to --build --install --images\n" \
            "   --clean          Clean build directory.\n" \
            "   --build          Build packages, only if not previously built.\n" \
            "   --install        Install built packages.\n" \
            "   --image          Make rootfs image.\n" \
            "   --packages=      Select packages for build/install. <package list> is a comma seperated list of packages.\n" \
            "   --config-linux   Run \"make menuconfig\" when building Linux.\n" \
            "   --qtopia-source  Location of Qtopia source tree.\n" \
            "   --source-url     Location to download source packages from.\n\n" \
            "Available packages:\n" \
            "$ALL_PACKAGES\n\n" \
            "Running without any options will build a rootfs image.\n"
}

. `dirname $0`/functions

DEFAULT_OPTIONS=1
OPTION_CLEAN=0
OPTION_BUILD=0
OPTION_INSTALL=0
OPTION_IMAGE=0
OPTION_CONFIG_LINUX=0

QTOPIA_SOURCE_PATH=$PWD
LOGFILE=$PWD/rootfs/build.log
mkdir -p $PWD/rootfs
touch $LOGFILE
echo 0>$LOGFILE

ALL_PACKAGES="toolchain busybox libungif dosfstools strace ppp fbv wu-ftpd samba armioctl tat prelink sqlite"
ALL_PACKAGES="$ALL_PACKAGES expat dbus bluez-libs bluez-utils bluez-hcidump bluez-firmware"
ALL_PACKAGES="$ALL_PACKAGES initrd linux"
ALL_PACKAGES="$ALL_PACKAGES basefiles"
PACKAGES=$ALL_PACKAGES

if [ $# -eq 0 ]; then
    usage
    exit 0
fi

while [ $# -ne 0 ]; do
    case $1 in
        --clean)
            OPTION_CLEAN=1
            DEFAULT_OPTIONS=0
            ;;
        --build)
            OPTION_BUILD=1
            DEFAULT_OPTIONS=0
            ;;
        --install)
            OPTION_INSTALL=1
            DEFAULT_OPTIONS=0
            ;;
        --image)
            OPTION_IMAGE=1
            DEFAULT_OPTIONS=0
            ;;
        --packages=*)
            PACKAGES=`echo ${1#--packages=} | tr ',' ' '`
            ;;
        --config-linux)
            OPTION_CONFIG_LINUX=1
            DEFAULT_OPTIONS=0
            ;;
        --qtopia-source)
            if [ $# -ge 2 ]; then
                QTOPIA_SOURCE_PATH="$2"
                shift 1
            else
                echo "$1 requires an argument"
                usage
                exit 1
            fi
            ;;
        --source-url)
            if [ $# -ge 2 ]; then
                PACKAGE_LOCATION="$2"
                shift 1
            else
                echo "$1 requires an argument"
                usage
                exit 1
            fi
            ;;
        --help)
            usage
            exit 0
            ;;
        *)
            echo "Unknown option: $1"
            usage
            exit 1
            ;;
    esac
    shift
done

# Default options if non specified
if [ $DEFAULT_OPTIONS -eq 1 ]; then
    OPTION_BUILD=1
    OPTION_INSTALL=1
    OPTION_IMAGE=1
fi

export PATH=/opt/teambuilder/bin:/opt/toolchains/greenphone/gcc-4.1.1-glibc-2.3.6/arm-linux/bin:$PATH
export TEAMBUILDER_CC_VERSION=4.1.1
export TEAMBUILDER=0

# Package location
[ -z $PACKAGE_LOCATION ] && PACKAGE_LOCATION=http://www.qtopia.net/greenphone/sources/rootfs/`version`

# Source and destination
ROOTFS_SOURCE_PATH=$QTOPIA_SOURCE_PATH/devices/greenphone/rootfs
if [ $QTOPIA_SOURCE_PATH = $PWD ]; then
    ROOTFS_BUILD_PATH=$QTOPIA_SOURCE_PATH/rootfs
    mkdir -p $ROOTFS_BUILD_PATH
    cd $ROOTFS_BUILD_PATH
else
    ROOTFS_BUILD_PATH=$PWD
fi

# Define locations where images will be built
ROOTFS_IMAGE_DIR=$ROOTFS_BUILD_PATH/rootfs
USERFS_IMAGE_DIR=$ROOTFS_BUILD_PATH/userfs
DEVFS_IMAGE_DIR=$ROOTFS_BUILD_PATH/devfs

# Outputs of this script
ROOTFS_FILENAME=greenphone_rootfs.ext2
KERNEL_FILENAME=greenphone_kernel
KERNEL_SIZELIMIT=1048576
INITRD_FILENAME=greenphone_initrd

# toolchain
CROSSTOOL=crosstool-0.42
export TARBALLS_DIR=$ROOTFS_BUILD_PATH
export RESULT_TOP=$ROOTFS_BUILD_PATH/$CROSSTOOL/build/opt/toolchains/greenphone
BINUTILS=binutils-2.17
GCCSTRAP=gcc-3.4.5
GCC=gcc-4.1.1
GDB=gdb-6.4
GLIBC=glibc-2.3.6
LINUXTHREADS=glibc-linuxthreads-2.3.6
LINUX=linux-2.4.19

# package versions
BUSYBOX=busybox-1.2.1
LIBUNGIF=libungif-4.1.4
DOSFSTOOLS=dosfstools-2.11
STRACE=strace-4.5.14
PPP=ppp-2.4.3
FBV=fbv-1.0b
WUFTPD=wu-ftpd-2.6.2
SAMBA=samba-2.2.9
LIBELF=libelf-0.8.6
PRELINK=prelink-0.0.20060712
SQLITE=sqlite-3.3.8
EXPAT=expat-2.0.0
DBUS=dbus-0.92
BLUEZLIBS=bluez-libs-3.7
BLUEZUTILS=bluez-utils-3.7
BLUEZHCIDUMP=bluez-hcidump-1.33
BLUEZFIRMWARE=bluez-firmware-1.2
GPH_LINUX=linux-2.4.19-rmk7-pxa2-greenphone-20070201


if [ $OPTION_CLEAN -eq 1 ]; then
    for package in $PACKAGES; do
        clean_package $package
    done

    sudo rm -rf $ROOTFS_IMAGE_DIR
    sudo rm -rf $USERFS_IMAGE_DIR
    sudo rm -rf $DEVFS_IMAGE_DIR
    rmdir host --ignore-fail-on-non-empty

    rm -f $ROOTFS_FILENAME
fi
if [ $OPTION_BUILD -eq 1 ]; then
    for package in $PACKAGES; do
        build_package $package
    done
fi
if [ $OPTION_INSTALL -eq 1 ]; then
    sudo rm -rf $ROOTFS_IMAGE_DIR $USERFS_IMAGE_DIR $DEVFS_IMAGE_DIR
    mkdir -p $ROOTFS_IMAGE_DIR $USERFS_IMAGE_DIR $DEVFS_IMAGE_DIR
    rm -rf $ROOTFS_BUILD_PATH/junk

    for package in $PACKAGES; do
        install_package $package
    done
fi
if [ $OPTION_IMAGE -eq 1 ]; then
    create_default_tgzs

    make_rootfs_image
fi

