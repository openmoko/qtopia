# Qt dialogs module

HEADERS += \
	dialogs/qabstractprintdialog.h \
	dialogs/qabstractprintdialog_p.h \
	dialogs/qabstractpagesetupdialog.h \
	dialogs/qabstractpagesetupdialog_p.h \
	dialogs/qcolordialog.h \
	dialogs/qdialog.h \
	dialogs/qdialog_p.h \
	dialogs/qerrormessage.h \
	dialogs/qfiledialog.h \
	dialogs/qfiledialog_p.h \
	dialogs/qfontdialog.h \
	dialogs/qinputdialog.h \
	dialogs/qmessagebox.h \
	dialogs/qpagesetupdialog.h \
	dialogs/qprintdialog.h \
	dialogs/qprogressdialog.h

!embedded:mac:SOURCES	+= dialogs/qcolordialog_mac.cpp \
                           dialogs/qfiledialog_mac.cpp
win32 {
    SOURCES += dialogs/qfiledialog_win.cpp \
	       dialogs/qpagesetupdialog_win.cpp \
	       dialogs/qprintdialog_win.cpp 
    
    !win32-borland:LIBS += -lshell32 	# the filedialog needs this library
}

mac {
	SOURCES += dialogs/qprintdialog_mac.cpp \
                   dialogs/qpagesetupdialog_mac.cpp
}
!mac:!embedded:unix {
	SOURCES += dialogs/qprintdialog_unix.cpp \
		   dialogs/qpagesetupdialog_unix.cpp
	FORMS += dialogs/qprintdialog.ui \
		dialogs/qprintpropertiesdialog.ui
}

embedded {
        contains(QT_CONFIG,qtopia) {
            DEFINES += QTOPIA_PRINTDIALOG
            SOURCES += dialogs/qprintdialog_qws.cpp \
                       dialogs/qpagesetupdialog_unix.cpp
        } else {
            SOURCES += dialogs/qprintdialog_unix.cpp \
                       dialogs/qpagesetupdialog_unix.cpp
            FORMS += dialogs/qprintdialog.ui \
                     dialogs/qprintpropertiesdialog.ui
        }
}

SOURCES += \
	dialogs/qabstractprintdialog.cpp \
	dialogs/qabstractpagesetupdialog.cpp \
	dialogs/qcolordialog.cpp \
	dialogs/qdialog.cpp \
	dialogs/qerrormessage.cpp \
	dialogs/qfiledialog.cpp \
	dialogs/qfontdialog.cpp \
	dialogs/qinputdialog.cpp \
	dialogs/qmessagebox.cpp \
	dialogs/qprogressdialog.cpp
