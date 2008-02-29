TEMPLATE	= app
CONFIG		+= qtopia warn_on release
DESTDIR		= $(QPEDIR)/bin
HEADERS		= performancetest.h
SOURCES		= main.cpp performancetest.cpp
INTERFACES	= performancetestbase.ui
TARGET		= performance

TRANSLATIONS = performance-en_GB.ts performance-de.ts performance-ja.ts performance-no.ts
