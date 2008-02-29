qtopia_project(qtopia plugin)
TARGET=greenphoneaudiohardware

CONFIG+=no_tr

HEADERS		=  greenphoneaudioplugin.h
SOURCES	        =  greenphoneaudioplugin.cpp

depends(libraries/qtopiaaudio)
enable_bluetooth {
    depends(libraries/qtopiacomm)
}
