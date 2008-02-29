VPATH += $$QTOPIA_DEPOT_PATH/src/libraries/qtopia2/
INCLUDEPATH	+= $$QTOPIA_DEPOT_PATH/src/server

QTOPIA2_SOURCES += qpeapplication2.cpp \
	   categories2.cpp \
	   global2.cpp \
	   password2.cpp \
	   image.cpp \
	   vscrollview.cpp \
	   stringutil2.cpp \
	   datetimeedit2.cpp \
	   pixmapdisplay.cpp \
	   locationcombo2.cpp \
	   categorydialog.cpp \
	   config2.cpp \
	   iconselect.cpp  \
	   inputmethodinterface_p.cpp  \
	   resource2.cpp \
	   actionconfirm.cpp\
	   timestring2.cpp\
	   fileselector2.cpp

QTOPIA2_HEADERS += vscrollview.h \
	   categorydialog.h \
	   pixmapdisplay.h \
	   iconselect.h  \
	   actionconfirm.h

QTOPIA2_PRIVATE_HEADERS += \
	   inputmethodinterface_p.h

QTOPIA_SQL {
    QTOPIA2_SOURCES += qpesql.cpp
    QTOPIA2_HEADERS += qpesql.h \
	   qsqldriverinterface.h
}
TRANSLATABLES += qpesql.cpp qpesql.h qsqldriverinterface.h

qtopiadesktop {
    HEADERS += $$QTOPIA2_HEADERS $$QTOPIA2_PRIVATE_HEADERS
    SOURCES += $$QTOPIA2_SOURCES
}

