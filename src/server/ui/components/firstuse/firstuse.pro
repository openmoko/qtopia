!qbuild{
qtopia_project(server_lib)
TARGET=qpe_ui_components_firstuse

depends(server/core_server)
depends(settings/language)
depends(settings/systemtime)

idep(INCLUDEPATH+=$$PWD)
idep(LIBS+=-l$$TARGET)
idep(app:register_static_dependency($$TARGET))
}

include(firstuse.pri)
