TEMPLATE	= lib
CONFIG		+= qtopia warn_on release
HEADERS		= flat.h
SOURCES		= flat.cpp
TARGET		= flatstyle
DESTDIR		= $(QPEDIR)/plugins/styles
INCLUDEPATH	+= $(QPEDIR)/include
DEPENDPATH      += $(QPEDIR)/include
LIBS            += -lqpe
VERSION		= 1.0.0

