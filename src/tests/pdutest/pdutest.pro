qtopia_project(qtopia core app)
depends(libraries/qtopiaphone)
TARGET=pdutest
CONFIG+=no_tr singleexec_main
CONFIG  += qtestlib


SOURCES = pdutest.cpp
