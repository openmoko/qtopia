TEMPLATE	= lib
CONFIG		+= qtopia warn_on release
HEADERS		= pickboard.h pickboardcfg.h pickboardimpl.h pickboardpicks.h
SOURCES		= pickboard.cpp pickboardcfg.cpp pickboardimpl.cpp pickboardpicks.cpp
TARGET		= qpickboard
DESTDIR		= $(QPEDIR)/plugins/inputmethods
INCLUDEPATH	+= $(QPEDIR)/include
DEPENDPATH      += $(QPEDIR)/include
LIBS            += -lqpe
VERSION		= 1.0.0

TRANSLATIONS	+= $(QPEDIR)/i18n/de/libqpickboard.ts
