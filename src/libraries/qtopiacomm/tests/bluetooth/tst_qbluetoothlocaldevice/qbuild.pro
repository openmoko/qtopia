include(tst_qbluetoothlocaldevice.pro)
TEMPLATE=app
CONFIG+=qtopia unittest
include(../bluetooth.pri)

SOURCEPATH+=/src/libraries/qtopiacomm/bluetooth


TARGET=tst_qbluetoothlocaldevice
