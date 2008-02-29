TEMPLATE	= app
CONFIG		+= qtopia warn_on release
DESTDIR		= $(QPEDIR)/bin
HEADERS		= audiodevice.h audioinput.h mediarecorder.h pluginlist.h \
		  samplebuffer.h timeprogressbar.h confrecorder.h waveform.h
SOURCES		= audiodevice.cpp audioinput.cpp main.cpp mediarecorder.cpp \
		  pluginlist.cpp samplebuffer.cpp timeprogressbar.cpp \
		  confrecorder.cpp waveform.cpp
INTERFACES      = mediarecorderbase.ui confrecorderbase.ui

TARGET		= mediarecorder
TRANSLATIONS = mediarecorder-en_GB.ts mediarecorder-de.ts mediarecorder-ja.ts mediarecorder-no.ts
