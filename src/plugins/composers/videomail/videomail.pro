!qbuild{
qtopia_project(qtopia plugin)
TARGET=videomailcomposer
depends(libraries/qtopiamail)
depends(libraries/qtopiamedia)
depends(libraries/homeui)
}

HEADERS+=\
    videomailcomposer.h
SOURCES+=\
    videomailcomposer.cpp

