!qbuild{
qtopia_project(qtopia plugin)
TARGET=emailcomposer
depends(libraries/qtopiamail)
}

HEADERS+=\
    addatt.h\
    addattdialogphone.h
SOURCES+=\
    addatt.cpp

HOME.TYPE=CONDITIONAL_SOURCES
HOME.CONDITION=equals(QTOPIA_UI,home)
HOME.HEADERS=deskphone_emailcomposer.h
HOME.SOURCES=deskphone_emailcomposer.cpp
!qbuild:CONDITIONAL_SOURCES(HOME)

NONHOME.TYPE=CONDITIONAL_SOURCES
NONHOME.CONDITION=!equals(QTOPIA_UI,home)
NONHOME.HEADERS=emailcomposer.h
NONHOME.SOURCES=emailcomposer.cpp
!qbuild:CONDITIONAL_SOURCES(NONHOME)

