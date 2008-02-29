#!/bin/sh

# This flash script is used to verify that the assumptions made in
# the Greenphone flash process are correct.

# To use this script copy it to a sd card as trolltech_flash.sh
# This script requires the followig additional files on the sd card.
# gifanim
# fail.gif
# pass.gif

echo "Trolltech verify assumptions startup hook"

fail()
{
    /mnt/sd/gifanim /mnt/sd/fail.gif
    echo "$@" >> /mnt/sd/trolltech_test.log
    exit 0
}

pass()
{
    /mnt/sd/gifanim /mnt/sd/pass.gif
    exit 0
}

# ASSUMPTION 1: /mnt/user/etc
[ -d /mnt/user/etc ] || fail "/mnt/user/etc not a directory"
[ -w /mnt/user/etc ] || fail "/mnt/user/etc not writeable"
[ -x /mnt/user/etc/run_dv2.sh ] || fail "/mnt/user/etc/run_dv2.sh not executable"
[ -w /mnt/user/etc/run_dv2.sh ] || fail "/mnt/user/etc/run_dv2.sh not writeable"
touch /mnt/user/etc/.trolltech || fail "/mnt/user/etc couldn't create file"
rm /mnt/user/etc/.trolltech

pass

