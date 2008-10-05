!qbuild{
qtopia_project(server_lib)
TARGET=qpe_bluetooth_audiovolumemgr

requires(enable_bluetooth)
requires(contains(PROJECTS,server/media/volumemanagement))

depends(server/core_server)
depends(libraries/qtopiacomm/bluetooth)

idep(INCLUDEPATH+=$$PWD)
idep(LIBS+=-l$$TARGET)
idep(app:register_static_dependency($$TARGET))
}

include(audiovolumemgr.pri)
