TEMPLATE	= lib
CONFIG		+= qtopia warn_on release
HEADERS		= volume.h volumeappletimpl.h
SOURCES		= volume.cpp volumeappletimpl.cpp
TARGET		= volumeapplet
DESTDIR		= $(QPEDIR)/plugins/applets
INCLUDEPATH	+= $(QPEDIR)/include
DEPENDPATH      += $(QPEDIR)/include
LIBS            += -lqpe
VERSION		= 1.0.0

TRANSLATIONS	+= $(QPEDIR)/i18n/de/libvolumeapplet.ts
