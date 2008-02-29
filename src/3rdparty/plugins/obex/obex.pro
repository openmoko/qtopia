TEMPLATE	= lib
CONFIG		+= qtopia warn_on release
win32:DEFINES += QTOPIA_PLUGIN_MAKEDLL QTOPIA_DLL
DESTDIR		= $(QPEDIR)/plugins/obex/
VERSION		= 1.0.0

HEADERS		= qir.h qirserver.h qobex.h receivedialog.h senddialog.h
SOURCES		= qir.cpp qirserver.cpp qobex.cpp receivedialog.cpp senddialog.cpp
INTERFACES 	+= receivedialogbase.ui senddialogbase.ui

INCLUDEPATH	+= ./openobex 
unix:LIBS	+= -lopenobex
win32:LIBS	+= $(QPEDIR)/lib/openobex.lib 
win32:DEFINES += QTOPIA_MAKEDLL

TARGET		= qobex

TRANSLATIONS = libqobex-en_GB.ts libqobex-de.ts libqobex-ja.ts libqobex-no.ts
