TEMPLATE	= lib
CONFIG		+= qtopia warn_on release
HEADERS		= fresh.h
SOURCES		= fresh.cpp
TARGET		= freshstyle
DESTDIR		= $(QPEDIR)/plugins/styles
INCLUDEPATH	+= $(QPEDIR)/include
DEPENDPATH      += $(QPEDIR)/include
LIBS            += -lqpe
VERSION		= 1.0.0

