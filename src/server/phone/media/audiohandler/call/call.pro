!qbuild{
requires(enable_telephony)
qtopia_project(server_lib)
TARGET=qpe_phone_media_audiohandler_call

depends(server/core_server)
depends(server/phone/media/audiohandler/abstract)

idep(INCLUDEPATH+=$$PWD)
idep(LIBS+=-l$$TARGET)
idep(app:register_static_dependency($$TARGET))
}

include(call.pri)
