!qbuild{
qtopia_project(qtopia plugin)
TARGET=directpaintervideooutput
CONFIG+=no_tr 

depends(libraries/qtopiavideo)
depends(libraries/qtopiamedia)
}

HEADERS = qdirectpaintervideooutput.h

SOURCES = qdirectpaintervideooutput.cpp


