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

optimize_prelink_all()
{
    echo "Optimizing dynamic shared objects"

    # Use $GREENPHONE_IP as the ip address of the Greenphone if it is defined,
    # otherwise default to 10.10.10.20
    GREENPHONE_IP=${GREENPHONE_IP-10.10.10.20}

    EXPORTED_ROOTFS=$PWD/$MNTPNT
    EXPORTED_QTOPIA=$QTOPIA_IMAGE_PATH

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

    sudo /usr/sbin/exportfs -u $GREENPHONE_IP:$EXPORTED_QTOPIA
    sudo /usr/sbin/exportfs -u $GREENPHONE_IP:$EXPORTED_ROOTFS
}

optimize_prelink_qtopia()
{
    echo "Optimizing Qtopia dynamic shared objects"

    # Use $GREENPHONE_IP as the ip address of the Greenphone if it is defined,
    # otherwise default to 10.10.10.20
    GREENPHONE_IP=${GREENPHONE_IP-10.10.10.20}

    EXPORTED_ROOTFS=$PWD/$MNTPNT
    EXPORTED_QTOPIA=$QTOPIA_IMAGE_PATH

    cat > $EXPORTED_QTOPIA/prelink_helper.sh <<EOF
#!/bin/sh
find /opt/Qtopia.rom/bin /opt/Qtopia.rom/lib -type f \
    -exec /opt/Qtopia.rom/prelink_helper2.sh \\{\\} \\; |
    xargs -0 /usr/sbin/prelink -v -N --ld-library-path=/opt/Qtopia.rom/lib
EOF

    cat > $EXPORTED_QTOPIA/prelink_helper2.sh <<EOF
#!/bin/sh
if [ \"\`hexdump -s 1 -n 3 -e \"%c\" "\$1"\`\" = \"ELF\" ]; then
    echo -en "\$1\\0"
fi
EOF

    chmod a+x $EXPORTED_QTOPIA/prelink_helper.sh $EXPORTED_QTOPIA/prelink_helper2.sh

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

        send "test -d \$MNTPNT && mount -o bind,ro / \$MNTPNT\r";
        expect root@;
        send "test -d \$MNTPNT/opt/Qtopia.rom && mount -t nfs -o nolock,tcp hostpc:'$EXPORTED_QTOPIA' \$MNTPNT/opt/Qtopia.rom\r";
        expect root@;

        send "chroot \$MNTPNT /opt/Qtopia.rom/prelink_helper.sh\r";
        
        expect {
            timeout exp_continue
            root@
        }

        send "umount \$MNTPNT/opt/Qtopia.rom\r"; expect root@;
        send "umount \$MNTPNT\r"; expect root@;
        send "rmdir \$MNTPNT\r"; expect root@;
    ' || die "Optimizing (prelinking Qtopia)"

    sudo /usr/sbin/exportfs -u $GREENPHONE_IP:$EXPORTED_QTOPIA

    rm -f $EXPOTED_QTOPIA/prelink_helper.sh
    rm -f $EXPOTED_QTOPIA/prelink_helper2.sh
}

install_extras()
{
    echo "Install extra files into rootfs"

    # atd needs to be in the rootfs so qtopia.cramfs can be easily unmounted
    strip_install all $QTOPIA_IMAGE_PATH/bin $MNTPNT/usr/sbin atd
    sudo chown root.root $MNTPNT/usr/sbin/atd

    # Import zoneinfo from Qtopia
    sudo mkdir -p $MNTPNT/usr/share
    sudo cp -a $QTOPIA_SOURCE_PATH/etc/zoneinfo $MNTPNT/usr/share
    sudo chown -R root.root $MNTPNT/usr/share/zoneinfo
}

partition_tffsa()
{
    echo "Partitioning disk1 image"

    ROOTFS_PARTITION=`mktemp rootfs_part.XXXXXX`
    QTOPIA_PARTITION=fimage/qtopia.cramfs

    sudo /sbin/mkfs.cramfs $MNTPNT $ROOTFS_PARTITION
    sudo umount $MNTPNT

    local FIRST_SECTOR=63
    local ROOTFS_DEVICE_SIZE=86016
    local ROOTFS_PARTITION_SIZE=20480
    local QTOPIA_PARTITION_SIZE=$(($ROOTFS_DEVICE_SIZE - $ROOTFS_PARTITION_SIZE - $FIRST_SECTOR))

    if [ `stat -c %s $ROOTFS_PARTITION` -gt $(($ROOTFS_PARTITION_SIZE * 512)) ]; then
        echo "Root filesystem partition is too big `stat -c %s $ROOTFS_PARTITION` maximum $(($ROOTFS_PARTITION_SIZE * 512))"
        exit 1
    fi
    if [ `stat -c %s $QTOPIA_PARTITION` -gt $(($QTOPIA_PARTITION_SIZE * 512)) ]; then
        echo "Qtopia filesystem partition is too big `stat -c %s $QTOPIA_PARTITION` maximum $(($QTOPIA_PARTITION_SIZE * 512))"
        exit 1
    fi

    dd if=/dev/zero of=$DISK1_FILENAME bs=1 count=0 seek=$(($ROOTFS_DEVICE_SIZE * 512)) 2>/dev/null

    for i in 0 1 2 3 4 5 6 7; do
        LOOP_DEVICE=/dev/loop$i
        sudo /sbin/losetup $LOOP_DEVICE $DISK1_FILENAME 2>/dev/null && break
    done

    # create partitions
    # rootfs 10MB
    # qtopia ~32MB
    sudo /sbin/sfdisk -uS -f -L -q $LOOP_DEVICE >/dev/null 2>&1 <<EOF
$FIRST_SECTOR,$ROOTFS_PARTITION_SIZE,L
$(($FIRST_SECTOR + $ROOTFS_PARTITION_SIZE)),$QTOPIA_PARTITION_SIZE,L
EOF

    sudo /sbin/losetup -d $LOOP_DEVICE

    dd if=$ROOTFS_PARTITION of=$DISK1_FILENAME bs=512 seek=$FIRST_SECTOR conv=notrunc 2>/dev/null
    dd if=$QTOPIA_PARTITION of=$DISK1_FILENAME bs=512 seek=$(($FIRST_SECTOR + $ROOTFS_PARTITION_SIZE)) conv=notrunc 2>/dev/null

    rm -f $ROOTFS_PARTITION
}

make_flash_image()
{
    echo "Creating USB Flash image"

    local OUTPUT_IMAGE="$1"
    shift

    rm -f flash.conf

    IMAGE_FILES=""

    if [ ! -z $KERNEL_IMAGE ] && [ ! -z $KERNEL_FILENAME ] && [ -f $KERNEL_FILENAME ]; then
        MD5SUM=`md5sum $KERNEL_FILENAME | awk '{print $1}'`
        echo "$KERNEL_NAME:$KERNEL_FILENAME:2:$MD5SUM" >> flash.conf
        IMAGE_FILES="$IMAGE_FILES $KERNEL_FILENAME"
    fi

    if [ ! -z $ROOTFS_IMAGE ] && [ ! -z $DISK1_FILENAME ]; then
        MD5SUM=`md5sum $DISK1_FILENAME | awk '{print $1}'`
        echo "$DISK1_NAME ($ROOTFS_VERSION):$DISK1_FILENAME:3:$MD5SUM" >> flash.conf
        IMAGE_FILES="$IMAGE_FILES $DISK1_FILENAME"
    fi

    while [ $# -gt 0 ]; do
        case $1 in
            --blank-disk2)
                dd if=/dev/zero of=$DISK2_FILENAME bs=1048576 count=32 2>/dev/null
                MD5SUM=`md5sum $DISK2_FILENAME | awk '{print $1}'`
                echo "$DISK2_NAME:$DISK2_FILENAME:4:$MD5SUM" >> flash.conf
                IMAGE_FILES="$IMAGE_FILES $DISK2_FILENAME"
                ;;
            --blank-user)
                dd if=/dev/zero of=$USER_FILENAME bs=1048576 count=5 2>/dev/null
                MD5SUM=`md5sum $USER_FILENAME | awk '{print $1}'`
                echo "$USER_NAME:$USER_FILENAME:5:$MD5SUM" >> flash.conf
                IMAGE_FILES="$IMAGE_FILES $USER_FILENAME"
                ;;
            --blank-doc)
                dd if=/dev/zero of=$DOC_FILENAME bs=1024 count=36480 2>/dev/null
                MD5SUM=`md5sum $DOC_FILENAME | awk '{print $1}'`
                echo "$DOC_NAME:$DOC_FILENAME:6:$MD5SUM" >> flash.conf
                IMAGE_FILES="$IMAGE_FILES $DOC_FILENAME"
                ;;
        esac

        shift
    done

    tar -czhf "$OUTPUT_IMAGE" flash.conf $IMAGE_FILES
    echo "Finished creating USB Flash image : $PWD/$OUTPUT_IMAGE"
}

