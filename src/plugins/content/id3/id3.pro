!qbuild{
qtopia_project(qtopia plugin)
TARGET=id3
CONFIG+=no_tr
depends(libraries/qtopia)
}

HEADERS		=  id3contentplugin.h \
                   id3tag.h

SOURCES	        =  id3contentplugin.cpp \
                   id3tag.cpp

