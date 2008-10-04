!qbuild{
qtopia_project(qtopia core app)
TARGET=qcop
CONFIG+=no_tr singleexec_main
qt_inc(qtopia)
depends(libraries/qtopia,fake)
}

HEADERS		= qcopimpl.h
SOURCES		= main.cpp qcopimpl.cpp 

pkg.desc=Interprocess communication client for Qtopia.
pkg.domain=trusted
