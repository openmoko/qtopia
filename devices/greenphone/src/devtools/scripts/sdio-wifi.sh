#!/bin/sh

# check if the WiFi module are already loaded
grep '^p300wlan' /proc/modules >/dev/null && exit

# unmount SD card
sdcard-umount.sh

# remove SD storage modules
rmmod mmc_omega 2>/dev/null
rmmod mmc_base 2>/dev/null

# load WiFi module
insmod p300wlan

