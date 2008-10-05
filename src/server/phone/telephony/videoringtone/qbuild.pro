!qbuild_server_deps{
TEMPLATE=lib
CONFIG+=qtopia staticlib
TARGET=qpe_phone_telephony_videoringtone


DEP.headers.TYPE=DEPENDS PERSISTED
DEP.headers.EVAL=\
    "INCLUDEPATH*="$$paths(.,project)
}
requires(enable_qtopiamedia)
QTOPIA*=media
DEPENDS*=\
    /src/server/core_server::persisted\
    /src/server/phone/telephony/ringcontrol::exists\

include(videoringtone.pro)
