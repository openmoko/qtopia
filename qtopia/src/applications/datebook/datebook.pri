CONFIG		+= qtopiaapp

TARGET		= datebook

VPATH		+= $$QTOPIA_DEPOT_PATH/src/applications/datebook/

unix:!mac:LIBS	+=-luuid

HEADERS		+= dayview.h \
		  datebook.h \
		  entrydialog.h \
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
		  findwidget_p.h \
		  eventpicker.h \
		  exceptiondialog.h \
		  alarmdialog.h \
		  eventview.h

SOURCES		+= dayview.cpp \
		  datebook.cpp \
		  entrydialog.cpp \
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
		  findwidget_p.cpp \
		  eventpicker.cpp \
		  exceptiondialog.cpp \
		  alarmdialog.cpp \
		  eventview.cpp

INTERFACES	+= datebookweekheader.ui \
		   findwidgetbase_p.ui \
		   exceptiondialogbase.ui \
		   alarmdialogbase.ui

TRANSLATABLES   +=  $$INTERFACES \
                    $$SOURCES \
                    $$HEADERS

PHONE_INTERFACES    += entrydetails_phone.ui \
                       datebooksettingsbase_phone.ui
PDA_INTERFACES      += entrydetails.ui \
                       datebooksettingsbase.ui

!qtopiadesktop:QTOPIA_PHONE {
    INTERFACES      += $$PHONE_INTERFACES
} else {
    INTERFACES      += $$PDA_INTERFACES 
}

TRANSLATABLES   +=  $$PHONE_INTERFACES \
                    $$PDA_INTERFACES