make_updateqtopia_image()
{
    echo "Creating updateqtopia image"

    rm -rf fimage
    mkdir fimage
    $QTOPIA_SOURCE_PATH/scripts/greenphone-make-cramfs.sh $QTOPIA_IMAGE_PATH fimage

    cp $QTOPIA_SOURCE_PATH/devices/greenphone/src/devtools/flash-files/flash-status-*.gif fimage
    chmod 660 fimage/*.gif
    cp $QTOPIA_SOURCE_PATH/devices/greenphone/src/devtools/flash-files/trolltech_flash.sh fimage
    chmod 775 fimage/trolltech_flash.sh

    if [ -s fimage/trolltech_flash.sh -a -s fimage/qtopia.cramfs -a -s fimage/flash-status-flashing.gif ]; then
        tar -czhf $UPDATE_IMAGE.tar.gz -C fimage .
        echo "Finished creating updateqtopia image : $PWD/$UPDATE_IMAGE.tar.gz"
        zip -jq $UPDATE_IMAGE.zip fimage/*
        echo "Finished creating updateqtopia image : $PWD/$UPDATE_IMAGE.zip"
    else
        rm -f $UPDATE_IMAGE.tar.gz # clean up after ourselves
        rm -f $UPDATE_IMAGE.zip
        echo "Error: Failed to create updateqtopia image"
        exit 1
    fi
}

usage()
{
    echo -e "Usage: `basename $0` [--clean] [--qtopia] [--flash] [--optimize] [--qtopia-build <path>] [--qtopia-source <path>] [--rootfs <file>]"
    echo -e "If no options are specified defaults to --qtopia --qtopia-build \$PWD/image --qtopia-source \$PWD --rootfs \$PWD/rootfs/greenphone_rootfs.ext2 --kernel \$PWD/rootfs/greenphone_kernel\n" \
            "   --clean          Clean image files.\n" \
            "   --qtopia         Make Qtopia image.\n" \
            "   --flash          Make flash image.\n" \
            "   --qtopia-build   Location of Qtopia build tree.\n" \
            "   --qtopia-source  Location of Qtopia source tree.\n" \
            "   --rootfs         Location of rootfs image.\n" \
            "   --kernel         Location of Linux kernel image.\n" \
            "   --build-path     Location of rootfs build directory.\n" \
            "   --optimize       Optimize dynamic shared objects. Requires network access to a Greenphone.\n" \
            "                    Greenphone IP address can be specified with the GREENPHONE_IP environment variable.\n"
}


DEFAULT_OPTIONS=1
OPTION_CLEAN=0
OPTION_OPTIMIZE=0
OPTION_QTOPIA_IMAGE=0
OPTION_FLASH_IMAGE=0

BUILD_PATH=$PWD/rootfs
 
DEFAULT_IMAGES=1

if [ -z $QTOPIA_DEPOT_PATH ]; then
		QTOPIA_SOURCE_PATH=$PWD
else
		QTOPIA_SOURCE_PATH=$QTOPIA_DEPOT_PATH
fi

QTOPIA_BUILD_PATH=$PWD/image

# Default Flash image data
KERNEL_NAME="Trolltech Greenphone Kernel (2.4.19 w/ LIDS)"
KERNEL_FILENAME=zImage_Truly_Trolltech
KERNEL_SIZELIMIT=1048576
DISK1_NAME="Trolltech Greenphone Root and Qtopia Filesystems"
ROOTFS_VERSION="unknown"
DISK1_FILENAME=greenphone_root_qtopia_FS.bin
DISK2_NAME="Blank Disk2"
DISK2_FILENAME=greenphone_disk2_FS.ext2
USER_NAME="Blank User"
USER_FILENAME=greenphone_user_FS.ext2
DOC_NAME="Blank Documents"
DOC_FILENAME=greenphone_doc_FS.vfat

# Output filenames
FLASH_IMAGE="qtopia-greenphone-flash"
UPDATE_IMAGE="qtopia-greenphone-update"

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
								unset QTOPIA_SOURCE_PATH
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
                DEFAULT_IMAGES=0
                shift 1
            else
                echo "$1 requires an argument"
                usage
                exit 1
            fi
            ;;
        --kernel)
            if [ $# -ge 2 ]; then
                KERNEL_IMAGE="$2"
                DEFAULT_IMAGES=0
                shift 1
            else
                echo "$1 requires an argument"
                usage
                exit 1
            fi
            ;;
        --optimize)
            OPTION_OPTIMIZE=1
            DEFAULT_OPTIONS=0
            ;;
        --build-path)
            BUILD_PATH="$2"
            shift 1
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

if [ $DEFAULT_IMAGES -eq 1 ]; then
    ROOTFS_IMAGE=$BUILD_PATH/greenphone_rootfs.ext2
    KERNEL_IMAGE=$BUILD_PATH/greenphone_kernel
fi
# Make sure all input files and directories exist
[ -d "$QTOPIA_BUILD_PATH" ] || die "$QTOPIA_BUILD_PATH does not exist or is not a directory"
if [ ! -f "$QTOPIA_BUILD_PATH/qpe.sh" ]; then
 cp -a $QTOPIA_SOURCE_PATH/devices/greenphone/src/devtools/startup/qpe.sh $QTOPIA_BUILD_PATH
 cp -a $QTOPIA_SOURCE_PATH/devices/greenphone/src/devtools/startup/qpe.env $QTOPIA_BUILD_PATH
fi
if [ ! -f "$QTOPIA_BUILD_PATH/qpe.sh" ]; then
    echo "$QTOPIA_BUILD_PATH does not contain an image - check \"--qtopia-build\" argument"
    if [ -f "$QTOPIA_BUILD_PATH/config.status" ]; then
        echo "    $QTOPIA_BUILD_PATH looks like the build tree"
        QTOPIA_BUILD_PATH="$QTOPIA_BUILD_PATH/image"
        if [ -f "$QTOPIA_BUILD_PATH/qpe.sh" ]; then
            echo "    using $QTOPIA_BUILD_PATH as image"
        else
            exit 1;
        fi
    else
        exit 1;
    fi
fi

[ -d "$QTOPIA_SOURCE_PATH" ] || die "Qtopia source path does not exist or is not a directory"

if [ $OPTION_CLEAN -eq 1 ]; then
    rm -rf fimage
    rm -f $KERNEL_FILENAME
    rm -f $DISK1_FILENAME
    rm -f $UPDATE_IMAGE.tar.gz
    rm -f $UPDATE_IMAGE.zip
    rm -f $FLASH_IMAGE
    rm -f $FLASH_IMAGE-full
fi

if [ $OPTION_FLASH_IMAGE -eq 1 ]; then
    if [ ! -z "$KERNEL_IMAGE" ]; then
        [ -f "$KERNEL_IMAGE" ] || die "Kernel image does not exist"
    fi
    if [ ! -z $KERNEL_IMAGE ]; then
        KERNEL_SIZE=`stat -c '%s' $KERNEL_IMAGE`

        [ $KERNEL_SIZE -le $KERNEL_SIZELIMIT ] ||
            die "linux kernel larger than allocated flash size. $KERNEL_SIZE > $KERNEL_SIZELIMIT."

        cp $KERNEL_IMAGE $KERNEL_FILENAME
    fi

    if [ ! -z "$ROOTFS_IMAGE" ]; then
        [ -f "$ROOTFS_IMAGE" ] || die "Rootfs image $ROOTFS_IMAGE does not exist"
    fi
    if [ ! -z $ROOTFS_IMAGE ]; then
        OPTION_QTOPIA_IMAGE=1

        cp $ROOTFS_IMAGE $DISK1_FILENAME

        MNTPNT=`mktemp -d flash.XXXXXX`
        [ -d $MNTPNT ] || die "Could not create temporary rootfs directory"
        sudo mount -o loop,rw $DISK1_FILENAME $MNTPNT

        if [ -r $MNTPNT/fs.ver ]; then
            ROOTFS_VERSION=`sed -ne '/^troll/ s/^troll v//p' < $MNTPNT/fs.ver`
        fi
    fi
fi

if [ $OPTION_QTOPIA_IMAGE -eq 1 ]; then
    QTOPIA_IMAGE_PATH=`mktemp -d $PWD/qtopia.XXXXXX`
    [ -d $QTOPIA_IMAGE_PATH ] || die "Could not create temporary qtopia directory"
    cp -a $QTOPIA_BUILD_PATH/* $QTOPIA_IMAGE_PATH
fi

if [ $OPTION_FLASH_IMAGE -eq 1 ] && [ ! -z $ROOTFS_IMAGE ]; then
    install_extras
fi

if [ $OPTION_OPTIMIZE -eq 1 ]; then
    if [ $OPTION_FLASH_IMAGE -eq 1 ] && [ ! -z $ROOTFS_IMAGE ]; then
        optimize_prelink_all
    elif [ $OPTION_QTOPIA_IMAGE -eq 1 ]; then
        optimize_prelink_qtopia
    fi
fi

if [ $OPTION_QTOPIA_IMAGE -eq 1 ]; then
    make_updateqtopia_image

    rm -rf $QTOPIA_IMAGE_PATH
fi

if [ $OPTION_FLASH_IMAGE -eq 1 ] && [ ! -z $ROOTFS_IMAGE ]; then
    partition_tffsa

    rmdir $MNTPNT
fi

if [ $OPTION_FLASH_IMAGE -eq 1 ]; then
    make_flash_image $FLASH_IMAGE
    make_flash_image $FLASH_IMAGE-full --blank-disk2 --blank-user --blank-doc

    rm -f $KERNEL_FILENAME $DISK1_FILENAME $DISK2_FILENAME $USER_FILENAME $DOC_FILENAME
fi


