TEMPLATE = lib
CONFIG -= moc
CONFIG += qtopia warn_on release

# Input
HEADERS = datebookplugin.h datebookpluginimpl.h datebookpluginoptions.h
SOURCES = datebookplugin.cpp datebookpluginimpl.cpp datebookpluginoptions.cpp
INTERFACES = datebookoptionsbase.ui

INCLUDEPATH     += $(QPEDIR)/include \
		../ ../library
DEPENDPATH      += $(QPEDIR)/include \
		../ ../library

LIBS+= -lqpe -lqpepim

DESTDIR = $(QPEDIR)/plugins/today
TARGET = datebookplugin

