TEMPLATE	= app
CONFIG		+= qtopia warn_on release
DESTDIR		= $(QPEDIR)/bin
HEADERS		= launchersettings.h wallpaperselector.h colorselector.h
SOURCES		= main.cpp launchersettings.cpp wallpaperselector.cpp colorselector.cpp
INTERFACES	= launchersettingsbase.ui wallpaperselectorbase.ui

TARGET		= launchersettings

TRANSLATIONS = launchersettings-en_GB.ts launchersettings-de.ts launchersettings-ja.ts launchersettings-no.ts
