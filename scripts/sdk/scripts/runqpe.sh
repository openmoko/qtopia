#!/bin/sh

. /opt/Qtopia/SDK/scripts/functions

QPEVER=`version`

export QPEDIR=/opt/Qtopia/SDK/$QPEVER/${DEVICE}
export PATH=/opt/Qtopia/SDK/scripts:$QPEDIR/bin:$QPEDIR/scripts:$TOOLCHAIN/bin:$PATH
