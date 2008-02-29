multiprocess:TEMPLATE	= app
multiprocess:DESTDIR	= $(QPEDIR)/bin
singleprocess:TEMPLATE	= lib
singleprocess:DESTDIR   = $(QPEDIR)/lib
quicklaunch:TEMPLATE    = lib
quicklaunch:DESTDIR     = $(QPEDIR)/plugins/application

CONFIG		+= qtopia warn_on release
HEADERS		= helpbrowser.h
SOURCES		= helpbrowser.cpp 

multiprocess:SOURCES+=main.cpp

TARGET		= helpbrowser
TRANSLATIONS = helpbrowser-en_GB.ts helpbrowser-de.ts helpbrowser-ja.ts helpbrowser-no.ts
