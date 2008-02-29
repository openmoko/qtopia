CONFIG	+= qtopia
TEMPLATE = lib

HEADERS += char.h \
           combining.h \
           match.h \
           profile.h \
           signiture.h \
           stroke.h

SOURCES += char.cpp \
           combining.cpp \
           match.cpp \
           profile.cpp \
           signiture.cpp \
           stroke.cpp

TRANSLATABLES = $${SOURCES} $${HEADERS}

help.source=$${QTOPIA_DEPOT_PATH}/help
help.files=handwriting*.html

HELP_INSTALLS+=help

#different name?
TARGET = qmstroke

DESTDIR	= $$(QPEDIR)/lib

PACKAGE_DESCRIPTION=Multi-stroke gesture recognition library for the Qtopia environment.


