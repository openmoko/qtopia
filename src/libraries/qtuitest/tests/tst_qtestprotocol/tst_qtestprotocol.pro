!qbuild:qtopia_project(unittest)

!qbuild:VPATH      +=$$QTOPIA_DEPOT_PATH/src/libraries/qtuitest
!qbuild:INCLUDEPATH+=$$QTOPIA_DEPOT_PATH/src/libraries/qtuitest

SOURCES+=                   \
    tst_qtestprotocol.cpp   \
    testprotocol.cpp        \
    testprotocolserver.cpp

HEADERS+=                 \
    testprotocol.h        \
    testprotocolserver.h

!qbuild:depends(libraries/qtopia)

