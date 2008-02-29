VPATH += $$QTOPIA_DEPOT_PATH/src/libraries/qtopiapim/

QTOPIA_SQL:!qtopiadesktop:LIBS += -lqtopiasql 

win32 {
    CONFIG += dll
    DEFINES += QTOPIAPIM_MAKEDLL QTOPIA_DLL
}

QTOPIAPIM_HEADERS	+=   task.h \
	event.h \
	contact.h \
	pimrecord.h \
	addressbookaccess.h \
        todoaccess.h \
        datebookaccess.h\
	qtopiapimwinexport.h

QTOPIAPIM_PRIVATE_HEADERS += \
	xmlio_p.h \
        contactio_p.h \
        contactxmlio_p.h \
	taskio_p.h \
	todoxmlio_p.h \
        eventio_p.h \
        eventxmlio_p.h \
	abtable_p.h  \
	contactfieldlist_p.h \
	contactfieldselector_p.h \
        numberentry_p.h

QTOPIAPIM_SOURCES	+=   task.cpp \
	event.cpp \
	contact.cpp \
	pimrecord.cpp \
	xmlio.cpp \
	addressbookaccess.cpp \
        contactxmlio.cpp \
        todoaccess.cpp \
	todoxmlio.cpp \
        datebookaccess.cpp \
        eventxmlio.cpp \
	abtable.cpp \
	contactfieldlist.cpp \
	contactfieldselector.cpp \
        numberentry.cpp

QTOPIA_SQL {
    QTOPIAPIM_PRIVATE_HEADERS += sqlio_p.h \
	contactsqlio_p.h \
	eventsqlio_p.h \
	todosqlio_p.h
    QTOPIAPIM_SOURCES += sqlio.cpp \
	eventsqlio.cpp \
	todosqlio.cpp \
	contactsqlio.cpp
}
TRANSLATABLES += sqlio_p.h \
	contactsqlio_p.h \
	eventsqlio_p.h \
	todosqlio_p.h \
	sqlio.cpp \
	eventsqlio.cpp \
	todosqlio.cpp \
	contactsqlio.cpp

qtopiadesktop {
    HEADERS+=$$QTOPIAPIM_HEADERS $$QTOPIAPIM_PRIVATE_HEADERS
    SOURCES+=$$QTOPIAPIM_SOURCES
}

