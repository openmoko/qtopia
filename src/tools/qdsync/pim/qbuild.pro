TEMPLATE=plugin
CONFIG+=qtopia singleexec
TARGET=pim

QTOPIA+=pim
!contains(PROJECTS,tools/qdsync/common/qdsync_common):disable_project("Missing Synchronization")
MODULES*=qdsync_common

PLUGIN_FOR=qtopia
PLUGIN_TYPE=qdsync

SOURCEPATH=..

include(pim.pro)

