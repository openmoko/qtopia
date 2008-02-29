TEMPLATE = lib
CONFIG -= moc
CONFIG += qtopia debug
win32:CONFIG += dll
win32:DEFINES += QTOPIA_PLUGIN_MAKEDLL QTOPIA_DLL

# Input
HEADERS = advanced.h  advancedfactory.h
SOURCES = advanced.cpp advancedfactory.cpp

unix:LIBS   +=-lqtopiacalc
win32:LIBS  +=$(QPEDIR)/lib/qtopiacalc.lib

DESTDIR = $(QPEDIR)/plugins/calculator
