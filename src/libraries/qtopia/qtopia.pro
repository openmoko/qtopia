singleprocess:singleprocess=true
TEMPLATE	= lib
CONFIG		+= qtopia warn_on release
win32:CONFIG	+= dll
DEFINES += QTOPIA_WIN32PROCESS_SUPPORT 

HEADERS	=   calendar.h \
	    global.h \
		qpeglobal.h \
	    resource.h \
	    xmlreader.h \
	    mimetype.h \
	    menubutton.h \
	    filemanager.h \
	    datebookmonth.h \
	    fileselector.h \
	    fileselector_p.h \
	    imageedit.h \
	    qpeapplication.h \
	    qpestyle.h \
	    qpedialog.h \
	    config.h \
	    applnk.h \
	    sound.h \
	    tzselect.h \
	    qmath.h \
	    datebookdb.h \
	    alarmserver.h \
	    password.h \
	    timestring.h \
	    fontfactoryinterface.h \
	    storage.h \
	    qpemessagebox.h \
	    timeconversion.h \
	    qpedebug.h \
	    qpemenubar.h \
	    qpetoolbar.h \
	    categories.h \
	    stringutil.h \
	    backend/palmtoprecord.h \
            backend/task.h \
	    backend/event.h \
	    backend/contact.h\
	    backend/qfiledirect_p.h \
	    categorymenu.h \
	    categoryedit_p.h \
	    categoryselect.h \
	    categorywidget.h \
	    backend/vobject_p.h \
	    styleinterface.h \
	    windowdecorationinterface.h \
	    qdawg.h 

SOURCES	=   calendar.cpp \
	    global.cpp \
	    custom.cpp \
	    xmlreader.cpp \
	    mimetype.cpp \
	    menubutton.cpp \
	    filemanager.cpp \
	    datebookmonth.cpp \
	    fileselector.cpp \
	    imageedit.cpp \
	    resource.cpp \
	    qpeapplication.cpp \
	    qpestyle.cpp \
	    qpedialog.cpp \
	    config.cpp \
	    applnk.cpp \
	    sound.cpp \
	    tzselect.cpp \
	    qmath.cpp \
	    datebookdb.cpp \
	    alarmserver.cpp \
	    password.cpp \
	    timestring.cpp \
	    storage.cpp \
	    qpemessagebox.cpp \
            backend/timeconversion.cpp \
	    qpedebug.cpp \
	    qpemenubar.cpp \
	    qpetoolbar.cpp \
	    backend/categories.cpp \
	    backend/stringutil.cpp \
	    backend/palmtoprecord.cpp \
            backend/task.cpp \
	    backend/event.cpp \
	    backend/contact.cpp \
	    categorymenu.cpp \
	    categoryedit_p.cpp \
	    categoryselect.cpp \
	    categorywidget.cpp \
	    backend/vcc_yacc.cpp \
	    backend/vobject.cpp \
	    mediarecorderplugininterface.cpp \
	    qdawg.cpp

QTOPIA1DIR = ../qtopia1
	
# Qt 3 compatibility
qt2:CONFIG+=notqt2unix notqt2win 
unix:CONFIG-=notqt2unix
win32:CONFIG-=notqt2win

qt2:HEADERS += quuid.h \
	    qcom.h \
	    qlibrary.h \
	    qlibrary_p.h \
	    process.h
qt2:SOURCES += quuid.cpp \
	qlibrary.cpp \
	process.cpp 

notqt2win:SOURCES += process_unix.cpp qlibrary_unix.cpp
notqt2unix:SOURCES += process_win.cpp qlibrary_win.cpp

embedded:HEADERS +=	fontmanager.h \
			fontdatabase.h \
			qpedecoration_p.h \
			network.h \
			networkinterface.h \
			qcopenvelope_qws.h \
			power.h \
			ir.h \
			pluginloader_p.h

embedded:SOURCES += 	fontmanager.cpp \
			fontdatabase.cpp \
			qpedecoration_qws.cpp \
			network.cpp \
		   	networkinterface.cpp \
	    		qcopenvelope_qws.cpp \
	    		power.cpp \
			ir.cpp \
			pluginloader_p.cpp

INCLUDEPATH += $(QPEDIR)/include
win32:INCLUDEPATH += $(QPEDIR)/src/server
dynamic:unix:LIBS	+= -ldl
unix:LIBS	+= -lcrypt -lm
win32:LIBS	+= rpcrt4.lib

INTERFACES += passwordbase_p.ui \
	    categoryeditbase_p.ui 

