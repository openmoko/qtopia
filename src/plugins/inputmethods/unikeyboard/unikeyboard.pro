TEMPLATE	= lib
CONFIG		+= qtopia warn_on release
HEADERS		= unikeyboard.h unikeyboardimpl.h
SOURCES		= unikeyboard.cpp unikeyboardimpl.cpp
TARGET		= qunikeyboard
DESTDIR		= $(QPEDIR)/plugins/inputmethods
INCLUDEPATH	+= $(QPEDIR)/include
DEPENDPATH      += $(QPEDIR)/include
LIBS            += -lqpe
VERSION		= 1.0.0

TRANSLATIONS	+= $(QPEDIR)/i18n/de/libqunikeyboard.ts
