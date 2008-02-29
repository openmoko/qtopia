TEMPLATE    = lib
CONFIG	    += qtopia warn_on release
qt2:DEFINES += QTOPIA_WIN32PROCESS_SUPPORT
QPESRCDIR    = ../qtopia
QTOPIASRCDIR = ../qtopia1
HEADERS	=  \
	$$QPESRCDIR/backend/contact.h\
	$$QPESRCDIR/backend/event.h \
	$$QPESRCDIR/backend/palmtoprecord.h \
	$$QPESRCDIR/backend/palmtopuidgen.h \
	$$QPESRCDIR/backend/record.h \
	$$QPESRCDIR/backend/task.h \
	$$QPESRCDIR/backend/vobject_p.h \
	$$QPESRCDIR/calendar.h \
	$$QPESRCDIR/categories.h \
	$$QPESRCDIR/categoryedit_p.h \
	$$QPESRCDIR/categorymenu.h \
	$$QPESRCDIR/categoryselect.h \
	$$QPESRCDIR/categorywidget.h \
	$$QPESRCDIR/config.h \
        $$QTOPIASRCDIR/datepicker.h \
        $$QTOPIASRCDIR/datetimeedit.h \
	$$QPESRCDIR/datebookdb.h \
	$$QPESRCDIR/datebookmonth.h \
	$$QPESRCDIR/global.h \
	$$QPESRCDIR/menubutton.h \
	$$QPESRCDIR/mimetype.h \
	$$QPESRCDIR/qmath.h \
	$$QPESRCDIR/qpeapplication.h \
	$$QPESRCDIR/qpedialog.h \
	$$QPESRCDIR/qpemessagebox.h \
	$$QPESRCDIR/resource.h \
	$$QPESRCDIR/stringutil.h \
	$$QPESRCDIR/timeconversion.h \
	$$QPESRCDIR/timestring.h \
	$$QPESRCDIR/tzselect.h \
	$$QPESRCDIR/xmlreader.h 


SOURCES	= \
	$$QPESRCDIR/applnk.cpp \
	$$QTOPIASRCDIR/applnk1.cpp \
	$$QPESRCDIR/backend/contact.cpp \
	$$QPESRCDIR/backend/categories.cpp \
	$$QTOPIASRCDIR/categories1.cpp \
	$$QPESRCDIR/backend/event.cpp \
	$$QPESRCDIR/backend/palmtoprecord.cpp \
	$$QPESRCDIR/backend/record.cpp \
	$$QPESRCDIR/backend/stringutil.cpp \
	$$QPESRCDIR/backend/vcc_yacc.cpp \
	$$QPESRCDIR/backend/vobject.cpp \
	$$QPESRCDIR/backend/task.cpp \
	$$QPESRCDIR/backend/timeconversion.cpp \
	$$QPESRCDIR/calendar.cpp \
	$$QTOPIASRCDIR/calendar1.cpp \
	$$QPESRCDIR/categoryedit_p.cpp \
	$$QPESRCDIR/categorymenu.cpp \
	$$QPESRCDIR/categoryselect.cpp \
	$$QTOPIASRCDIR/categoryselect1.cpp \
	$$QPESRCDIR/categorywidget.cpp \
	$$QPESRCDIR/config.cpp \
        $$QTOPIASRCDIR/datepicker.cpp \
        $$QTOPIASRCDIR/datetimeedit.cpp \
	$$QPESRCDIR/datebookdb.cpp \
	$$QPESRCDIR/datebookmonth.cpp \
	$$QPESRCDIR/global.cpp \
	$$QTOPIASRCDIR/global1.cpp \
	$$QPESRCDIR/menubutton.cpp \
	$$QPESRCDIR/qmath.cpp \
	$$QPESRCDIR/qpeapplication.cpp \
	$$QTOPIASRCDIR/qpeapplication1.cpp \
	$$QPESRCDIR/qpedialog.cpp \
	$$QPESRCDIR/qpemessagebox.cpp \
	$$QPESRCDIR/resource.cpp \
	$$QPESRCDIR/timestring.cpp \
	$$QPESRCDIR/tzselect.cpp \
	$$QPESRCDIR/xmlreader.cpp 


# Qt 3 compatibility
qt2:HEADERS += \
	    $$QPESRCDIR/quuid.h \ 
	    $$QPESRCDIR/qcom.h \
	    $$QPESRCDIR/qlibrary.h \
	    $$QPESRCDIR/qlibrary_p.h
