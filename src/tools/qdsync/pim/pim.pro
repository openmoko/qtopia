!qbuild{
qtopia_project(qtopia plugin)
TARGET=pim
CONFIG+=no_tr
# Packaged by tools/qdsync/app
CONFIG+=no_pkg
plugin_type=qdsync
VPATH+=..
INCLUDEPATH+=..
depends(libraries/qtopiapim)
requires(contains(PROJECTS,tools/qdsync/common))
contains(PROJECTS,tools/qdsync/common):depends(tools/qdsync/common)
}

DEFINES+=PIMXML_NAMESPACE=QDSync

HEADERS+=\
    qpimsyncstorage.h\
    qpimxml_p.h\

SOURCES+=\
    qpimsyncstorage.cpp\
    qpimxml.cpp\

