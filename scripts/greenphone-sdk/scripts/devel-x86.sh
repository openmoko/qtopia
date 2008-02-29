#!/bin/sh

. /opt/Qtopia/SDK/scripts/functions

QPEVER=`version`

if [ -z ${ORG_PATH} ];	then
	ORG_PATH=${PATH}
	export ORG_PATH
fi

if [ -z ${ORG_LD_LIBRARY_PATH} ]; then
	ORG_LD_LIBRARY_PATH=${LD_LIBRARY_PATH}
	export ORG_LD_LIBRARY_PATH
fi

export QPEDIR=/opt/Qtopia/SDK/$QPEVER/x86
export QTDIR=$QPEDIR/qtopiacore/target

export PATH=/opt/Qtopia/SDK/scripts:$QPEDIR/bin:$QPEDIR/scripts:$PATH

export LD_LIBRARY_PATH=$QPEDIR/lib:$QTDIR/lib:$ORG_LD_LIBRARY_PATH
