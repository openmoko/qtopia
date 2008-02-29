TEMPLATE	= app
CONFIG		+= qtopia warn_on release
DESTDIR		= $(QPEDIR)/bin

HEADERS		= pluginmanager.h
SOURCES		= pluginmanager.cpp main.cpp
INTERFACES	= pluginmanagerbase.ui

INCLUDEPATH	+= $(QPEDIR)/include
DEPENDPATH	+= $(QPEDIR)/include

TARGET		= pluginmanager
TRANSLATIONS	= pluginmanager-en_GB.ts pluginmanager-de.ts pluginmanager-ja.ts pluginmanager-no.ts
