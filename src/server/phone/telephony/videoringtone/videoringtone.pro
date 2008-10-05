!qbuild{
qtopia_project(server_lib)
TARGET=qpe_phone_telephony_videoringtone

requires(enable_qtopiamedia)
requires(contains(PROJECTS,server/phone/telephony/ringcontrol))

depends(server/core_server)
depends(libraries/qtopiamedia)

idep(INCLUDEPATH+=$$PWD)
idep(LIBS+=-l$$TARGET)
idep(app:register_static_dependency($$TARGET))
}

include(videoringtone.pri)
