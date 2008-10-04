TEMPLATE=plugin
CONFIG+=qtopia singleexec
TARGET=base

PLUGIN_FOR=qtopia
PLUGIN_TYPE=qdsync

QTOPIA+=comm
MODULES*=qdsync_common crypt

SOURCEPATH=..

include(base.pro)

