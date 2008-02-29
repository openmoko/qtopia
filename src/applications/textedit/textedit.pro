multiprocess:TEMPLATE	= app
multiprocess:DESTDIR	= $(QPEDIR)/bin
singleprocess:TEMPLATE	= lib
singleprocess:DESTDIR   = $(QPEDIR)/lib
quicklaunch:TEMPLATE    = lib
quicklaunch:DESTDIR     = $(QPEDIR)/plugins/application

CONFIG		+= qtopia warn_on release
HEADERS		= textedit.h
SOURCES		= textedit.cpp

multiprocess:SOURCES+=main.cpp

TARGET		= textedit

TRANSLATIONS = textedit-en_GB.ts textedit-de.ts textedit-ja.ts textedit-no.ts
