!qbuild{
qtopia_project(qtopia lib)
TARGET=qdsync_common
CONFIG+=no_tr
# Packaged by tools/qdsync/app
CONFIG+=no_pkg
include(common.pri)
PREFIX=QTOPIADESKTOP
resolve_include()
}

HEADERS+=\
    qtopia4sync.h\
    qdglobal.h\

SOURCES+=\
    qtopia4sync.cpp\

!qbuild{
idep(LIBS+=-l$$TARGET)
idep(INCLUDEPATH+=$$PWD $$PWD/..,INCLUDEPATH)
}
