!qbuild{
qtopia_project(server_lib)
TARGET=qpe_phone_ui_components_presenceeditor

requires(enable_voip)

depends(server/core_server)
depends(libraries/qtopiapim)
depends(libraries/qtopiacollective)
depends(libraries/qtopiacomm)

idep(INCLUDEPATH+=$$PWD)
idep(LIBS+=-l$$TARGET)
idep(app:register_static_dependency($$TARGET))
}

include(presenceeditor.pri)
