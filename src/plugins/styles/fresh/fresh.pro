TEMPLATE	= lib
CONFIG		+= qtopia warn_on release
win32:CONFIG += dll
win32:DEFINES += QTOPIA_PLUGIN_MAKEDLL QTOPIA_DLL
HEADERS		= fresh.h
SOURCES		= fresh.cpp
TARGET		= freshstyle
DESTDIR		= $(QPEDIR)/plugins/styles
VERSION		= 1.0.0

