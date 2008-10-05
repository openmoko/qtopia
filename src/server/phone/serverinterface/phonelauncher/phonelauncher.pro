!qbuild{
qtopia_project(server_lib)
TARGET=qpe_phone_serverinterface_phonelauncher

depends(server/core_server)
depends(server/phone/browserscreen/abstract)
depends(server/phone/contextlabel/abstract)
depends(server/phone/header/abstract)
depends(server/phone/homescreen/abstract)
depends(server/phone/secondarydisplay/abstract)
depends(server/processctrl/appmonitor)
depends(server/ui/abstractinterfaces/taskmanager)

enable_telephony {
    depends(server/phone/callhistory/abstract)
    depends(server/phone/dialer/abstract)
    depends(server/phone/callscreen/abstract)
    depends(server/phone/telephony/callpolicymanager/abstract)
    depends(server/phone/telephony/dialercontrol)
    depends(server/phone/telephony/dialproxy)
    depends(server/phone/media/audiohandler/abstract)
}

idep(INCLUDEPATH+=$$PWD)
idep(LIBS+=-l$$TARGET)
idep(app:register_static_dependency($$TARGET))
}

include(phonelauncher.pri)
