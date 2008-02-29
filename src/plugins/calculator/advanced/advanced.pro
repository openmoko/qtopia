singleprocess:singleprocess=true

TEMPLATE = lib
CONFIG -= moc
CONFIG += qtopia warn_on release
win32:CONFIG += dll
win32:DEFINES += QTOPIA_PLUGIN_MAKEDLL QTOPIA_DLL

# Input
HEADERS = advanced.h  advancedfactory.h
SOURCES = advanced.cpp advancedfactory.cpp

unix:LIBS   +=-lqtopiacalc
win32:LIBS  +=$(QPEDIR)/lib/qtopiacalc.lib

DESTDIR = $(QPEDIR)/plugins/calculator

TRANSLATIONS = libadvanced-en_GB.ts libadvanced-de.ts libadvanced-ja.ts libadvanced-no.ts
