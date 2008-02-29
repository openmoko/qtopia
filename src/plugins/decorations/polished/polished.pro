TEMPLATE	= lib
CONFIG		+= qtopia warn_on release
HEADERS		= polished.h
SOURCES		= polished.cpp
TARGET		= polished
DESTDIR		= $(QPEDIR)/plugins/decorations
INCLUDEPATH	+= $(QPEDIR)/include
DEPENDPATH      += $(QPEDIR)/include
LIBS            += -lqpe
VERSION		= 1.0.0

