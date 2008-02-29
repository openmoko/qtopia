qtopia_project(qtopia plugin)
TARGET=wbmp

CONFIG+=no_tr

HEADERS		= wbmpimpl.h wbmphandler.h
SOURCES		= wbmpimpl.cpp wbmphandler.cpp

pkg.name=qpe-wbmp-imagecodec
pkg.domain=libs
