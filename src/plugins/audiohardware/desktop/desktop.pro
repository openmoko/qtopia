!qbuild{
qtopia_project(qtopia plugin)
TARGET=desktopaudiohardware
depends(libraries/qtopiaaudio)
depends(libraries/qtopiacomm)
}

HEADERS		=  desktopaudioplugin.h
SOURCES	        =  desktopaudioplugin.cpp
