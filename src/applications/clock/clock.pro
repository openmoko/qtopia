TEMPLATE	= app
CONFIG		= qtopia warn_on release
DESTDIR		= $(QPEDIR)/bin
HEADERS		= clock.h analogclock.h
SOURCES		= clock.cpp analogclock.cpp \
		  main.cpp
INCLUDEPATH	+= $(QPEDIR)/include
DEPENDPATH	+= $(QPEDIR)/include
LIBS            += -lqpe
INTERFACES	= clockbase.ui
TARGET		= clock

TRANSLATIONS	= $(QPEDIR)/i18n/de/clock.ts
