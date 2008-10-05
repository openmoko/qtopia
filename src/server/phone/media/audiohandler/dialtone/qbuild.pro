!qbuild_server_deps{
TEMPLATE=lib
CONFIG+=qtopia staticlib
TARGET=qpe_phone_media_audiohandler_dialtone



DEP.headers.TYPE=DEPENDS PERSISTED
DEP.headers.EVAL=\
    "INCLUDEPATH*="$$paths(.,project)
}
requires(enable_telephony)
QTOPIA*=phone
DEPENDS*=\
    /src/server/core_server::persisted\
    /src/server/phone/media/audiohandler/abstract::persisted\
    /src/server/phone/media/dtmfaudio::persisted\
    /src/server/phone/telephony/callpolicymanager/abstract::persisted\

include(dialtone.pro)
