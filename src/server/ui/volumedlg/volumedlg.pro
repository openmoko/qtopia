!qbuild{
qtopia_project(server_lib)
TARGET=qpe_ui_volumedlg

depends(server/core_server)
depends(server/media/volumemanagement)

idep(INCLUDEPATH+=$$PWD)
idep(LIBS+=-l$$TARGET)
idep(app:register_static_dependency($$TARGET))
}

include(volumedlg.pri)
