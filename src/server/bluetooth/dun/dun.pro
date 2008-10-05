!qbuild{
qtopia_project(server_lib)
TARGET=qpe_bluetooth_dun

requires(enable_bluetooth)

depends(server/core_server)
depends(libraries/qtopiacomm/bluetooth)
depends(server/bluetooth/serial)

idep(INCLUDEPATH+=$$PWD)
idep(LIBS+=-l$$TARGET)
idep(app:register_static_dependency($$TARGET))
}

include(dun.pri)

btdunservices.files=$$QTOPIA_DEPOT_PATH/etc/bluetooth/sdp/dun.xml
btdunservices.path=/etc/bluetooth/sdp
INSTALLS+=btdunservices
