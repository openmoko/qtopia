qtopia_project(qtopia plugin)
TARGET=ota

CONFIG+=no_tr

HEADERS		= otaimpl.h otahandler.h
SOURCES		= otaimpl.cpp otahandler.cpp

pkg.name=qpe-ota-imagecodec
pkg.domain=libs