qt2:SOURCES += \
	    $$QPESRCDIR/process_win.cpp \
	    $$QPESRCDIR/quuid.cpp \
	    $$QPESRCDIR/qlibrary.cpp \ 
	    $$QPESRCDIR/qlibrary_win.cpp

embedded:SOURCES = \
	$$QPESRCDIR/alarmserver.cpp \
	$$QTOPIASRCDIR/devicebutton.cpp \
	$$QTOPIASRCDIR/devicebuttonmanager.cpp \
	$$QPESRCDIR/filemanager.cpp \
	$$QPESRCDIR/fileselector.cpp \
	$$QTOPIASRCDIR/fileselector1.cpp \
	$$QPESRCDIR/fontdatabase.cpp \
	$$QPESRCDIR/fontmanager.cpp \
	$$QPESRCDIR/imageedit.cpp \
	$$QPESRCDIR/ir.cpp \ 
	$$QPESRCDIR/lnkproperties.cpp \
	$$QPESRCDIR/mimetype.cpp \
	$$QPESRCDIR/network.cpp \
	$$QPESRCDIR/networkinterface.cpp \
	$$QPESRCDIR/password.cpp \
	$$QPESRCDIR/power.cpp \
	$$QPESRCDIR/process.cpp \
	$$QPESRCDIR/qcopenvelope_qws.cpp \
	$$QPESRCDIR/qdawg.cpp \
	$$QPESRCDIR/qpedebug.cpp \
	$$QPESRCDIR/qpedecoration_qws.cpp \
	$$QPESRCDIR/qpemenubar.cpp \
	$$QPESRCDIR/qpestyle.cpp \
	$$QPESRCDIR/qpetoolbar.cpp \
	$$QTOPIASRCDIR/qwizard.cpp \
	$$QPESRCDIR/sound.cpp \
	$$QPESRCDIR/timestring.cpp \
	$$QTOPIASRCDIR/services.cpp \
	$$QPESRCDIR/storage.cpp \
	$$QTOPIASRCDIR/storage1.cpp 


embedded:HEADERS = \
	$$QTOPIASRCDIR/accessory.h \ 
	$$QPESRCDIR/alarmserver.h \
	$$QPESRCDIR/applnk.h \
	$$QPESRCDIR/backend/stringutil.h \
	$$QPESRCDIR/backend/timeconversion.h \
	$$QTOPIASRCDIR/devicebutton.h \
	$$QTOPIASRCDIR/devicebuttonmanager.h \
	$$QPESRCDIR/filemanager.h \
	$$QPESRCDIR/fileselector.h \
	$$QPESRCDIR/fileselector_p.h \
	$$QPESRCDIR/fontfactoryinterface.h \
	$$QPESRCDIR/fontdatabase.h \
	$$QPESRCDIR/fontmanager.h \
	$$QPESRCDIR/imageedit.h \
	$$QPESRCDIR/ir.h \
	$$QPESRCDIR/lnkproperties.h \
	$$QPESRCDIR/network.h \
	$$QPESRCDIR/networkinterface.h \
	$$QPESRCDIR/password.h \
	$$QPESRCDIR/power.h \
	$$QPESRCDIR/process.h \
	$$QPESRCDIR/qcopenvelope_qws.h \
	$$QPESRCDIR/qdawg.h \
	$$QPESRCDIR/qpedebug.h \
	$$QPESRCDIR/qpedecoration_qws.h \
        $$QPESRCDIR/qpemenubar.h \
	$$QPESRCDIR/qpestyle.h \
	$$QPESRCDIR/qpetoolbar.h \
	$$QTOPIASRCDIR/qwizard.h \
	$$QTOPIASRCDIR/services.h \
	$$QPESRCDIR/sound.h \
	$$QPESRCDIR/storage.h \
	$$QPESRCDIR/styleinterface.h \
	$$QPESRCDIR/timestring.h \
	$$QPESRCDIR/windowdecorationinterface.h


INCLUDEPATH += $(QPEDIR)/src/server

INTERFACES = \
	    $$QPESRCDIR/passwordbase_p.ui \
	    $$QPESRCDIR/categoryeditbase_p.ui \
	    $$QPESRCDIR/lnkpropertiesbase_p.ui

DEPENDPATH += $$QPESRCDIR $$QTOPIASRCDIR
TARGET		= qtopia 
qdesktop:TARGET = qd-qpe

DESTDIR		= $(QPEDIR)/lib$(PROJMAK)

VERSION		= 1.6.0 # Note: this is the library version. The Qtopia version is different
LIBS	    += rpcrt4.lib
TRANSLATIONS = libqtopia-en_GB.ts libqtopia-de.ts libqtopia-ja.ts libqtopia-no.ts
