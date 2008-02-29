CONFIG		+= qtopiaapp
HEADERS		= clockmain.h \
		    clock.h \
		    stopwatch.h \
		    alarm.h \
		    analogclock.h
SOURCES		= clockmain.cpp \
		    clock.cpp \
		    stopwatch.cpp \
		    analogclock.cpp \
		    alarm.cpp \
		    main.cpp

INTERFACES	= clockbase.ui stopwatchbase.ui alarmbase.ui
TARGET		= clock

TRANSLATABLES   =   $$INTERFACES \
                    $$SOURCES \
                    $$HEADERS

desktop.files=$${QTOPIA_DEPOT_PATH}/apps/Applications/clock.desktop
desktop.path=/apps/Applications
service.files=$${QTOPIA_DEPOT_PATH}/services/Alarm/clock
service.path=/services/Alarm
help.source=$${QTOPIA_DEPOT_PATH}/help
help.files=clock*
pics.files=$${QTOPIA_DEPOT_PATH}/pics/clock/*
pics.path=/pics/clock
INSTALLS+=desktop service
HELP_INSTALLS+=help
PICS_INSTALLS+=pics

PACKAGE_DESCRIPTION=A simple clock and stop-watch for the Qtopia environment.

