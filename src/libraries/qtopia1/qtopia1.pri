VPATH += $$QTOPIA_DEPOT_PATH/src/libraries/qtopia1/
INCLUDEPATH	+= $$QTOPIA_DEPOT_PATH/src/server

QTOPIA1_HEADERS += accessory.h \
	datepicker.h \
	datetimeedit.h \
	fieldmapimpl.h \
	timezone.h

QTOPIA1_PRIVATE_HEADERS +=\
	pluginloaderlib_p.h\
	qcleanuphandler_p.h

QTOPIA1_SOURCES += applnk1.cpp \
	categories1.cpp \
	categoryselect1.cpp \
	config1.cpp \
	qpeapplication1.cpp \
	storage1.cpp \
	accessory.cpp \
	calendar1.cpp \
	resource1.cpp \
	datepicker.cpp \
	datetimeedit.cpp \
	timeconversion1.cpp \
	timestring1.cpp \
	global1.cpp \
	fieldmapimpl.cpp \
	timezone.cpp

win32:LIBS *= rpcrt4.lib

qtopiadesktop {
    HEADERS += $${QTOPIA1_HEADERS} $$QTOPIA1_PRIVATE_HEADERS
    SOURCES += $${QTOPIA1_SOURCES}
}

