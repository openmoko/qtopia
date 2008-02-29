TEMPLATE = lib
TARGET = kgsm
DESTDIR         = $$(QPEDIR)/lib
CONFIG-=qt

DEFINES += FAST SASR WAV49

QMAKE_CXXFLAGS_RELEASE = -O2

SOURCES += add.cpp \
code.cpp \
debug.cpp \
decode.cpp \
gsm_create.cpp \
gsm_decode.cpp \
gsm_destroy.cpp \
gsm_encode.cpp \
gsm_explode.cpp \
gsm_implode.cpp \
gsm_option.cpp \
gsm_print.cpp \
long_term.cpp \
lpc.cpp \
preprocess.cpp \
rpe.cpp \
short_term.cpp \
table.cpp 

# toast_alaw.cpp \
# toast_audio.cpp 


HEADERS += config.h \
gsm.h \
private.h \
proto.h \
unproto.h 

TRANSLATIONS=
