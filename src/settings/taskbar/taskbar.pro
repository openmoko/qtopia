TEMPLATE	= app
CONFIG		+= qtopia warn_on release
DESTDIR		= $(QPEDIR)/bin

HEADERS		= taskbarsettings.h
SOURCES		= taskbarsettings.cpp main.cpp
INTERFACES	= taskbarsettingsbase.ui

INCLUDEPATH	+= $(QPEDIR)/include
DEPENDPATH	+= $(QPEDIR)/include

TARGET		= taskbarsettings
TRANSLATIONS = taskbarsettings-en_GB.ts taskbarsettings-de.ts taskbarsettings-ja.ts taskbarsettings-no.ts
