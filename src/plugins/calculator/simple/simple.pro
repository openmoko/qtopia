TEMPLATE = lib
CONFIG -= moc
CONFIG += qtopia debug

# Input
HEADERS = simple.h  simplefactory.h
SOURCES = simple.cpp simplefactory.cpp

INCLUDEPATH     += $(QPEDIR)/include
DEPENDPATH      += $(QPEDIR)/include

LIBS+=-lqtopiacalc

DESTDIR = $(QPEDIR)/plugins/calculator
