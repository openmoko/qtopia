TEMPLATE	= app
CONFIG		+= qtopia warn_on release
DESTDIR		= $(QPEDIR)/bin

HEADERS		= security.h
SOURCES		= security.cpp main.cpp
INTERFACES	= securitybase.ui

TARGET		= security

TRANSLATIONS = security-en_GB.ts security-de.ts security-ja.ts security-no.ts
