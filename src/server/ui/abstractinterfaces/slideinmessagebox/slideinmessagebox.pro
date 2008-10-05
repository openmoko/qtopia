!qbuild{
qtopia_project(server_lib)
TARGET=qpe_ui_abstractinterfaces_slideinmessagebox

depends(server/core_server)

idep(INCLUDEPATH+=$$PWD)
idep(LIBS+=-l$$TARGET)
idep(app:register_static_dependency($$TARGET))
}

include(slideinmessagebox.pri)
