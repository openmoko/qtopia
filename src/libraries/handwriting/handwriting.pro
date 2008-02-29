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

help.files=$${QTOPIA_DEPOT_PATH}/help/html/handwriting*.html
help.path=/help/html

INSTALLS += help

#different name?
TARGET = qmstroke

DESTDIR	= $$(QPEDIR)/lib

PACKAGE_DESCRIPTION=Multi-stroke gesture recognition library for the Qtopia environment.


