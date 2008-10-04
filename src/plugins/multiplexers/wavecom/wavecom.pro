!qbuild{
qtopia_project(qtopia plugin)
TARGET=wavecommultiplex
CONFIG+=no_tr
depends(libraries/qtopiacomm/serial)
}

HEADERS		=  wavecommultiplexer.h
SOURCES	        =  wavecommultiplexer.cpp

