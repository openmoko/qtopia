!qbuild{
qtopia_project(qtopia lib)
TARGET=qtopiaaudio
CONFIG+=no_tr qtopia_visibility
}

HEADERS=\
    qaudioinput.h \
    qaudiooutput.h \
    qaudiostateconfiguration.h \
    qaudiostateplugin.h \
    qaudiostate.h \
    qaudiostateinfo.h \
    qaudiostatemanager.h \
    qaudionamespace.h\
    qaudiointerface.h \
    qaudiomixer.h

SOURCES=\
    qaudiostateconfiguration.cpp \
    qaudiostateplugin.cpp \
    qaudiostate.cpp \
    qaudiostateinfo.cpp \
    qaudiostatemanager.cpp \
    qaudionamespace.cpp \
    qaudiostatemanagerservice.cpp \
    qaudiointerface.cpp

PRIVATE_HEADERS=\
    qaudiostatemanagerservice_p.h

equals(QTOPIA_SOUND_SYSTEM,alsa) {
    MODULES*=alsa
    !qbuild:depends(3rdparty/libraries/alsa)
    SOURCES+=\
        qaudioinput_alsa.cpp\
        qaudiooutput_alsa.cpp\
        qaudiomixer_alsa.cpp
    DEFINES+=QTOPIA_HAVE_ALSA
}
equals(QTOPIA_SOUND_SYSTEM,pulse) {
    MODULES*=alsa
    !qbuild:depends(3rdparty/libraries/alsa)
    SOURCES+=\
        qaudioinput_pulse.cpp\
        qaudiooutput_pulse.cpp\
        qaudiomixer_alsa.cpp
    DEFINES+=QTOPIA_HAVE_PULSE QTOPIA_HAVE_ALSA
    LIBS+=-lpulse-simple
}
equals(QTOPIA_SOUND_SYSTEM,oss) {
    SOURCES+=\
        qaudioinput_oss.cpp\
        qaudiomixer_oss.cpp
    enable_qtopiamedia {
        SOURCES+=\
            qaudiooutput_oss.cpp
        DEFINES+=QTOPIA_HAVE_OSS
    } else {
        SOURCES+=\
            qaudiooutput_qss.cpp
        DEFINES+=QTOPIA_HAVE_QSS
    }
}

!qbuild{
headers.files=$$HEADERS
headers.path=/include/qtopia/audio
headers.hint=sdk headers
INSTALLS+=headers

idep(LIBS+=-l$$TARGET)
qt_inc($$TARGET)
}
