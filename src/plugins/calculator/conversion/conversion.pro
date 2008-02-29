TEMPLATE = lib
CONFIG -= moc
CONFIG += qtopia debug
win32:CONFIG += dll
win32:DEFINES += QTOPIA_PLUGIN_MAKEDLL QTOPIA_DLL

# Input
HEADERS = conversion.h  conversionfactory.h
SOURCES = conversion.cpp conversionfactory.cpp

unix:LIBS   +=-lqtopiacalc
win32:LIBS  +=$(QPEDIR)/lib/qtopiacalc.lib

DESTDIR = $(QPEDIR)/plugins/calculator
