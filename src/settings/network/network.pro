TEMPLATE	= app
CONFIG		+= qtopia warn_on release
DESTDIR		= $(QPEDIR)/bin

HEADERS		= networksettings.h addnetworksetting.h
SOURCES		= main.cpp networksettings.cpp addnetworksetting.cpp
INTERFACES	= addnetworksettingbase.ui networksettingsbase.ui

TARGET		= netsetup

TRANSLATIONS = netsetup-en_GB.ts netsetup-de.ts netsetup-ja.ts netsetup-no.ts
