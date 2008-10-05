!qbuild{
qtopia_project(server_lib)
TARGET=qpe_bluetooth_filepush

requires(enable_bluetooth)
requires(contains(PROJECTS,server/bluetooth/powermgr))

depends(server/core_server)
depends(server/comm/obex)
depends(server/comm/filetransfer)
depends(libraries/qtopiacomm/bluetooth)
depends(libraries/qtopiapim)

idep(INCLUDEPATH+=$$PWD)
idep(LIBS+=-l$$TARGET)
idep(app:register_static_dependency($$TARGET))
}

include(filepush.pri)

btpushservice.files=$$QTOPIA_DEPOT_PATH/services/BluetoothPush/qpe
btpushservice.path=/services/BluetoothPush
INSTALLS+=btpushservice

btqdsservice.files=$$QTOPIA_DEPOT_PATH/etc/qds/BluetoothPush
btqdsservice.path=/etc/qds
INSTALLS+=btqdsservice

