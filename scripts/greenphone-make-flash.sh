#!/bin/sh

strip_install()
{
    local stripwhat=$1
    local src=$2
    local dest=$3
    shift 3

    [ -e $dest ] || mkdir -p $dest

    for i in "$@"; do
        sudo cp -a $src/$i $dest/$i
        [ -f $dest/$i ] && sudo arm-linux-strip --strip-$stripwhat $dest/$i
    done
}

die()
{
    echo "Error: $@"
    exit 1
}

optimize_prelink()
{
    echo "Optimizing dynamic shared objects"

    GREENPHONE_IP=${GREENPHONE_IP-10.10.10.20}

    EXPORTED_ROOTFS=$PWD/$MNTPNT
    EXPORTED_QTOPIA=$QTOPIA_BUILD_PATH

    sudo /usr/sbin/exportfs -o rw,no_root_squash,async $GREENPHONE_IP:$EXPORTED_ROOTFS
    sudo /usr/sbin/exportfs -o rw,no_root_squash,async $GREENPHONE_IP:$EXPORTED_QTOPIA

    expect -c '
        set timeout 300;
        log_user 0;

        spawn telnet '$GREENPHONE_IP';
        expect {
            default {
                send_user "Could not connect to Greenphone."
                exit 1
            } root@
        }

        send "MNTPNT=`mktemp -d /tmp/prelink.XXXXXX`\r";
        expect root@

        send "test -d \$MNTPNT && mount -t nfs -o nolock,tcp hostpc:'$EXPORTED_ROOTFS' \$MNTPNT\r";
        expect root@;
        send "test -d \$MNTPNT/opt/Qtopia.rom && mount -t nfs -o nolock,tcp hostpc:'$EXPORTED_QTOPIA' \$MNTPNT/opt/Qtopia.rom\r";
        expect root@;

        send "chroot \$MNTPNT /usr/sbin/prelink -v -a --ld-library-path=/opt/Qtopia.rom/lib\r";
        expect {
            timeout exp_continue
            root@
        }

        send "umount \$MNTPNT/opt/Qtopia.rom\r"; expect root@;
        send "umount \$MNTPNT\r"; expect root@;
        send "rmdir \$MNTPNT\r"; expect root@;
    ' || die "Optimizing (prelinking)"

    sudo /usr/sbin/exportfs -u $GREENPHONE_IP:$EXPORTED_ROOTFS
    sudo /usr/sbin/exportfs -u $GREENPHONE_IP:$EXPORTED_QTOPIA
}

install_extras()
{
    echo "Install extra files into rootfs"

    # atd needs to be in the rootfs so qtopia.cramfs can be easily unmounted
    strip_install all $QTOPIA_BUILD_PATH/bin $MNTPNT/usr/sbin atd

    # Import zoneinfo from Qtopia
    sudo mkdir -p $MNTPNT/usr/share
    sudo cp -a $QTOPIA_SOURCE_PATH/etc/zoneinfo $MNTPNT/usr/share
}

make_flash_image()
{
    echo "Creating USB Flash image"
    rm -f flash.conf

    if [ ! -z $ROOTFS_FILENAME ]; then
        MD5SUM=`md5sum $ROOTFS_FILENAME | awk '{print $1}'`
        echo "$ROOTFS_NAME:$ROOTFS_FILENAME:3:$MD5SUM" >> flash.conf
        IMAGE_FILES="$IMAGE_FILES $ROOTFS_FILENAME"
    fi

    tar --remove-files -czhf qtopia_greenphone_flash flash.conf $IMAGE_FILES
}

