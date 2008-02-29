TEMPLATE	= app
CONFIG		+= qtopia warn_on release
DESTDIR		= $(QPEDIR)/bin

HEADERS		= settings.h
SOURCES		= rotation.cpp main.cpp
INTERFACES	= rotationsettingsbase.ui

TARGET		= rotation 
TRANSLATIONS = rotation-en_GB.ts rotation-de.ts rotation-ja.ts rotation-no.ts
