multiprocess:TEMPLATE	= app
multiprocess:DESTDIR	= $(QPEDIR)/bin
singleprocess:TEMPLATE	= lib
singleprocess:DESTDIR   = $(QPEDIR)/lib
quicklaunch:TEMPLATE    = lib
quicklaunch:DESTDIR     = $(QPEDIR)/plugins/application

CONFIG          += qtopia warn_on release

HEADERS		= showimg.h \
		  settingsdialog.h

SOURCES		= settingsdialog.cpp \
		  showimg.cpp

multiprocess:SOURCES+=main.cpp

INTERFACES	= settingsdialogbase.ui

TARGET          = showimg

TRANSLATIONS = showimg-en_GB.ts showimg-de.ts showimg-ja.ts showimg-no.ts
