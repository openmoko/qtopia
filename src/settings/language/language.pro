TEMPLATE	= app
CONFIG		+= qtopia warn_on release
DESTDIR		= $(QPEDIR)/bin

HEADERS		= settings.h
SOURCES		= language.cpp main.cpp
INTERFACES	= languagesettingsbase.ui

TARGET		= language

TRANSLATIONS = language-en_GB.ts language-de.ts language-ja.ts language-no.ts
