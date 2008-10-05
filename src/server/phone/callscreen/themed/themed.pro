!qbuild{

requires(enable_telephony)
qtopia_project(server_lib)
TARGET=qpe_phone_callscreen_themed

depends(server/core_server)
depends(server/phone/callscreen/abstract)
depends(server/pim/servercontactmodel)
depends(server/phone/media/audiohandler/abstract)
depends(server/phone/media/audiohandler/call)
depends(server/phone/telephony/dialercontrol)
depends(server/phone/telephony/dialfilter/abstract)
depends(server/phone/themecontrol)
depends(server/processctrl/taskmanagerentry)

enable_qtopiamedia:depends(server/phone/telephony/videoringtone)

idep(INCLUDEPATH+=$$PWD)
idep(LIBS+=-l$$TARGET)
idep(app:register_static_dependency($$TARGET))
}

include(themed.pri)
