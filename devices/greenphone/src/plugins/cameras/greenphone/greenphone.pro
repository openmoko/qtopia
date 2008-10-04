!qbuild {
qtopia_project(qtopia plugin)
TARGET=greenphone
CONFIG+=no_tr

depends(libraries/qtopiavideo)
depends(libraries/qtopia)
}

HEADERS = greenphonecamera.h greenphonecameraplugin.h

SOURCES = greenphonecamera.cpp greenphonecameraplugin.cpp
