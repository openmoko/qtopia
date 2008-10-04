requires(enable_sxe)
TEMPLATE=app
CONFIG+=qt
TARGET=sxe_installer

MODULES*=\
    qtopiabase::headers\
    qtopiasecurity::headers
SOURCEPATH+=\
    /src/libraries/qtopiabase\
    /src/libraries/qtopiasecurity\
    /qtopiacore/qt/src/gui/embedded

DEFINES+=SXE_INSTALLER
SOURCES=main.cpp

HEADERS+=\
    qtopiasxe.h\
    qpackageregistry.h\
    qtopianamespace.h\
    keyfiler_p.h\
    qsxepolicy.h

SOURCES+=\
    qtopiasxe.cpp\
    qpackageregistry.cpp\
    qtopianamespace_lock.cpp\
    keyfiler.cpp\
    qsxepolicy.cpp

