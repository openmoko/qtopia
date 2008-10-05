!qbuild{
qtopia_project(qtopia plugin)
TARGET=neomultiplex
CONFIG+=no_tr
requires(enable_cell)
depends(libraries/qtopiacomm/serial)
}

HEADERS		=  neomultiplexer.h
SOURCES	        =  neomultiplexer.cpp

