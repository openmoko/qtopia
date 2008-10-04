!qbuild{
qtopia_project(qtopia plugin)
TARGET=greenphonemultiplex
CONFIG+=no_tr
requires(enable_cell)
depends(libraries/qtopiacomm/serial)
}

HEADERS		=  greenphonemultiplexer.h
SOURCES	        =  greenphonemultiplexer.cpp

