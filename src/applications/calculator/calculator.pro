multiprocess:TEMPLATE   = app
multiprocess:DESTDIR    = $(QPEDIR)/bin
singleprocess:TEMPLATE  = lib
singleprocess:DESTDIR   = $(QPEDIR)/lib
quicklaunch:TEMPLATE    = lib
quicklaunch:DESTDIR     = $(QPEDIR)/plugins/application

CONFIG += qtopia warn_on release

# Input
HEADERS += 
SOURCES += main.cpp 

INCLUDEPATH += $(QPEDIR)/src/libraries/qtopiacalc	

unix:LIBS   += -lqtopiacalc -Wl,-export-dynamic
win32:LIBS  += $(QPEDIR)/lib/qtopiacalc.lib

#-Wl,-export-dynamic
#export-dynamic costs around 3.5kb

TARGET = calculator

TRANSLATIONS = calculator-en_GB.ts calculator-de.ts calculator-ja.ts calculator-no.ts
