TEMPLATE=lib
CONFIG+=qt
TARGET=phonesim

# This project has to be here for MODULE_NAME. Use SOURCEPATH because the sources aren't here.
MODULE_NAME=phonesim
SOURCEPATH=..

MODULES*=\
    qtopiaphone::headers\
    qtopiacomm::headers\
    qtopia::headers\
    qtopiabase::headers
QT*=xml network

SOURCEPATH+=\
    /src/libraries/qtopiaphone\
    /src/libraries/qtopiacomm\
    /src/libraries/qtopia\
    /src/libraries/qtopiabase

include(../lib.pro)


