!qbuild{
qtopia_project(qtopia plugin)
TARGET=genericcomposer
depends(libraries/qtopiamail)
equals(QTOPIA_UI,home):depends(libraries/homeui)
}

!enable_cell:DEFINES+=QTOPIA_NO_SMS

HEADERS+=\
    genericcomposer.h\
    templatetext.h

SOURCES+=\
    genericcomposer.cpp\
    templatetext.cpp

