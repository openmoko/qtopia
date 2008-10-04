!qbuild{
qtopia_project(qtopia plugin)
TARGET=wavrecord
contains(PROJECTS,3rdparty/libraries/gsm):depends(3rdparty/libraries/gsm)
CONFIG+=no_tr
}

HEADERS		=  wavrecord.h wavrecordimpl.h
SOURCES	        =  wavrecord.cpp wavrecordimpl.cpp

contains(PROJECTS,3rdparty/libraries/gsm):DEFINES+=WAVGSM_SUPPORTED

