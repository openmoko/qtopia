!qbuild{
qtopia_project(qtopia plugin)
TARGET=neoaudiohardware
depends(libraries/qtopiaaudio)
enable_bluetooth {
    depends(libraries/qtopiacomm)
}
}

HEADERS		=  neoaudioplugin.h
SOURCES	        =  neoaudioplugin.cpp

