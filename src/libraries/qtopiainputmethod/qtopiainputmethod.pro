qtopia_project(qtopia core lib)
TARGET=qtopiainputmethod
CONFIG += qtopia_visibility

HEADERS += qwsinputmethod_x11.h
SOURCES += qwsinputmethod_x11.cpp
LIBS += -lXtst

idep(LIBS+=-l$$TARGET)
qt_inc($$TARGET)
