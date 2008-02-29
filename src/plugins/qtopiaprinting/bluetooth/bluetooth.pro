qtopia_project(qtopia plugin)
TARGET=bluetoothprinting

CONFIG+=no_pkg

HEADERS = bluetoothplugin.h qbluetoothobexagent.h
SOURCES = bluetoothplugin.cpp qbluetoothobexagent.cpp

depends(libraries/qtopiaprinting)
depends(libraries/qtopiacomm/bluetooth)
