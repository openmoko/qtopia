TEMPLATE = lib
CONFIG -= moc
CONFIG += qtopia debug

# Input
HEADERS = conversion.h  conversionfactory.h
SOURCES = conversion.cpp conversionfactory.cpp

INCLUDEPATH     += $(QPEDIR)/include
DEPENDPATH      += $(QPEDIR)/include

LIBS+=-lqtopiacalc

DESTDIR = $(QPEDIR)/plugins/calculator
