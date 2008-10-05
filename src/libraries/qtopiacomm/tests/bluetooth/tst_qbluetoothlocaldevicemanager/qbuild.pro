include(tst_qbluetoothlocaldevicemanager.pro)
TEMPLATE=app
CONFIG+=qtopia unittest
include(../bluetooth.pri)

SOURCEPATH+=/src/libraries/qtopiacomm/bluetooth


TARGET=tst_qbluetoothlocaldevicemanager
