qtopia_project(qtopia app)
TARGET=sipagent
CONFIG+=no_tr no_singleexec
!no_singleexec:CONFIG+=singleexec_main

INCLUDEPATH+=$$QTOPIA_DEPOT_PATH/src/3rdparty/libraries

HEADERS		= \
    siplayer.h \
    sdpparser.h \
    mediasessionservice.h \
    mediasessionclient.h \
    sipagentmedia.h \
    mediastream.h \
    rtpsinkstream.h \
    rtpsourcestream.h \
    commonencodestream.h \
    gsmencodestream.h \
    pcmuencodestream.h \
    pcmaencodestream.h \
    commondecodestream.h \
    gsmdecodestream.h \
    pcmudecodestream.h \
    pcmadecodestream.h \
    g711.h \
    audioinputstream.h \
    audiooutputstream.h \
    dtmfstream.h \
    wavoutputstream.h
SOURCES		= \
    main.cpp \
    siplayer.cpp \
    sdpparser.cpp \
    mediasessionservice.cpp \
    mediasessionclient.cpp \
    sipagentmedia.cpp \
    mediastream.cpp \
    rtpsinkstream.cpp \
    rtpsourcestream.cpp \
    commonencodestream.cpp \
    gsmencodestream.cpp \
    pcmuencodestream.cpp \
    pcmaencodestream.cpp \
    commondecodestream.cpp \
    gsmdecodestream.cpp \
    pcmudecodestream.cpp \
    pcmadecodestream.cpp \
    g711.c \
    audioinputstream.cpp \
    audiooutputstream.cpp \
    dtmfstream.cpp \
    wavoutputstream.cpp

pkg.desc=SIP agent
pkg.domain=trusted

depends(libraries/qtopiaphone)
depends(libraries/qtopiaaudio)
depends(libraries/qtopiacomm/network)
depends(3rdparty/libraries/dissipate2)
depends(3rdparty/libraries/gsm)

HEADERS += telephonyservice.h siptelephonyservice.h
SOURCES += telephonyservice.cpp siptelephonyservice.cpp
telephonyservice.files=$$QTOPIA_DEPOT_PATH/services/Telephony/sipagent
telephonyservice.path=/services/Telephony
INSTALLS+=telephonyservice

mediaservice.files=$$QTOPIA_DEPOT_PATH/services/MediaSession/sipagent
mediaservice.path=/services/MediaSession
INSTALLS+=mediaservice

