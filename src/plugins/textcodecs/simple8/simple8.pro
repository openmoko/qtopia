singleprocess:singleprocess=true

TEMPLATE	= lib
CONFIG		+= qtopia warn_on release
win32:CONFIG += dll
win32:DEFINES += QTOPIA_PLUGIN_MAKEDLL QTOPIA_DLL
HEADERS		= simple8impl.h
SOURCES		= simple8impl.cpp
TARGET		= simple8
DESTDIR		= $(QPEDIR)/plugins/textcodecs
VERSION		= 1.0.0
