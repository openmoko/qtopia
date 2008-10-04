!qbuild{
qtopia_project(qtopia plugin)
TARGET=n810gsplugin
CONFIG+=no_tr
depends(libraries/qtopiawhereabouts)
requires(enable_qtopiawhereabouts)
}

HEADERS	= n810gpsplugin.h
SOURCES = n810gpsplugin.cpp

