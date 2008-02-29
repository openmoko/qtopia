TEMPLATE	= app
CONFIG		+= qtopia warn_on release
DESTDIR		= $(QPEDIR)/bin

HEADERS		= packagewizard.h
SOURCES		= packagewizard.cpp \
		  main.cpp
INTERFACES	= pkwizard.ui pkdesc.ui

TARGET		= qipkg

TRANSLATIONS = qipkg-en_GB.ts qipkg-de.ts qipkg-ja.ts qipkg-no.ts
