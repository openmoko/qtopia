qtopia_project(qtopia plugin)
TARGET=ficgta01audiohardware

CONFIG+=no_tr

HEADERS		=  ficgta01audioplugin.h
SOURCES	        =  ficgta01audioplugin.cpp

depends(libraries/qtopiaaudio)
enable_bluetooth {
    depends(libraries/qtopiacomm)
}
