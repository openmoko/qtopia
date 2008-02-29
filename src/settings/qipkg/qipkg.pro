TEMPLATE	= app
CONFIG		+= qtopia warn_on release
DESTDIR		= $(QPEDIR)/bin

HEADERS		= packagemanager.h
SOURCES		= packagemanager.cpp \
		  main.cpp
INTERFACES	= packagemanagerbase.ui pkdesc.ui pkfind.ui pksettings.ui

TARGET		= qipkg

TRANSLATIONS = qipkg-en_GB.ts qipkg-de.ts qipkg-ja.ts qipkg-no.ts
