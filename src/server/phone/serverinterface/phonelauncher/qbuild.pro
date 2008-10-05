!qbuild_server_deps{
TEMPLATE=lib
CONFIG+=qtopia staticlib
TARGET=qpe_phone_serverinterface_phonelauncher

DEP.headers.TYPE=DEPENDS PERSISTED
DEP.headers.EVAL=\
    "INCLUDEPATH*="$$paths(.,project)
}
DEPENDS*=\
    /src/server/core_server::persisted\
    /src/server/phone/browserscreen/abstract::persisted\
    /src/server/phone/contextlabel/abstract::persisted\
    /src/server/phone/header/abstract::persisted\
    /src/server/phone/homescreen/abstract::persisted\
    /src/server/phone/secondarydisplay/abstract::persisted\
    /src/server/ui/abstractinterfaces/taskmanager::persisted\
    /src/server/processctrl/appmonitor::persisted\

enable_telephony:DEPENDS*=\
    /src/server/phone/callhistory/abstract::persisted\
    /src/server/phone/dialer/abstract::persisted\
    /src/server/phone/callscreen/abstract::persisted\
    /src/server/phone/telephony/callpolicymanager/abstract::persisted\
    /src/server/phone/telephony/dialercontrol::persisted\
    /src/server/phone/telephony/dialproxy::persisted\
    /src/server/phone/media/audiohandler/abstract::persisted\

include(phonelauncher.pro)
