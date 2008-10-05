!qbuild{

requires(enable_telephony)
qtopia_project(server_lib)
TARGET=qpe_phone_callscreen_deskphone

depends(server/core_server)
depends(server/phone/callscreen/abstract)
depends(server/pim/servercontactmodel)
depends(server/phone/media/audiohandler/abstract)
depends(server/phone/telephony/dialercontrol)
depends(libraries/homeui)

idep(INCLUDEPATH+=$$PWD)
idep(LIBS+=-l$$TARGET)
idep(app:register_static_dependency($$TARGET))
}

include(deskphone.pri)
