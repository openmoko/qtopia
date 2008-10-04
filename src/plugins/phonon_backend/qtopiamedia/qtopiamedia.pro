
!qbuild{
qtopia_project(qtopia plugin embedded)
depends(libraries/qtopiamedia)
CONFIG += qtopia no_tr
}

TARGET = phonon_qtopiamedia

HEADERS	= \
            backend.h \
            mediaobject.h \
            audiooutput.h \
            videowidget.h

SOURCES	= \
            backend.cpp \
            mediaobject.cpp \
            audiooutput.cpp \
            videowidget.cpp
