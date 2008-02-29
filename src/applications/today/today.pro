TEMPLATE = app
CONFIG += qtopia warn_on release
DESTDIR = $(QPEDIR)/bin

# Input
HEADERS += today.h todayoptions.h
SOURCES += today.cpp todayoptions.cpp main.cpp

INCLUDEPATH += 	library
DEPENDPATH += 	library

unix:LIBS += -lqtopia -Wl,-export-dynamic

INTERFACES = todayoptionsbase.ui

TARGET = today
TRANSLATIONS = today-en_GB.ts today-de.ts today-ja.ts today-no.ts
