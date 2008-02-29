qtopia_project(qtopia lib)
TARGET=qdsync_common
CONFIG+=no_tr no_pkg

include(common.pri)
PREFIX=QTOPIADESKTOP
resolve_include()

HEADERS+=\
    qcopadaptor_qd.h\

SOURCES+=\
    qcopadaptor_qd.cpp\

idep(LIBS+=-l$$TARGET)
idep(INCLUDEPATH+=$$PWD $$PWD/..,INCLUDEPATH)
