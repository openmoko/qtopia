VPATH += $$QTOPIA_DEPOT_PATH/src/libraries/qtopia/
INCLUDEPATH += $$(QPEDIR)/include/qtopia/private

# we need to supply QMemoryFile for Qt2.x and Qt3.x for the moment
QTOPIA_SOURCES += qmemoryfile.cpp
QTOPIA_PRIVATE_HEADERS += qmemoryfile_p.h

QTOPIA_INTERFACES += passwordbase_p.ui \
              categoryeditbase_p.ui 

qtest {
    include($$(QTESTDIR)/include/qtest_qtopia.pri)
}

QTOPIA_HEADERS	+=   calendar.h \
	global.h \
	resource.h \
	mimetype.h \
	menubutton.h \
	filemanager.h \
	fileselector.h \
	qpeapplication.h \
	qpestyle.h \
	qpedialog.h \
	config.h \
	applnk.h \
	sound.h \
	tzselect.h \
	qmath.h \
	alarmserver.h \
	password.h \
	timestring.h \
	fontfactoryinterface.h \
	storage.h \
	qpemessagebox.h \
	qpedebug.h \
	qpemenubar.h \
	qpetoolbar.h \
	categories.h \
	stringutil.h \
	categorymenu.h \
	categoryselect.h \
	categorywidget.h \
	styleinterface.h \
	windowdecorationinterface.h \
	mediarecorderplugininterface.h \
	mediaplayerplugininterface.h \
	qdawg.h \
	custom.h\
	qpeglobal.h\
	applicationinterface.h\
	imagecodecinterface.h\
	inputmethodinterface.h\
	qtopiawinexport.h\
	taskbarappletinterface.h\
	textcodecinterface.h\
	todayplugininterface.h\
	version.h

QTOPIA_PRIVATE_HEADERS += \
	localtr_p.h\
	quickexec_p.h\
	fileselector_p.h \
	categoryedit_p.h

QTOPIA_SOURCES	+=   calendar.cpp \
	global.cpp \
	localtr.cpp \
	mimetype.cpp \
	menubutton.cpp \
	filemanager.cpp \
	fileselector.cpp \
	resource.cpp \
	qpeapplication.cpp \
	qpestyle.cpp \
	qpedialog.cpp \
	config.cpp \
	applnk.cpp \
	sound.cpp \
	tzselect.cpp \
	qmath.cpp \
	alarmserver.cpp \
	password.cpp \
	timestring.cpp \
	storage.cpp \
	qpemessagebox.cpp \
	qpedebug.cpp \
	qpemenubar.cpp \
	qpetoolbar.cpp \
	categorymenu.cpp \
	categoryedit_p.cpp \
	categoryselect.cpp \
	categorywidget.cpp \
	mediarecorderplugininterface.cpp \
	mediaplayerplugininterface.cpp \
	qdawg.cpp

BACKEND_PRIVATE_HEADERS+= backend/qfiledirect_p.h \
	backend/vobject_p.h \
        backend/cp1252textcodec_p.h

BACKEND_SOURCES += backend/categories.cpp \
	backend/stringutil.cpp \
	backend/vcc_yacc.cpp \
	backend/vobject.cpp \
        backend/cp1252textcodec.cpp

#These are being marked for future removal.
#dealing with these should save 200k of the
#stripped lib size.
# contact - used by passwd to print contact info
# event - used by datebookdb
# datebookdb - used by settime to reset alarms
# timeconversion - used by alarm server
# palmtoprecord - used by contact && event

QTOPIA_HEADERS += datebookdb.h timeconversion.h

BACKEND_PRIVATE_HEADERS += backend/contact.h \
	backend/event.h \
	backend/palmtoprecord.h

QTOPIA_SOURCES += datebookdb.cpp

BACKEND_SOURCES += backend/contact.cpp \
	backend/event.cpp \
	backend/timeconversion.cpp \
	backend/palmtoprecord.cpp

!nocompat {
	QTOPIA_HEADERS += xmlreader.h \
	    imageedit.h \
	    datebookmonth.h

	BACKEND_PRIVATE_HEADERS += backend/task.h
    
	QTOPIA_SOURCES += xmlreader.cpp \
	    imageedit.cpp \
	    datebookmonth.cpp

	BACKEND_SOURCES += backend/task.cpp
}

TRANSLATABLES += xmlreader.h \
	imageedit.h \
	datebookmonth.h \
        backend/task.h \
        xmlreader.cpp \
	imageedit.cpp \
	datebookmonth.cpp \
        backend/task.cpp

win32 { 
    QTOPIA_SOURCES  += qmemoryfile_win.cpp
    INCLUDEPATH += $$QTOPIA_DEPOT_PATH/src/server
    #LIBS	+= rpcrt4.lib
    DEFINES += QTOPIA_MAKEDLL QTOPIA_PLUGIN_MAKEDLL\
	QTOPIA_INTERNAL_APPLNKASSIGN QTOPIA_INTERNAL_FSLP \
	QTOPIA_INTERNAL_PRELOADACCESS QTOPIA_INTERNAL_FD  \
	QTOPIA_WIN32PROCESS_SUPPORT
}

TRANSLATABLES += qmemoryfile_win.cpp

unix {
    QTOPIA_SOURCES += qmemoryfile_unix.cpp
    !mac:LIBS      += -lcrypt -luuid
    LIBS           += -lm
    !staticlib:LIBS+=$$QMAKE_LIBS_DYNLOAD
}

TRANSLATABLES += qmemoryfile_unix.cpp

qtopiadesktop {
    HEADERS+=$$QTOPIA_HEADERS $$QTOPIA_PRIVATE_HEADERS $$BACKEND_PRIVATE_HEADERS
    SOURCES+=$$QTOPIA_SOURCES $$BACKEND_SOURCES
    INTERFACES+=$$QTOPIA_INTERFACES
}

