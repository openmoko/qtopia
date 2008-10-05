!qbuild{
qtopia_project(server_lib)
TARGET=qpe_ui_launcherviews_hierarchdocumentview

depends(server/core_server)
depends(server/ui/launcherviews/base)
depends(server/ui/launcherviews/documentview)

idep(INCLUDEPATH+=$$PWD)
idep(LIBS+=-l$$TARGET)
idep(app:register_static_dependency($$TARGET))
}

include(hierarchdocumentview.pri)
