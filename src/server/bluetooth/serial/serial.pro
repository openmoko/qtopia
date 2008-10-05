!qbuild{
qtopia_project(server_lib)
TARGET=qpe_bluetooth_serial

requires(enable_bluetooth)
requires(contains(PROJECTS,server/bluetooth/powermgr))
requires(contains(PROJECTS,server/bluetooth/servicemgr))

depends(server/core_server)
depends(libraries/qtopiacomm/bluetooth)

idep(INCLUDEPATH+=$$PWD)
idep(LIBS+=-l$$TARGET)
idep(app:register_static_dependency($$TARGET))
}

include(serial.pri)

btsppservices.files=$$QTOPIA_DEPOT_PATH/etc/bluetooth/sdp/spp.xml
btsppservices.path=/etc/bluetooth/sdp
INSTALLS+=btsppservices
