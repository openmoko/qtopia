CONFIG += qtopiaplugin

TARGET = simple

HEADERS = simple.h  simplefactory.h
SOURCES = simple.cpp simplefactory.cpp
TRANSLATABLES = $${HEADERS} $${SOURCES}

LIBS+=-lqtopiacalc

