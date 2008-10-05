!qbuild{
qtopia_project(qtopia lib)
TARGET=qmstroke
CONFIG += qtopia_visibility
}

HEADERS=\
    char.h \
    profile.h \
    signature.h \
    stroke.h

PRIVATE_HEADERS=\
    combining_p.h \

SOURCES=\
    char.cpp \
    combining.cpp \
    profile.cpp \
    signature.cpp \
    stroke.cpp

pkg.desc=Multi-stroke gesture recognition library for Qtopia.
pkg.domain=trusted

!qbuild{
headers.files=$$HEADERS
headers.path=/include/qtopia/mstroke
headers.hint=sdk headers
INSTALLS+=headers

idep(LIBS+=-l$$TARGET)
qt_inc($$TARGET)
}

help.source=$$QTOPIA_DEPOT_PATH/help
help.files=handwriting*.html
help.hint=help
INSTALLS+=help

