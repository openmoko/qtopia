# Qt util module

HEADERS += \
        util/qsystemtrayicon.h \
		util/qcompleter.h \
		util/qcompleter_p.h \
                util/qdesktopservices.h \
                util/qsystemtrayicon_p.h
		
SOURCES += \
        util/qsystemtrayicon.cpp \
		util/qcompleter.cpp \
                util/qdesktopservices.cpp
		
win32 {
		SOURCES += \
				util/qsystemtrayicon_win.cpp
}

unix:x11 {
		SOURCES += \
				util/qsystemtrayicon_x11.cpp
}

embedded {
		SOURCES += \
				util/qsystemtrayicon_qws.cpp
}

!embedded:!x11:mac {
		OBJECTIVE_SOURCES += util/qsystemtrayicon_mac.mm
}
