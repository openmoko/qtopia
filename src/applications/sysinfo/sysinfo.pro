multiprocess:TEMPLATE	= app
multiprocess:DESTDIR	= $(QPEDIR)/bin
#singleprocess:TEMPLATE	= lib
#singleprocess:DESTDIR   = $(QPEDIR)/lib

CONFIG		+= qtopia warn_on release
HEADERS		= memory.h \
		  graph.h \
		  load.h \
		  storage.h \
		  versioninfo.h \
		  sysinfo.h
SOURCES		= memory.cpp \
		  graph.cpp \
		  load.cpp \
		  storage.cpp \
		  versioninfo.cpp \
		  sysinfo.cpp

multiprocess:SOURCES+=main.cpp

INTERFACES	=

TARGET		= sysinfo

TRANSLATIONS = sysinfo-en_GB.ts sysinfo-de.ts sysinfo-ja.ts sysinfo-no.ts
