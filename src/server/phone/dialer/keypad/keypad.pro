!qbuild{

requires(enable_telephony)
qtopia_project(server_lib)
TARGET=qpe_phone_dialer_keypad

depends(server/core_server)
depends(server/phone/dialer/abstract)
depends(server/phone/ui/callcontactmodelview)
depends(server/phone/media/dtmfaudio)
depends(server/phone/telephony/dialfilter/abstract)
depends(server/phone/telephony/dialercontrol)
depends(server/pim/servercontactmodel)

idep(INCLUDEPATH+=$$PWD)
idep(LIBS+=-l$$TARGET)
idep(app:register_static_dependency($$TARGET))
}

include(keypad.pri)
