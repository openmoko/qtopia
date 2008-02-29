singleprocess:singleprocess=true
TEMPLATE	= lib
CONFIG		+= qtopia warn_on release 
win32:CONFIG	+= dll

HEADERS	+=   task.h \
	event.h \
	contact.h \
	pimrecord.h \
	xmlio_p.h \
	addressbookaccess.h \
        contactio_p.h \
        contactxmlio_p.h \
        todoaccess.h \
	taskio_p.h \
	todoxmlio_p.h \
        datebookaccess.h \
        eventio_p.h \
        eventxmlio_p.h

SOURCES	+=   task.cpp \
	event.cpp \
	contact.cpp \
	pimrecord.cpp \
	xmlio.cpp \
	addressbookaccess.cpp \
        contactxmlio.cpp \
        todoaccess.cpp \
	todoxmlio.cpp \
        datebookaccess.cpp \
        eventxmlio.cpp

# compile in qtopia1 libraries under Windows
CONFIG += win32qdesktop unixdesktop
CONFIG += notwin32 notqdesktop notunix
win32:CONFIG -= notwin32
unix:CONFIG -= notunix
notwin32:CONFIG-=win32qdesktop
notunix:CONFIG-=unixdesktop
qdesktop:CONFIG-=notqdesktop
notqdesktop:CONFIG-=unixdesktop
notqdesktop:CONFIG-=win32qdesktop

win32qdesktop:LIBS += $(QPEDIR)/lib/qd-qpe.lib
unixdesktop:LIBS += -lqd-qpe
mac:LIBS += -lqd-qtopia

TARGET		    = qpepim
qdesktop:TARGET	    = qd-qpepim
DESTDIR		    = $(QPEDIR)/lib$(PROJMAK)
win32:DLLDESTDIR = $(QPEDIR)/bin
# This is set by configure$(QPEDIR).
VERSION         = 1.6.2

win32:DEFINES += QTOPIAPIM_MAKEDLL QTOPIA_DLL

TRANSLATIONS = libqpepim-en_GB.ts libqpepim-de.ts libqpepim-ja.ts libqpepim-no.ts
