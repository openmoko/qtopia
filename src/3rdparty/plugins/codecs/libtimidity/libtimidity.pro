qtopia_project(qtopia plugin)
TARGET=timidityplugin
CONFIG+=no_tr

HEADERS		= midiplugin.h mididecoder.h
SOURCES	    = midiplugin.cpp mididecoder.cpp

depends(3rdparty/libraries/libtimidity)
depends(libraries/qtopiamedia)
