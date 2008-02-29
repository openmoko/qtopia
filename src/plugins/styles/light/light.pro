TEMPLATE	= lib
CONFIG		+= qtopia warn_on release
HEADERS		= lightstyle.h
SOURCES		= lightstyle.cpp
TARGET		= lightstyle
DESTDIR		= $(QPEDIR)/plugins/styles
INCLUDEPATH	+= $(QPEDIR)/include
DEPENDPATH      += $(QPEDIR)/include
LIBS            += -lqpe
VERSION		= 1.0.0

