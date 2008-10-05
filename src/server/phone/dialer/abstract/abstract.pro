!qbuild{

requires(enable_telephony)
qtopia_project(server_lib)
TARGET=qpe_phone_dialer_abstract

depends(server/core_server)

idep(INCLUDEPATH+=$$PWD)
idep(LIBS+=-l$$TARGET)
idep(app:register_static_dependency($$TARGET))
}

include(abstract.pri)
