TEMPLATE	= app
TARGET		= qdawggen
DESTDIR		= $$(QPEDIR)/bin
CONFIG		+= desktop

HEADERS		= qdawg.h qmemoryfile_p.h global.h
SOURCES		= main.cpp qdawg.cpp qmemoryfile.cpp global.cpp

unix:SOURCES    += qmemoryfile_unix.cpp
    
win32:SOURCES    += qmemoryfile_win.cpp

VPATH           += $${QTOPIA_DEPOT_PATH}/src/libraries/qtopia
TRANSLATIONS	=

