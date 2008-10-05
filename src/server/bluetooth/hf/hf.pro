!qbuild{
qtopia_project(server_lib)
TARGET=qpe_bluetooth_hf

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

include(hf.pri)

bthfservices.files=$$QTOPIA_DEPOT_PATH/etc/bluetooth/sdp/hfag.xml
bthfservices.path=/etc/bluetooth/sdp
INSTALLS+=bthfservices
