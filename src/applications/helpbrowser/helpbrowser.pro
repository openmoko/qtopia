TEMPLATE	= app
CONFIG		= qtopia warn_on release
DESTDIR		= $(QPEDIR)/bin
HEADERS		= helpbrowser.h
SOURCES		= helpbrowser.cpp \
		  main.cpp
INCLUDEPATH	+= $(QPEDIR)/include
DEPENDPATH	+= $(QPEDIR)/include
LIBS            += -lqpe
INTERFACES	= 

TRANSLATIONS	= $(QPEDIR)/i18n/de/helpbrowser.ts
