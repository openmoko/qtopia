!qbuild:qtopia_project(unittest)
!qbuild:depends(libraries/qtopiacomm/serial)

HEADERS=testserialiodevice.h
SOURCES=tst_qatchat.cpp testserialiodevice.cpp
TARGET=tst_qatchat

!qbuild:include($$QTOPIA_DEPOT_PATH/tests/shared/qfuturesignal.pri)
