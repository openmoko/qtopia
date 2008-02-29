#! /usr/bin/bash


# Example bash setup for Qtopia under Cygwin
export QPEDIR=/opt/Qtopia
export QTDIR=$QPEDIR
export QCONFIG="CONFIG+=qtopia CONFIG+=cygwin DEFINES+=QCONFIG=\\\"qconfig-qpe.h\\\""
export TMAKEDIR=/opt/Qtopia/tmake
export TMAKEPATH=$TMAKEDIR/lib/qws/cygwin-sharp-g++
export PATH=$QPEDIR/bin:$TMAKEDIR/bin:/tools/arm/bin:$PATH
