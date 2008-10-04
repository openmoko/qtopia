TEMPLATE=app
CONFIG+=qt
TARGET=qdawggen

MODULES*=\
    qtopia::headers\
    qtopiabase::headers

SOURCEPATH+=/src/libraries/qtopiabase

include(qdawggen.pro)

