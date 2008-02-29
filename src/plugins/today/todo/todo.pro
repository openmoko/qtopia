TEMPLATE = lib
CONFIG -=
CONFIG += qtopia warn_on release

# Input
HEADERS = todoplugin.h todopluginimpl.h todopluginoptions.h
SOURCES = todoplugin.cpp todopluginimpl.cpp todopluginoptions.cpp
INTERFACES = todooptionsbase.ui

INCLUDEPATH     += $(QPEDIR)/include \
		../ ../library
DEPENDPATH      += $(QPEDIR)/include \
		../ ../library

LIBS+= -lqpe -lqpepim

DESTDIR = $(QPEDIR)/plugins/today
TARGET = todoplugin

