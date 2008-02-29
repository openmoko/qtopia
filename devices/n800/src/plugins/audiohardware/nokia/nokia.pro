qtopia_project(qtopia plugin)
TARGET=nokiaaudiohardware

CONFIG+=no_tr

HEADERS		=  nokiaaudioplugin.h
SOURCES	        =  nokiaaudioplugin.cpp

depends(libraries/qtopiaaudio)
depends(libraries/qtopiacomm)
