qtopia_project(qtopia plugin)
TARGET=desktopaudiohardware

CONFIG+=no_tr

HEADERS		=  desktopaudioplugin.h
SOURCES	        =  desktopaudioplugin.cpp

depends(libraries/qtopiaaudio)
depends(libraries/qtopiacomm)
