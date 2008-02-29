qtopia_project(qtopia plugin)
TARGET=pim
CONFIG+=no_tr no_pkg
plugin_type=qdsync

DEFINES+=PIMXML_NAMESPACE=QDSync

VPATH+=..
INCLUDEPATH+=..

HEADERS+=\
    qpimsync_p.h\
    qpimsyncstorage.h\
    qpimxml_p.h\

SOURCES+=\
    qpimsync.cpp\
    qpimsyncstorage.cpp\
    qpimxml.cpp\

depends(libraries/qtopiapim)
depends(tools/qdsync/common)

