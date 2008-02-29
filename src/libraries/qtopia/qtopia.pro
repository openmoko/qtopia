TEMPLATE	= lib
CONFIG		+= qtopia warn_on release
HEADERS	=   calendar.h \
	    global.h \
	    resource.h \
	    xmlreader.h \
	    mimetype.h \
	    menubutton.h \
	    network.h \
	    networkinterface.h \
	    filemanager.h \
	    fontmanager.h \
	    qdawg.h \
	    datebookmonth.h \
	    fileselector.h \
	    fileselector_p.h \
	    imageedit.h \
	    qcopenvelope_qws.h \
	    qpedecoration_qws.h \
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
	    process.h \
	    password.h \
	    timestring.h \
	    fontfactoryinterface.h \
	    fontdatabase.h \
	    power.h \
	    storage.h \
	    qpemessagebox.h \
	    timeconversion.h \
	    qpedebug.h \
	    qpemenubar.h \
	    qpetoolbar.h \
	    categories.h \
	    stringutil.h \
	    backend/palmtopuid.h \
	    backend/palmtoprecord.h \
            backend/task.h \
	    backend/event.h \
	    backend/contact.h\
	    categorymenu.h \
	    categoryedit_p.h \
	    categoryselect.h \
	    categorywidget.h \
	    ir.h \
	    backend/vobject_p.h \
	    findwidget_p.h \
	    finddialog.h \
	    lnkproperties.h \
	    styleinterface.h \
	    windowdecorationinterface.h

SOURCES	=   calendar.cpp \
	    global.cpp \
	    xmlreader.cpp \
	    mimetype.cpp \
	    menubutton.cpp \
	    network.cpp \
	    networkinterface.cpp \
	    filemanager.cpp \
	    fontmanager.cpp \
	    qdawg.cpp \
	    datebookmonth.cpp \
	    fileselector.cpp \
	    imageedit.cpp \
	    resource.cpp \
	    qpedecoration_qws.cpp \
	    qcopenvelope_qws.cpp \
	    qpeapplication.cpp \
	    qpestyle.cpp \
	    qpedialog.cpp \
	    config.cpp \
	    applnk.cpp \
	    sound.cpp \
	    tzselect.cpp \
	    qmath.c \
	    datebookdb.cpp \
	    alarmserver.cpp \
	    password.cpp \
	    process.cpp \
	    process_unix.cpp \
	    timestring.cpp \
	    fontdatabase.cpp \
	    power.cpp \
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
	ir.cpp \
	backend/vcc_yacc.cpp \
	backend/vobject.cpp \
	findwidget_p.cpp \
	finddialog.cpp \
	lnkproperties.cpp

# Qt 3 compatibility
qt2:HEADERS += quuid.h qcom.h qlibrary.h qlibrary_p.h
qt2:SOURCES += quuid.cpp qlibrary.cpp qlibrary_unix.cpp

INCLUDEPATH += $(QPEDIR)/include
LIBS		+= -ldl -lcrypt -lm

INTERFACES = passwordbase_p.ui categoryeditbase_p.ui findwidgetbase_p.ui lnkpropertiesbase_p.ui

TARGET		= qpe
DESTDIR		= $(QPEDIR)/lib$(PROJMAK)
VERSION		= 1.5.2 # Note: this is the library version. The Qtopia version is different

TRANSLATIONS = ../i18n/de/libqpe.ts
