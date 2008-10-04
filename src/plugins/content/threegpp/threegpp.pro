!qbuild{
qtopia_project(qtopia plugin)
TARGET=threegpp
CONFIG+=no_tr
depends(libraries/qtopia)
}

HEADERS = threegppcontentplugin.h 
SOURCES = threegppcontentplugin.cpp 

