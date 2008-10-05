!qbuild{
qtopia_project(qtopia plugin)
TARGET=neogsplugin
CONFIG+=no_tr
depends(libraries/qtopiawhereabouts)
requires(enable_qtopiawhereabouts)
}

HEADERS	= neogpsplugin.h
SOURCES = neogpsplugin.cpp


