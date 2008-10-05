!qbuild{
qtopia_project(server_lib)
TARGET=qpe_ui_launcherviews_contentsetview

depends(server/core_server)
depends(server/ui/launcherviews/base)

idep(INCLUDEPATH+=$$PWD)
idep(LIBS+=-l$$TARGET)
idep(app:register_static_dependency($$TARGET))
}

include(contentsetview.pri)

contentsetviewservice.files=$$QTOPIA_DEPOT_PATH/services/ContentSetView/qpe
contentsetviewservice.path=/services/ContentSetView
INSTALLS+=contentsetviewservice

