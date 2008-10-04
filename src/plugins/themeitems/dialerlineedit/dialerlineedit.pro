!qbuild{
qtopia_project(qtopia plugin)
TARGET=dialerlineedit
CONFIG+=no_tr
depends(libraries/qtopiatheming)
}

HEADERS =   dialerlineedit.h

SOURCES =   dialerlineedit.cpp

