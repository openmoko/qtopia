CONFIG		+= qtopia warn_on release
TEMPLATE	= app
DESTDIR		= $(QPEDIR)/bin

HEADERS		= worldtime.h zonemap.h sun.h stylusnormalizer.h
SOURCES		= worldtime.cpp zonemap.cpp main.cpp sun.cpp stylusnormalizer.cpp
INTERFACES	= worldtimebase.ui

TARGET		= worldtime

TRANSLATIONS = worldtime-en_GB.ts worldtime-de.ts worldtime-ja.ts worldtime-no.ts