make_updateqtopia_image()
{
    echo "Creating updateqtopia image"

    rm -rf fimage
    mkdir fimage
    $QTOPIA_SOURCE_PATH/scripts/greenphone-make-cramfs.sh $QTOPIA_BUILD_PATH fimage

    cp $QTOPIA_SOURCE_PATH/devices/greenphone/src/devtools/flash-files/*.gif fimage
    chmod 660 fimage/*.gif
    cp $QTOPIA_SOURCE_PATH/devices/greenphone/src/devtools/flash-files/trolltech_flash.sh fimage
    chmod 775 fimage/trolltech_flash.sh

    tar -czhf qtopia_greenphone -C fimage .
}

usage()
{
    echo -e "Usage: `basename $0` [--clean] [--qtopia] [--flash] [--optimize] [--qtopia-build <path>] [--qtopia-source <path>] [--rootfs <file>]"
    echo -e "If no options are specified defaults to --qtopia --qtopia-build \$PWD/image/opt/Qtopia --qtopia-source \$PWD --rootfs \$PWD/rootfs/greenphone_rootfs.ext2\n" \
            "   --clean          Clean image files.\n" \
            "   --qtopia         Make Qtopia image.\n" \
            "   --flash          Make flash image.\n" \
            "   --qtopia-build   Location of Qtopia build tree.\n" \
            "   --qtopia-source  Location of Qtopia source tree.\n" \
            "   --rootfs         Location of rootfs image.\n" \
            "   --optimize       Optimize dynamic shared objects. Requires network access to a Greenphone.\n"
}


DEFAULT_OPTIONS=1
OPTION_CLEAN=0
OPTION_OPTIMIZE=0
OPTION_QTOPIA_IMAGE=0
OPTION_FLASH_IMAGE=0

QTOPIA_SOURCE_PATH=$PWD
QTOPIA_BUILD_PATH=$PWD/image/opt/Qtopia
ROOTFS_IMAGE=$PWD/rootfs/greenphone_rootfs.ext2

# Default Flash image data
#KERNEL_NAME="Trolltech Greenphone Kernel (2.4.19)"
#KERNEL_FILENAME=zImage_Truly_Trolltech
ROOTFS_NAME="Trolltech Greenphone Root Filesystem (1.0)"
ROOTFS_FILENAME=greenphone_root_prg_FS.ext2

while [ $# -ne 0 ]; do
    case $1 in
        --qtopia)
            OPTION_QTOPIA_IMAGE=1
            DEFAULT_OPTIONS=0
            ;;
        --flash)
            OPTION_FLASH_IMAGE=1
            DEFAULT_OPTIONS=0
            ;;
        --clean)
            OPTION_CLEAN=1
            DEFAULT_OPTIONS=0
            ;;
        --qtopia-build)
            if [ $# -ge 2 ]; then
                QTOPIA_BUILD_PATH="$2"
                shift 1
            else
                echo "$1 requires an argument"
                usage
                exit 1
            fi
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
        --rootfs)
            if [ $# -ge 2 ]; then
                ROOTFS_IMAGE="$2"
                shift 1
            else
                echo "$1 requires an argument"
                usage
                exit 1
            fi
            ;;
#        --kernel)
#            if [ $# -ge 2 ]; then
#                KERNEL_IMAGE="$2"
#                shift 1
#            else
#                echo "$1 requires an argument"
#                usage
#                exit 1
#            fi
#            ;;
        --optimize)
            OPTION_OPTIMIZE=1
            DEFAULT_OPTIONS=0
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

if [ $DEFAULT_OPTIONS -eq 1 ]; then
    OPTION_QTOPIA_IMAGE=1
fi

if [ $OPTION_CLEAN -eq 1 ]; then
    rm -rf fimage
    rm -rf $ROOTFS_FILENAME
    rm -rf qtopia_greenphone
    rm -rf qtopia_greenphone_flash
fi

if [ $OPTION_OPTIMIZE -eq 1 ] || [ $OPTION_FLASH_IMAGE -eq 1 ]; then
    cp $ROOTFS_IMAGE $ROOTFS_FILENAME

    MNTPNT=`mktemp -d flash.XXXXXX`
    [ -d $MNTPNT ] || die "Could not create temporary directory"
    sudo mount -o loop,rw $ROOTFS_FILENAME $MNTPNT
fi

if [ $OPTION_FLASH_IMAGE -eq 1 ]; then
    install_extras
fi

if [ $OPTION_OPTIMIZE -eq 1 ]; then
    optimize_prelink
fi

if [ $OPTION_QTOPIA_IMAGE -eq 1 ] || [ $OPTION_FLASH_IMAGE -eq 1 ]; then
    make_updateqtopia_image
fi

if [ $OPTION_FLASH_IMAGE -eq 1 ]; then
    cp fimage/qtopia.cramfs $MNTPNT
    cp fimage/qtopia_default.tgz $MNTPNT
fi

if [ $OPTION_OPTIMIZE -eq 1 ] || [ $OPTION_FLASH_IMAGE -eq 1 ]; then
    sudo umount $MNTPNT
    rmdir $MNTPNT
fi

if [ $OPTION_FLASH_IMAGE -eq 1 ]; then
    make_flash_image
fi


