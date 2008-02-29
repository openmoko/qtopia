CONFIG += qtopiaplugin 

TARGET = scientific

HEADERS = advanced.h  advancedfactory.h
SOURCES = advanced.cpp advancedfactory.cpp

TRANSLATABLES = $${HEADERS} $${SOURCES}

LIBS+=-lqtopiacalc

DESTDIR = $$(QPEDIR)/plugins/calculator

PACKAGE_NAME = qpe-calculator-advanced
