TEMPLATE	= lib
CONFIG		+= qtopia warn_on release
win32:DEFINES += QTOPIA_PLUGIN_MAKEDLL QTOPIA_DLL
HEADERS		= jpimpl.h qeucjpcodec.h qjiscodec.h qjpunicode.h qsjiscodec.h
SOURCES		= jpimpl.cpp qeucjpcodec.cpp qjiscodec.cpp qjpunicode.cpp qsjiscodec.cpp
TARGET		= jp
DESTDIR		= $(QPEDIR)/plugins/textcodecs
VERSION		= 1.0.0

