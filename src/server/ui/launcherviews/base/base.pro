!qbuild{
qtopia_project(server_lib)
TARGET=qpe_ui_launcherviews_base

depends(server/core_server)

idep(INCLUDEPATH+=$$PWD)
idep(LIBS+=-l$$TARGET)
idep(app:register_static_dependency($$TARGET))
}

include(base.pri)
