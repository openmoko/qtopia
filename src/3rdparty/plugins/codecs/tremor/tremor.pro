!qbuild{
qtopia_project(qtopia plugin)
TARGET=tremorplugin
CONFIG+=no_tr
license(FREEWARE)
depends(3rdparty/libraries/tremor)
depends(libraries/qtopiamedia)
}

HEADERS = \
        oggplugin.h \
        oggdecoder.h

SOURCES = \
        oggplugin.cpp \
        oggdecoder.cpp

