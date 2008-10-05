!qbuild:qtopia_project(unittest)

!qbuild:VPATH+=$$QTOPIA_DEPOT_PATH/src/libraries/qtopiacomm/serial
!qbuild:INCLUDEPATH+=$$QTOPIA_DEPOT_PATH/src/libraries/qtopiacomm/serial

HEADERS=gsm0710_p.h
SOURCES=tst_gsm0710.cpp gsm0710.c
TARGET=tst_gsm0710
