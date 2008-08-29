qtopia_project(qtopia core app)
depends(libraries/qtopiaphone)
depends(libraries/qt/qtestlib)
TARGET=pdutest
CONFIG+=no_tr singleexec_main
CONFIG  += qtestlib


SOURCES = pdutest.cpp
