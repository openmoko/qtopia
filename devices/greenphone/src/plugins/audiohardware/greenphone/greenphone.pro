!qbuild{
qtopia_project(qtopia plugin)
TARGET=greenphoneaudiohardware
depends(libraries/qtopiaaudio)
enable_bluetooth {
    depends(libraries/qtopiacomm)
}
}

HEADERS		=  greenphoneaudioplugin.h
SOURCES	        =  greenphoneaudioplugin.cpp

