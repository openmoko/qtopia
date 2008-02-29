singleprocess:singleprocess=true
TEMPLATE	= lib
CONFIG		+= qtopia warn_on release
win32:CONFIG += dll
win32:DEFINES += QTOPIA_PLUGIN_MAKEDLL QTOPIA_DLL
HEADERS		= keyboard.h \
		    pickboardcfg.h \
		    pickboardpicks.h \
		    keyboardimpl.h
SOURCES		= keyboard.cpp \
		    pickboardcfg.cpp \
		    pickboardpicks.cpp \
		    keyboardimpl.cpp
TARGET		= qkeyboard
DESTDIR		= $(QPEDIR)/plugins/inputmethods
VERSION		= 1.0.0

TRANSLATIONS = libqkeyboard-en_GB.ts libqkeyboard-de.ts libqkeyboard-ja.ts libqkeyboard-no.ts
