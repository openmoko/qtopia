qtopia_project(qtopia plugin)
TARGET=builtin
CONFIG+=no_tr

HEADERS		= builtin.h \
                qtopiaaudioinputdevice.h \
                qtopiaaudiooutputdevice.h 
SOURCES	    = builtin.cpp \
                qtopiaaudioinputdevice.cpp \
                qtopiaaudiooutputdevice.cpp 

depends(libraries/qtopiaaudio)
depends(libraries/qtopiamedia)

