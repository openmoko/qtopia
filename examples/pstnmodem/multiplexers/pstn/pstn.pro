qtopia_project(qtopia plugin)
requires(enable_cell)
TARGET=pstnmultiplex

CONFIG+=no_tr

HEADERS		=  pstnmultiplexer.h
SOURCES	        =  pstnmultiplexer.cpp

depends(libraries/qtopiacomm/serial)

