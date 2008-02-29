singleprocess:singleprocess=true
TEMPLATE	= lib
CONFIG		+= qtopia warn_on release
win32:CONFIG += dll
win32:DEFINES += QTOPIA_PLUGIN_MAKEDLL QTOPIA_DLL
HEADERS		= unikeyboard.h unikeyboardimpl.h
SOURCES		= unikeyboard.cpp unikeyboardimpl.cpp
TARGET		= qunikeyboard
DESTDIR		= $(QPEDIR)/plugins/inputmethods
VERSION		= 1.0.0

TRANSLATIONS = libqunikeyboard-en_GB.ts libqunikeyboard-de.ts libqunikeyboard-ja.ts libqunikeyboard-no.ts
