#!/bin/sh
ipkg -d /opt/Qtopia install $1 >/tmp/ipkg.log 2>&1
qcop QPE/DocAPI 'scanPath(QString,int)' 'all' '1'
if [ `grep -c Done /tmp/ipkg.log` = "2" ] ; then
  exit 0
else
  exit 1
fi
