!qbuild:include(../bluetooth.pri)
SOURCES += tst_qbluetoothlocaldevicemanager.cpp
!qbuild:VPATH+=$$QTOPIA_DEPOT_PATH/src/libraries/qtopiacomm/bluetooth
SOURCES += qbluetoothlocaldevicemanager.cpp
!qbuild:depends(libraries/qtopia)

