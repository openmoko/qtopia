TEMPLATE	= lib
CONFIG		+= qtopia warn_on release
HEADERS		= flat.h
SOURCES		= flat.cpp
TARGET		= flat
DESTDIR		= $(QPEDIR)/plugins/decorations
INCLUDEPATH	+= $(QPEDIR)/include
DEPENDPATH      += $(QPEDIR)/include
LIBS            += -lqpe
VERSION		= 1.0.0

