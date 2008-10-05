!qbuild{
qtopia_project(server_lib)
TARGET=qpe_phone_telephony_dialproxy

#add dep when compiling
requires(enable_telephony)

depends(server/core_server)
depends(server/phone/telephony/dialercontrol)
depends(server/phone/media/audiohandler/abstract)

idep(INCLUDEPATH+=$$PWD)
idep(LIBS+=-l$$TARGET)
idep(app:register_static_dependency($$TARGET))
}

dialerservice.files=$$QTOPIA_DEPOT_PATH/services/Dialer/qpe
dialerservice.path=/services/Dialer
INSTALLS+=dialerservice

include(dialproxy.pri)
