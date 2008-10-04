TEMPLATE=app
CONFIG+=qtopia
TARGET=sysinfo

QTOPIA*=comm
enable_cell:QTOPIA*=phone
CONFIG+=quicklaunch singleexec

include(sysinfo.pro)

STRING_LANGUAGE=en_US
AVAILABLE_LANGUAGES=$$QTOPIA_AVAILABLE_LANGUAGES
LANGUAGES=$$QTOPIA_LANGUAGES

# These defines are for versioninfo.cpp only
SOURCES-=versioninfo.cpp
volatile.SOURCES=versioninfo.cpp
volatile.TYPE=CPP_SOURCES COMPILER_CONFIG
!isEmpty(QT_CHANGE):volatile.DEFINES+=QT_CHANGE=$$define_string($$QT_CHANGE)
!isEmpty(QTOPIA_CHANGE):volatile.DEFINES+=QTOPIA_CHANGE=$$define_string($$QTOPIA_CHANGE)

# Always rebuild versioninfo.o so that the reported build date is correct
force_rebuild.TYPE=RULE
force_rebuild.outputFiles=force_rebuild $$path(versioninfo.cpp,existing)
force_rebuild.commands="#(e)true"

