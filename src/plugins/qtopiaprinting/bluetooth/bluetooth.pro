!qbuild{
qtopia_project(qtopia plugin)
TARGET=bluetoothprinting
depends(libraries/qtopiaprinting)
depends(libraries/qtopiacomm/bluetooth)
}

HEADERS = bluetoothplugin.h qbluetoothobexagent.h
SOURCES = bluetoothplugin.cpp qbluetoothobexagent.cpp

