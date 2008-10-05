!qbuild{
qtopia_project(server_lib)
TARGET=qpe_phone_telephony_phoneserver_base

requires(enable_telephony)

depends(server/core_server)
depends(libraries/qtopiaphone)

idep(INCLUDEPATH+=$$PWD)
idep(LIBS+=-l$$TARGET)
idep(app:register_static_dependency($$TARGET))
}

include(base.pri)
