TEMPLATE = lib
CONFIG -= moc
CONFIG += qtopia debug

# Input
HEADERS = advanced.h  advancedfactory.h
SOURCES = advanced.cpp advancedfactory.cpp

INCLUDEPATH     += $(QPEDIR)/include
DEPENDPATH      += $(QPEDIR)/include

LIBS+=-lqtopiacalc

DESTDIR = $(QPEDIR)/plugins/calculator
