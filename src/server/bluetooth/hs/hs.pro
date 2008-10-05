!qbuild{
qtopia_project(server_lib)
TARGET=qpe_bluetooth_hs

requires(enable_bluetooth)
requires(contains(PROJECTS,server/bluetooth/powermgr))
requires(contains(PROJECTS,server/bluetooth/servicemgr))

depends(server/core_server)
depends(server/bluetooth/scomisc)
depends(server/bluetooth/audiovolumemgr)
depends(libraries/qtopiacomm/bluetooth)

idep(INCLUDEPATH+=$$PWD)
idep(LIBS+=-l$$TARGET)
idep(app:register_static_dependency($$TARGET))
}

include(hs.pri)

bthsservices.files=$$QTOPIA_DEPOT_PATH/etc/bluetooth/sdp/hsag.xml
bthsservices.path=/etc/bluetooth/sdp
INSTALLS+=bthsservices
