!qbuild{
qtopia_project(qtopia plugin)
TARGET=nokiaaudiohardware
depends(libraries/qtopiaaudio)
depends(libraries/qtopiacomm)
}

HEADERS		=  nokiaaudioplugin.h
SOURCES	        =  nokiaaudioplugin.cpp

