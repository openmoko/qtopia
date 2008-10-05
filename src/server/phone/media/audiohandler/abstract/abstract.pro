!qbuild{
qtopia_project(server_lib)
TARGET=qpe_phone_media_audiohandler_abstract

requires(enable_telephony)

depends(server/core_server)
depends(libraries/qtopiaaudio)

idep(INCLUDEPATH+=$$PWD)
idep(LIBS+=-l$$TARGET)
idep(app:register_static_dependency($$TARGET))
}

include(abstract.pri)
