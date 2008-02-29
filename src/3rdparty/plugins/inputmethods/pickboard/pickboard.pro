TEMPLATE	= lib
CONFIG		+= qtopia warn_on release
win32:DEFINES += QTOPIA_PLUGIN_MAKEDLL QTOPIA_DLL
HEADERS		= pickboard.h pickboardcfg.h pickboardimpl.h pickboardpicks.h
SOURCES		= pickboard.cpp pickboardcfg.cpp pickboardimpl.cpp pickboardpicks.cpp
TARGET		= qpickboard
DESTDIR		= $(QPEDIR)/plugins/inputmethods
VERSION		= 1.0.0
win32:DEFINES += QTOPIA_MAKEDLL

TRANSLATIONS = libqpickboard-en_GB.ts libqpickboard-de.ts libqpickboard-ja.ts libqpickboard-no.ts
