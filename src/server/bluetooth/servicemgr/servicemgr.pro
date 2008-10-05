!qbuild{
qtopia_project(server_lib)
TARGET=qpe_bluetooth_servicemgr

requires(enable_bluetooth)

depends(server/core_server)
depends(libraries/qtopiacomm/bluetooth)

idep(INCLUDEPATH+=$$PWD)
idep(LIBS+=-l$$TARGET)
idep(app:register_static_dependency($$TARGET))
}

include(servicemgr.pri)

bluetooth_serv_settings.files=$$device_overrides(/etc/default/Trolltech/BluetoothServices.conf)
bluetooth_serv_settings.path=/etc/default/Trolltech
INSTALLS+=bluetooth_serv_settings
