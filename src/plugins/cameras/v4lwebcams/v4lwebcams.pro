!qbuild{
qtopia_project(qtopia plugin)
TARGET=v4lwebcams
CONFIG+=no_tr 
depends(libraries/qtopiavideo)
depends(libraries/qtopia)
}

HEADERS = \
        webcams.h\
        plugin.h

SOURCES = \
        webcams.cpp \
        plugin.cpp 

