TEMPLATE	= lib
CONFIG		+= qtopia warn_on release
HEADERS		= clipboard.h clipboardappletimpl.h
SOURCES		= clipboard.cpp clipboardappletimpl.cpp
TARGET		= clipboardapplet
DESTDIR		= $(QPEDIR)/plugins/applets
INCLUDEPATH	+= $(QPEDIR)/include
DEPENDPATH      += $(QPEDIR)/include
LIBS            += -lqpe
VERSION		= 1.0.0
