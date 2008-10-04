qtopia_project(qtopia plugin)
TARGET=exampletask
plugin_type=servertask

HEADERS+=\
    exampletask.h


SOURCES+=\
    exampletask.cpp

depends(libraries/qtopia)

