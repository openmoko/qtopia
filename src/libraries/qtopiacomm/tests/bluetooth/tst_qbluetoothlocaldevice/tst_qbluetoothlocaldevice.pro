!qbuild:include(../bluetooth.pri)
SOURCES += tst_qbluetoothlocaldevice.cpp
!qbuild:VPATH+=$$QTOPIA_DEPOT_PATH/src/libraries/qtopiacomm/bluetooth
SOURCES += qbluetoothlocaldevicemanager.cpp \
           qbluetoothlocaldevice.cpp
!qbuild:depends(libraries/qtopia)
