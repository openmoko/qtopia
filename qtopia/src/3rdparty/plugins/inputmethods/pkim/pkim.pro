TEMPLATE     = qtopiaplugin
CONFIG      += qtopiaplugin warn_on release
HEADERS      = pkim.h \
               pkimmatcher.h \
               pkimpl.h \
	       picker.h \
	       modepicker.h \
	       symbolpicker.h \
	       wordpicker.h \
	       charlist.h
SOURCES      = pkimpl.cpp \
               pkim.cpp \
               pkimmatcher.cpp \
	       pkimcustom.cpp \
	       picker.cpp \
	       modepicker.cpp \
	       symbolpicker.cpp \
	       wordpicker.cpp \
	       charlist.cpp
TARGET       = pkim
DESTDIR      = $$(QPEDIR)/plugins/inputmethods
INCLUDEPATH += $$(QPEDIR)/include
LIBS        += -lqpe
VERSION      = 1.0.0

TRANSLATABLES=$${SOURCES} $${HEADERS}                 

im.files=$${QTOPIA_DEPOT_PATH}/etc/im/pkim/*
im.path=/etc/im/pkim
pics.files=$${QTOPIA_DEPOT_PATH}/pics/pkim/*
pics.path=/pics/pkim
dictinternet.files=$${QTOPIA_DEPOT_PATH}/etc/dict/internet.dawg
dictinternet.path=/etc/dict

INSTALLS+=im dictinternet
PICS_INSTALLS+=pics

