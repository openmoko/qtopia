!qbuild{
qtopia_project(qtopia lib)
TARGET=qtopiaprinting
CONFIG+=no_tr
}

FORMS += printdialogbase.ui

HEADERS += qprinterinterface.h

SOURCES += qprinterinterface.cpp
           
UNIX.TYPE=CONDITIONAL_SOURCES
UNIX.CONDITION=unix:!x11
UNIX.PRIVATE_HEADERS=qprintdialogcreator_p.h
UNIX.SOURCES=qprintdialogcreator.cpp
!qbuild:CONDITIONAL_SOURCES(UNIX)

!qbuild{
headers.files=$$HEADERS
headers.path=/include/qtopia/printing
headers.hint=sdk headers
INSTALLS+=headers

idep(LIBS+=-l$$TARGET)
qt_inc($$TARGET)
}

