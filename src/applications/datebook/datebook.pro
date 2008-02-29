TEMPLATE	= app
CONFIG		+= qtopia warn_on release
DESTDIR		= $(QPEDIR)/bin

HEADERS		= datebookday.h \
		  datebook.h \
		  dateentryimpl.h \
		  datebookdayheaderimpl.h \
		  datebooksettings.h \
		  datebookweek.h \
		  datebookweekheaderimpl.h \
		  monthview.h \
		  layoutmanager.h \
		  periodview.h \
		  repeatentry.h \
		  datebookdb.h \
		  datepicker.h \
		  datetimeedit.h

SOURCES		= main.cpp \
		  datebookday.cpp \
		  datebook.cpp \
		  dateentryimpl.cpp \
		  datebookdayheaderimpl.cpp \
		  datebooksettings.cpp \
		  datebookweek.cpp \
		  datebookweekheaderimpl.cpp \
		  monthview.cpp \
		  layoutmanager.cpp \
		  periodview.cpp \
		  repeatentry.cpp \
		  datebookdb.cpp \
		  datepicker.cpp \
		  datetimeedit.cpp

INTERFACES	= dateentry.ui \
		  datebookdayheader.ui \
		  datebooksettingsbase.ui \
		  datebookweekheader.ui

INCLUDEPATH	+= $(QPEDIR)/include
DEPENDPATH	+= $(QPEDIR)/include
LIBS            += -lqpe -lqpepim -luuid

TARGET		= datebook

TRANSLATIONS	= $(QPEDIR)/i18n/de/datebook.ts
