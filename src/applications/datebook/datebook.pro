!qbuild {
qtopia_project(qtopia app)
TARGET=datebook
CONFIG+=qtopia_main
depends(libraries/qtopiapim)
}

HEADERS		+= dayview.h \
		  datebook.h \
		  entrydialog.h \
		  dayviewheaderimpl.h \
		  datebooksettings.h \
                  datebookcategoryselector.h\
		  monthview.h \
                  timedview.h \
		  finddialog.h \
		  findwidget_p.h \
		  appointmentpicker.h \
		  exceptiondialog.h \
		  alarmview.h \
		  appointmentdetails.h \
                  appointmentlist.h

SOURCES		+= dayview.cpp \
		  datebook.cpp \
		  entrydialog.cpp \
		  dayviewheaderimpl.cpp \
		  datebooksettings.cpp \
		  monthview.cpp \
                  timedview.cpp \
		  finddialog.cpp \
		  findwidget_p.cpp \
		  appointmentpicker.cpp \
		  exceptiondialog.cpp \
		  alarmview.cpp \
		  appointmentdetails.cpp \
                  appointmentlist.cpp \
                  main.cpp

FORMS	+=         findwidgetbase_p.ui \
		   exceptiondialogbase.ui

HEADERS += googleaccount.h \
           accounteditor.h
SOURCES += googleaccount.cpp \
           accounteditor.cpp

#DEFINES += GOOGLE_CALENDAR_CONTEXT

dynamic.TYPE=CONDITIONAL_SOURCES
dynamic.CONDITION=!enable_singleexec
dynamic.SOURCES=\
    ../todolist/reminderpicker.cpp \
    ../todolist/recurrencedetails.cpp\
    ../todolist/qdelayedscrollarea.cpp\
    ../todolist/qtopiatabwidget.cpp
dynamic.HEADERS=\
    ../todolist/reminderpicker.h \
    ../todolist/recurrencedetails.h\
    ../todolist/qdelayedscrollarea.h\
    ../todolist/qtopiatabwidget.h
!qbuild:CONDITIONAL_SOURCES(dynamic)


desktop.files=$$QTOPIA_DEPOT_PATH/apps/Applications/datebook.desktop
desktop.path=/apps/Applications
desktop.hint=desktop
INSTALLS+=desktop
calservice.files=$$QTOPIA_DEPOT_PATH/services/Calendar/datebook
calservice.path=/services/Calendar
INSTALLS+=calservice
timeservice.files=$$QTOPIA_DEPOT_PATH/services/TimeMonitor/datebook
timeservice.path=/services/TimeMonitor
INSTALLS+=timeservice
recservice.files=$$QTOPIA_DEPOT_PATH/services/Receive/text/x-vcalendar-Events/datebook
recservice.path=/services/Receive/text/x-vcalendar-Events
INSTALLS+=recservice
qdsservice.files=$$QTOPIA_DEPOT_PATH/etc/qds/Calendar
qdsservice.path=/etc/qds
INSTALLS+=qdsservice
help.source=$$QTOPIA_DEPOT_PATH/help
help.files=datebook*
help.hint=help
INSTALLS+=help

# pics are installed by libqtopiapim since they're shared

pkg.desc=Calendar for Qtopia.
pkg.domain=trusted
