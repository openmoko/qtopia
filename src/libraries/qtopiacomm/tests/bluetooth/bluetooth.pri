!qbuild:qtopia_project(unittest)
requires(enable_bluetooth)
CONFIG+=no_tr
qbuild {
    PWD=$$path(.,existing)
}
HEADERS+=$$PWD/qtopiabluetoothunittest.h
SOURCES+=$$PWD/qtopiabluetoothunittest.cpp
qbuild {
    MODULES*=qtopiacomm
} else {
    depends(libraries/qtopiacomm/bluetooth)
}
