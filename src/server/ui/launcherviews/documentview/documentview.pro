!qbuild{
qtopia_project(server_lib)
TARGET=qpe_ui_launcherviews_documentview

depends(server/core_server)
depends(server/ui/launcherviews/base)

idep(INCLUDEPATH+=$$PWD)
idep(LIBS+=-l$$TARGET)
idep(app:register_static_dependency($$TARGET))
}

include(documentview.pri)
