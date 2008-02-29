TEMPLATE	= app
CONFIG		+= qtopia warn_on release
DESTDIR		= $(QPEDIR)/bin

HEADERS		= appservices.h
SOURCES		= appservices.cpp main.cpp

INTERFACES	= appservicesbase.ui

TARGET		= appservices

TRANSLATIONS = appservices-en_GB.ts appservices-de.ts appservices-ja.ts appservices-no.ts
