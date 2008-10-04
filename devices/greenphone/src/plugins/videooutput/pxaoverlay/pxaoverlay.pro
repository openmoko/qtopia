!qbuild {
qtopia_project(qtopia plugin)
TARGET=pxaoverlay
CONFIG+=no_tr
depends(libraries/qtopiavideo)
depends(libraries/qtopia)
}


HEADERS=\
    qpxavideooutput.h \
    pxaoverlay.h \


SOURCES=\
    qpxavideooutput.cpp \
    pxaoverlay.cpp \