TARGET = qpe
qdesktop:TARGET	= qd-qpe
DESTDIR		= $(QPEDIR)/lib
win32:DLLDESTDIR = $(QPEDIR)/bin
VERSION		= 1.5.2 # Note: this is the library version. The Qtopia version is different
win32:DEFINES += QTOPIA_MAKEDLL QTOPIA_PLUGIN_MAKEDLL\
		QTOPIA_INTERNAL_APPLNKASSIGN QTOPIA_INTERNAL_FSLP QTOPIA_INTERNAL_PRELOADACCESS QTOPIA_INTERNAL_FD 

TRANSLATIONS = libqpe-en_GB.ts libqpe-de.ts libqpe-ja.ts libqpe-no.ts



# don't touch if you don't know what you are doing
# compile in qtopia1 libraries under Windows
CONFIG += win32emb
CONFIG += notwin32 notembedded
win32:CONFIG -= notwin32
notwin32:CONFIG-=win32emb
embedded:CONFIG-=notembedded
notembedded:CONFIG-=win32emb

win32emb:HEADERS +=	$$QTOPIA1DIR/services.h \
			$$QTOPIA1DIR/devicebuttonmanager.h \
		    	$$QTOPIA1DIR/devicebutton.h \
			$$QTOPIA1DIR/qwizard.h	\
    			$$QTOPIA1DIR/docproperties.h \
			$$QTOPIA1DIR/pluginloader.h \
		    	$$QTOPIA1DIR/pluginloaderlib_p.h \
			$$QTOPIA1DIR/locationcombo.h \
			$$QTOPIA1DIR/qprocess.h

		    

win32emb:SOURCES +=  	$$QTOPIA1DIR/services.cpp \
			$$QTOPIA1DIR/devicebuttonmanager.cpp \
			$$QTOPIA1DIR/devicebutton.cpp \
			$$QTOPIA1DIR/qwizard.cpp \
    			$$QTOPIA1DIR/docproperties.cpp \
			$$QTOPIA1DIR/pluginloader.cpp \
			$$QTOPIA1DIR/pluginloaderlib.cpp \
			$$QTOPIA1DIR/locationcombo.cpp \ 
			$$QTOPIA1DIR/qprocess.cpp \ 	
			$$QTOPIA1DIR/qprocess_win.cpp \
			$$QTOPIA1DIR/quuid1.cpp

win32:HEADERS	+=  $$QTOPIA1DIR/accessory.h \
	    	    $$QTOPIA1DIR/datepicker.h \
		    $$QTOPIA1DIR/datetimeedit.h \
		    $$QTOPIA1DIR/fieldmapimpl.h \
		    $$QTOPIA1DIR/timezone.h 

win32:SOURCES	+=  $$QTOPIA1DIR/applnk1.cpp \
		    $$QTOPIA1DIR/config1.cpp \
		    $$QTOPIA1DIR/categories1.cpp \
		    $$QTOPIA1DIR/categoryselect1.cpp \
		    $$QTOPIA1DIR/fileselector1.cpp \
		    $$QTOPIA1DIR/qpeapplication1.cpp \
		    $$QTOPIA1DIR/accessory.cpp \
		    $$QTOPIA1DIR/calendar1.cpp \
		    $$QTOPIA1DIR/datepicker.cpp \
		    $$QTOPIA1DIR/datetimeedit.cpp \
		    $$QTOPIA1DIR/global1.cpp \
		    $$QTOPIA1DIR/storage1.cpp \
		    $$QTOPIA1DIR/resource1.cpp \
		    $$QTOPIA1DIR/fieldmapimpl.cpp \
		    $$QTOPIA1DIR/timezone.cpp \
		    $$QTOPIA1DIR/timestring1.cpp \
		    $$QTOPIA1DIR/timeconversion1.cpp
		

win32emb:INTERFACES +=	
win32emb:DEPENDPATH +=  $$QTOPIA1DIR

qt3:CONFIG += notqt3win32 notqt3unix
win32:CONFIG -= notqt3win32 
unix:CONFIG -= notqt3unix
qt2:CONFIG -= notqt3win32 notqt3unix

# we need to supply QMemoryFile for Qt2.x and Qt3.x for the moment
#qt3:SOURCES += qmemoryfile.cpp
#qt3:HEADERS += qmemoryfile_p.h
#notqt3unix:SOURCES  += qmemoryfile_win.cpp
#notqt3win32:SOURCES += qmemoryfile_unix.cpp
SOURCES += qmemoryfile.cpp
HEADERS += qmemoryfile_p.h
win32:SOURCES  += qmemoryfile_win.cpp
unix:SOURCES += qmemoryfile_unix.cpp

