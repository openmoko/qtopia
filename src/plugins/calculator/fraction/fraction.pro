TEMPLATE = lib
CONFIG -= moc
CONFIG += qtopia debug

# Input
HEADERS = fraction.h  fractionfactory.h
SOURCES = fraction.cpp fractionfactory.cpp

INCLUDEPATH     += $(QPEDIR)/include
DEPENDPATH      += $(QPEDIR)/include

LIBS+=-lqtopiacalc

DESTDIR = $(QPEDIR)/plugins/calculator
