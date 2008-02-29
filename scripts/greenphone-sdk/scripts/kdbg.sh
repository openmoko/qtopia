#!/bin/sh

source /opt/Qtopia/SDK/scripts/devel-x86.sh

if [ "$1" = "-arm" ] ; then
  kdbg --config arm-linux-kdbgrc -r 10.10.10.20:22222
else
  kdbg
fi
