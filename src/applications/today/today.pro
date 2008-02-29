multiprocess:TEMPLATE   = app
multiprocess:DESTDIR    = $(QPEDIR)/bin
singleprocess:TEMPLATE  = lib
singleprocess:DESTDIR   = $(QPEDIR)/lib
quicklaunch:TEMPLATE    = lib
quicklaunch:DESTDIR     = $(QPEDIR)/plugins/application

CONFIG += qtopia warn_on release

# Input
HEADERS += today.h todayoptions.h
SOURCES += today.cpp todayoptions.cpp
multiprocess:SOURCES+=main.cpp

INCLUDEPATH += 	library
DEPENDPATH += 	library

unix:LIBS += -lqtopia -Wl,-export-dynamic

INTERFACES = todayoptionsbase.ui

TARGET = today
TRANSLATIONS = today-en_GB.ts today-de.ts today-ja.ts today-no.ts
