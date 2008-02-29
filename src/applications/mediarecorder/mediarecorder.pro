multiprocess:TEMPLATE   = app
multiprocess:DESTDIR    = $(QPEDIR)/bin
singleprocess:TEMPLATE  = lib
singleprocess:DESTDIR   = $(QPEDIR)/lib
quicklaunch:TEMPLATE    = lib
quicklaunch:DESTDIR     = $(QPEDIR)/plugins/application

CONFIG		+= qtopia warn_on release
HEADERS		= audiodevice.h audioinput.h mediarecorder.h pluginlist.h \
		  samplebuffer.h timeprogressbar.h confrecorder.h waveform.h
SOURCES		= audiodevice.cpp audioinput.cpp mediarecorder.cpp \
		  pluginlist.cpp samplebuffer.cpp timeprogressbar.cpp \
		  confrecorder.cpp waveform.cpp
multiprocess:SOURCES+=main.cpp
INTERFACES      = mediarecorderbase.ui confrecorderbase.ui

TARGET		= mediarecorder
TRANSLATIONS = mediarecorder-en_GB.ts mediarecorder-de.ts mediarecorder-ja.ts mediarecorder-no.ts

unix:LIBS       += -lpthread
