!qbuild{
qtopia_project(qtopia plugin)
TARGET=mmscomposer
requires(enable_cell)
depends(libraries/qtopiamail)
}

HEADERS+=\
    mmscomposer.h\
    videoselector.h

SOURCES+=\
    mmscomposer.cpp\
    videoselector.cpp


