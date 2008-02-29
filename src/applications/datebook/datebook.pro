multiprocess:TEMPLATE	= app
multiprocess:DESTDIR	= $(QPEDIR)/bin
singleprocess:TEMPLATE	= lib
singleprocess:DESTDIR   = $(QPEDIR)/lib

CONFIG		+= qtopia warn_on release

HEADERS		= dayview.h \
		  datebook.h \
		  dateentryimpl.h \
		  dayviewheaderimpl.h \
		  datebooksettings.h \
		  datebookweek.h \
		  datebookweekheaderimpl.h \
		  monthview.h \
		  layoutmanager.h \
		  periodview.h \
		  repeatentry.h \
		  datebookdb.h \
		  finddialog.h \
		  findwidget_p.h

SOURCES		= dayview.cpp \
		  datebook.cpp \
		  dateentryimpl.cpp \
		  dayviewheaderimpl.cpp \
		  datebooksettings.cpp \
		  datebookweek.cpp \
		  datebookweekheaderimpl.cpp \
		  monthview.cpp \
		  layoutmanager.cpp \
		  periodview.cpp \
		  repeatentry.cpp \
		  datebookdb.cpp \
		  finddialog.cpp \
		  findwidget_p.cpp


INTERFACES	= dateentry.ui \
		  dayviewheader.ui \
		  datebooksettingsbase.ui \
		  datebookweekheader.ui \
		  findwidgetbase_p.ui

multiprocess:SOURCES += main.cpp

unix:LIBS       += -lqpepim -luuid
win32:LIBS      += $(QPEDIR)/lib/qpepim.lib 

TARGET		= datebook

TRANSLATIONS = datebook-en_GB.ts datebook-de.ts datebook-ja.ts datebook-no.ts
