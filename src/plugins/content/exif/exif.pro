!qbuild{
qtopia_project(qtopia plugin)
TARGET=exif
CONFIG+=no_tr
depends(libraries/qtopia)
}

HEADERS		=  exifcontentplugin.h \
                   exifcontentproperties.h

SOURCES	        =  exifcontentplugin.cpp \
                   exifcontentproperties.cpp

