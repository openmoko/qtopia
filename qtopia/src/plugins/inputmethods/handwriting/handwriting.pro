CONFIG	    += qtopiaplugin 

TARGET	    = qhandwriting

HEADERS	    = handwritingimpl.h \
		help.h \
		penwidget.h \
		qimpeninput.h \
		wordpick.h
SOURCES	    = handwritingimpl.cpp \
		help.cpp \
		penwidget.cpp \
		qimpeninput.cpp \
		wordpick.cpp

LIBS += -lqmstroke

TRANSLATABLES = $${SOURCES} $${HEADERS}

TRANSLATABLES +=  qimpeninput.cpp \
                    qimpeninput.h


INTERFACES  = prefbase.ui

TRANSLATABLES += $${INTERFACES}

etc.files = $${QTOPIA_DEPOT_PATH}/etc/qimpen/asciilower.qpt\
	$${QTOPIA_DEPOT_PATH}/etc/qimpen/asciiupper.qpt\
	$${QTOPIA_DEPOT_PATH}/etc/qimpen/combining.qpt\
	$${QTOPIA_DEPOT_PATH}/etc/qimpen/numeric.qpt\
	$${QTOPIA_DEPOT_PATH}/etc/qimpen/popup.conf

etc.path=/etc/qimpen/

desktop.files=$${QTOPIA_DEPOT_PATH}/plugins/inputmethods/qhandwriting.desktop
desktop.path=/plugins/inputmethods/

INSTALLS+=etc desktop

pics.files=$${QTOPIA_DEPOT_PATH}/pics/hw/*
pics.path=/pics/hw

PICS_INSTALLS += pics

PACKAGE_NAME = qpe-handwriting
