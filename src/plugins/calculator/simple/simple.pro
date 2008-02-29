singleprocess:singleprocess=true

TEMPLATE = lib
CONFIG -= moc
CONFIG += qtopia warn_on release
win32:CONFIG += dll
win32:DEFINES += QTOPIA_PLUGIN_MAKEDLL QTOPIA_DLL

# Input
HEADERS = simple.h  simplefactory.h
SOURCES = simple.cpp simplefactory.cpp

unix:LIBS	+=-lqtopiacalc
win32:LIBS	+=$(QPEDIR)/lib/qtopiacalc.lib

DESTDIR = $(QPEDIR)/plugins/calculator

TRANSLATIONS = libsimple-en_GB.ts libsimple-de.ts libsimple-ja.ts libsimple-no.ts
