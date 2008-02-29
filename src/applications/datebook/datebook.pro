qtopia_project(qtopia app)
TARGET=datebook
CONFIG+=qtopia_main

HEADERS		+= dayview.h \
		  datebook.h \
		  entrydialog.h \
		  dayviewheaderimpl.h \
		  datebooksettings.h \
                  datebookcategoryselector.h\
		  #datebookweek.h \
		  #datebookweekheaderimpl.h \
		  monthview.h \
                  timedview.h \
		  periodview.h \
		  repeatentry.h \
		  finddialog.h \
		  findwidget_p.h \
		  appointmentpicker.h \
		  exceptiondialog.h \
		  alarmdialog.h \
		  appointmentdetails.h \
                  appointmentlist.h \
                  datebookgui.h

SOURCES		+= dayview.cpp \
		  datebook.cpp \
		  entrydialog.cpp \
		  dayviewheaderimpl.cpp \
		  datebooksettings.cpp \
		  #datebookweek.cpp \
		  #datebookweekheaderimpl.cpp \
		  monthview.cpp \
                  timedview.cpp \
		  periodview.cpp \
		  repeatentry.cpp \
		  finddialog.cpp \
		  findwidget_p.cpp \
		  appointmentpicker.cpp \
		  exceptiondialog.cpp \
		  alarmdialog.cpp \
		  appointmentdetails.cpp \
                  appointmentlist.cpp \
                  datebookgui.cpp \
                  main.cpp


FORMS	+=        repeatentry.ui \
                   #datebookweekheader.ui \
		   findwidgetbase_p.ui \
		   exceptiondialogbase.ui \
		   alarmdialogbase.ui

PHONE_FORMS    += entrydetails_phone.ui \
                       datebooksettingsbase_phone.ui
PDA_FORMS      += entrydetails.ui \
                       datebooksettingsbase.ui

phone {
    FORMS      += $$PHONE_FORMS
} else {
    FORMS      += $$PDA_FORMS 
}

enable_ssl {
    HEADERS += googleaccount.h \
               accounteditor.h
    SOURCES += googleaccount.cpp \
               accounteditor.cpp
    FORMS   += googleaccount.ui
    DEFINES += GOOGLE_CALENDAR_CONTEXT
}

TRANSLATABLES   +=  $$PHONE_FORMS \
                    $$PDA_FORMS \
                    googleaccount.h \
                    googleaccount.cpp \
                    accounteditor.h \
                    accounteditor.cpp \
                    googleaccount.ui


depends(libraries/qtopiapim)

desktop.files=$$QTOPIA_DEPOT_PATH/apps/Applications/datebook.desktop
desktop.path=/apps/Applications
desktop.hint=desktop
calservice.files=$$QTOPIA_DEPOT_PATH/services/Calendar/datebook
calservice.path=/services/Calendar
timeservice.files=$$QTOPIA_DEPOT_PATH/services/TimeMonitor/datebook
timeservice.path=/services/TimeMonitor
recservice.files=$$QTOPIA_DEPOT_PATH/services/Receive/text/x-vcalendar-Events/datebook
recservice.path=/services/Receive/text/x-vcalendar-Events
qdsservice.files=$$QTOPIA_DEPOT_PATH/etc/qds/Calendar
qdsservice.path=/etc/qds
help.source=$$QTOPIA_DEPOT_PATH/help
help.files=datebook*
help.hint=help
pics.files=$$QTOPIA_DEPOT_PATH/pics/datebook/*
pics.path=/pics/datebook
pics.hint=pics
INSTALLS+=desktop calservice timeservice recservice qdsservice help pics
 
pkg.desc=Calendar for Qtopia.
pkg.domain=window,pim,alarm,qdl,qds,docapi,beaming,cardreader,bluetooth
