CONFIG += qtopiaplugin

TARGET=fraction

HEADERS = fraction.h  fractionfactory.h
SOURCES = fraction.cpp fractionfactory.cpp

TRANSLATABLES = $${HEADERS} $${SOURCES}
LIBS   +=-lqtopiacalc
