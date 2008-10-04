!qbuild{
qtopia_project(qtopia plugin)
TARGET=wavplay
CONFIG+=no_tr
depends(libraries/qtopiamedia)
contains(PROJECTS,3rdparty/libraries/gsm):depends(3rdparty/libraries/gsm)
}

DEFINES+=WAV49 FAST SASR

HEADERS = \
        wavplugin.h \
        wavdecoder.h

SOURCES = \
        wavplugin.cpp \
        wavdecoder.cpp

contains(PROJECTS,3rdparty/libraries/gsm):DEFINES+=WAVGSM_SUPPORTED
