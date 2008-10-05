!qbuild{
qtopia_project(server_lib)
TARGET=qpe_bluetooth_obexpush

requires(enable_bluetooth)
requires(contains(PROJECTS,server/bluetooth/powermgr))
requires(contains(PROJECTS,server/bluetooth/servicemgr))

depends(server/core_server)
depends(server/comm/obex)
depends(server/comm/filetransfer)
depends(libraries/qtopiacomm/bluetooth)

idep(INCLUDEPATH+=$$PWD)
idep(LIBS+=-l$$TARGET)
idep(app:register_static_dependency($$TARGET))
}

include(obexpush.pri)

btoppservices.files=$$QTOPIA_DEPOT_PATH/etc/bluetooth/sdp/opp.xml
btoppservices.path=/etc/bluetooth/sdp
INSTALLS+=btoppservices
