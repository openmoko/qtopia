!qbuild{
qtopia_project(server_lib)
TARGET=qpe_ui_dfltcrashdlg

depends(server/core_server)

idep(INCLUDEPATH+=$$PWD)
idep(LIBS+=-l$$TARGET)
idep(app:register_static_dependency($$TARGET))
}

include(dfltcrashdlg.pri)
