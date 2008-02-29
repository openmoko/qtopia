TEMPLATE	= app
CONFIG		+= qtopia warn_on release
DESTDIR		= $(QPEDIR)/bin

HEADERS		= soundsettings.h
SOURCES		= soundsettings.cpp main.cpp
INTERFACES	= soundsettingsbase.ui

TARGET		= sound
TRANSLATIONS = sound-en_GB.ts sound-de.ts sound-ja.ts sound-no.ts
