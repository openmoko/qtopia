TEMPLATE	= lib
CONFIG		+= qtopia warn_on debug
HEADERS	=   task.h \
	event.h \
	calendar.h \
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

SOURCES	=   task.cpp \
	event.cpp \
	calendar.cpp \
	contact.cpp \
	pimrecord.cpp \
	xmlio.cpp \
	addressbookaccess.cpp \
        contactxmlio.cpp \
        todoaccess.cpp \
	todoxmlio.cpp \
        datebookaccess.cpp \
        eventxmlio.cpp

INCLUDEPATH += $(QPEDIR)/include
LIBS		+= -lqtopiaservices -lqpe -luuid

TARGET		= qpepim
DESTDIR		= $(QPEDIR)/lib$(PROJMAK)
# This is set by configure$(QPEDIR).
VERSION         = 1.6.0

