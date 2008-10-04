!qbuild{
qtopia_project(desktop app)
TARGET=qdawggen
CONFIG+=no_tr no_install
VPATH           += $${QTOPIA_DEPOT_PATH}/src/libraries/qtopiabase
INCLUDEPATH     += $${QTOPIA_DEPOT_PATH}/src/libraries/qtopiabase
# Dodgy, but get us the Qtopia includes
qt_inc(qtopia)
depends(libraries/qtopiabase,fake)
depends(libraries/qtopia,fake)
}

HEADERS		= qdawg.h global.h
SOURCES		= main.cpp qdawg.cpp global.cpp

HEADERS+=qmemoryfile_p.h
SOURCES+=qmemoryfile.cpp
unix:SOURCES+=qmemoryfile_unix.cpp
win32:SOURCES+=qmemoryfile_win.cpp

pkg.desc=QDawg file generator
pkg.domain=trusted
