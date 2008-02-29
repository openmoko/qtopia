qtopia_project(qtopia core app)
TARGET=qcop
CONFIG+=no_tr singleexec_main

HEADERS		= qcopimpl.h
SOURCES		= main.cpp qcopimpl.cpp 

enable_singleexec {
    INCLUDEPATH+=$$QPEDIR/include/qtopia
}

pkg.desc=Interprocess communication client for Qtopia.
#pkg.domain=comm
pkg.domain=base
