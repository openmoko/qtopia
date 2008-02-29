qtopia_project(qtopia lib)
TARGET=qtopiaaudio
CONFIG+=no_tr qtopia_visibility

HEADERS	= \
    qaudioinput.h \
    qaudiooutput.h

SOURCES	= \
    qaudioinput.cpp \
    qaudiooutput.cpp

equals(QTOPIA_SOUND_SYSTEM,alsa) {
    depends(3rdparty/libraries/alsa)
    DEFINES+=HAVE_ALSA
}
equals(QTOPIA_SOUND_SYSTEM,oss) {
    DEFINES+=HAVE_OSS
}

sdk_qtopiaaudio_headers.files=$${HEADERS}
sdk_qtopiaaudio_headers.path=/include/qtopia/audio
sdk_qtopiaaudio_headers.hint=sdk headers
INSTALLS+=sdk_qtopiaaudio_headers

idep(LIBS+=-l$$TARGET)
qt_inc($$TARGET)
