!qbuild{
qtopia_project(server_lib)
TARGET=qpe_phone_contextlabel_base

depends(server/core_server)
depends(server/phone/contextlabel/abstract)
depends(server/infrastructure/softmenubar)

idep(INCLUDEPATH+=$$PWD)
idep(LIBS+=-l$$TARGET)
idep(app:register_static_dependency($$TARGET))
}

include(base.pri)
