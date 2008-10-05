!qbuild{
qtopia_project(qtopia core lib)
TARGET=qtopiasecurity
VERSION=4.0.0
CONFIG+=qtopia_visibility no_tr
}

HEADERS = \
    qsxepolicy.h \
    qpackageregistry.h

PRIVATE_HEADERS = \
    keyfiler_p.h

SOURCES = \
    qsxepolicy.cpp \
    keyfiler.cpp \
    qpackageregistry.cpp

!qbuild{
headers.files=$$HEADERS
headers.path=/include/qtopia/security
headers.hint=headers sdk
INSTALLS+=headers

idep(LIBS+=-l$$TARGET)
qt_inc($$TARGET)
}
