!qbuild{
qtopia_project(server_lib)
TARGET=qpe_ui_filetransferwindow

depends(server/core_server)
depends(server/comm/filetransfer)
depends(server/processctrl/taskmanagerentry)

idep(INCLUDEPATH+=$$PWD)
idep(LIBS+=-l$$TARGET)
idep(app:register_static_dependency($$TARGET))
}

include(filetransferwindow.pri)
